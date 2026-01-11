/**
 * @file cycle_detector.c
 * @brief DFS-based cycle detection algorithm implementation
 */

#include "cycle_detector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * Internal State for DFS
 * ============================================================================ */

typedef enum {
    COLOR_WHITE,  /* Not visited */
    COLOR_GRAY,   /* Currently in DFS stack */
    COLOR_BLACK   /* Finished processing */
} NodeColor;

/* Combined node representation for DFS */
typedef struct {
    int id;
    NodeType type;
} GraphNode;

/* DFS state structure */
typedef struct {
    const RAG *rag;
    NodeColor process_colors[MAX_PROCESSES];
    NodeColor resource_colors[MAX_RESOURCES];
    GraphNode path[MAX_CYCLE_LENGTH];
    int path_length;
    bool cycle_found;
    Cycle *output_cycle;
} DFSState;

/* ============================================================================
 * Internal Helper Functions
 * ============================================================================ */

static void init_dfs_state(DFSState *state, const RAG *rag, Cycle *output) {
    state->rag = rag;
    memset(state->process_colors, COLOR_WHITE, sizeof(state->process_colors));
    memset(state->resource_colors, COLOR_WHITE, sizeof(state->resource_colors));
    state->path_length = 0;
    state->cycle_found = false;
    state->output_cycle = output;
}

static void extract_cycle(DFSState *state, int cycle_start_idx) {
    if (!state->output_cycle) return;
    
    Cycle *cycle = state->output_cycle;
    cycle->length = 0;
    
    for (int i = cycle_start_idx; i < state->path_length && cycle->length < MAX_CYCLE_LENGTH; i++) {
        cycle->nodes[cycle->length].id = state->path[i].id;
        cycle->nodes[cycle->length].type = state->path[i].type;
        cycle->length++;
    }
    cycle->valid = true;
}

/* Forward declaration for mutual recursion */
static bool dfs_visit_resource(DFSState *state, int resource_id);

static bool dfs_visit_process(DFSState *state, int process_id) {
    if (state->cycle_found) return true;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (!state->rag->processes[process_id].active) return false;
    
    /* Check for cycle (back edge) */
    if (state->process_colors[process_id] == COLOR_GRAY) {
        /* Found cycle - find start of cycle in path */
        for (int i = 0; i < state->path_length; i++) {
            if (state->path[i].type == NODE_PROCESS && state->path[i].id == process_id) {
                extract_cycle(state, i);
                state->cycle_found = true;
                return true;
            }
        }
    }
    
    /* Skip if already processed */
    if (state->process_colors[process_id] == COLOR_BLACK) return false;
    
    /* Mark as in-progress */
    state->process_colors[process_id] = COLOR_GRAY;
    
    /* Add to path */
    if (state->path_length < MAX_CYCLE_LENGTH) {
        state->path[state->path_length].id = process_id;
        state->path[state->path_length].type = NODE_PROCESS;
        state->path_length++;
    }
    
    /* Visit all resources this process is requesting */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        if (state->rag->request_matrix[process_id][r] > 0) {
            if (dfs_visit_resource(state, r)) {
                return true;
            }
        }
    }
    
    /* Remove from path */
    state->path_length--;
    
    /* Mark as finished */
    state->process_colors[process_id] = COLOR_BLACK;
    
    return false;
}

static bool dfs_visit_resource(DFSState *state, int resource_id) {
    if (state->cycle_found) return true;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    if (!state->rag->resources[resource_id].active) return false;
    
    /* Check for cycle (back edge) */
    if (state->resource_colors[resource_id] == COLOR_GRAY) {
        /* Found cycle - find start of cycle in path */
        for (int i = 0; i < state->path_length; i++) {
            if (state->path[i].type == NODE_RESOURCE && state->path[i].id == resource_id) {
                extract_cycle(state, i);
                state->cycle_found = true;
                return true;
            }
        }
    }
    
    /* Skip if already processed */
    if (state->resource_colors[resource_id] == COLOR_BLACK) return false;
    
    /* Mark as in-progress */
    state->resource_colors[resource_id] = COLOR_GRAY;
    
    /* Add to path */
    if (state->path_length < MAX_CYCLE_LENGTH) {
        state->path[state->path_length].id = resource_id;
        state->path[state->path_length].type = NODE_RESOURCE;
        state->path_length++;
    }
    
    /* Visit all processes holding this resource */
    for (int p = 0; p < MAX_PROCESSES; p++) {
        if (state->rag->assignment_matrix[p][resource_id] > 0) {
            if (dfs_visit_process(state, p)) {
                return true;
            }
        }
    }
    
    /* Remove from path */
    state->path_length--;
    
    /* Mark as finished */
    state->resource_colors[resource_id] = COLOR_BLACK;
    
    return false;
}

