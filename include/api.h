/**
 * @file api.h
 * @brief JSON API layer for external integration (Python GUI, etc.)
 * 
 * This module provides a JSON-based API for interacting with the deadlock
 * detection system. It enables integration with Python GUI or other
 * external applications through stdin/stdout JSON communication.
 */

#ifndef API_H
#define API_H

#include "rag.h"
#include "cycle_detector.h"
#include "recovery.h"
#include "simulator.h"
#include <stdbool.h>
#include <stddef.h>

/* ============================================================================
 * Constants
 * ============================================================================ */

#define API_VERSION "1.0.0"
#define API_MAX_REQUEST_SIZE 65536
#define API_MAX_RESPONSE_SIZE 131072

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

/**
 * @brief API command types
 */
typedef enum {
    /* RAG Operations */
    CMD_RAG_INIT,
    CMD_RAG_RESET,
    CMD_RAG_GET_STATE,
    
    /* Process Operations */
    CMD_ADD_PROCESS,
    CMD_REMOVE_PROCESS,
    CMD_LIST_PROCESSES,
    CMD_GET_PROCESS,
    
    /* Resource Operations */
    CMD_ADD_RESOURCE,
    CMD_REMOVE_RESOURCE,
    CMD_LIST_RESOURCES,
    CMD_GET_RESOURCE,
    
    /* Edge Operations */
    CMD_REQUEST_RESOURCE,
    CMD_CANCEL_REQUEST,
    CMD_ALLOCATE_RESOURCE,
    CMD_RELEASE_RESOURCE,
    CMD_RELEASE_ALL,
    
    /* Detection Operations */
    CMD_DETECT_DEADLOCK,
    CMD_DETECT_ALL_CYCLES,
    CMD_IS_PROCESS_DEADLOCKED,
    CMD_GET_WAIT_FOR_GRAPH,
    
    /* Recovery Operations */
    CMD_RECOVER,
    CMD_RECOMMEND_STRATEGY,
    CMD_ANALYZE_OPTIONS,
    
    /* Simulation Operations */
    CMD_SIM_INIT,
    CMD_SIM_LOAD_SCENARIO,
    CMD_SIM_START,
    CMD_SIM_PAUSE,
    CMD_SIM_RESUME,
    CMD_SIM_STOP,
    CMD_SIM_TICK,
    CMD_SIM_RUN,
    CMD_SIM_GET_STATE,
    CMD_SIM_GET_EVENTS,
    
    /* System Operations */
    CMD_GET_VERSION,
    CMD_GET_HELP,
    CMD_PING,
    CMD_SHUTDOWN,
    
    CMD_UNKNOWN
} APICommand;

/**
 * @brief API response status
 */
typedef enum {
    STATUS_SUCCESS,
    STATUS_ERROR,
    STATUS_INVALID_COMMAND,
    STATUS_INVALID_PARAMS,
    STATUS_NOT_FOUND,
    STATUS_ALREADY_EXISTS,
    STATUS_OPERATION_FAILED
} APIStatus;

/**
 * @brief API context (holds global state)
 */
typedef struct {
    RAG rag;                        /**< Current RAG instance */
    SimulationState simulation;     /**< Simulation state */
    bool initialized;               /**< Whether API is initialized */
    bool running;                   /**< Whether API server is running */
    char last_error[256];           /**< Last error message */
} APIContext;

/**
 * @brief API request structure (parsed from JSON)
 */
typedef struct {
    APICommand command;             /**< Command to execute */
    char raw_json[API_MAX_REQUEST_SIZE]; /**< Raw JSON request */
    /* Parsed parameters - populated based on command */
    int process_id;
    int resource_id;
    char name[MAX_NAME_LENGTH];
    int priority;
    int instances;
    int scenario;
    int strategy;
    int criteria;
    int num_processes;
    int num_resources;
    int seed;
    int max_ticks;
    bool auto_detect;
    bool auto_recover;
} APIRequest;

/**
 * @brief API response structure (serialized to JSON)
 */
typedef struct {
    APIStatus status;               /**< Response status */
    char message[256];              /**< Status message */
    char data[API_MAX_RESPONSE_SIZE]; /**< JSON data payload */
    int data_length;                /**< Length of data */
} APIResponse;

/* ============================================================================
 * API Lifecycle Functions
 * ============================================================================ */

/**
 * @brief Initialize API context
 * @param ctx Pointer to API context
 */
void api_init(APIContext *ctx);

/**
 * @brief Destroy API context
 * @param ctx Pointer to API context
 */
void api_destroy(APIContext *ctx);

/**
 * @brief Reset API to initial state
 * @param ctx Pointer to API context
 */
void api_reset(APIContext *ctx);

/* ============================================================================
 * Request/Response Functions
 * ============================================================================ */

