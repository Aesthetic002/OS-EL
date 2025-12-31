/**
 * @file recovery.h
 * @brief Deadlock recovery strategies
 * 
 * This module implements various deadlock recovery strategies including
 * process termination, resource preemption, and rollback mechanisms.
 */

#ifndef RECOVERY_H
#define RECOVERY_H

#include "rag.h"
#include "cycle_detector.h"
#include <stdbool.h>

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief Recovery strategy enumeration
 */
typedef enum {
    RECOVERY_TERMINATE_ALL,      /**< Terminate all deadlocked processes */
    RECOVERY_TERMINATE_ONE,      /**< Terminate one process at a time */
    RECOVERY_TERMINATE_LOWEST,   /**< Terminate lowest priority process */
    RECOVERY_TERMINATE_YOUNGEST, /**< Terminate most recently created */
    RECOVERY_TERMINATE_OLDEST,   /**< Terminate oldest process */
    RECOVERY_PREEMPT_RESOURCES,  /**< Preempt resources from processes */
    RECOVERY_ROLLBACK            /**< Rollback processes to safe state */
} RecoveryStrategy;

/**
 * @brief Process selection criteria for termination
 */
typedef enum {
    SELECT_LOWEST_PRIORITY,      /**< Select process with lowest priority */
    SELECT_FEWEST_RESOURCES,     /**< Select process holding fewest resources */
    SELECT_MOST_RESOURCES,       /**< Select process holding most resources */
    SELECT_SHORTEST_RUNTIME,     /**< Select process with shortest runtime */
    SELECT_LONGEST_RUNTIME,      /**< Select process with longest runtime */
    SELECT_MINIMUM_COST          /**< Select process with minimum termination cost */
} SelectionCriteria;

/**
 * @brief Recovery action record
 */
typedef struct {
    int process_id;              /**< Affected process ID */
    int resource_id;             /**< Affected resource ID (-1 if N/A) */
    RecoveryStrategy strategy;   /**< Strategy used */
    char description[256];       /**< Human-readable description */
    bool success;                /**< Whether action succeeded */
} RecoveryAction;

/**
 * @brief Recovery result structure
 */
typedef struct {
    bool success;                        /**< Overall recovery success */
    RecoveryAction actions[MAX_PROCESSES]; /**< Actions taken */
    int action_count;                    /**< Number of actions taken */
    int processes_terminated;            /**< Number of processes terminated */
    int resources_preempted;             /**< Number of resources preempted */
    int iterations;                      /**< Iterations needed for recovery */
    char summary[512];                   /**< Summary message */
} RecoveryResult;

/**
 * @brief Recovery configuration options
 */
typedef struct {
    RecoveryStrategy strategy;           /**< Primary recovery strategy */
    SelectionCriteria selection;         /**< Process selection criteria */
    int max_terminations;                /**< Max processes to terminate (0=unlimited) */
    bool preserve_critical;              /**< Don't terminate critical processes */
    int critical_priority_threshold;     /**< Priority threshold for critical */
    bool verbose;                        /**< Verbose logging */
} RecoveryConfig;

/* ============================================================================
 * Initialization Functions
 * ============================================================================ */

/**
 * @brief Initialize recovery result structure
 * @param result Pointer to result structure
 */
void recovery_result_init(RecoveryResult *result);

/**
 * @brief Initialize recovery configuration with defaults
 * @param config Pointer to config structure
 */
void recovery_config_init(RecoveryConfig *config);

/* ============================================================================
 * Core Recovery Functions
 * ============================================================================ */

/**
 * @brief Attempt to recover from deadlock
 * @param rag Pointer to RAG (will be modified)
 * @param deadlock Deadlock detection result
 * @param config Recovery configuration
 * @param result Output recovery result
 * @return true if recovery successful, false otherwise
 */
bool recover_from_deadlock(RAG *rag, const DeadlockResult *deadlock,
                           const RecoveryConfig *config, RecoveryResult *result);

/**
 * @brief Recover using default strategy
 * @param rag Pointer to RAG (will be modified)
 * @param deadlock Deadlock detection result
 * @param result Output recovery result
 * @return true if recovery successful, false otherwise
 */
bool recover_default(RAG *rag, const DeadlockResult *deadlock, RecoveryResult *result);

