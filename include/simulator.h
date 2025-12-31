/**
 * @file simulator.h
 * @brief Deadlock simulation scenarios
 * 
 * This module provides predefined and custom deadlock simulation scenarios
 * for testing and educational purposes.
 */

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "rag.h"
#include "cycle_detector.h"
#include "recovery.h"
#include <stdbool.h>

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief Predefined simulation scenarios
 */
typedef enum {
    SCENARIO_SIMPLE_DEADLOCK,       /**< Two processes, two resources */
    SCENARIO_CIRCULAR_WAIT,         /**< Classic circular wait (N processes) */
    SCENARIO_DINING_PHILOSOPHERS,   /**< Dining philosophers problem */
    SCENARIO_PRODUCER_CONSUMER,     /**< Producer-consumer with shared buffers */
    SCENARIO_READER_WRITER,         /**< Reader-writer problem */
    SCENARIO_BANKER_SAFE,           /**< Banker's algorithm - safe state */
    SCENARIO_BANKER_UNSAFE,         /**< Banker's algorithm - unsafe state */
    SCENARIO_NO_DEADLOCK,           /**< Scenario without deadlock */
    SCENARIO_MULTIPLE_CYCLES,       /**< Multiple independent cycles */
    SCENARIO_COMPLEX_DEPENDENCY,    /**< Complex resource dependencies */
    SCENARIO_RANDOM,                /**< Random scenario generation */
    SCENARIO_CUSTOM                 /**< User-defined scenario */
} SimulationScenario;

/**
 * @brief Simulation event type
 */
typedef enum {
    EVENT_PROCESS_CREATE,           /**< New process created */
    EVENT_PROCESS_TERMINATE,        /**< Process terminated */
    EVENT_RESOURCE_REQUEST,         /**< Resource requested */
    EVENT_RESOURCE_ALLOCATE,        /**< Resource allocated */
    EVENT_RESOURCE_RELEASE,         /**< Resource released */
    EVENT_DEADLOCK_DETECTED,        /**< Deadlock detected */
    EVENT_RECOVERY_STARTED,         /**< Recovery initiated */
    EVENT_RECOVERY_COMPLETED,       /**< Recovery completed */
    EVENT_TICK                      /**< Simulation time tick */
} SimulationEventType;

/**
 * @brief Simulation event record
 */
typedef struct {
    int timestamp;                  /**< Simulation timestamp */
    SimulationEventType type;       /**< Event type */
    int process_id;                 /**< Related process ID (-1 if N/A) */
    int resource_id;                /**< Related resource ID (-1 if N/A) */
    char description[256];          /**< Event description */
} SimulationEvent;

/**
 * @brief Simulation state
 */
typedef struct {
    RAG rag;                        /**< Current RAG state */
    SimulationScenario scenario;    /**< Current scenario */
    int current_tick;               /**< Current simulation tick */
    bool running;                   /**< Whether simulation is running */
    bool paused;                    /**< Whether simulation is paused */
    bool deadlock_occurred;         /**< Whether deadlock has occurred */
    SimulationEvent events[1024];   /**< Event log */
    int event_count;                /**< Number of events logged */
    DeadlockResult last_detection;  /**< Last deadlock detection result */
    RecoveryResult last_recovery;   /**< Last recovery result */
} SimulationState;

/**
 * @brief Simulation configuration
 */
typedef struct {
    SimulationScenario scenario;    /**< Scenario to run */
    int num_processes;              /**< Number of processes (for random) */
    int num_resources;              /**< Number of resources (for random) */
    bool auto_detect;               /**< Auto-detect deadlock each tick */
    bool auto_recover;              /**< Auto-recover from deadlock */
    RecoveryStrategy recovery_strategy; /**< Recovery strategy to use */
    int random_seed;                /**< Random seed (0 = random) */
    int max_ticks;                  /**< Maximum simulation ticks (0 = unlimited) */
    bool verbose;                   /**< Verbose output */
} SimulationConfig;

/**
 * @brief Callback for simulation events
 */
typedef void (*SimulationCallback)(const SimulationEvent *event, void *user_data);

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

/**
 * @brief Initialize simulation state
 * @param state Pointer to simulation state
 */
void simulation_init(SimulationState *state);

/**
 * @brief Initialize simulation configuration with defaults
 * @param config Pointer to configuration
 */
void simulation_config_init(SimulationConfig *config);

/**
 * @brief Reset simulation to initial state
 * @param state Pointer to simulation state
 */
void simulation_reset(SimulationState *state);

/**
 * @brief Destroy simulation state and free resources
 * @param state Pointer to simulation state
 */
void simulation_destroy(SimulationState *state);

/* ============================================================================
 * Scenario Setup Functions
 * ============================================================================ */

/**
 * @brief Load a predefined scenario
 * @param state Pointer to simulation state
 * @param scenario Scenario to load
 * @return true if successful
 */
bool simulation_load_scenario(SimulationState *state, SimulationScenario scenario);

/**
 * @brief Load scenario with configuration
 * @param state Pointer to simulation state
 * @param config Configuration
 * @return true if successful
 */
bool simulation_load_with_config(SimulationState *state, const SimulationConfig *config);

/**
 * @brief Setup simple two-process deadlock
 * @param state Pointer to simulation state
 */
void setup_simple_deadlock(SimulationState *state);

/**
 * @brief Setup circular wait with N processes
 * @param state Pointer to simulation state
 * @param n Number of processes
 */
void setup_circular_wait(SimulationState *state, int n);

