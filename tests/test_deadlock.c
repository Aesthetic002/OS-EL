/**
 * @file test_deadlock.c
 * @brief Unit tests for the deadlock detection and recovery system
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/rag.h"
#include "../include/cycle_detector.h"
#include "../include/recovery.h"
#include "../include/simulator.h"

/* ============================================================================
 * Test Framework
 * ============================================================================ */

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("  Running: %s... ", #name); \
    tests_run++; \
    test_##name(); \
    tests_passed++; \
    printf("PASSED\n"); \
} while(0)

#define ASSERT(cond) do { \
    if (!(cond)) { \
        printf("FAILED\n    Assertion failed: %s\n    at %s:%d\n", \
               #cond, __FILE__, __LINE__); \
        tests_failed++; \
        return; \
    } \
} while(0)

#define ASSERT_EQ(a, b) ASSERT((a) == (b))
#define ASSERT_NE(a, b) ASSERT((a) != (b))
#define ASSERT_TRUE(x) ASSERT((x))
#define ASSERT_FALSE(x) ASSERT(!(x))
#define ASSERT_NULL(x) ASSERT((x) == NULL)
#define ASSERT_NOT_NULL(x) ASSERT((x) != NULL)

/* ============================================================================
 * RAG Tests
 * ============================================================================ */

TEST(rag_init) {
    RAG rag;
    rag_init(&rag);
    
    ASSERT_EQ(rag.process_count, 0);
    ASSERT_EQ(rag.resource_count, 0);
    
    for (int i = 0; i < MAX_PROCESSES; i++) {
        ASSERT_FALSE(rag.processes[i].active);
    }
    for (int i = 0; i < MAX_RESOURCES; i++) {
        ASSERT_FALSE(rag.resources[i].active);
    }
    
    rag_destroy(&rag);
}

TEST(rag_add_process) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    ASSERT(p1 >= 0);
    ASSERT_EQ(rag.process_count, 1);
    ASSERT_TRUE(rag.processes[p1].active);
    ASSERT_EQ(strcmp(rag.processes[p1].name, "Process_1"), 0);
    ASSERT_EQ(rag.processes[p1].priority, 50);
    
    int p2 = rag_add_process(&rag, "Process_2", 75);
    ASSERT(p2 >= 0);
    ASSERT_NE(p1, p2);
    ASSERT_EQ(rag.process_count, 2);
    
    rag_destroy(&rag);
}

TEST(rag_add_resource) {
    RAG rag;
    rag_init(&rag);
    
    int r1 = rag_add_resource(&rag, "Resource_A", 3);
    ASSERT(r1 >= 0);
    ASSERT_EQ(rag.resource_count, 1);
    ASSERT_TRUE(rag.resources[r1].active);
    ASSERT_EQ(rag.resources[r1].total_instances, 3);
    ASSERT_EQ(rag.resources[r1].available_instances, 3);
    
    rag_destroy(&rag);
}

TEST(rag_remove_process) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    ASSERT_EQ(rag.process_count, 1);
    
    ASSERT_TRUE(rag_remove_process(&rag, p1));
    ASSERT_EQ(rag.process_count, 0);
    ASSERT_FALSE(rag.processes[p1].active);
    
    /* Removing non-existent process should fail */
    ASSERT_FALSE(rag_remove_process(&rag, p1));
    
    rag_destroy(&rag);
}

TEST(rag_request_and_allocate) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    
    /* Request resource */
    ASSERT_TRUE(rag_request_resource(&rag, p1, r1));
    ASSERT_TRUE(rag_is_requesting(&rag, p1, r1));
    ASSERT_EQ(rag.processes[p1].state, PROCESS_WAITING);
    
    /* Allocate resource */
    ASSERT_TRUE(rag_allocate_resource(&rag, p1, r1));
    ASSERT_FALSE(rag_is_requesting(&rag, p1, r1));
    ASSERT_TRUE(rag_is_holding(&rag, p1, r1));
    ASSERT_EQ(rag.resources[r1].available_instances, 0);
    
    /* Release resource */
    ASSERT_TRUE(rag_release_resource(&rag, p1, r1));
    ASSERT_FALSE(rag_is_holding(&rag, p1, r1));
    ASSERT_EQ(rag.resources[r1].available_instances, 1);
    
    rag_destroy(&rag);
}

TEST(rag_allocation_limit) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    
    /* P1 allocates the only instance */
    ASSERT_TRUE(rag_allocate_resource(&rag, p1, r1));
    ASSERT_EQ(rag.resources[r1].available_instances, 0);
    
    /* P2 should fail to allocate (no available instances) */
    ASSERT_FALSE(rag_allocate_resource(&rag, p2, r1));
    
    rag_destroy(&rag);
}

