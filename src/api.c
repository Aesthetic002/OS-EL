/**
 * @file api.c
 * @brief JSON API layer implementation for Python GUI integration
 */

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

/* ============================================================================
 * Command and Status Names
 * ============================================================================ */

typedef struct {
    const char *name;
    APICommand cmd;
} CommandMapping;

static const CommandMapping command_map[] = {
    {"rag_init", CMD_RAG_INIT},
    {"rag_reset", CMD_RAG_RESET},
    {"rag_get_state", CMD_RAG_GET_STATE},
    {"add_process", CMD_ADD_PROCESS},
    {"remove_process", CMD_REMOVE_PROCESS},
    {"list_processes", CMD_LIST_PROCESSES},
    {"get_process", CMD_GET_PROCESS},
    {"add_resource", CMD_ADD_RESOURCE},
    {"remove_resource", CMD_REMOVE_RESOURCE},
    {"list_resources", CMD_LIST_RESOURCES},
    {"get_resource", CMD_GET_RESOURCE},
    {"request_resource", CMD_REQUEST_RESOURCE},
    {"cancel_request", CMD_CANCEL_REQUEST},
    {"allocate_resource", CMD_ALLOCATE_RESOURCE},
    {"release_resource", CMD_RELEASE_RESOURCE},
    {"release_all", CMD_RELEASE_ALL},
    {"detect_deadlock", CMD_DETECT_DEADLOCK},
    {"detect_all_cycles", CMD_DETECT_ALL_CYCLES},
    {"is_process_deadlocked", CMD_IS_PROCESS_DEADLOCKED},
    {"get_wait_for_graph", CMD_GET_WAIT_FOR_GRAPH},
    {"recover", CMD_RECOVER},
    {"recommend_strategy", CMD_RECOMMEND_STRATEGY},
    {"analyze_options", CMD_ANALYZE_OPTIONS},
    {"sim_init", CMD_SIM_INIT},
    {"sim_load_scenario", CMD_SIM_LOAD_SCENARIO},
    {"sim_start", CMD_SIM_START},
    {"sim_pause", CMD_SIM_PAUSE},
    {"sim_resume", CMD_SIM_RESUME},
    {"sim_stop", CMD_SIM_STOP},
    {"sim_tick", CMD_SIM_TICK},
    {"sim_run", CMD_SIM_RUN},
    {"sim_get_state", CMD_SIM_GET_STATE},
    {"sim_get_events", CMD_SIM_GET_EVENTS},
    {"get_version", CMD_GET_VERSION},
    {"get_help", CMD_GET_HELP},
    {"ping", CMD_PING},
    {"shutdown", CMD_SHUTDOWN},
    {NULL, CMD_UNKNOWN}
};

static const char* status_names[] = {
    "success",
    "error",
    "invalid_command",
    "invalid_params",
    "not_found",
    "already_exists",
    "operation_failed"
};

/* ============================================================================
 * Simple JSON Parser Helpers
 * ============================================================================ */

static const char* skip_whitespace(const char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static bool extract_string(const char *json, const char *key, char *value, size_t value_size) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    
    const char *pos = strstr(json, pattern);
    if (!pos) return false;
    
    pos += strlen(pattern);
    pos = skip_whitespace(pos);
    if (*pos != ':') return false;
    pos++;
    pos = skip_whitespace(pos);
    
    if (*pos != '"') return false;
    pos++;
    
    size_t i = 0;
    while (*pos && *pos != '"' && i < value_size - 1) {
        if (*pos == '\\' && *(pos + 1)) {
            pos++;
        }
        value[i++] = *pos++;
    }
    value[i] = '\0';
    
    return true;
}

static bool extract_int(const char *json, const char *key, int *value) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    
    const char *pos = strstr(json, pattern);
    if (!pos) return false;
    
    pos += strlen(pattern);
    pos = skip_whitespace(pos);
    if (*pos != ':') return false;
    pos++;
    pos = skip_whitespace(pos);
    
    char *end;
    long v = strtol(pos, &end, 10);
    if (end == pos) return false;
    
    *value = (int)v;
    return true;
}

