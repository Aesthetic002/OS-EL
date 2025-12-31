/**
 * @file simulator.c
 * @brief Deadlock simulation scenarios implementation
 */

#include "simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ============================================================================
 * Scenario Descriptions
 * ============================================================================ */

static const char* scenario_names[] = {
    "Simple Deadlock",
    "Circular Wait",
    "Dining Philosophers",
    "Producer-Consumer",
    "Reader-Writer",
    "Banker's Safe State",
    "Banker's Unsafe State",
    "No Deadlock",
    "Multiple Cycles",
    "Complex Dependency",
    "Random",
    "Custom"
};

static const char* scenario_descriptions[] = {
    "Two processes each holding one resource and requesting the other",
    "N processes in circular wait chain (P1->R1->P2->R2->...->Pn->Rn->P1)",
    "Classic dining philosophers problem with N philosophers and N forks",
    "Producer and consumer processes sharing buffer resources",
    "Multiple readers and writers competing for shared resources",
    "A state where deadlock is possible but not present",
    "A state where deadlock is unavoidable",
    "A scenario that demonstrates no deadlock occurring",
    "Multiple independent cycles in the resource allocation graph",
    "Complex dependencies between processes and resources",
    "Randomly generated scenario",
    "User-defined custom scenario"
};

static const char* event_type_names[] = {
    "PROCESS_CREATE",
    "PROCESS_TERMINATE",
    "RESOURCE_REQUEST",
    "RESOURCE_ALLOCATE",
    "RESOURCE_RELEASE",
    "DEADLOCK_DETECTED",
    "RECOVERY_STARTED",
    "RECOVERY_COMPLETED",
    "TICK"
};

/* Callback storage */
static SimulationCallback g_callback = NULL;
static void *g_callback_data = NULL;

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

void simulation_init(SimulationState *state) {
    if (!state) return;
    
    rag_init(&state->rag);
    state->scenario = SCENARIO_CUSTOM;
    state->current_tick = 0;
    state->running = false;
    state->paused = false;
    state->deadlock_occurred = false;
    state->event_count = 0;
    
    deadlock_result_init(&state->last_detection);
    recovery_result_init(&state->last_recovery);
}

void simulation_config_init(SimulationConfig *config) {
    if (!config) return;
    
    config->scenario = SCENARIO_SIMPLE_DEADLOCK;
    config->num_processes = 4;
    config->num_resources = 4;
    config->auto_detect = true;
    config->auto_recover = false;
    config->recovery_strategy = RECOVERY_TERMINATE_LOWEST;
    config->random_seed = 0;
    config->max_ticks = 100;
    config->verbose = true;
}

void simulation_reset(SimulationState *state) {
    if (!state) return;
    
    rag_reset(&state->rag);
    state->current_tick = 0;
    state->running = false;
    state->paused = false;
    state->deadlock_occurred = false;
    state->event_count = 0;
    
    deadlock_result_init(&state->last_detection);
    recovery_result_init(&state->last_recovery);
}

void simulation_destroy(SimulationState *state) {
    if (!state) return;
    rag_destroy(&state->rag);
    simulation_init(state);
}

/* ============================================================================
 * Event Logging
 * ============================================================================ */

void simulation_log_event(SimulationState *state, SimulationEventType type,
                          int process_id, int resource_id, const char *description) {
    if (!state || state->event_count >= 1024) return;
    
    SimulationEvent *event = &state->events[state->event_count++];
    event->timestamp = state->current_tick;
    event->type = type;
    event->process_id = process_id;
    event->resource_id = resource_id;
    
    if (description) {
        strncpy(event->description, description, sizeof(event->description) - 1);
        event->description[sizeof(event->description) - 1] = '\0';
    } else {
        event->description[0] = '\0';
    }
    
    /* Invoke callback if set */
    if (g_callback) {
        g_callback(event, g_callback_data);
    }
}

void simulation_set_callback(SimulationState *state, SimulationCallback callback,
                             void *user_data) {
    (void)state; /* Unused for now */
    g_callback = callback;
    g_callback_data = user_data;
}

