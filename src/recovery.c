/**
 * @file recovery.c
 * @brief Deadlock recovery strategies implementation
 */

#include "recovery.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Strategy and Criteria Names
 * ============================================================================ */

static const char* strategy_names[] = {
    "Terminate All",
    "Terminate One",
    "Terminate Lowest Priority",
    "Terminate Youngest",
    "Terminate Oldest",
    "Preempt Resources",
    "Rollback"
};

static const char* criteria_names[] = {
    "Lowest Priority",
    "Fewest Resources",
    "Most Resources",
    "Shortest Runtime",
    "Longest Runtime",
    "Minimum Cost"
};

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

void recovery_result_init(RecoveryResult *result) {
    if (!result) return;
    
    result->success = false;
    result->action_count = 0;
    result->processes_terminated = 0;
    result->resources_preempted = 0;
    result->iterations = 0;
    result->summary[0] = '\0';
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        result->actions[i].success = false;
        result->actions[i].process_id = -1;
        result->actions[i].resource_id = -1;
        result->actions[i].description[0] = '\0';
    }
}

void recovery_config_init(RecoveryConfig *config) {
    if (!config) return;
    
    config->strategy = RECOVERY_TERMINATE_LOWEST;
    config->selection = SELECT_LOWEST_PRIORITY;
    config->max_terminations = 0;  /* Unlimited */
    config->preserve_critical = true;
    config->critical_priority_threshold = 90;
    config->verbose = false;
}

/* ============================================================================
 * Core Recovery Functions
 * ============================================================================ */

bool recover_from_deadlock(RAG *rag, const DeadlockResult *deadlock,
                           const RecoveryConfig *config, RecoveryResult *result) {
    if (!rag || !deadlock || !config || !result) return false;
    
    recovery_result_init(result);
    
    if (!deadlock->deadlock_detected) {
        snprintf(result->summary, sizeof(result->summary), "No deadlock to recover from");
        result->success = true;
        return true;
    }
    
    switch (config->strategy) {
        case RECOVERY_TERMINATE_ALL:
            return recovery_terminate_all(rag, deadlock, result);
            
        case RECOVERY_TERMINATE_ONE:
        case RECOVERY_TERMINATE_LOWEST:
        case RECOVERY_TERMINATE_YOUNGEST:
        case RECOVERY_TERMINATE_OLDEST:
            return recovery_terminate_one(rag, deadlock, config->selection, result);
            
        case RECOVERY_PREEMPT_RESOURCES: {
            /* Preempt from lowest priority deadlocked process */
            int victim = select_victim_process(rag, deadlock, config->selection);
            if (victim >= 0) {
                int preempted = recovery_preempt_resources(rag, victim, NULL, 0, result);
                result->success = (preempted > 0);
                return result->success;
            }
            return false;
        }
            
        case RECOVERY_ROLLBACK: {
            /* Rollback lowest priority deadlocked process */
            int victim = select_victim_process(rag, deadlock, config->selection);
            if (victim >= 0) {
                return recovery_rollback_process(rag, victim, result);
            }
            return false;
        }
            
        default:
            snprintf(result->summary, sizeof(result->summary), "Unknown recovery strategy");
            return false;
    }
}

bool recover_default(RAG *rag, const DeadlockResult *deadlock, RecoveryResult *result) {
    RecoveryConfig config;
    recovery_config_init(&config);
    return recover_from_deadlock(rag, deadlock, &config, result);
}

/* ============================================================================
 * Specific Recovery Strategies
 * ============================================================================ */

bool recovery_terminate_all(RAG *rag, const DeadlockResult *deadlock, RecoveryResult *result) {
    if (!rag || !deadlock || !result) return false;
    
    int terminated = 0;
    
    for (int i = 0; i < deadlock->deadlocked_process_count; i++) {
        int pid = deadlock->deadlocked_processes[i];
        Process *p = rag_get_process(rag, pid);
        
        if (p) {
            /* Record action */
            if (result->action_count < MAX_PROCESSES) {
                RecoveryAction *action = &result->actions[result->action_count++];
                action->process_id = pid;
                action->resource_id = -1;
                action->strategy = RECOVERY_TERMINATE_ALL;
                snprintf(action->description, sizeof(action->description),
                         "Terminated process P%d (%s)", pid, p->name);
                action->success = true;
            }
            
            /* Release all resources and remove process */
            rag_release_all_resources(rag, pid);
            rag_remove_process(rag, pid);
            terminated++;
        }
    }
    
    result->processes_terminated = terminated;
    result->success = (terminated > 0);
    snprintf(result->summary, sizeof(result->summary),
             "Terminated %d deadlocked processes", terminated);
    
    return result->success;
}

