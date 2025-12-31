/**
 * @file rag.h
 * @brief Resource Allocation Graph (RAG) data structure and operations
 * 
 * This module implements the Resource Allocation Graph used for deadlock
 * detection. The graph contains two types of nodes (processes and resources)
 * and two types of edges (request and assignment).
 */

#ifndef RAG_H
#define RAG_H

#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Constants and Limits
 * ============================================================================ */

#define MAX_PROCESSES 64
#define MAX_RESOURCES 64
#define MAX_NAME_LENGTH 32

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief Node type in the Resource Allocation Graph
 */
typedef enum {
    NODE_PROCESS,   /**< Process node (circle in RAG) */
    NODE_RESOURCE   /**< Resource node (rectangle in RAG) */
} NodeType;

/**
 * @brief Edge type in the Resource Allocation Graph
 */
typedef enum {
    EDGE_REQUEST,    /**< Process -> Resource (process is requesting) */
    EDGE_ASSIGNMENT  /**< Resource -> Process (resource is allocated) */
} EdgeType;

/**
 * @brief Process state enumeration
 */
typedef enum {
    PROCESS_RUNNING,
    PROCESS_WAITING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} ProcessState;

/**
 * @brief Process node structure
 */
typedef struct {
    int id;                         /**< Unique process ID */
    char name[MAX_NAME_LENGTH];     /**< Process name */
    ProcessState state;             /**< Current process state */
    int priority;                   /**< Process priority (for recovery) */
    bool active;                    /**< Whether this slot is in use */
} Process;

/**
 * @brief Resource node structure
 */
typedef struct {
    int id;                         /**< Unique resource ID */
    char name[MAX_NAME_LENGTH];     /**< Resource name */
    int total_instances;            /**< Total instances of this resource */
    int available_instances;        /**< Currently available instances */
    bool active;                    /**< Whether this slot is in use */
} Resource;

/**
 * @brief Edge structure representing request/assignment relationship
 */
typedef struct {
    int from_id;                    /**< Source node ID */
    int to_id;                      /**< Destination node ID */
    EdgeType type;                  /**< Type of edge */
    bool active;                    /**< Whether this edge exists */
} Edge;

/**
 * @brief Resource Allocation Graph structure
 */
typedef struct {
    Process processes[MAX_PROCESSES];
    Resource resources[MAX_RESOURCES];
    
    /* Adjacency matrix representation for edges */
    /* request_matrix[p][r] = 1 if process p is requesting resource r */
    int request_matrix[MAX_PROCESSES][MAX_RESOURCES];
    
    /* assignment_matrix[p][r] = number of instances of r allocated to p */
    int assignment_matrix[MAX_PROCESSES][MAX_RESOURCES];
    
    int process_count;
    int resource_count;
} RAG;

/* ============================================================================
 * RAG Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Initialize a new Resource Allocation Graph
 * @param rag Pointer to RAG structure to initialize
 */
void rag_init(RAG *rag);

/**
 * @brief Free resources associated with a RAG
 * @param rag Pointer to RAG structure to cleanup
 */
void rag_destroy(RAG *rag);

/**
 * @brief Reset RAG to initial empty state
 * @param rag Pointer to RAG structure to reset
 */
void rag_reset(RAG *rag);

/**
 * @brief Create a deep copy of a RAG
 * @param dest Destination RAG
 * @param src Source RAG
 */
void rag_copy(RAG *dest, const RAG *src);

/* ============================================================================
 * Process Management Functions
 * ============================================================================ */

/**
 * @brief Add a new process to the RAG
 * @param rag Pointer to RAG
 * @param name Process name
 * @param priority Process priority
 * @return Process ID on success, -1 on failure
 */
int rag_add_process(RAG *rag, const char *name, int priority);

/**
 * @brief Remove a process from the RAG
 * @param rag Pointer to RAG
 * @param process_id Process ID to remove
 * @return true on success, false on failure
 */
bool rag_remove_process(RAG *rag, int process_id);