static bool extract_bool(const char *json, const char *key, bool *value) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    
    const char *pos = strstr(json, pattern);
    if (!pos) return false;
    
    pos += strlen(pattern);
    pos = skip_whitespace(pos);
    if (*pos != ':') return false;
    pos++;
    pos = skip_whitespace(pos);
    
    if (strncmp(pos, "true", 4) == 0) {
        *value = true;
        return true;
    } else if (strncmp(pos, "false", 5) == 0) {
        *value = false;
        return true;
    }
    
    return false;
}

/* ============================================================================
 * API Lifecycle Functions
 * ============================================================================ */

void api_init(APIContext *ctx) {
    if (!ctx) return;
    
    rag_init(&ctx->rag);
    simulation_init(&ctx->simulation);
    ctx->initialized = true;
    ctx->running = false;
    ctx->last_error[0] = '\0';
}

void api_destroy(APIContext *ctx) {
    if (!ctx) return;
    
    rag_destroy(&ctx->rag);
    simulation_destroy(&ctx->simulation);
    ctx->initialized = false;
    ctx->running = false;
}

void api_reset(APIContext *ctx) {
    if (!ctx) return;
    
    rag_reset(&ctx->rag);
    simulation_reset(&ctx->simulation);
    ctx->last_error[0] = '\0';
}

/* ============================================================================
 * Error Handling
 * ============================================================================ */

void api_set_error(APIContext *ctx, const char *format, ...) {
    if (!ctx) return;
    
    va_list args;
    va_start(args, format);
    vsnprintf(ctx->last_error, sizeof(ctx->last_error), format, args);
    va_end(args);
}

const char* api_get_error(const APIContext *ctx) {
    if (!ctx) return "Invalid context";
    return ctx->last_error;
}

void api_error_response(APIResponse *response, APIStatus status, const char *message) {
    if (!response) return;
    
    response->status = status;
    strncpy(response->message, message, sizeof(response->message) - 1);
    response->message[sizeof(response->message) - 1] = '\0';
    response->data[0] = '\0';
    response->data_length = 0;
}

void api_success_response(APIResponse *response, const char *message) {
    if (!response) return;
    
    response->status = STATUS_SUCCESS;
    strncpy(response->message, message, sizeof(response->message) - 1);
    response->message[sizeof(response->message) - 1] = '\0';
}

/* ============================================================================
 * Command Helpers
 * ============================================================================ */

APICommand api_get_command(const char *name) {
    if (!name) return CMD_UNKNOWN;
    
    for (int i = 0; command_map[i].name != NULL; i++) {
        if (strcmp(name, command_map[i].name) == 0) {
            return command_map[i].cmd;
        }
    }
    
    return CMD_UNKNOWN;
}

const char* api_command_name(APICommand cmd) {
    for (int i = 0; command_map[i].name != NULL; i++) {
        if (command_map[i].cmd == cmd) {
            return command_map[i].name;
        }
    }
    return "unknown";
}

const char* api_status_name(APIStatus status) {
    if (status >= 0 && status < sizeof(status_names) / sizeof(status_names[0])) {
        return status_names[status];
    }
    return "unknown";
}

/* ============================================================================
 * JSON Serialization Helpers
 * ============================================================================ */

int api_process_to_json(const Process *process, char *buffer, size_t buffer_size) {
    if (!process || !buffer || buffer_size == 0) return 0;
    
    const char *state_str;
    switch (process->state) {
        case PROCESS_RUNNING:    state_str = "running"; break;
        case PROCESS_WAITING:    state_str = "waiting"; break;
        case PROCESS_BLOCKED:    state_str = "blocked"; break;
        case PROCESS_TERMINATED: state_str = "terminated"; break;
        default: state_str = "unknown";
    }
    
    return snprintf(buffer, buffer_size,
        "{\"id\": %d, \"name\": \"%s\", \"priority\": %d, \"state\": \"%s\", \"active\": %s}",
        process->id, process->name, process->priority, state_str,
        process->active ? "true" : "false");
}

int api_resource_to_json(const Resource *resource, char *buffer, size_t buffer_size) {
    if (!resource || !buffer || buffer_size == 0) return 0;
    
    return snprintf(buffer, buffer_size,
        "{\"id\": %d, \"name\": \"%s\", \"total_instances\": %d, \"available_instances\": %d, \"active\": %s}",
        resource->id, resource->name, resource->total_instances,
        resource->available_instances, resource->active ? "true" : "false");
}