int simulation_get_events(const SimulationState *state, SimulationEvent *events, int max_events) {
    if (!state || !events || max_events <= 0) return 0;
    
    int count = (state->event_count < max_events) ? state->event_count : max_events;
    memcpy(events, state->events, count * sizeof(SimulationEvent));
    return count;
}

void simulation_clear_events(SimulationState *state) {
    if (!state) return;
    state->event_count = 0;
}

/* ============================================================================
 * Scenario Setup Functions
 * ============================================================================ */

void setup_simple_deadlock(SimulationState *state) {
    if (!state) return;
    
    simulation_reset(state);
    state->scenario = SCENARIO_SIMPLE_DEADLOCK;
    
    /* Create 2 processes and 2 resources */
    int p1 = rag_add_process(&state->rag, "Process_1", 50);
    int p2 = rag_add_process(&state->rag, "Process_2", 50);
    int r1 = rag_add_resource(&state->rag, "Resource_A", 1);
    int r2 = rag_add_resource(&state->rag, "Resource_B", 1);
    
    simulation_log_event(state, EVENT_PROCESS_CREATE, p1, -1, "Created Process_1");
    simulation_log_event(state, EVENT_PROCESS_CREATE, p2, -1, "Created Process_2");
    
    /* P1 holds R1, requests R2 */
    rag_allocate_resource(&state->rag, p1, r1);
    simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, p1, r1, "P1 allocated R1");
    
    /* P2 holds R2, requests R1 */
    rag_allocate_resource(&state->rag, p2, r2);
    simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, p2, r2, "P2 allocated R2");
    
    /* Create deadlock: P1 requests R2, P2 requests R1 */
    rag_request_resource(&state->rag, p1, r2);
    simulation_log_event(state, EVENT_RESOURCE_REQUEST, p1, r2, "P1 requests R2 (BLOCKED)");
    
    rag_request_resource(&state->rag, p2, r1);
    simulation_log_event(state, EVENT_RESOURCE_REQUEST, p2, r1, "P2 requests R1 (BLOCKED)");
    
    state->deadlock_occurred = true;
}

void setup_circular_wait(SimulationState *state, int n) {
    if (!state || n < 2) return;
    if (n > MAX_PROCESSES) n = MAX_PROCESSES;
    
    simulation_reset(state);
    state->scenario = SCENARIO_CIRCULAR_WAIT;
    
    int processes[MAX_PROCESSES];
    int resources[MAX_RESOURCES];
    char name[MAX_NAME_LENGTH];
    
    /* Create N processes and N resources */
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "Process_%d", i + 1);
        processes[i] = rag_add_process(&state->rag, name, 50);
        simulation_log_event(state, EVENT_PROCESS_CREATE, processes[i], -1, name);
        
        snprintf(name, sizeof(name), "Resource_%c", 'A' + i);
        resources[i] = rag_add_resource(&state->rag, name, 1);
    }
    
    /* Each process holds one resource and requests the next */
    for (int i = 0; i < n; i++) {
        /* Pi holds Ri */
        rag_allocate_resource(&state->rag, processes[i], resources[i]);
        snprintf(name, sizeof(name), "P%d allocated R%d", i + 1, i + 1);
        simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, processes[i], resources[i], name);
        
        /* Pi requests R(i+1) mod n */
        int next_resource = resources[(i + 1) % n];
        rag_request_resource(&state->rag, processes[i], next_resource);
        snprintf(name, sizeof(name), "P%d requests R%d", i + 1, ((i + 1) % n) + 1);
        simulation_log_event(state, EVENT_RESOURCE_REQUEST, processes[i], next_resource, name);
    }
    
    state->deadlock_occurred = true;
}

