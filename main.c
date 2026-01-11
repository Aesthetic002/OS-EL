#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "include/rag.h"
#include "include/cycle_detector.h"
#include "include/recovery.h"
#include "include/simulator.h"
#include "include/api.h"

static RAG g_rag;
static SimulationState g_simulation;
static bool g_running = true;

static void print_header(void) {
    printf("\nOS-EL: Deadlock Detection & Recovery Module\n");
}

static void print_menu(void) {
    printf("\n--- Menu ---\n\n");
    printf("Process:   1.Add      2.Remove     3.List\n");
    printf("Resource:  4.Add      5.Remove     6.List\n");
    printf("Edges:     7.Request  8.Allocate   9.Release\n");
    printf("Deadlock:  10.Detect  11.Recover   12.ShowRAG\n");
    printf("Demo:      13.Simple  14.Circular  15.Philosophers\n");
    printf("Other:     16.API     17.Reset     0.Exit\n\n");
    printf("Choice: ");
}

static int read_int(const char *prompt) {
    printf("%s", prompt);
    char buffer[64];
    if (!fgets(buffer, sizeof(buffer), stdin)) return -1;
    return atoi(buffer);
}

static void read_string(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);
    if (!fgets(buffer, size, stdin)) {
        buffer[0] = '\0';
        return;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
}

static void press_enter(void) {
    printf("\nPress Enter...");
    getchar();
}

static void action_add_process(void) {
    printf("\n");
    char name[MAX_NAME_LENGTH];
    read_string("Process name: ", name, sizeof(name));
    int priority = read_int("Priority (0-100): ");
    int pid = rag_add_process(&g_rag, name, priority);
    if (pid >= 0) {
        printf("\nAdded process '%s' as P%d\n", name, pid);
    } else {
        printf("\nFailed to add process\n");
    }
}

static void action_remove_process(void) {
    printf("\n");
    int pid = read_int("Process ID: ");
    if (rag_remove_process(&g_rag, pid)) {
        printf("Removed P%d\n", pid);
    } else {
        printf("Failed to remove process\n");
    }
}

static void action_list_processes(void) {
    printf("\n%-5s %-20s %-10s %-12s\n", "ID", "Name", "Priority", "State");
    bool found = false;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (g_rag.processes[i].active) {
            const Process *p = &g_rag.processes[i];
            const char *state;
            switch (p->state) {
                case PROCESS_RUNNING:    state = "RUNNING"; break;
                case PROCESS_WAITING:    state = "WAITING"; break;
                case PROCESS_BLOCKED:    state = "BLOCKED"; break;
                case PROCESS_TERMINATED: state = "TERMINATED"; break;
                default: state = "UNKNOWN";
            }
            printf("P%-4d %-20s %-10d %-12s\n", p->id, p->name, p->priority, state);
            found = true;
        }
    }
    if (!found) printf("(no processes)\n");
}

static void action_add_resource(void) {
    printf("\n");
    char name[MAX_NAME_LENGTH];
    read_string("Resource name: ", name, sizeof(name));
    int instances = read_int("Instances: ");
    int rid = rag_add_resource(&g_rag, name, instances);
    if (rid >= 0) {
        printf("\nAdded resource '%s' as R%d (%d instances)\n", name, rid, instances);
    } else {
        printf("\nFailed to add resource\n");
    }
}

static void action_remove_resource(void) {
    printf("\n");
    int rid = read_int("Resource ID: ");
    if (rag_remove_resource(&g_rag, rid)) {
        printf("Removed R%d\n", rid);
    } else {
        printf("Failed to remove resource\n");
    }
}

static void action_list_resources(void) {
    printf("\n%-5s %-20s %-10s %-12s\n", "ID", "Name", "Total", "Available");
    bool found = false;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (g_rag.resources[i].active) {
            const Resource *r = &g_rag.resources[i];
            printf("R%-4d %-20s %-10d %-12d\n", r->id, r->name, r->total_instances, r->available_instances);
            found = true;
        }
    }
    if (!found) printf("(no resources)\n");
}

static void action_request_resource(void) {
    printf("\n");
    int pid = read_int("Process ID: ");
    int rid = read_int("Resource ID: ");
    if (rag_request_resource(&g_rag, pid, rid)) {
        printf("P%d requesting R%d\n", pid, rid);
    } else {
        printf("Failed to create request\n");
    }
}