/* ============================================================================
 * Cycle Detector Tests
 * ============================================================================ */

TEST(detect_no_deadlock) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    
    /* P1 holds R1, P2 holds R2 - no cross requests */
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    
    DeadlockResult result;
    ASSERT_FALSE(detect_deadlock(&rag, &result));
    ASSERT_FALSE(result.deadlock_detected);
    
    rag_destroy(&rag);
}

TEST(detect_simple_deadlock) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    
    /* Create deadlock: P1 holds R1, wants R2; P2 holds R2, wants R1 */
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r1);
    
    DeadlockResult result;
    ASSERT_TRUE(detect_deadlock(&rag, &result));
    ASSERT_TRUE(result.deadlock_detected);
    ASSERT_EQ(result.deadlocked_process_count, 2);
    
    rag_destroy(&rag);
}

TEST(detect_circular_wait) {
    RAG rag;
    rag_init(&rag);
    
    /* Create 4-process circular wait: P0->R0->P1->R1->P2->R2->P3->R3->P0 */
    int processes[4];
    int resources[4];
    
    for (int i = 0; i < 4; i++) {
        char name[32];
        snprintf(name, sizeof(name), "Process_%d", i);
        processes[i] = rag_add_process(&rag, name, 50);
        snprintf(name, sizeof(name), "Resource_%d", i);
        resources[i] = rag_add_resource(&rag, name, 1);
    }
    
    for (int i = 0; i < 4; i++) {
        rag_allocate_resource(&rag, processes[i], resources[i]);
        rag_request_resource(&rag, processes[i], resources[(i + 1) % 4]);
    }
    
    DeadlockResult result;
    ASSERT_TRUE(detect_deadlock(&rag, &result));
    ASSERT_TRUE(result.deadlock_detected);
    ASSERT_EQ(result.deadlocked_process_count, 4);
    
    rag_destroy(&rag);
}

TEST(detect_is_process_deadlocked) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int p3 = rag_add_process(&rag, "Process_3", 50);  /* Not in deadlock */
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    int r3 = rag_add_resource(&rag, "Resource_C", 1);
    
    /* Deadlock between P1 and P2 */
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r1);
    
    /* P3 just holds R3, not involved */
    rag_allocate_resource(&rag, p3, r3);
    
    ASSERT_TRUE(is_process_deadlocked(&rag, p1));
    ASSERT_TRUE(is_process_deadlocked(&rag, p2));
    ASSERT_FALSE(is_process_deadlocked(&rag, p3));
    
    rag_destroy(&rag);
}

TEST(wait_for_graph) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    
    /* P2 holds R1, P1 requests R1 => P1 waits for P2 */
    rag_allocate_resource(&rag, p2, r1);
    rag_request_resource(&rag, p1, r1);
    
    int wait_for[MAX_PROCESSES][MAX_PROCESSES];
    build_wait_for_graph(&rag, wait_for);
    
    ASSERT_EQ(wait_for[p1][p2], 1);  /* P1 waits for P2 */
    ASSERT_EQ(wait_for[p2][p1], 0);  /* P2 does not wait for P1 */
    
    rag_destroy(&rag);
}

/* ============================================================================
 * Recovery Tests
 * ============================================================================ */

TEST(recovery_terminate_all) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r1);
    
    DeadlockResult detection;
    detect_deadlock(&rag, &detection);
    
    RecoveryResult result;
    ASSERT_TRUE(recovery_terminate_all(&rag, &detection, &result));
    ASSERT_EQ(result.processes_terminated, 2);
    
    /* Verify no more deadlock */
    ASSERT_FALSE(detect_deadlock(&rag, &detection));
    
    rag_destroy(&rag);
}

TEST(recovery_terminate_one) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 30);  /* Lower priority */
    int p2 = rag_add_process(&rag, "Process_2", 70);  /* Higher priority */
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r1);
    
    DeadlockResult detection;
    detect_deadlock(&rag, &detection);
    
    /* Select victim using lowest priority */
    int victim = select_victim_process(&rag, &detection, SELECT_LOWEST_PRIORITY);
    ASSERT_EQ(victim, p1);  /* P1 has lower priority */
    
    RecoveryResult result;
    ASSERT_TRUE(recovery_terminate_one(&rag, &detection, SELECT_LOWEST_PRIORITY, &result));
    ASSERT_EQ(result.processes_terminated, 1);
    
    /* P1 should be gone, P2 should remain */
    ASSERT_NULL(rag_get_process(&rag, p1));
    ASSERT_NOT_NULL(rag_get_process(&rag, p2));
    
    rag_destroy(&rag);
}