/**
 * @brief Get process by ID
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @return Pointer to Process, or NULL if not found
 */
Process* rag_get_process(RAG *rag, int process_id);

/**
 * @brief Set process state
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param state New state
 * @return true on success, false on failure
 */
bool rag_set_process_state(RAG *rag, int process_id, ProcessState state);

/* ============================================================================
 * Resource Management Functions
 * ============================================================================ */

/**
 * @brief Add a new resource to the RAG
 * @param rag Pointer to RAG
 * @param name Resource name
 * @param instances Number of instances
 * @return Resource ID on success, -1 on failure
 */
int rag_add_resource(RAG *rag, const char *name, int instances);

/**
 * @brief Remove a resource from the RAG
 * @param rag Pointer to RAG
 * @param resource_id Resource ID to remove
 * @return true on success, false on failure
 */
bool rag_remove_resource(RAG *rag, int resource_id);

/**
 * @brief Get resource by ID
 * @param rag Pointer to RAG
 * @param resource_id Resource ID
 * @return Pointer to Resource, or NULL if not found
 */
Resource* rag_get_resource(RAG *rag, int resource_id);

/* ============================================================================
 * Edge (Request/Assignment) Management Functions
 * ============================================================================ */

/**
 * @brief Process requests a resource
 * @param rag Pointer to RAG
 * @param process_id Requesting process ID
 * @param resource_id Requested resource ID
 * @return true on success, false on failure
 */
bool rag_request_resource(RAG *rag, int process_id, int resource_id);

/**
 * @brief Cancel a resource request
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true on success, false on failure
 */
bool rag_cancel_request(RAG *rag, int process_id, int resource_id);

/**
 * @brief Allocate resource to process (converts request to assignment)
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true on success, false on failure
 */
bool rag_allocate_resource(RAG *rag, int process_id, int resource_id);

/**
 * @brief Release resource from process
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true on success, false on failure
 */
bool rag_release_resource(RAG *rag, int process_id, int resource_id);

/**
 * @brief Release all resources held by a process
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @return Number of resources released
 */
int rag_release_all_resources(RAG *rag, int process_id);

/* ============================================================================
 * Query Functions
 * ============================================================================ */

/**
 * @brief Check if process is requesting a resource
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true if requesting, false otherwise
 */
bool rag_is_requesting(const RAG *rag, int process_id, int resource_id);

/**
 * @brief Check if process holds a resource
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resource_id Resource ID
 * @return true if holding, false otherwise
 */
bool rag_is_holding(const RAG *rag, int process_id, int resource_id);

/**
 * @brief Get list of resources held by a process
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resources Output array of resource IDs
 * @param max_size Maximum size of output array
 * @return Number of resources held
 */
int rag_get_held_resources(const RAG *rag, int process_id, int *resources, int max_size);

/**
 * @brief Get list of resources requested by a process
 * @param rag Pointer to RAG
 * @param process_id Process ID
 * @param resources Output array of resource IDs
 * @param max_size Maximum size of output array
 * @return Number of resources requested
 */
int rag_get_requested_resources(const RAG *rag, int process_id, int *resources, int max_size);

/**
 * @brief Get process holding a resource
 * @param rag Pointer to RAG
 * @param resource_id Resource ID
 * @param processes Output array of process IDs
 * @param max_size Maximum size of output array
 * @return Number of processes holding the resource
 */
int rag_get_holding_processes(const RAG *rag, int resource_id, int *processes, int max_size);

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

/**
 * @brief Print RAG state to stdout (for debugging)
 * @param rag Pointer to RAG
 */
void rag_print(const RAG *rag);

/**
 * @brief Get RAG statistics
 * @param rag Pointer to RAG
 * @param total_processes Output: total active processes
 * @param total_resources Output: total active resources
 * @param total_requests Output: total pending requests
 * @param total_assignments Output: total assignments
 */
void rag_get_stats(const RAG *rag, int *total_processes, int *total_resources,
                   int *total_requests, int *total_assignments);

#endif /* RAG_H */