static void action_allocate_resource(void) {
    printf("\n");
    int pid = read_int("Process ID: ");
    int rid = read_int("Resource ID: ");
    if (rag_allocate_resource(&g_rag, pid, rid)) {
        printf("R%d allocated to P%d\n", rid, pid);
    } else {
        printf("Failed to allocate\n");
    }
}

static void action_release_resource(void) {
    printf("\n");
    int pid = read_int("Process ID: ");
    int rid = read_int("Resource ID: ");
    if (rag_release_resource(&g_rag, pid, rid)) {
        printf("P%d released R%d\n", pid, rid);
    } else {
        printf("Failed to release\n");
    }
}

static void action_detect_deadlock(void) {
    DeadlockResult result;
    bool deadlock = detect_deadlock(&g_rag, &result);
    if (deadlock) {
        printf("\nDEADLOCK DETECTED!\n");
        print_deadlock_result(&g_rag, &result);
    } else {
        printf("\nNo deadlock. System is safe.\n");
    }
}

static void action_recover(void) {
    DeadlockResult detection;
    if (!detect_deadlock(&g_rag, &detection)) {
        printf("\nNo deadlock to recover from.\n");
        return;
    }
    
    printf("\nRecovery strategy:\n");
    printf("  1. Terminate All\n");
    printf("  2. Terminate Lowest Priority\n");
    printf("  3. Terminate Fewest Resources\n");
    printf("  4. Iterative Termination\n");
    printf("  5. Preempt Resources\n");
    printf("  6. Rollback\n\n");
    int choice = read_int("Choice: ");
    
    RecoveryConfig config;
    recovery_config_init(&config);
    RecoveryResult result;
    
    switch (choice) {
        case 1: config.strategy = RECOVERY_TERMINATE_ALL; break;
        case 2: config.strategy = RECOVERY_TERMINATE_LOWEST; config.selection = SELECT_LOWEST_PRIORITY; break;
        case 3: config.strategy = RECOVERY_TERMINATE_ONE; config.selection = SELECT_FEWEST_RESOURCES; break;
        case 4:
            recovery_terminate_iterative(&g_rag, SELECT_LOWEST_PRIORITY, 10, &result);
            print_recovery_result(&result);
            return;
        case 5: config.strategy = RECOVERY_PREEMPT_RESOURCES; break;
        case 6: config.strategy = RECOVERY_ROLLBACK; break;
        default: printf("Invalid choice\n"); return;
    }
    
    if (recover_from_deadlock(&g_rag, &detection, &config, &result)) {
        printf("Recovery successful\n");
    } else {
        printf("Recovery failed\n");
    }
    print_recovery_result(&result);
}

static void action_show_rag(void) {
    rag_print(&g_rag);
}

static void action_reset_rag(void) {
    rag_reset(&g_rag);
    printf("\nRAG reset.\n");
}

static void run_demo(SimulationScenario scenario, const char *title) {
    printf("\n--- %s ---\n", title);
    simulation_load_scenario(&g_simulation, scenario);
    rag_copy(&g_rag, &g_simulation.rag);
    
    printf("\nInitial State:\n");
    rag_print(&g_rag);
    
    DeadlockResult detection;
    if (detect_deadlock(&g_rag, &detection)) {
        printf("\nDEADLOCK DETECTED!\n");
        print_deadlock_result(&g_rag, &detection);
        
        printf("\nRecovering...\n");
        RecoveryConfig config;
        recovery_config_init(&config);
        config.strategy = RECOVERY_TERMINATE_LOWEST;
        
        RecoveryResult recovery;
        if (recover_from_deadlock(&g_rag, &detection, &config, &recovery)) {
            printf("Recovery successful\n");
        }
        print_recovery_result(&recovery);
        
        printf("\nFinal State:\n");
        rag_print(&g_rag);
        
        if (!detect_deadlock(&g_rag, &detection)) {
            printf("\nDeadlock resolved.\n");
        }
    } else {
        printf("\nNo deadlock.\n");
    }
}

static void action_simple_deadlock_demo(void) {
    run_demo(SCENARIO_SIMPLE_DEADLOCK, "Simple Two-Process Deadlock");
}