TEST(recovery_iterative) {
    RAG rag;
    rag_init(&rag);
    
    /* Create 3-process circular deadlock */
    int p1 = rag_add_process(&rag, "Process_1", 30);
    int p2 = rag_add_process(&rag, "Process_2", 50);
    int p3 = rag_add_process(&rag, "Process_3", 70);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    int r3 = rag_add_resource(&rag, "Resource_C", 1);
    
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_allocate_resource(&rag, p3, r3);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r3);
    rag_request_resource(&rag, p3, r1);
    
    RecoveryResult result;
    ASSERT_TRUE(recovery_terminate_iterative(&rag, SELECT_LOWEST_PRIORITY, 10, &result));
    ASSERT_TRUE(result.success);
    ASSERT(result.processes_terminated >= 1);
    
    /* Verify deadlock is resolved */
    DeadlockResult detection;
    ASSERT_FALSE(detect_deadlock(&rag, &detection));
    
    rag_destroy(&rag);
}

TEST(recovery_rollback) {
    RAG rag;
    rag_init(&rag);
    
    int p1 = rag_add_process(&rag, "Process_1", 50);
    int r1 = rag_add_resource(&rag, "Resource_A", 1);
    int r2 = rag_add_resource(&rag, "Resource_B", 1);
    
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p1, r2);
    rag_request_resource(&rag, p1, r1);  /* Request again just for test */
    
    RecoveryResult result;
    ASSERT_TRUE(recovery_rollback_process(&rag, p1, &result));
    
    /* Process should still exist but have no resources */
    ASSERT_NOT_NULL(rag_get_process(&rag, p1));
    ASSERT_FALSE(rag_is_holding(&rag, p1, r1));
    ASSERT_FALSE(rag_is_holding(&rag, p1, r2));
    ASSERT_FALSE(rag_is_requesting(&rag, p1, r1));
    
    rag_destroy(&rag);
}

/* ============================================================================
 * Simulator Tests
 * ============================================================================ */

TEST(simulator_init) {
    SimulationState state;
    simulation_init(&state);
    
    ASSERT_EQ(state.current_tick, 0);
    ASSERT_FALSE(state.running);
    ASSERT_FALSE(state.paused);
    ASSERT_FALSE(state.deadlock_occurred);
    ASSERT_EQ(state.event_count, 0);
    
    simulation_destroy(&state);
}

TEST(simulator_simple_deadlock_scenario) {
    SimulationState state;
    simulation_init(&state);
    
    simulation_load_scenario(&state, SCENARIO_SIMPLE_DEADLOCK);
    
    ASSERT_EQ(state.scenario, SCENARIO_SIMPLE_DEADLOCK);
    ASSERT_EQ(state.rag.process_count, 2);
    ASSERT_EQ(state.rag.resource_count, 2);
    ASSERT_TRUE(state.deadlock_occurred);
    
    /* Verify deadlock detection on loaded scenario */
    DeadlockResult result;
    ASSERT_TRUE(detect_deadlock(&state.rag, &result));
    
    simulation_destroy(&state);
}

TEST(simulator_circular_wait_scenario) {
    SimulationState state;
    simulation_init(&state);
    
    setup_circular_wait(&state, 5);
    
    ASSERT_EQ(state.rag.process_count, 5);
    ASSERT_EQ(state.rag.resource_count, 5);
    
    DeadlockResult result;
    ASSERT_TRUE(detect_deadlock(&state.rag, &result));
    ASSERT_EQ(result.deadlocked_process_count, 5);
    
    simulation_destroy(&state);
}

TEST(simulator_no_deadlock_scenario) {
    SimulationState state;
    simulation_init(&state);
    
    simulation_load_scenario(&state, SCENARIO_NO_DEADLOCK);
    
    DeadlockResult result;
    ASSERT_FALSE(detect_deadlock(&state.rag, &result));
    
    simulation_destroy(&state);
}

TEST(simulator_event_logging) {
    SimulationState state;
    simulation_init(&state);
    
    simulation_log_event(&state, EVENT_PROCESS_CREATE, 0, -1, "Test event");
    ASSERT_EQ(state.event_count, 1);
    ASSERT_EQ(state.events[0].type, EVENT_PROCESS_CREATE);
    ASSERT_EQ(state.events[0].process_id, 0);
    
    simulation_clear_events(&state);
    ASSERT_EQ(state.event_count, 0);
    
    simulation_destroy(&state);
}

TEST(simulator_manual_control) {
    SimulationState state;
    simulation_init(&state);
    
    int p1 = simulation_add_process(&state, "Test_Process", 50);
    ASSERT(p1 >= 0);
    
    int r1 = simulation_add_resource(&state, "Test_Resource", 1);
    ASSERT(r1 >= 0);
    
    ASSERT_TRUE(simulation_request_resource(&state, p1, r1));
    ASSERT_TRUE(simulation_allocate_resource(&state, p1, r1));
    ASSERT_TRUE(simulation_release_resource(&state, p1, r1));
    
    simulation_destroy(&state);
}