/* ============================================================================
 * Core Detection Functions
 * ============================================================================ */

void deadlock_result_init(DeadlockResult *result) {
    if (!result) return;
    
    result->deadlock_detected = false;
    result->cycle_count = 0;
    result->deadlocked_process_count = 0;
    result->deadlocked_resource_count = 0;
    
    for (int i = 0; i < MAX_CYCLES; i++) {
        result->cycles[i].valid = false;
        result->cycles[i].length = 0;
    }
    
    memset(result->deadlocked_processes, -1, sizeof(result->deadlocked_processes));
    memset(result->deadlocked_resources, -1, sizeof(result->deadlocked_resources));
}

bool detect_deadlock(const RAG *rag, DeadlockResult *result) {
    return detect_deadlock_with_algorithm(rag, result, DETECT_DFS);
}

bool detect_deadlock_with_algorithm(const RAG *rag, DeadlockResult *result, 
                                    DetectionAlgorithm algorithm) {
    if (!rag || !result) return false;
    
    deadlock_result_init(result);
    
    switch (algorithm) {
        case DETECT_DFS:
        case DETECT_DFS_ALL_CYCLES: {
            DFSState state;
            Cycle cycle;
            cycle.valid = false;
            
            init_dfs_state(&state, rag, &cycle);
            
            /* Start DFS from each process that has pending requests */
            for (int p = 0; p < MAX_PROCESSES; p++) {
                if (!rag->processes[p].active) continue;
                
                /* Check if process has any pending requests */
                bool has_request = false;
                for (int r = 0; r < MAX_RESOURCES; r++) {
                    if (rag->request_matrix[p][r] > 0) {
                        has_request = true;
                        break;
                    }
                }
                
                if (has_request && state.process_colors[p] == COLOR_WHITE) {
                    if (dfs_visit_process(&state, p)) {
                        result->deadlock_detected = true;
                        
                        /* Copy found cycle to result */
                        if (result->cycle_count < MAX_CYCLES) {
                            memcpy(&result->cycles[result->cycle_count], &cycle, sizeof(Cycle));
                            result->cycle_count++;
                        }
                        
                        if (algorithm == DETECT_DFS) {
                            break; /* Stop at first cycle */
                        } else {
                            /* Reset for finding more cycles */
                            cycle.valid = false;
                            state.cycle_found = false;
                        }
                    }
                }
            }
            
            /* Extract deadlocked processes and resources from cycles */
            if (result->deadlock_detected) {
                bool process_in_deadlock[MAX_PROCESSES] = {false};
                bool resource_in_deadlock[MAX_RESOURCES] = {false};
                
                for (int c = 0; c < result->cycle_count; c++) {
                    Cycle *cyc = &result->cycles[c];
                    for (int n = 0; n < cyc->length; n++) {
                        if (cyc->nodes[n].type == NODE_PROCESS) {
                            process_in_deadlock[cyc->nodes[n].id] = true;
                        } else {
                            resource_in_deadlock[cyc->nodes[n].id] = true;
                        }
                    }
                }
                
                for (int p = 0; p < MAX_PROCESSES; p++) {
                    if (process_in_deadlock[p]) {
                        result->deadlocked_processes[result->deadlocked_process_count++] = p;
                    }
                }
                for (int r = 0; r < MAX_RESOURCES; r++) {
                    if (resource_in_deadlock[r]) {
                        result->deadlocked_resources[result->deadlocked_resource_count++] = r;
                    }
                }
            }
            break;
        }
    }
    
    return result->deadlock_detected;
}