static void action_circular_wait_demo(void) {
    printf("\n");
    int n = read_int("Number of processes (2-10): ");
    if (n < 2) n = 2;
    if (n > 10) n = 10;
    
    simulation_reset(&g_simulation);
    setup_circular_wait(&g_simulation, n);
    rag_copy(&g_rag, &g_simulation.rag);
    
    printf("\n--- Circular Wait (%d processes) ---\n", n);
    rag_print(&g_rag);
    
    DeadlockResult detection;
    if (detect_deadlock(&g_rag, &detection)) {
        printf("\nDEADLOCK DETECTED!\n");
        print_deadlock_result(&g_rag, &detection);
    }
}

static void action_dining_philosophers_demo(void) {
    printf("\n");
    int n = read_int("Number of philosophers (2-10): ");
    if (n < 2) n = 2;
    if (n > 10) n = 10;
    
    simulation_reset(&g_simulation);
    setup_dining_philosophers(&g_simulation, n);
    rag_copy(&g_rag, &g_simulation.rag);
    
    printf("\n--- Dining Philosophers (%d) ---\n", n);
    rag_print(&g_rag);
    
    DeadlockResult detection;
    if (detect_deadlock(&g_rag, &detection)) {
        printf("\nDEADLOCK DETECTED!\n");
        print_deadlock_result(&g_rag, &detection);
    }
}

static void action_random_scenario_demo(void) {
    printf("\n");
    int np = read_int("Number of processes (2-20): ");
    int nr = read_int("Number of resources (2-20): ");
    int seed = read_int("Random seed (0 for time-based): ");
    
    if (np < 2) np = 2;
    if (np > 20) np = 20;
    if (nr < 2) nr = 2;
    if (nr > 20) nr = 20;
    
    simulation_reset(&g_simulation);
    setup_random_scenario(&g_simulation, np, nr, seed);
    rag_copy(&g_rag, &g_simulation.rag);
    
    printf("\n--- Random Scenario ---\n");
    rag_print(&g_rag);
    
    DeadlockResult detection;
    if (detect_deadlock(&g_rag, &detection)) {
        printf("\nDEADLOCK DETECTED!\n");
        print_deadlock_result(&g_rag, &detection);
    } else {
        printf("\nNo deadlock.\n");
    }
}

static void action_start_api_server(void) {
    printf("\nStarting API server...\n");
    printf("Send JSON via stdin. Use {\"command\":\"shutdown\"} to exit.\n\n");
    
    APIContext ctx;
    api_init(&ctx);
    api_run_server(&ctx);
    api_destroy(&ctx);
    
    printf("\nAPI server stopped.\n");
}

int main(int argc, char *argv[]) {
    if (argc > 1 && (strcmp(argv[1], "--api") == 0 || strcmp(argv[1], "-a") == 0)) {
        APIContext ctx;
        api_init(&ctx);
        int result = api_run_server(&ctx);
        api_destroy(&ctx);
        return result;
    }
    
    rag_init(&g_rag);
    simulation_init(&g_simulation);
    print_header();
    
    while (g_running) {
        print_menu();
        char buffer[64];
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        int choice = atoi(buffer);
        
        switch (choice) {
            case 0: g_running = false; break;
            case 1: action_add_process(); break;
            case 2: action_remove_process(); break;
            case 3: action_list_processes(); break;
            case 4: action_add_resource(); break;
            case 5: action_remove_resource(); break;
            case 6: action_list_resources(); break;
            case 7: action_request_resource(); break;
            case 8: action_allocate_resource(); break;
            case 9: action_release_resource(); break;
            case 10: action_detect_deadlock(); break;
            case 11: action_recover(); break;
            case 12: action_show_rag(); break;
            case 13: action_simple_deadlock_demo(); press_enter(); break;
            case 14: action_circular_wait_demo(); press_enter(); break;
            case 15: action_dining_philosophers_demo(); press_enter(); break;
            case 16: action_random_scenario_demo(); press_enter(); break;
            case 17: action_start_api_server(); break;
            case 18: action_reset_rag(); break;
            default: printf("Invalid choice\n");
        }
    }
    
    rag_destroy(&g_rag);
    simulation_destroy(&g_simulation);
    printf("\nGoodbye!\n");
    return 0;
}