void setup_dining_philosophers(SimulationState *state, int n) {
    if (!state || n < 2) return;
    if (n > MAX_PROCESSES) n = MAX_PROCESSES;
    
    simulation_reset(state);
    state->scenario = SCENARIO_DINING_PHILOSOPHERS;
    
    int philosophers[MAX_PROCESSES];
    int forks[MAX_RESOURCES];
    char name[MAX_NAME_LENGTH];
    
    /* Create N philosophers and N forks */
    for (int i = 0; i < n; i++) {
        snprintf(name, sizeof(name), "Philosopher_%d", i + 1);
        philosophers[i] = rag_add_process(&state->rag, name, 50);
        simulation_log_event(state, EVENT_PROCESS_CREATE, philosophers[i], -1, name);
        
        snprintf(name, sizeof(name), "Fork_%d", i + 1);
        forks[i] = rag_add_resource(&state->rag, name, 1);
    }
    
    /* Each philosopher picks up left fork, then tries to pick up right fork */
    /* This creates deadlock when all philosophers pick up their left fork */
    for (int i = 0; i < n; i++) {
        int left_fork = forks[i];
        int right_fork = forks[(i + 1) % n];
        
        /* Pick up left fork */
        rag_allocate_resource(&state->rag, philosophers[i], left_fork);
        snprintf(name, sizeof(name), "Philosopher %d picks up left fork %d", i + 1, i + 1);
        simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, philosophers[i], left_fork, name);
        
        /* Request right fork */
        rag_request_resource(&state->rag, philosophers[i], right_fork);
        snprintf(name, sizeof(name), "Philosopher %d requests right fork %d", i + 1, ((i + 1) % n) + 1);
        simulation_log_event(state, EVENT_RESOURCE_REQUEST, philosophers[i], right_fork, name);
    }
    
    state->deadlock_occurred = true;
}

void setup_random_scenario(SimulationState *state, int num_processes,
                           int num_resources, int seed) {
    if (!state) return;
    if (num_processes > MAX_PROCESSES) num_processes = MAX_PROCESSES;
    if (num_resources > MAX_RESOURCES) num_resources = MAX_RESOURCES;
    if (num_processes < 2) num_processes = 2;
    if (num_resources < 2) num_resources = 2;
    
    simulation_reset(state);
    state->scenario = SCENARIO_RANDOM;
    
    /* Seed random number generator */
    if (seed == 0) {
        srand((unsigned int)time(NULL));
    } else {
        srand(seed);
    }
    
    char name[MAX_NAME_LENGTH];
    
    /* Create processes */
    for (int i = 0; i < num_processes; i++) {
        snprintf(name, sizeof(name), "Process_%d", i + 1);
        int priority = rand() % 100;
        int pid = rag_add_process(&state->rag, name, priority);
        simulation_log_event(state, EVENT_PROCESS_CREATE, pid, -1, name);
    }
    
    /* Create resources with random instances */
    for (int i = 0; i < num_resources; i++) {
        snprintf(name, sizeof(name), "Resource_%c", 'A' + i);
        int instances = 1 + rand() % 3; /* 1-3 instances */
        rag_add_resource(&state->rag, name, instances);
    }
    
    /* Randomly allocate and request resources */
    for (int p = 0; p < num_processes; p++) {
        /* Randomly allocate 0-2 resources */
        int to_allocate = rand() % 3;
        for (int a = 0; a < to_allocate; a++) {
            int r = rand() % num_resources;
            if (state->rag.resources[r].available_instances > 0) {
                rag_allocate_resource(&state->rag, p, r);
                snprintf(name, sizeof(name), "P%d allocated R%d", p + 1, r + 1);
                simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, p, r, name);
            }
        }
        
        /* Randomly request 1-2 resources */
        int to_request = 1 + rand() % 2;
        for (int req = 0; req < to_request; req++) {
            int r = rand() % num_resources;
            if (!rag_is_holding(&state->rag, p, r) && !rag_is_requesting(&state->rag, p, r)) {
                rag_request_resource(&state->rag, p, r);
                snprintf(name, sizeof(name), "P%d requests R%d", p + 1, r + 1);
                simulation_log_event(state, EVENT_RESOURCE_REQUEST, p, r, name);
            }
        }
    }
}

