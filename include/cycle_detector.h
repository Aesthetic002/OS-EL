/**
 * @file cycle_detector.h
 * @brief DFS-based cycle detection algorithm for deadlock detection
 * 
 * This module implements cycle detection in the Resource Allocation Graph
 * using Depth-First Search (DFS). A cycle in the RAG indicates a deadlock.
 */

#ifndef CYCLE_DETECTOR_H
#define CYCLE_DETECTOR_H

#include "rag.h"
#include <stdbool.h>

/* ============================================================================
 * Constants
 * ============================================================================ */

#define MAX_CYCLE_LENGTH 128
#define MAX_CYCLES 32

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief Node in a detected cycle
 */
typedef struct {
    int id;                 /**< Node ID (process or resource) */
    NodeType type;          /**< Type of node */
} CycleNode;

/**
 * @brief Represents a detected cycle (deadlock)
 */
typedef struct {
    CycleNode nodes[MAX_CYCLE_LENGTH];  /**< Nodes in the cycle */
    int length;                          /**< Number of nodes in cycle */
    bool valid;                          /**< Whether this cycle is valid */
} Cycle;

/**
 * @brief Result of deadlock detection
 */
typedef struct {
    bool deadlock_detected;             /**< Whether deadlock was found */
    Cycle cycles[MAX_CYCLES];           /**< Detected cycles */
    int cycle_count;                    /**< Number of cycles found */
    int deadlocked_processes[MAX_PROCESSES]; /**< Process IDs in deadlock */
    int deadlocked_process_count;       /**< Number of deadlocked processes */
    int deadlocked_resources[MAX_RESOURCES]; /**< Resource IDs in deadlock */
    int deadlocked_resource_count;      /**< Number of deadlocked resources */
} DeadlockResult;

/**
 * @brief Detection algorithm type
 */
typedef enum {
    DETECT_DFS,             /**< Standard DFS-based detection */
    DETECT_DFS_ALL_CYCLES,  /**< Find all cycles (slower) */
    DETECT_BANKER           /**< Banker's algorithm style */
} DetectionAlgorithm;

/* ============================================================================
 * Core Detection Functions
 * ============================================================================ */

/**
 * @brief Initialize a DeadlockResult structure
 * @param result Pointer to result structure
 */
void deadlock_result_init(DeadlockResult *result);

/**
 * @brief Detect deadlock in the RAG using DFS
 * @param rag Pointer to RAG
 * @param result Pointer to store detection result
 * @return true if deadlock detected, false otherwise
 */
bool detect_deadlock(const RAG *rag, DeadlockResult *result);

/**
 * @brief Detect deadlock using specified algorithm
 * @param rag Pointer to RAG
 * @param result Pointer to store detection result
 * @param algorithm Algorithm to use
 * @return true if deadlock detected, false otherwise
 */
bool detect_deadlock_with_algorithm(const RAG *rag, DeadlockResult *result, 
                                    DetectionAlgorithm algorithm);

/**
 * @brief Find all cycles in the RAG (not just first one)
 * @param rag Pointer to RAG
 * @param result Pointer to store detection result
 * @return Number of cycles found
 */
int detect_all_cycles(const RAG *rag, DeadlockResult *result);

/* ============================================================================
 * Specific Detection Algorithms
 * ============================================================================ */

/**
 * @brief DFS-based cycle detection starting from a specific process
 * @param rag Pointer to RAG
 * @param start_process_id Starting process ID
 * @param cycle Output cycle if found
 * @return true if cycle found, false otherwise
 */
bool dfs_from_process(const RAG *rag, int start_process_id, Cycle *cycle);

/**
 * @brief Check if a specific process is in deadlock
 * @param rag Pointer to RAG
 * @param process_id Process ID to check
 * @return true if process is deadlocked, false otherwise
 */
bool is_process_deadlocked(const RAG *rag, int process_id);

/**
 * @brief Check if a specific resource is involved in deadlock
 * @param rag Pointer to RAG
 * @param resource_id Resource ID to check
 * @return true if resource is in deadlock cycle, false otherwise
 */
bool is_resource_in_deadlock(const RAG *rag, int resource_id);

/* ============================================================================
 * Wait-For Graph Functions
 * ============================================================================ */

/**
 * @brief Build wait-for graph from RAG
 * 
 * Wait-for graph is a simplified graph where edges exist only between
 * processes (P1 -> P2 if P1 is waiting for a resource held by P2)
 * 
 * @param rag Pointer to RAG
 * @param wait_for_matrix Output adjacency matrix [MAX_PROCESSES][MAX_PROCESSES]
 */
void build_wait_for_graph(const RAG *rag, int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES]);

/**
 * @brief Detect cycle in wait-for graph
 * @param wait_for_matrix Wait-for adjacency matrix
 * @param process_count Number of processes
 * @param cycle Output cycle of process IDs
 * @param cycle_length Output cycle length
 * @return true if cycle found, false otherwise
 */
bool detect_cycle_in_wait_for(int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES],
                               int process_count, int *cycle, int *cycle_length);

/* ============================================================================
 * Analysis Functions
 * ============================================================================ */

/**
 * @brief Get processes involved in a cycle
 * @param cycle Pointer to cycle
 * @param processes Output array of process IDs
 * @param max_size Maximum output array size
 * @return Number of processes in cycle
 */
int get_cycle_processes(const Cycle *cycle, int *processes, int max_size);

/**
 * @brief Get resources involved in a cycle
 * @param cycle Pointer to cycle
 * @param resources Output array of resource IDs
 * @param max_size Maximum output array size
 * @return Number of resources in cycle
 */
int get_cycle_resources(const Cycle *cycle, int *resources, int max_size);

/**
 * @brief Calculate the "depth" of deadlock (how many processes affected)
 * @param result Pointer to deadlock result
 * @return Deadlock depth score
 */
int calculate_deadlock_depth(const DeadlockResult *result);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Print cycle information
 * @param rag Pointer to RAG (for getting names)
 * @param cycle Pointer to cycle
 */
void print_cycle(const RAG *rag, const Cycle *cycle);

/**
 * @brief Print deadlock detection result
 * @param rag Pointer to RAG (for getting names)
 * @param result Pointer to deadlock result
 */
void print_deadlock_result(const RAG *rag, const DeadlockResult *result);

/**
 * @brief Convert detection result to human-readable string
 * @param rag Pointer to RAG
 * @param result Pointer to deadlock result
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Number of characters written
 */
int deadlock_result_to_string(const RAG *rag, const DeadlockResult *result,
                               char *buffer, size_t buffer_size);

#endif /* CYCLE_DETECTOR_H */