int api_rag_to_json(const RAG *rag, char *buffer, size_t buffer_size) {
    if (!rag || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{");
    
    /* Processes */
    written += snprintf(buffer + written, buffer_size - written, "\"processes\": [");
    bool first = true;
    for (int i = 0; i < MAX_PROCESSES && written < (int)buffer_size - 100; i++) {
        if (rag->processes[i].active) {
            if (!first) written += snprintf(buffer + written, buffer_size - written, ", ");
            written += api_process_to_json(&rag->processes[i], buffer + written, buffer_size - written);
            first = false;
        }
    }
    written += snprintf(buffer + written, buffer_size - written, "], ");
    
    /* Resources */
    written += snprintf(buffer + written, buffer_size - written, "\"resources\": [");
    first = true;
    for (int i = 0; i < MAX_RESOURCES && written < (int)buffer_size - 100; i++) {
        if (rag->resources[i].active) {
            if (!first) written += snprintf(buffer + written, buffer_size - written, ", ");
            written += api_resource_to_json(&rag->resources[i], buffer + written, buffer_size - written);
            first = false;
        }
    }
    written += snprintf(buffer + written, buffer_size - written, "], ");
    
    /* Request edges */
    written += snprintf(buffer + written, buffer_size - written, "\"requests\": [");
    first = true;
    for (int p = 0; p < MAX_PROCESSES && written < (int)buffer_size - 50; p++) {
        for (int r = 0; r < MAX_RESOURCES; r++) {
            if (rag->request_matrix[p][r] > 0) {
                if (!first) written += snprintf(buffer + written, buffer_size - written, ", ");
                written += snprintf(buffer + written, buffer_size - written,
                    "{\"process\": %d, \"resource\": %d}", p, r);
                first = false;
            }
        }
    }
    written += snprintf(buffer + written, buffer_size - written, "], ");
    
    /* Assignment edges */
    written += snprintf(buffer + written, buffer_size - written, "\"assignments\": [");
    first = true;
    for (int p = 0; p < MAX_PROCESSES && written < (int)buffer_size - 50; p++) {
        for (int r = 0; r < MAX_RESOURCES; r++) {
            if (rag->assignment_matrix[p][r] > 0) {
                if (!first) written += snprintf(buffer + written, buffer_size - written, ", ");
                written += snprintf(buffer + written, buffer_size - written,
                    "{\"process\": %d, \"resource\": %d, \"count\": %d}",
                    p, r, rag->assignment_matrix[p][r]);
                first = false;
            }
        }
    }
    written += snprintf(buffer + written, buffer_size - written, "]");
    
    written += snprintf(buffer + written, buffer_size - written, "}");
    
    return written;
}

int api_deadlock_result_to_json(const RAG *rag, const DeadlockResult *result,
                                 char *buffer, size_t buffer_size) {
    if (!result || !buffer || buffer_size == 0) return 0;
    (void)rag; /* May use later for names */
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{");
    written += snprintf(buffer + written, buffer_size - written,
        "\"deadlock_detected\": %s, ", result->deadlock_detected ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written,
        "\"cycle_count\": %d, ", result->cycle_count);
    
    /* Deadlocked processes */
    written += snprintf(buffer + written, buffer_size - written, "\"deadlocked_processes\": [");
    for (int i = 0; i < result->deadlocked_process_count; i++) {
        if (i > 0) written += snprintf(buffer + written, buffer_size - written, ", ");
        written += snprintf(buffer + written, buffer_size - written, "%d", 
                            result->deadlocked_processes[i]);
    }
    written += snprintf(buffer + written, buffer_size - written, "], ");
    
    /* Deadlocked resources */
    written += snprintf(buffer + written, buffer_size - written, "\"deadlocked_resources\": [");
    for (int i = 0; i < result->deadlocked_resource_count; i++) {
        if (i > 0) written += snprintf(buffer + written, buffer_size - written, ", ");
        written += snprintf(buffer + written, buffer_size - written, "%d", 
                            result->deadlocked_resources[i]);
    }
    written += snprintf(buffer + written, buffer_size - written, "]");
    
    written += snprintf(buffer + written, buffer_size - written, "}");
    
    return written;
}

int api_recovery_result_to_json(const RecoveryResult *result, char *buffer, size_t buffer_size) {
    if (!result || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{");
    written += snprintf(buffer + written, buffer_size - written,
        "\"success\": %s, ", result->success ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written,
        "\"processes_terminated\": %d, ", result->processes_terminated);
    written += snprintf(buffer + written, buffer_size - written,
        "\"resources_preempted\": %d, ", result->resources_preempted);
    written += snprintf(buffer + written, buffer_size - written,
        "\"iterations\": %d, ", result->iterations);
    written += snprintf(buffer + written, buffer_size - written,
        "\"summary\": \"%s\"", result->summary);
    written += snprintf(buffer + written, buffer_size - written, "}");
    
    return written;
}

int api_simulation_state_to_json(const SimulationState *state, char *buffer, size_t buffer_size) {
    if (!state || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{");
    written += snprintf(buffer + written, buffer_size - written,
        "\"scenario\": \"%s\", ", simulation_scenario_name(state->scenario));
    written += snprintf(buffer + written, buffer_size - written,
        "\"current_tick\": %d, ", state->current_tick);
    written += snprintf(buffer + written, buffer_size - written,
        "\"running\": %s, ", state->running ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written,
        "\"paused\": %s, ", state->paused ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written,
        "\"deadlock_occurred\": %s, ", state->deadlock_occurred ? "true" : "false");
    written += snprintf(buffer + written, buffer_size - written,
        "\"event_count\": %d, ", state->event_count);
    written += snprintf(buffer + written, buffer_size - written,
        "\"process_count\": %d, ", state->rag.process_count);
    written += snprintf(buffer + written, buffer_size - written,
        "\"resource_count\": %d", state->rag.resource_count);
    written += snprintf(buffer + written, buffer_size - written, "}");
    
    return written;
}

int api_wait_for_graph_to_json(int wait_for_matrix[MAX_PROCESSES][MAX_PROCESSES],
                                int process_count, const RAG *rag,
                                char *buffer, size_t buffer_size) {
    if (!wait_for_matrix || !buffer || buffer_size == 0) return 0;
    (void)rag; /* May use for names */
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{\"edges\": [");
    
    bool first = true;
    for (int i = 0; i < process_count && written < (int)buffer_size - 50; i++) {
        for (int j = 0; j < process_count; j++) {
            if (wait_for_matrix[i][j]) {
                if (!first) written += snprintf(buffer + written, buffer_size - written, ", ");
                written += snprintf(buffer + written, buffer_size - written,
                    "{\"from\": %d, \"to\": %d}", i, j);
                first = false;
            }
        }
    }
    
    written += snprintf(buffer + written, buffer_size - written, "]}");
    return written;
}

/* ============================================================================
 * Request Parsing
 * ============================================================================ */

bool api_parse_request(const char *json, APIRequest *request) {
    if (!json || !request) return false;
    
    memset(request, 0, sizeof(APIRequest));
    strncpy(request->raw_json, json, sizeof(request->raw_json) - 1);
    
    /* Extract command */
    char cmd_str[64];
    if (!extract_string(json, "command", cmd_str, sizeof(cmd_str))) {
        request->command = CMD_UNKNOWN;
        return false;
    }
    request->command = api_get_command(cmd_str);
    
    /* Extract optional parameters */
    extract_int(json, "process_id", &request->process_id);
    extract_int(json, "resource_id", &request->resource_id);
    extract_string(json, "name", request->name, sizeof(request->name));
    extract_int(json, "priority", &request->priority);
    extract_int(json, "instances", &request->instances);
    extract_int(json, "scenario", &request->scenario);
    extract_int(json, "strategy", &request->strategy);
    extract_int(json, "criteria", &request->criteria);
    extract_int(json, "num_processes", &request->num_processes);
    extract_int(json, "num_resources", &request->num_resources);
    extract_int(json, "seed", &request->seed);
    extract_int(json, "max_ticks", &request->max_ticks);
    extract_bool(json, "auto_detect", &request->auto_detect);
    extract_bool(json, "auto_recover", &request->auto_recover);
    
    return true;
}

/* ============================================================================
 * Request Execution
 * ============================================================================ */

bool api_execute(APIContext *ctx, const APIRequest *request, APIResponse *response) {
    if (!ctx || !request || !response) return false;
    
    memset(response, 0, sizeof(APIResponse));
    
    switch (request->command) {
        case CMD_RAG_INIT:
            rag_init(&ctx->rag);
            api_success_response(response, "RAG initialized");
            return true;
            
        case CMD_RAG_RESET:
            rag_reset(&ctx->rag);
            api_success_response(response, "RAG reset");
            return true;
            
        case CMD_RAG_GET_STATE:
            api_success_response(response, "RAG state retrieved");
            response->data_length = api_rag_to_json(&ctx->rag, response->data, 
                                                     sizeof(response->data));
            return true;
            
        case CMD_ADD_PROCESS: {
            const char *name = request->name[0] ? request->name : "Process";
            int pid = rag_add_process(&ctx->rag, name, request->priority);
            if (pid >= 0) {
                api_success_response(response, "Process added");
                response->data_length = snprintf(response->data, sizeof(response->data),
                    "{\"process_id\": %d}", pid);
                return true;
            }
            api_error_response(response, STATUS_OPERATION_FAILED, "Failed to add process");
            return false;
        }
            
        case CMD_REMOVE_PROCESS:
            if (rag_remove_process(&ctx->rag, request->process_id)) {
                api_success_response(response, "Process removed");
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Process not found");
            return false;
            
        case CMD_LIST_PROCESSES:
            api_success_response(response, "Processes listed");
            {
                int written = snprintf(response->data, sizeof(response->data), "[");
                bool first = true;
                for (int i = 0; i < MAX_PROCESSES; i++) {
                    if (ctx->rag.processes[i].active) {
                        if (!first) written += snprintf(response->data + written, 
                                                        sizeof(response->data) - written, ", ");
                        written += api_process_to_json(&ctx->rag.processes[i], 
                                                       response->data + written,
                                                       sizeof(response->data) - written);
                        first = false;
                    }
                }
                written += snprintf(response->data + written, sizeof(response->data) - written, "]");
                response->data_length = written;
            }
            return true;
            
        case CMD_GET_PROCESS: {
            Process *p = rag_get_process(&ctx->rag, request->process_id);
            if (p) {
                api_success_response(response, "Process retrieved");
                response->data_length = api_process_to_json(p, response->data, 
                                                            sizeof(response->data));
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Process not found");
            return false;
        }
            
        case CMD_ADD_RESOURCE: {
            const char *name = request->name[0] ? request->name : "Resource";
            int instances = request->instances > 0 ? request->instances : 1;
            int rid = rag_add_resource(&ctx->rag, name, instances);
            if (rid >= 0) {
                api_success_response(response, "Resource added");
                response->data_length = snprintf(response->data, sizeof(response->data),
                    "{\"resource_id\": %d}", rid);
                return true;
            }
            api_error_response(response, STATUS_OPERATION_FAILED, "Failed to add resource");
            return false;
        }
            
        case CMD_REMOVE_RESOURCE:
            if (rag_remove_resource(&ctx->rag, request->resource_id)) {
                api_success_response(response, "Resource removed");
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Resource not found or in use");
            return false;
            
        case CMD_LIST_RESOURCES:
            api_success_response(response, "Resources listed");
            {
                int written = snprintf(response->data, sizeof(response->data), "[");
                bool first = true;
                for (int i = 0; i < MAX_RESOURCES; i++) {
                    if (ctx->rag.resources[i].active) {
                        if (!first) written += snprintf(response->data + written,
                                                        sizeof(response->data) - written, ", ");
                        written += api_resource_to_json(&ctx->rag.resources[i],
                                                        response->data + written,
                                                        sizeof(response->data) - written);
                        first = false;
                    }
                }
                written += snprintf(response->data + written, sizeof(response->data) - written, "]");
                response->data_length = written;
            }
            return true;
            
        case CMD_GET_RESOURCE: {
            Resource *r = rag_get_resource(&ctx->rag, request->resource_id);
            if (r) {
                api_success_response(response, "Resource retrieved");
                response->data_length = api_resource_to_json(r, response->data,
                                                              sizeof(response->data));
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Resource not found");
            return false;
        }
            
        case CMD_REQUEST_RESOURCE:
            if (rag_request_resource(&ctx->rag, request->process_id, request->resource_id)) {
                api_success_response(response, "Resource requested");
                return true;
            }
            api_error_response(response, STATUS_OPERATION_FAILED, "Failed to request resource");
            return false;
            
        case CMD_CANCEL_REQUEST:
            if (rag_cancel_request(&ctx->rag, request->process_id, request->resource_id)) {
                api_success_response(response, "Request cancelled");
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Request not found");
            return false;
            
        case CMD_ALLOCATE_RESOURCE:
            if (rag_allocate_resource(&ctx->rag, request->process_id, request->resource_id)) {
                api_success_response(response, "Resource allocated");
                return true;
            }
            api_error_response(response, STATUS_OPERATION_FAILED, 
                              "Failed to allocate (not available?)");
            return false;
            
        case CMD_RELEASE_RESOURCE:
            if (rag_release_resource(&ctx->rag, request->process_id, request->resource_id)) {
                api_success_response(response, "Resource released");
                return true;
            }
            api_error_response(response, STATUS_NOT_FOUND, "Resource not held");
            return false;
            
        case CMD_RELEASE_ALL: {
            int released = rag_release_all_resources(&ctx->rag, request->process_id);
            api_success_response(response, "Resources released");
            response->data_length = snprintf(response->data, sizeof(response->data),
                "{\"released\": %d}", released);
            return true;
        }
            
        case CMD_DETECT_DEADLOCK: {
            DeadlockResult result;
            detect_deadlock(&ctx->rag, &result);
            api_success_response(response, result.deadlock_detected ? 
                                "Deadlock detected!" : "No deadlock");
            response->data_length = api_deadlock_result_to_json(&ctx->rag, &result,
                                                                 response->data,
                                                                 sizeof(response->data));
            return true;
        }
            
        case CMD_DETECT_ALL_CYCLES: {
            DeadlockResult result;
            detect_all_cycles(&ctx->rag, &result);
            api_success_response(response, "Cycle detection complete");
            response->data_length = api_deadlock_result_to_json(&ctx->rag, &result,
                                                                 response->data,
                                                                 sizeof(response->data));
            return true;
        }
            
        case CMD_IS_PROCESS_DEADLOCKED: {
            bool deadlocked = is_process_deadlocked(&ctx->rag, request->process_id);
            api_success_response(response, deadlocked ? "Process is deadlocked" : 
                                                        "Process is not deadlocked");
            response->data_length = snprintf(response->data, sizeof(response->data),
                "{\"deadlocked\": %s}", deadlocked ? "true" : "false");
            return true;
        }
            
        case CMD_GET_WAIT_FOR_GRAPH: {
            int wait_for[MAX_PROCESSES][MAX_PROCESSES];
            build_wait_for_graph(&ctx->rag, wait_for);
            api_success_response(response, "Wait-for graph built");
            response->data_length = api_wait_for_graph_to_json(wait_for, 
                                                                ctx->rag.process_count,
                                                                &ctx->rag,
                                                                response->data,
                                                                sizeof(response->data));
            return true;
        }
            
        case CMD_RECOVER: {
            DeadlockResult detection;
            detect_deadlock(&ctx->rag, &detection);
            
            if (!detection.deadlock_detected) {
                api_success_response(response, "No deadlock to recover from");
                return true;
            }
            
            RecoveryConfig config;
            recovery_config_init(&config);
            config.strategy = (RecoveryStrategy)request->strategy;
            config.selection = (SelectionCriteria)request->criteria;
            
            RecoveryResult result;
            if (recover_from_deadlock(&ctx->rag, &detection, &config, &result)) {
                api_success_response(response, "Recovery successful");
            } else {
                api_success_response(response, "Recovery attempted");
            }
            response->data_length = api_recovery_result_to_json(&result, response->data,
                                                                 sizeof(response->data));
            return true;
        }
            
        case CMD_RECOMMEND_STRATEGY: {
            DeadlockResult detection;
            detect_deadlock(&ctx->rag, &detection);
            RecoveryStrategy strategy = recommend_recovery_strategy(&ctx->rag, &detection);
            api_success_response(response, "Strategy recommended");
            response->data_length = snprintf(response->data, sizeof(response->data),
                "{\"strategy\": %d, \"name\": \"%s\"}",
                strategy, recovery_strategy_name(strategy));
            return true;
        }
            
        case CMD_SIM_INIT:
            simulation_init(&ctx->simulation);
            api_success_response(response, "Simulation initialized");
            return true;
            
        case CMD_SIM_LOAD_SCENARIO:
            if (simulation_load_scenario(&ctx->simulation, (SimulationScenario)request->scenario)) {
                api_success_response(response, "Scenario loaded");
                response->data_length = api_simulation_state_to_json(&ctx->simulation,
                                                                      response->data,
                                                                      sizeof(response->data));
                return true;
            }
            api_error_response(response, STATUS_INVALID_PARAMS, "Invalid scenario");
            return false;
            
        case CMD_SIM_START:
            simulation_start(&ctx->simulation);
            api_success_response(response, "Simulation started");
            return true;
            
        case CMD_SIM_PAUSE:
            simulation_pause(&ctx->simulation);
            api_success_response(response, "Simulation paused");
            return true;
            
        case CMD_SIM_RESUME:
            simulation_resume(&ctx->simulation);
            api_success_response(response, "Simulation resumed");
            return true;
            
        case CMD_SIM_STOP:
            simulation_stop(&ctx->simulation);
            api_success_response(response, "Simulation stopped");
            return true;
            
        case CMD_SIM_TICK: {
            SimulationConfig config;
            simulation_config_init(&config);
            config.auto_detect = request->auto_detect;
            config.auto_recover = request->auto_recover;
            
            bool continued = simulation_tick(&ctx->simulation, &config);
            api_success_response(response, continued ? "Tick executed" : "Simulation ended");
            response->data_length = api_simulation_state_to_json(&ctx->simulation,
                                                                  response->data,
                                                                  sizeof(response->data));
            return true;
        }
            
        case CMD_SIM_GET_STATE:
            api_success_response(response, "Simulation state retrieved");
            response->data_length = api_simulation_state_to_json(&ctx->simulation,
                                                                  response->data,
                                                                  sizeof(response->data));
            return true;
            
        case CMD_SIM_GET_EVENTS:
            api_success_response(response, "Events retrieved");
            {
                int written = snprintf(response->data, sizeof(response->data), "[");
                for (int i = 0; i < ctx->simulation.event_count && 
                     written < (int)sizeof(response->data) - 200; i++) {
                    if (i > 0) written += snprintf(response->data + written,
                                                   sizeof(response->data) - written, ", ");
                    SimulationEvent *e = &ctx->simulation.events[i];
                    written += snprintf(response->data + written, sizeof(response->data) - written,
                        "{\"timestamp\": %d, \"type\": %d, \"process_id\": %d, "
                        "\"resource_id\": %d, \"description\": \"%s\"}",
                        e->timestamp, e->type, e->process_id, e->resource_id, e->description);
                }
                written += snprintf(response->data + written, 
                                    sizeof(response->data) - written, "]");
                response->data_length = written;
            }
            return true;
            
        case CMD_GET_VERSION:
            api_success_response(response, "Version retrieved");
            response->data_length = snprintf(response->data, sizeof(response->data),
                "{\"version\": \"%s\", \"name\": \"OS-EL Deadlock Detection\"}", API_VERSION);
            return true;
            
        case CMD_GET_HELP:
            api_success_response(response, "Help retrieved");
            response->data_length = api_get_documentation(response->data, sizeof(response->data));
            return true;
            
        case CMD_PING:
            api_success_response(response, "pong");
            return true;
            
        case CMD_SHUTDOWN:
            ctx->running = false;
            api_success_response(response, "Shutting down");
            return true;
            
        case CMD_UNKNOWN:
        default:
            api_error_response(response, STATUS_INVALID_COMMAND, "Unknown command");
            return false;
    }
}

/* ============================================================================
 * Response Serialization
 * ============================================================================ */

int api_serialize_response(const APIResponse *response, char *buffer, size_t buffer_size) {
    if (!response || !buffer || buffer_size == 0) return 0;
    
    int written = 0;
    /* Output compact single-line JSON for Python GUI compatibility */
    written += snprintf(buffer + written, buffer_size - written, "{");
    written += snprintf(buffer + written, buffer_size - written,
        "\"status\": \"%s\", ", api_status_name(response->status));
    written += snprintf(buffer + written, buffer_size - written,
        "\"message\": \"%s\"", response->message);
    
    if (response->data_length > 0) {
        written += snprintf(buffer + written, buffer_size - written,
            ", \"data\": %s", response->data);
    }
    
    written += snprintf(buffer + written, buffer_size - written, "}");
    
    return written;
}

int api_process_request(APIContext *ctx, const char *request_json,
                        char *response_buffer, size_t buffer_size) {
    if (!ctx || !request_json || !response_buffer || buffer_size == 0) return 0;
    
    APIRequest request;
    APIResponse response;
    
    if (!api_parse_request(request_json, &request)) {
        api_error_response(&response, STATUS_INVALID_PARAMS, "Failed to parse request");
    } else {
        api_execute(ctx, &request, &response);
    }
    
    return api_serialize_response(&response, response_buffer, buffer_size);
}

/* ============================================================================
 * Server Mode Functions
 * ============================================================================ */

bool api_process_stdin(APIContext *ctx) {
    char request_buffer[API_MAX_REQUEST_SIZE];
    char response_buffer[API_MAX_RESPONSE_SIZE];
    
    if (!fgets(request_buffer, sizeof(request_buffer), stdin)) {
        return false; /* EOF or error */
    }
    
    /* Remove trailing newline */
    size_t len = strlen(request_buffer);
    if (len > 0 && request_buffer[len - 1] == '\n') {
        request_buffer[len - 1] = '\0';
    }
    
    /* Process request */
    int response_len = api_process_request(ctx, request_buffer, 
                                            response_buffer, sizeof(response_buffer));
    
    /* Write response */
    if (response_len > 0) {
        printf("%s\n", response_buffer);
        fflush(stdout);
    }
    
    return ctx->running;
}

int api_run_server(APIContext *ctx) {
    if (!ctx) return 1;
    
    ctx->running = true;
    
    /* Print ready message */
    printf("{\"status\": \"ready\", \"version\": \"%s\"}\n", API_VERSION);
    fflush(stdout);
    
    while (ctx->running) {
        if (!api_process_stdin(ctx)) {
            break;
        }
    }
    
    return 0;
}

void api_shutdown_server(APIContext *ctx) {
    if (!ctx) return;
    ctx->running = false;
}

/* ============================================================================
 * Documentation
 * ============================================================================ */

const char* api_get_command_help(APICommand cmd) {
    switch (cmd) {
        case CMD_RAG_INIT: return "Initialize the Resource Allocation Graph";
        case CMD_RAG_RESET: return "Reset the RAG to empty state";
        case CMD_RAG_GET_STATE: return "Get current RAG state as JSON";
        case CMD_ADD_PROCESS: return "Add a process: {name, priority}";
        case CMD_REMOVE_PROCESS: return "Remove a process: {process_id}";
        case CMD_ADD_RESOURCE: return "Add a resource: {name, instances}";
        case CMD_REQUEST_RESOURCE: return "Request resource: {process_id, resource_id}";
        case CMD_ALLOCATE_RESOURCE: return "Allocate resource: {process_id, resource_id}";
        case CMD_RELEASE_RESOURCE: return "Release resource: {process_id, resource_id}";
        case CMD_DETECT_DEADLOCK: return "Detect deadlock using DFS";
        case CMD_RECOVER: return "Recover from deadlock: {strategy, criteria}";
        case CMD_SIM_LOAD_SCENARIO: return "Load simulation scenario: {scenario}";
        case CMD_PING: return "Health check - returns 'pong'";
        case CMD_SHUTDOWN: return "Shutdown the API server";
        default: return "No help available";
    }
}

int api_get_documentation(char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return 0;
    
    return snprintf(buffer, buffer_size,
        "{"
        "\"title\": \"OS-EL Deadlock Detection API\","
        "\"version\": \"%s\","
        "\"commands\": ["
        "\"rag_init\", \"rag_reset\", \"rag_get_state\","
        "\"add_process\", \"remove_process\", \"list_processes\","
        "\"add_resource\", \"remove_resource\", \"list_resources\","
        "\"request_resource\", \"allocate_resource\", \"release_resource\","
        "\"detect_deadlock\", \"recover\", \"recommend_strategy\","
        "\"sim_init\", \"sim_load_scenario\", \"sim_start\", \"sim_tick\","
        "\"ping\", \"shutdown\""
        "]"
        "}",
        API_VERSION);
}