bool simulation_load_scenario(SimulationState *state, SimulationScenario scenario) {
    if (!state) return false;
    
    switch (scenario) {
        case SCENARIO_SIMPLE_DEADLOCK:
            setup_simple_deadlock(state);
            break;
            
        case SCENARIO_CIRCULAR_WAIT:
            setup_circular_wait(state, 4);
            break;
            
        case SCENARIO_DINING_PHILOSOPHERS:
            setup_dining_philosophers(state, 5);
            break;
            
        case SCENARIO_NO_DEADLOCK:
            simulation_reset(state);
            state->scenario = SCENARIO_NO_DEADLOCK;
            {
                int p1 = rag_add_process(&state->rag, "Process_1", 50);
                int p2 = rag_add_process(&state->rag, "Process_2", 50);
                int r1 = rag_add_resource(&state->rag, "Resource_A", 1);
                int r2 = rag_add_resource(&state->rag, "Resource_B", 1);
                
                /* P1 holds R1, P2 holds R2 - no cross requests */
                rag_allocate_resource(&state->rag, p1, r1);
                rag_allocate_resource(&state->rag, p2, r2);
            }
            break;
            
        case SCENARIO_MULTIPLE_CYCLES:
            simulation_reset(state);
            state->scenario = SCENARIO_MULTIPLE_CYCLES;
            {
                /* Create two independent cycles */
                /* Cycle 1: P1 <-> P2 */
                int p1 = rag_add_process(&state->rag, "Process_1", 50);
                int p2 = rag_add_process(&state->rag, "Process_2", 50);
                int r1 = rag_add_resource(&state->rag, "Resource_A", 1);
                int r2 = rag_add_resource(&state->rag, "Resource_B", 1);
                
                rag_allocate_resource(&state->rag, p1, r1);
                rag_allocate_resource(&state->rag, p2, r2);
                rag_request_resource(&state->rag, p1, r2);
                rag_request_resource(&state->rag, p2, r1);
                
                /* Cycle 2: P3 <-> P4 */
                int p3 = rag_add_process(&state->rag, "Process_3", 50);
                int p4 = rag_add_process(&state->rag, "Process_4", 50);
                int r3 = rag_add_resource(&state->rag, "Resource_C", 1);
                int r4 = rag_add_resource(&state->rag, "Resource_D", 1);
                
                rag_allocate_resource(&state->rag, p3, r3);
                rag_allocate_resource(&state->rag, p4, r4);
                rag_request_resource(&state->rag, p3, r4);
                rag_request_resource(&state->rag, p4, r3);
            }
            state->deadlock_occurred = true;
            break;
            
        case SCENARIO_RANDOM:
            setup_random_scenario(state, 4, 4, 0);
            break;
            
        case SCENARIO_CUSTOM:
            simulation_reset(state);
            state->scenario = SCENARIO_CUSTOM;
            break;
            
        default:
            return false;
    }
    
    return true;
}

bool simulation_load_with_config(SimulationState *state, const SimulationConfig *config) {
    if (!state || !config) return false;
    
    if (config->scenario == SCENARIO_RANDOM) {
        setup_random_scenario(state, config->num_processes, config->num_resources,
                              config->random_seed);
        return true;
    } else if (config->scenario == SCENARIO_CIRCULAR_WAIT) {
        setup_circular_wait(state, config->num_processes);
        return true;
    } else if (config->scenario == SCENARIO_DINING_PHILOSOPHERS) {
        setup_dining_philosophers(state, config->num_processes);
        return true;
    }
    
    return simulation_load_scenario(state, config->scenario);
}

/* ============================================================================
 * Simulation Control Functions
 * ============================================================================ */

bool simulation_start(SimulationState *state) {
    if (!state) return false;
    state->running = true;
    state->paused = false;
    return true;
}

void simulation_pause(SimulationState *state) {
    if (!state) return;
    state->paused = true;
}

void simulation_resume(SimulationState *state) {
    if (!state) return;
    state->paused = false;
}

void simulation_stop(SimulationState *state) {
    if (!state) return;
    state->running = false;
    state->paused = false;
}