/**
 * @brief Setup dining philosophers problem
 * @param state Pointer to simulation state
 * @param n Number of philosophers
 */
void setup_dining_philosophers(SimulationState *state, int n);

/**
 * @brief Setup random scenario
 * @param state Pointer to simulation state
 * @param num_processes Number of processes
 * @param num_resources Number of resources
 * @param seed Random seed
 */
void setup_random_scenario(SimulationState *state, int num_processes, 
                           int num_resources, int seed);

/* ============================================================================
 * Simulation Control Functions
 * ============================================================================ */

/**
 * @brief Start the simulation
 * @param state Pointer to simulation state
 * @return true if started successfully
 */
bool simulation_start(SimulationState *state);

/**
 * @brief Pause the simulation
 * @param state Pointer to simulation state
 */
void simulation_pause(SimulationState *state);

/**
 * @brief Resume paused simulation
 * @param state Pointer to simulation state
 */
void simulation_resume(SimulationState *state);

/**
 * @brief Stop the simulation
 * @param state Pointer to simulation state
 */
void simulation_stop(SimulationState *state);

/**
 * @brief Advance simulation by one tick
 * @param state Pointer to simulation state
 * @param config Configuration for this tick
 * @return true if tick executed, false if simulation ended
 */
bool simulation_tick(SimulationState *state, const SimulationConfig *config);

/**
 * @brief Run simulation until deadlock or completion
 * @param state Pointer to simulation state
 * @param config Configuration
 * @param max_ticks Maximum ticks to run (0 = unlimited)
 * @return Number of ticks executed
 */
int simulation_run_until_deadlock(SimulationState *state, const SimulationConfig *config,
                                   int max_ticks);

/**
 * @brief Run complete simulation with recovery
 * @param state Pointer to simulation state
 * @param config Configuration
 * @return true if completed successfully
 */
bool simulation_run_complete(SimulationState *state, const SimulationConfig *config);

/* ============================================================================
 * Event Functions
 * ============================================================================ */

/**
 * @brief Register event callback
 * @param state Pointer to simulation state
 * @param callback Callback function
 * @param user_data User data passed to callback
 */
void simulation_set_callback(SimulationState *state, SimulationCallback callback, 
                             void *user_data);

/**
 * @brief Log a simulation event
 * @param state Pointer to simulation state
 * @param type Event type
 * @param process_id Process ID (-1 if N/A)
 * @param resource_id Resource ID (-1 if N/A)
 * @param description Event description
 */
void simulation_log_event(SimulationState *state, SimulationEventType type,
                          int process_id, int resource_id, const char *description);

/**
 * @brief Get event log
 * @param state Pointer to simulation state
 * @param events Output array
 * @param max_events Maximum events to retrieve
 * @return Number of events retrieved
 */
int simulation_get_events(const SimulationState *state, SimulationEvent *events, 
                          int max_events);

/**
 * @brief Clear event log
 * @param state Pointer to simulation state
 */
void simulation_clear_events(SimulationState *state);

/* ============================================================================
 * Manual Control Functions (for interactive use)
 * ============================================================================ */

/**
 * @brief Manually add a process
 * @param state Pointer to simulation state
 * @param name Process name
 * @param priority Priority
 * @return Process ID
 */
int simulation_add_process(SimulationState *state, const char *name, int priority);

/**
 * @brief Manually add a resource
 * @param state Pointer to simulation state
 * @param name Resource name
 * @param instances Number of instances
 * @return Resource ID
 */
int simulation_add_resource(SimulationState *state, const char *name, int instances);

/**
 * @brief Manually request resource
 * @param state Pointer to simulation state
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true if request added
 */
bool simulation_request_resource(SimulationState *state, int process_id, int resource_id);

/**
 * @brief Manually allocate resource
 * @param state Pointer to simulation state
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true if allocated
 */
bool simulation_allocate_resource(SimulationState *state, int process_id, int resource_id);

/**
 * @brief Manually release resource
 * @param state Pointer to simulation state
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true if released
 */
bool simulation_release_resource(SimulationState *state, int process_id, int resource_id);

/**
 * @brief Trigger deadlock detection
 * @param state Pointer to simulation state
 * @return true if deadlock detected
 */
bool simulation_detect_deadlock(SimulationState *state);

/**
 * @brief Trigger recovery
 * @param state Pointer to simulation state
 * @param strategy Recovery strategy
 * @return true if recovery successful
 */
bool simulation_recover(SimulationState *state, RecoveryStrategy strategy);

/* ============================================================================
 * Query Functions
 * ============================================================================ */

/**
 * @brief Get scenario name
 * @param scenario Scenario enum
 * @return Scenario name string
 */
const char* simulation_scenario_name(SimulationScenario scenario);

/**
 * @brief Get scenario description
 * @param scenario Scenario enum
 * @return Scenario description
 */
const char* simulation_scenario_description(SimulationScenario scenario);

/**
 * @brief Get current simulation statistics
 * @param state Pointer to simulation state
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int simulation_get_stats(const SimulationState *state, char *buffer, size_t buffer_size);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Print current simulation state
 * @param state Pointer to simulation state
 */
void simulation_print_state(const SimulationState *state);

/**
 * @brief Print event log
 * @param state Pointer to simulation state
 */
void simulation_print_events(const SimulationState *state);

/**
 * @brief Export simulation to string (for analysis)
 * @param state Pointer to simulation state
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int simulation_export(const SimulationState *state, char *buffer, size_t buffer_size);

#endif /* SIMULATOR_H */