int detect_all_cycles(const RAG *rag, DeadlockResult *result) {
    if (!rag || !result) return 0;
    
    detect_deadlock_with_algorithm(rag, result, DETECT_DFS_ALL_CYCLES);
    return result->cycle_count;
}

/* ============================================================================
 * Specific Detection Algorithms
 * ============================================================================ */

bool dfs_from_process(const RAG *rag, int start_process_id, Cycle *cycle) {
    if (!rag || !cycle) return false;
    if (start_process_id < 0 || start_process_id >= MAX_PROCESSES) return false;
    if (!rag->processes[start_process_id].active) return false;
    
    DFSState state;
    init_dfs_state(&state, rag, cycle);
    
    return dfs_visit_process(&state, start_process_id);
}

bool is_process_deadlocked(const RAG *rag, int process_id) {
    if (!rag) return false;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (!rag->processes[process_id].active) return false;
    
    DeadlockResult result;
    detect_deadlock(rag, &result);
    
    if (!result.deadlock_detected) return false;
    
    for (int i = 0; i < result.deadlocked_process_count; i++) {
        if (result.deadlocked_processes[i] == process_id) {
            return true;
        }
    }
    
    return false;
}

bool is_resource_in_deadlock(const RAG *rag, int resource_id) {
    if (!rag) return false;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    if (!rag->resources[resource_id].active) return false;
    
    DeadlockResult result;
    detect_deadlock(rag, &result);
    
    if (!result.deadlock_detected) return false;
    
    for (int i = 0; i < result.deadlocked_resource_count; i++) {
        if (result.deadlocked_resources[i] == resource_id) {
            return true;
        }
    }
    
    return false;
}

/* ============================================================================
 * Wait-For Graph Functions
 * ============================================================================ */

void build_wait_for_graph(const RAG *rag, int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES]) {
    if (!rag || !wait_for_matrix) return;
    
    /* Clear matrix */
    memset(wait_for_matrix, 0, MAX_PROCESSES * MAX_PROCESSES * sizeof(int));
    
    /* P1 waits for P2 if:
     * - P1 is requesting resource R
     * - P2 is holding resource R
     */
    for (int p1 = 0; p1 < MAX_PROCESSES; p1++) {
        if (!rag->processes[p1].active) continue;
        
        for (int r = 0; r < MAX_RESOURCES; r++) {
            if (rag->request_matrix[p1][r] > 0) {
                /* P1 is requesting R, find who holds R */
                for (int p2 = 0; p2 < MAX_PROCESSES; p2++) {
                    if (p1 != p2 && rag->assignment_matrix[p2][r] > 0) {
                        wait_for_matrix[p1][p2] = 1;
                    }
                }
            }
        }
    }
}

bool detect_cycle_in_wait_for(int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES],
                               int process_count, int *cycle, int *cycle_length) {
    if (!wait_for_matrix || !cycle || !cycle_length) return false;
    
    NodeColor colors[MAX_PROCESSES];
    int parent[MAX_PROCESSES];
    memset(colors, COLOR_WHITE, sizeof(colors));
    memset(parent, -1, sizeof(parent));
    
    *cycle_length = 0;
    
    for (int start = 0; start < process_count; start++) {
        if (colors[start] != COLOR_WHITE) continue;
        
        int stack[MAX_PROCESSES];
        int stack_size = 0;
        stack[stack_size++] = start;
        
        while (stack_size > 0) {
            int current = stack[stack_size - 1];
            
            if (colors[current] == COLOR_WHITE) {
                colors[current] = COLOR_GRAY;
            }
            
            bool found_unvisited = false;
            for (int next = 0; next < process_count; next++) {
                if (wait_for_matrix[current][next] == 0) continue;
                
                if (colors[next] == COLOR_GRAY) {
                    /* Found cycle - extract it */
                    cycle[(*cycle_length)++] = next;
                    int trace = current;
                    while (trace != next && *cycle_length < MAX_PROCESSES) {
                        cycle[(*cycle_length)++] = trace;
                        trace = parent[trace];
                    }
                    return true;
                }
                
                if (colors[next] == COLOR_WHITE) {
                    parent[next] = current;
                    stack[stack_size++] = next;
                    found_unvisited = true;
                    break;
                }
            }
            
            if (!found_unvisited) {
                colors[current] = COLOR_BLACK;
                stack_size--;
            }
        }
    }
    
    return false;
}