bool recovery_terminate_one(RAG *rag, const DeadlockResult *deadlock,
                            SelectionCriteria criteria, RecoveryResult *result) {
    if (!rag || !deadlock || !result) return false;
    
    int victim = select_victim_process(rag, deadlock, criteria);
    
    if (victim < 0) {
        snprintf(result->summary, sizeof(result->summary),
                 "No suitable victim process found");
        return false;
    }
    
    Process *p = rag_get_process(rag, victim);
    if (!p) return false;
    
    /* Record action */
    if (result->action_count < MAX_PROCESSES) {
        RecoveryAction *action = &result->actions[result->action_count++];
        action->process_id = victim;
        action->resource_id = -1;
        action->strategy = RECOVERY_TERMINATE_ONE;
        snprintf(action->description, sizeof(action->description),
                 "Terminated process P%d (%s) using %s criteria",
                 victim, p->name, criteria_names[criteria]);
        action->success = true;
    }
    
    /* Release resources and remove process */
    int released = rag_release_all_resources(rag, victim);
    rag_remove_process(rag, victim);
    
    result->processes_terminated = 1;
    result->resources_preempted = released;
    result->success = true;
    snprintf(result->summary, sizeof(result->summary),
             "Terminated P%d (%s), released %d resources",
             victim, p->name, released);
    
    return true;
}

bool recovery_terminate_iterative(RAG *rag, SelectionCriteria criteria,
                                   int max_iterations, RecoveryResult *result) {
    if (!rag || !result) return false;
    
    recovery_result_init(result);
    
    DeadlockResult detection;
    int iterations = 0;
    
    while (iterations < max_iterations || max_iterations == 0) {
        detect_deadlock(rag, &detection);
        
        if (!detection.deadlock_detected) {
            result->success = true;
            snprintf(result->summary, sizeof(result->summary),
                     "Deadlock resolved after %d iterations, %d processes terminated",
                     iterations, result->processes_terminated);
            result->iterations = iterations;
            return true;
        }
        
        /* Terminate one process */
        RecoveryResult single_result;
        recovery_terminate_one(rag, &detection, criteria, &single_result);
        
        if (!single_result.success) {
            snprintf(result->summary, sizeof(result->summary),
                     "Failed to select victim at iteration %d", iterations);
            return false;
        }
        
        /* Copy action to main result */
        if (result->action_count < MAX_PROCESSES && single_result.action_count > 0) {
            result->actions[result->action_count++] = single_result.actions[0];
        }
        
        result->processes_terminated += single_result.processes_terminated;
        result->resources_preempted += single_result.resources_preempted;
        iterations++;
        
        if (max_iterations > 0 && iterations >= max_iterations) {
            break;
        }
    }
    
    /* Final check */
    detect_deadlock(rag, &detection);
    result->success = !detection.deadlock_detected;
    result->iterations = iterations;
    snprintf(result->summary, sizeof(result->summary),
             "%s after %d iterations, %d processes terminated",
             result->success ? "Resolved" : "Not resolved",
             iterations, result->processes_terminated);
    
    return result->success;
}

int recovery_preempt_resources(RAG *rag, int process_id, const int *resource_ids,
                                int resource_count, RecoveryResult *result) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return 0;
    
    Process *p = rag_get_process(rag, process_id);
    if (!p) return 0;
    
    int preempted = 0;
    
    if (resource_ids && resource_count > 0) {
        /* Preempt specific resources */
        for (int i = 0; i < resource_count; i++) {
            int rid = resource_ids[i];
            if (rag_release_resource(rag, process_id, rid)) {
                preempted++;
                
                if (result && result->action_count < MAX_PROCESSES) {
                    RecoveryAction *action = &result->actions[result->action_count++];
                    action->process_id = process_id;
                    action->resource_id = rid;
                    action->strategy = RECOVERY_PREEMPT_RESOURCES;
                    snprintf(action->description, sizeof(action->description),
                             "Preempted R%d from P%d", rid, process_id);
                    action->success = true;
                }
            }
        }
    } else {
        /* Preempt all resources */
        preempted = rag_release_all_resources(rag, process_id);
        
        if (result && result->action_count < MAX_PROCESSES) {
            RecoveryAction *action = &result->actions[result->action_count++];
            action->process_id = process_id;
            action->resource_id = -1;
            action->strategy = RECOVERY_PREEMPT_RESOURCES;
            snprintf(action->description, sizeof(action->description),
                     "Preempted all %d resources from P%d (%s)",
                     preempted, process_id, p->name);
            action->success = true;
        }
    }
    
    if (result) {
        result->resources_preempted += preempted;
    }
    
    /* Update process state to blocked */
    p->state = PROCESS_BLOCKED;
    
    return preempted;
}