/**
 * @brief Parse JSON request string into APIRequest
 * @param json JSON request string
 * @param request Output request structure
 * @return true if parsing successful
 */
bool api_parse_request(const char *json, APIRequest *request);

/**
 * @brief Execute an API request
 * @param ctx API context
 * @param request Request to execute
 * @param response Output response
 * @return true if execution successful
 */
bool api_execute(APIContext *ctx, const APIRequest *request, APIResponse *response);

/**
 * @brief Serialize response to JSON string
 * @param response Response to serialize
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_serialize_response(const APIResponse *response, char *buffer, size_t buffer_size);

/**
 * @brief Process a complete JSON request and return JSON response
 * @param ctx API context
 * @param request_json JSON request string
 * @param response_buffer Output buffer for JSON response
 * @param buffer_size Response buffer size
 * @return Characters written to response buffer
 */
int api_process_request(APIContext *ctx, const char *request_json, 
                        char *response_buffer, size_t buffer_size);

/* ============================================================================
 * Server Mode Functions (stdin/stdout communication)
 * ============================================================================ */

/**
 * @brief Run API in server mode (reads JSON from stdin, writes to stdout)
 * @param ctx API context
 * @return Exit code
 */
int api_run_server(APIContext *ctx);

/**
 * @brief Process single request from stdin and write response to stdout
 * @param ctx API context
 * @return true if request processed, false on EOF or error
 */
bool api_process_stdin(APIContext *ctx);

/**
 * @brief Signal server to shutdown
 * @param ctx API context
 */
void api_shutdown_server(APIContext *ctx);

/* ============================================================================
 * JSON Serialization Helpers
 * ============================================================================ */

/**
 * @brief Serialize RAG to JSON
 * @param rag Pointer to RAG
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_rag_to_json(const RAG *rag, char *buffer, size_t buffer_size);

/**
 * @brief Serialize process to JSON
 * @param process Pointer to process
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_process_to_json(const Process *process, char *buffer, size_t buffer_size);

/**
 * @brief Serialize resource to JSON
 * @param resource Pointer to resource
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_resource_to_json(const Resource *resource, char *buffer, size_t buffer_size);

/**
 * @brief Serialize deadlock result to JSON
 * @param rag Pointer to RAG (for names)
 * @param result Pointer to deadlock result
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_deadlock_result_to_json(const RAG *rag, const DeadlockResult *result,
                                 char *buffer, size_t buffer_size);

/**
 * @brief Serialize recovery result to JSON
 * @param result Pointer to recovery result
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_recovery_result_to_json(const RecoveryResult *result, char *buffer, size_t buffer_size);

/**
 * @brief Serialize simulation state to JSON
 * @param state Pointer to simulation state
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_simulation_state_to_json(const SimulationState *state, char *buffer, size_t buffer_size);

/**
 * @brief Serialize wait-for graph to JSON
 * @param wait_for_matrix Wait-for adjacency matrix
 * @param process_count Number of processes
 * @param rag Pointer to RAG (for names)
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_wait_for_graph_to_json(int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES],
                                int process_count, const RAG *rag,
                                char *buffer, size_t buffer_size);

/* ============================================================================
 * Command Helpers
 * ============================================================================ */

/**
 * @brief Get command from string name
 * @param name Command name
 * @return APICommand enum value
 */
APICommand api_get_command(const char *name);

/**
 * @brief Get command name string
 * @param cmd APICommand enum
 * @return Command name string
 */
const char* api_command_name(APICommand cmd);

/**
 * @brief Get status name string
 * @param status APIStatus enum
 * @return Status name string
 */
const char* api_status_name(APIStatus status);

/**
 * @brief Get help text for a command
 * @param cmd APICommand enum
 * @return Help text
 */
const char* api_get_command_help(APICommand cmd);

/**
 * @brief Get full API documentation
 * @param buffer Output buffer
 * @param buffer_size Buffer size
 * @return Characters written
 */
int api_get_documentation(char *buffer, size_t buffer_size);

/* ============================================================================
 * Error Handling
 * ============================================================================ */

/**
 * @brief Set last error message
 * @param ctx API context
 * @param format Printf format string
 * @param ... Format arguments
 */
void api_set_error(APIContext *ctx, const char *format, ...);

/**
 * @brief Get last error message
 * @param ctx API context
 * @return Error message string
 */
const char* api_get_error(const APIContext *ctx);

/**
 * @brief Create error response
 * @param response Response to populate
 * @param status Error status
 * @param message Error message
 */
void api_error_response(APIResponse *response, APIStatus status, const char *message);

/**
 * @brief Create success response
 * @param response Response to populate
 * @param message Success message
 */
void api_success_response(APIResponse *response, const char *message);

#endif /* API_H */