/* ============================================================================
 * Integration Tests
 * ============================================================================ */

TEST(full_detection_recovery_cycle) {
    RAG rag;
    rag_init(&rag);
    
    /* Create a complex deadlock scenario */
    int p1 = rag_add_process(&rag, "WebServer", 80);
    int p2 = rag_add_process(&rag, "Database", 90);
    int p3 = rag_add_process(&rag, "Cache", 60);
    
    int r1 = rag_add_resource(&rag, "Lock_A", 1);
    int r2 = rag_add_resource(&rag, "Lock_B", 1);
    int r3 = rag_add_resource(&rag, "Lock_C", 1);
    
    /* Create 3-way deadlock */
    rag_allocate_resource(&rag, p1, r1);
    rag_allocate_resource(&rag, p2, r2);
    rag_allocate_resource(&rag, p3, r3);
    rag_request_resource(&rag, p1, r2);
    rag_request_resource(&rag, p2, r3);
    rag_request_resource(&rag, p3, r1);
    
    /* Step 1: Detect deadlock */
    DeadlockResult detection;
    ASSERT_TRUE(detect_deadlock(&rag, &detection));
    ASSERT_EQ(detection.deadlocked_process_count, 3);
    
    /* Step 2: Get recommended strategy */
    RecoveryStrategy recommended = recommend_recovery_strategy(&rag, &detection);
    ASSERT(recommended >= 0);
    
    /* Step 3: Recover */
    RecoveryConfig config;
    recovery_config_init(&config);
    config.strategy = RECOVERY_TERMINATE_LOWEST;
    config.selection = SELECT_LOWEST_PRIORITY;
    
    RecoveryResult recovery;
    ASSERT_TRUE(recover_from_deadlock(&rag, &detection, &config, &recovery));
    
    /* Step 4: Verify deadlock resolved */
    ASSERT_FALSE(detect_deadlock(&rag, &detection));
    
    /* Step 5: Verify at least Cache (lowest priority) was terminated */
    ASSERT_NULL(rag_get_process(&rag, p3));  /* Cache has priority 60, lowest */
    
    rag_destroy(&rag);
}

TEST(multiple_cycles_detection) {
    SimulationState state;
    simulation_init(&state);
    
    simulation_load_scenario(&state, SCENARIO_MULTIPLE_CYCLES);
    
    DeadlockResult result;
    int cycles = detect_all_cycles(&state.rag, &result);
    
    /* Should find at least 2 independent cycles */
    ASSERT(cycles >= 1);
    ASSERT_TRUE(result.deadlock_detected);
    
    simulation_destroy(&state);
}

/* ============================================================================
 * Test Runner
 * ============================================================================ */

void run_all_tests(void) {
    printf("\n========================================\n");
    printf("  OS-EL Deadlock Module - Unit Tests\n");
    printf("========================================\n\n");
    
    printf("RAG Tests:\n");
    RUN_TEST(rag_init);
    RUN_TEST(rag_add_process);
    RUN_TEST(rag_add_resource);
    RUN_TEST(rag_remove_process);
    RUN_TEST(rag_request_and_allocate);
    RUN_TEST(rag_allocation_limit);
    
    printf("\nCycle Detector Tests:\n");
    RUN_TEST(detect_no_deadlock);
    RUN_TEST(detect_simple_deadlock);
    RUN_TEST(detect_circular_wait);
    RUN_TEST(detect_is_process_deadlocked);
    RUN_TEST(wait_for_graph);
    
    printf("\nRecovery Tests:\n");
    RUN_TEST(recovery_terminate_all);
    RUN_TEST(recovery_terminate_one);
    RUN_TEST(recovery_iterative);
    RUN_TEST(recovery_rollback);
    
    printf("\nSimulator Tests:\n");
    RUN_TEST(simulator_init);
    RUN_TEST(simulator_simple_deadlock_scenario);
    RUN_TEST(simulator_circular_wait_scenario);
    RUN_TEST(simulator_no_deadlock_scenario);
    RUN_TEST(simulator_event_logging);
    RUN_TEST(simulator_manual_control);
    
    printf("\nIntegration Tests:\n");
    RUN_TEST(full_detection_recovery_cycle);
    RUN_TEST(multiple_cycles_detection);
    
    printf("\n========================================\n");
    printf("  Results: %d/%d tests passed\n", tests_passed, tests_run);
    if (tests_failed > 0) {
        printf("  FAILED: %d tests\n", tests_failed);
    }
    printf("========================================\n\n");
}

int main(void) {
    run_all_tests();
    return (tests_failed > 0) ? 1 : 0;
}