bool recovery_rollback_process(RAG *rag, int process_id, RecoveryResult *result) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return false;
    
    Process *p = rag_get_process(rag, process_id);
    if (!p) return false;
    
    /* Release all resources */
    int released = rag_release_all_resources(rag, process_id);
    
    /* Cancel all requests */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        rag_cancel_request(rag, process_id, r);
    }
    
    /* Reset process state */
    p->state = PROCESS_RUNNING;
    
    if (result) {
        if (result->action_count < MAX_PROCESSES) {
            RecoveryAction *action = &result->actions[result->action_count++];
            action->process_id = process_id;
            action->resource_id = -1;
            action->strategy = RECOVERY_ROLLBACK;
            snprintf(action->description, sizeof(action->description),
                     "Rolled back P%d (%s), released %d resources",
                     process_id, p->name, released);
            action->success = true;
        }
        result->resources_preempted += released;
        result->success = true;
        snprintf(result->summary, sizeof(result->summary),
                 "Process P%d rolled back, %d resources released",
                 process_id, released);
    }
    
    return true;
}

/* ============================================================================
 * Process Selection Functions
 * ============================================================================ */

int select_victim_process(const RAG *rag, const DeadlockResult *deadlock,
                          SelectionCriteria criteria) {
    if (!rag || !deadlock || deadlock->deadlocked_process_count == 0) return -1;
    
    int victim = -1;
    int best_score = 0;
    bool first = true;
    
    for (int i = 0; i < deadlock->deadlocked_process_count; i++) {
        int pid = deadlock->deadlocked_processes[i];
        const Process *p = &rag->processes[pid];
        
        if (!p->active) continue;
        
        int score = 0;
        
        switch (criteria) {
            case SELECT_LOWEST_PRIORITY:
                /* Lower priority = higher score (more likely to be selected) */
                score = 100 - p->priority;
                break;
                
            case SELECT_FEWEST_RESOURCES: {
                /* Count held resources - fewer = higher score */
                int held = 0;
                for (int r = 0; r < MAX_RESOURCES; r++) {
                    held += rag->assignment_matrix[pid][r];
                }
                score = MAX_RESOURCES - held;
                break;
            }
                
            case SELECT_MOST_RESOURCES: {
                /* Count held resources - more = higher score */
                int held = 0;
                for (int r = 0; r < MAX_RESOURCES; r++) {
                    held += rag->assignment_matrix[pid][r];
                }
                score = held;
                break;
            }
                
            case SELECT_SHORTEST_RUNTIME:
                /* Use process ID as proxy for age (higher ID = younger) */
                score = pid;
                break;
                
            case SELECT_LONGEST_RUNTIME:
                /* Use process ID as proxy for age (lower ID = older) */
                score = MAX_PROCESSES - pid;
                break;
                
            case SELECT_MINIMUM_COST:
                score = calculate_termination_cost(rag, pid);
                /* Invert so lower cost = higher score */
                score = 1000 - score;
                break;
        }
        
        if (first || score > best_score) {
            best_score = score;
            victim = pid;
            first = false;
        }
    }
    
    return victim;
}

int calculate_termination_cost(const RAG *rag, int process_id) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return 0;
    
    const Process *p = &rag->processes[process_id];
    if (!p->active) return 0;
    
    /* Cost factors:
     * - Priority (higher priority = higher cost)
     * - Resources held (more resources = higher cost)
     * - Processes waiting for this process's resources
     */
    int cost = 0;
    
    /* Priority component */
    cost += p->priority * 10;
    
    /* Resources held component */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        cost += rag->assignment_matrix[process_id][r] * 20;
    }
    
    /* Dependency component - count processes waiting for resources held by this process */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        if (rag->assignment_matrix[process_id][r] > 0) {
            for (int p2 = 0; p2 < MAX_PROCESSES; p2++) {
                if (rag->request_matrix[p2][r] > 0) {
                    cost += 15;
                }
            }
        }
    }
    
    return cost;
}

bool is_critical_process(const RAG *rag, int process_id, int priority_threshold) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return false;
    
    const Process *p = &rag->processes[process_id];
    if (!p->active) return false;
    
    return p->priority >= priority_threshold;
}

/* ============================================================================
 * Analysis Functions
 * ============================================================================ */