bool simulation_tick(SimulationState *state, const SimulationConfig *config) {
    if (!state) return false;
    if (!state->running || state->paused) return false;
    
    state->current_tick++;
    simulation_log_event(state, EVENT_TICK, -1, -1, "Simulation tick");
    
    /* Auto-detect deadlock if enabled */
    if (config && config->auto_detect) {
        if (detect_deadlock(&state->rag, &state->last_detection)) {
            state->deadlock_occurred = true;
            simulation_log_event(state, EVENT_DEADLOCK_DETECTED, -1, -1, "Deadlock detected!");
            
            /* Auto-recover if enabled */
            if (config->auto_recover) {
                simulation_log_event(state, EVENT_RECOVERY_STARTED, -1, -1, "Starting recovery");
                
                RecoveryConfig rec_config;
                recovery_config_init(&rec_config);
                rec_config.strategy = config->recovery_strategy;
                
                if (recover_from_deadlock(&state->rag, &state->last_detection,
                                          &rec_config, &state->last_recovery)) {
                    simulation_log_event(state, EVENT_RECOVERY_COMPLETED, -1, -1, 
                                         state->last_recovery.summary);
                    state->deadlock_occurred = false;
                }
            }
        }
    }
    
    /* Check if max ticks reached */
    if (config && config->max_ticks > 0 && state->current_tick >= config->max_ticks) {
        simulation_stop(state);
        return false;
    }
    
    return true;
}

int simulation_run_until_deadlock(SimulationState *state, const SimulationConfig *config,
                                   int max_ticks) {
    if (!state) return 0;
    
    simulation_start(state);
    int ticks = 0;
    
    while (state->running && (max_ticks == 0 || ticks < max_ticks)) {
        simulation_tick(state, config);
        ticks++;
        
        if (state->deadlock_occurred) {
            break;
        }
    }
    
    return ticks;
}

bool simulation_run_complete(SimulationState *state, const SimulationConfig *config) {
    if (!state || !config) return false;
    
    SimulationConfig run_config = *config;
    run_config.auto_detect = true;
    run_config.auto_recover = true;
    
    simulation_start(state);
    
    int max = config->max_ticks > 0 ? config->max_ticks : 100;
    
    for (int i = 0; i < max && state->running; i++) {
        simulation_tick(state, &run_config);
    }
    
    return !state->deadlock_occurred;
}

/* ============================================================================
 * Manual Control Functions
 * ============================================================================ */

int simulation_add_process(SimulationState *state, const char *name, int priority) {
    if (!state || !name) return -1;
    
    int pid = rag_add_process(&state->rag, name, priority);
    if (pid >= 0) {
        char desc[256];
        snprintf(desc, sizeof(desc), "Added process %s with priority %d", name, priority);
        simulation_log_event(state, EVENT_PROCESS_CREATE, pid, -1, desc);
    }
    return pid;
}

int simulation_add_resource(SimulationState *state, const char *name, int instances) {
    if (!state || !name) return -1;
    
    int rid = rag_add_resource(&state->rag, name, instances);
    return rid;
}

bool simulation_request_resource(SimulationState *state, int process_id, int resource_id) {
    if (!state) return false;
    
    bool result = rag_request_resource(&state->rag, process_id, resource_id);
    if (result) {
        char desc[256];
        snprintf(desc, sizeof(desc), "P%d requests R%d", process_id, resource_id);
        simulation_log_event(state, EVENT_RESOURCE_REQUEST, process_id, resource_id, desc);
    }
    return result;
}

bool simulation_allocate_resource(SimulationState *state, int process_id, int resource_id) {
    if (!state) return false;
    
    bool result = rag_allocate_resource(&state->rag, process_id, resource_id);
    if (result) {
        char desc[256];
        snprintf(desc, sizeof(desc), "R%d allocated to P%d", resource_id, process_id);
        simulation_log_event(state, EVENT_RESOURCE_ALLOCATE, process_id, resource_id, desc);
    }
    return result;
}

bool simulation_release_resource(SimulationState *state, int process_id, int resource_id) {
    if (!state) return false;
    
    bool result = rag_release_resource(&state->rag, process_id, resource_id);
    if (result) {
        char desc[256];
        snprintf(desc, sizeof(desc), "P%d released R%d", process_id, resource_id);
        simulation_log_event(state, EVENT_RESOURCE_RELEASE, process_id, resource_id, desc);
    }
    return result;
}

bool simulation_detect_deadlock(SimulationState *state) {
    if (!state) return false;
    
    bool result = detect_deadlock(&state->rag, &state->last_detection);
    state->deadlock_occurred = result;
    
    if (result) {
        simulation_log_event(state, EVENT_DEADLOCK_DETECTED, -1, -1, "Deadlock detected");
    }
    
    return result;
}