/* ============================================================================
 * Analysis Functions
 * ============================================================================ */

int get_cycle_processes(const Cycle *cycle, int *processes, int max_size) {
    if (!cycle || !processes || max_size <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < cycle->length && count < max_size; i++) {
        if (cycle->nodes[i].type == NODE_PROCESS) {
            processes[count++] = cycle->nodes[i].id;
        }
    }
    return count;
}

int get_cycle_resources(const Cycle *cycle, int *resources, int max_size) {
    if (!cycle || !resources || max_size <= 0) return 0;
    
    int count = 0;
    for (int i = 0; i < cycle->length && count < max_size; i++) {
        if (cycle->nodes[i].type == NODE_RESOURCE) {
            resources[count++] = cycle->nodes[i].id;
        }
    }
    return count;
}

int calculate_deadlock_depth(const DeadlockResult *result) {
    if (!result || !result->deadlock_detected) return 0;
    return result->deadlocked_process_count;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

void print_cycle(const RAG *rag, const Cycle *cycle) {
    if (!cycle || !cycle->valid) {
        printf("No cycle\n");
        return;
    }
    
    printf("Cycle: ");
    for (int i = 0; i < cycle->length; i++) {
        if (cycle->nodes[i].type == NODE_PROCESS) {
            if (rag) {
                printf("P%d(%s)", cycle->nodes[i].id, rag->processes[cycle->nodes[i].id].name);
            } else {
                printf("P%d", cycle->nodes[i].id);
            }
        } else {
            if (rag) {
                printf("R%d(%s)", cycle->nodes[i].id, rag->resources[cycle->nodes[i].id].name);
            } else {
                printf("R%d", cycle->nodes[i].id);
            }
        }
        if (i < cycle->length - 1) printf(" -> ");
    }
    printf(" -> (back to start)\n");
}

void print_deadlock_result(const RAG *rag, const DeadlockResult *result) {
    if (!result) {
        printf("No result\n");
        return;
    }
    
    printf("\n========== Deadlock Detection Result ==========\n");
    printf("Deadlock detected: %s\n", result->deadlock_detected ? "YES" : "NO");
    
    if (result->deadlock_detected) {
        printf("\nCycles found: %d\n", result->cycle_count);
        for (int c = 0; c < result->cycle_count; c++) {
            printf("  Cycle %d: ", c + 1);
            print_cycle(rag, &result->cycles[c]);
        }
        
        printf("\nDeadlocked processes (%d): ", result->deadlocked_process_count);
        for (int i = 0; i < result->deadlocked_process_count; i++) {
            int pid = result->deadlocked_processes[i];
            if (rag && rag->processes[pid].active) {
                printf("P%d(%s) ", pid, rag->processes[pid].name);
            } else {
                printf("P%d ", pid);
            }
        }
        printf("\n");
        
        printf("Deadlocked resources (%d): ", result->deadlocked_resource_count);
        for (int i = 0; i < result->deadlocked_resource_count; i++) {
            int rid = result->deadlocked_resources[i];
            if (rag && rag->resources[rid].active) {
                printf("R%d(%s) ", rid, rag->resources[rid].name);
            } else {
                printf("R%d ", rid);
            }
        }
        printf("\n");
    }
    
    printf("================================================\n\n");
}

int deadlock_result_to_string(const RAG *rag, const DeadlockResult *result,
                               char *buffer, size_t buffer_size) {
    if (!result || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    written += snprintf(buffer + written, buffer_size - written,
                        "Deadlock: %s\n", result->deadlock_detected ? "YES" : "NO");
    
    if (result->deadlock_detected && written < (int)buffer_size) {
        written += snprintf(buffer + written, buffer_size - written,
                            "Cycles: %d\n", result->cycle_count);
        written += snprintf(buffer + written, buffer_size - written,
                            "Deadlocked processes: %d\n", result->deadlocked_process_count);
        written += snprintf(buffer + written, buffer_size - written,
                            "Deadlocked resources: %d\n", result->deadlocked_resource_count);
    }
    
    return written;
}