int analyze_recovery_option(const RAG *rag, const DeadlockResult *deadlock,
                            RecoveryStrategy strategy, int *estimated_terminations,
                            int *estimated_resources_freed) {
    if (!rag || !deadlock) return 0;
    
    int terminations = 0;
    int resources = 0;
    int feasibility = 0;
    
    switch (strategy) {
        case RECOVERY_TERMINATE_ALL:
            terminations = deadlock->deadlocked_process_count;
            for (int i = 0; i < deadlock->deadlocked_process_count; i++) {
                int pid = deadlock->deadlocked_processes[i];
                for (int r = 0; r < MAX_RESOURCES; r++) {
                    resources += rag->assignment_matrix[pid][r];
                }
            }
            feasibility = 100; /* Always feasible */
            break;
            
        case RECOVERY_TERMINATE_ONE:
        case RECOVERY_TERMINATE_LOWEST:
        case RECOVERY_TERMINATE_YOUNGEST:
        case RECOVERY_TERMINATE_OLDEST:
            terminations = 1;
            /* Estimate: may need to terminate more */
            feasibility = 70;
            break;
            
        case RECOVERY_PREEMPT_RESOURCES:
            terminations = 0;
            resources = 1; /* At least one */
            feasibility = 50; /* May not fully resolve */
            break;
            
        case RECOVERY_ROLLBACK:
            terminations = 0;
            feasibility = 60;
            break;
            
        default:
            feasibility = 0;
    }
    
    if (estimated_terminations) *estimated_terminations = terminations;
    if (estimated_resources_freed) *estimated_resources_freed = resources;
    
    return feasibility;
}

RecoveryStrategy recommend_recovery_strategy(const RAG *rag, const DeadlockResult *deadlock) {
    if (!rag || !deadlock || !deadlock->deadlock_detected) {
        return RECOVERY_TERMINATE_LOWEST;
    }
    
    /* If only one process in deadlock, terminate it */
    if (deadlock->deadlocked_process_count == 1) {
        return RECOVERY_TERMINATE_ONE;
    }
    
    /* If small number of processes, try iterative termination */
    if (deadlock->deadlocked_process_count <= 3) {
        return RECOVERY_TERMINATE_LOWEST;
    }
    
    /* For larger deadlocks, consider preemption first */
    if (deadlock->deadlocked_process_count > 5) {
        /* Check if preemption might work */
        bool can_preempt = false;
        for (int i = 0; i < deadlock->deadlocked_process_count; i++) {
            int pid = deadlock->deadlocked_processes[i];
            int held = 0;
            for (int r = 0; r < MAX_RESOURCES; r++) {
                held += rag->assignment_matrix[pid][r];
            }
            if (held > 1) {
                can_preempt = true;
                break;
            }
        }
        if (can_preempt) {
            return RECOVERY_PREEMPT_RESOURCES;
        }
    }
    
    /* Default: terminate lowest priority */
    return RECOVERY_TERMINATE_LOWEST;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

const char* recovery_strategy_name(RecoveryStrategy strategy) {
    if (strategy >= 0 && strategy < sizeof(strategy_names) / sizeof(strategy_names[0])) {
        return strategy_names[strategy];
    }
    return "Unknown";
}

const char* selection_criteria_name(SelectionCriteria criteria) {
    if (criteria >= 0 && criteria < sizeof(criteria_names) / sizeof(criteria_names[0])) {
        return criteria_names[criteria];
    }
    return "Unknown";
}

void print_recovery_result(const RecoveryResult *result) {
    if (!result) {
        printf("No recovery result\n");
        return;
    }
    
    printf("\n========== Recovery Result ==========\n");
    printf("Success: %s\n", result->success ? "YES" : "NO");
    printf("Processes terminated: %d\n", result->processes_terminated);
    printf("Resources preempted: %d\n", result->resources_preempted);
    printf("Iterations: %d\n", result->iterations);
    printf("Summary: %s\n", result->summary);
    
    if (result->action_count > 0) {
        printf("\nActions taken:\n");
        for (int i = 0; i < result->action_count; i++) {
            printf("  %d. %s\n", i + 1, result->actions[i].description);
        }
    }
    printf("=====================================\n\n");
}

int recovery_result_to_string(const RecoveryResult *result, char *buffer, size_t buffer_size) {
    if (!result || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    written += snprintf(buffer + written, buffer_size - written,
                        "Success: %s\n", result->success ? "YES" : "NO");
    written += snprintf(buffer + written, buffer_size - written,
                        "Processes terminated: %d\n", result->processes_terminated);
    written += snprintf(buffer + written, buffer_size - written,
                        "Resources preempted: %d\n", result->resources_preempted);
    written += snprintf(buffer + written, buffer_size - written,
                        "Summary: %s\n", result->summary);
    
    return written;
}