bool simulation_recover(SimulationState *state, RecoveryStrategy strategy) {
    if (!state) return false;
    
    if (!state->deadlock_occurred) {
        return true; /* Nothing to recover from */
    }
    
    RecoveryConfig config;
    recovery_config_init(&config);
    config.strategy = strategy;
    
    simulation_log_event(state, EVENT_RECOVERY_STARTED, -1, -1, "Starting recovery");
    
    bool result = recover_from_deadlock(&state->rag, &state->last_detection,
                                         &config, &state->last_recovery);
    
    if (result) {
        simulation_log_event(state, EVENT_RECOVERY_COMPLETED, -1, -1, 
                             state->last_recovery.summary);
        state->deadlock_occurred = false;
    }
    
    return result;
}

/* ============================================================================
 * Query Functions
 * ============================================================================ */

const char* simulation_scenario_name(SimulationScenario scenario) {
    if (scenario >= 0 && scenario < sizeof(scenario_names) / sizeof(scenario_names[0])) {
        return scenario_names[scenario];
    }
    return "Unknown";
}

const char* simulation_scenario_description(SimulationScenario scenario) {
    if (scenario >= 0 && scenario < sizeof(scenario_descriptions) / sizeof(scenario_descriptions[0])) {
        return scenario_descriptions[scenario];
    }
    return "Unknown scenario";
}

int simulation_get_stats(const SimulationState *state, char *buffer, size_t buffer_size) {
    if (!state || !buffer || buffer_size == 0) return 0;
    
    int processes, resources, requests, assignments;
    rag_get_stats(&state->rag, &processes, &resources, &requests, &assignments);
    
    return snprintf(buffer, buffer_size,
        "Scenario: %s\n"
        "Tick: %d\n"
        "Running: %s\n"
        "Paused: %s\n"
        "Deadlock: %s\n"
        "Processes: %d\n"
        "Resources: %d\n"
        "Requests: %d\n"
        "Assignments: %d\n"
        "Events: %d\n",
        simulation_scenario_name(state->scenario),
        state->current_tick,
        state->running ? "Yes" : "No",
        state->paused ? "Yes" : "No",
        state->deadlock_occurred ? "Yes" : "No",
        processes, resources, requests, assignments,
        state->event_count);
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

void simulation_print_state(const SimulationState *state) {
    if (!state) return;
    
    char stats[1024];
    simulation_get_stats(state, stats, sizeof(stats));
    
    printf("\n========== Simulation State ==========\n");
    printf("%s", stats);
    printf("======================================\n");
    
    rag_print(&state->rag);
}

void simulation_print_events(const SimulationState *state) {
    if (!state) return;
    
    printf("\n========== Event Log ==========\n");
    printf("Total events: %d\n\n", state->event_count);
    
    for (int i = 0; i < state->event_count; i++) {
        const SimulationEvent *e = &state->events[i];
        printf("[T%03d] %-20s ", e->timestamp, event_type_names[e->type]);
        if (e->process_id >= 0) printf("P%d ", e->process_id);
        if (e->resource_id >= 0) printf("R%d ", e->resource_id);
        if (e->description[0]) printf("- %s", e->description);
        printf("\n");
    }
    
    printf("===============================\n\n");
}

int simulation_export(const SimulationState *state, char *buffer, size_t buffer_size) {
    if (!state || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    
    /* Export basic state */
    written += snprintf(buffer + written, buffer_size - written,
        "{\n"
        "  \"scenario\": \"%s\",\n"
        "  \"tick\": %d,\n"
        "  \"running\": %s,\n"
        "  \"deadlock\": %s,\n"
        "  \"processes\": %d,\n"
        "  \"resources\": %d,\n"
        "  \"events\": %d\n"
        "}\n",
        simulation_scenario_name(state->scenario),
        state->current_tick,
        state->running ? "true" : "false",
        state->deadlock_occurred ? "true" : "false",
        state->rag.process_count,
        state->rag.resource_count,
        state->event_count);
    
    return written;
}