/* ============================================================================
 * Specific Recovery Strategies
 * ============================================================================ */

/**
 * @brief Terminate all deadlocked processes
 * @param rag Pointer to RAG
 * @param deadlock Deadlock detection result
 * @param result Output recovery result
 * @return true if successful
 */
bool recovery_terminate_all(RAG *rag, const DeadlockResult *deadlock, RecoveryResult *result);

/**
 * @brief Terminate one process to break deadlock
 * @param rag Pointer to RAG
 * @param deadlock Deadlock detection result
 * @param criteria Selection criteria
 * @param result Output recovery result
 * @return true if successful
 */
bool recovery_terminate_one(RAG *rag, const DeadlockResult *deadlock,
                            SelectionCriteria criteria, RecoveryResult *result);

/**
 * @brief Iteratively terminate processes until deadlock resolved
 * @param rag Pointer to RAG
 * @param criteria Selection criteria
 * @param max_iterations Maximum iterations
 * @param result Output recovery result
 * @return true if successful
 */
bool recovery_terminate_iterative(RAG *rag, SelectionCriteria criteria,
                                   int max_iterations, RecoveryResult *result);

/**
 * @brief Preempt resources from a process
 * @param rag Pointer to RAG
 * @param process_id Process to preempt from
 * @param resource_ids Resources to preempt (NULL = all)
 * @param resource_count Number of resources (0 = all)
 * @param result Output recovery result
 * @return Number of resources preempted
 */
int recovery_preempt_resources(RAG *rag, int process_id, const int *resource_ids,
                                int resource_count, RecoveryResult *result);

/**
 * @brief Rollback a process (release all resources, reset state)
 * @param rag Pointer to RAG
 * @param process_id Process to rollback
 * @param result Output recovery result
 * @return true if successful
 */
bool recovery_rollback_process(RAG *rag, int process_id, RecoveryResult *result);

/* ============================================================================
 * Process Selection Functions
 * ============================================================================ */

/**
 * @brief Select victim process based on criteria
 * @param rag Pointer to RAG
 * @param deadlock Deadlock detection result
 * @param criteria Selection criteria
 * @return Process ID of victim, -1 if none found
 */
int select_victim_process(const RAG *rag, const DeadlockResult *deadlock,
                          SelectionCriteria criteria);

/**
 * @brief Calculate termination cost for a process
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @return Cost score (lower = cheaper to terminate)
 */
int calculate_termination_cost(const RAG *rag, int process_id);

/**
 * @brief Check if process is critical (should not be terminated)
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param priority_threshold Threshold for critical status
 * @return true if critical
 */
bool is_critical_process(const RAG *rag, int process_id, int priority_threshold);

/* ============================================================================
 * Analysis Functions
 * ============================================================================ */

/**
 * @brief Analyze recovery options without executing
 * @param rag Pointer to RAG
 * @param deadlock Deadlock detection result
 * @param strategy Strategy to analyze
 * @param estimated_terminations Output: estimated terminations needed
 * @param estimated_resources_freed Output: estimated resources freed
 * @return Feasibility score (0-100)
 */
int analyze_recovery_option(const RAG *rag, const DeadlockResult *deadlock,
                            RecoveryStrategy strategy, int *estimated_terminations,
                            int *estimated_resources_freed);

/**
 * @brief Get recommended recovery strategy
 * @param rag Pointer to RAG
 * @param deadlock Deadlock detection result
 * @return Recommended strategy
 */
RecoveryStrategy recommend_recovery_strategy(const RAG *rag, const DeadlockResult *deadlock);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Get strategy name as string
 * @param strategy Recovery strategy
 * @return Strategy name
 */
const char* recovery_strategy_name(RecoveryStrategy strategy);

/**
 * @brief Get selection criteria name as string
 * @param criteria Selection criteria
 * @return Criteria name
 */
const char* selection_criteria_name(SelectionCriteria criteria);

/**
 * @brief Print recovery result
 * @param result Recovery result
 */
void print_recovery_result(const RecoveryResult *result);

/**
 * @brief Convert recovery result to string
 * @param result Recovery result
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int recovery_result_to_string(const RecoveryResult *result, char *buffer, size_t buffer_size);

#endif /* RECOVERY_H */
