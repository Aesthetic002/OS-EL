/**
 * @file rag.c
 * @brief Resource Allocation Graph (RAG) implementation
 */

#include "rag.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================================
 * RAG Lifecycle Functions
 * ============================================================================ */

void rag_init(RAG *rag) {
    if (!rag) return;
    
    memset(rag, 0, sizeof(RAG));
    
    /* Initialize all processes as inactive */
    for (int i = 0; i < MAX_PROCESSES; i++) {
        rag->processes[i].active = false;
        rag->processes[i].id = -1;
    }
    
    /* Initialize all resources as inactive */
    for (int i = 0; i < MAX_RESOURCES; i++) {
        rag->resources[i].active = false;
        rag->resources[i].id = -1;
    }
    
    /* Clear adjacency matrices */
    memset(rag->request_matrix, 0, sizeof(rag->request_matrix));
    memset(rag->assignment_matrix, 0, sizeof(rag->assignment_matrix));
    
    rag->process_count = 0;
    rag->resource_count = 0;
}

void rag_destroy(RAG *rag) {
    if (!rag) return;
    /* RAG uses static arrays, just reset */
    rag_init(rag);
}

void rag_reset(RAG *rag) {
    rag_init(rag);
}

void rag_copy(RAG *dest, const RAG *src) {
    if (!dest || !src) return;
    memcpy(dest, src, sizeof(RAG));
}

/* ============================================================================
 * Process Management Functions
 * ============================================================================ */

int rag_add_process(RAG *rag, const char *name, int priority) {
    if (!rag || !name) return -1;
    
    /* Find first available slot */
    int slot = -1;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (!rag->processes[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum processes (%d) reached\n", MAX_PROCESSES);
        return -1;
    }
    
    /* Initialize process */
    Process *p = &rag->processes[slot];
    p->id = slot;
    strncpy(p->name, name, MAX_NAME_LENGTH - 1);
    p->name[MAX_NAME_LENGTH - 1] = '\0';
    p->priority = priority;
    p->state = PROCESS_RUNNING;
    p->active = true;
    
    /* Clear this process's rows in matrices */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        rag->request_matrix[slot][r] = 0;
        rag->assignment_matrix[slot][r] = 0;
    }
    
    rag->process_count++;
    return slot;
}

bool rag_remove_process(RAG *rag, int process_id) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (!rag->processes[process_id].active) return false;
    
    /* Release all resources held by this process */
    rag_release_all_resources(rag, process_id);
    
    /* Cancel all pending requests */
    for (int r = 0; r < MAX_RESOURCES; r++) {
        if (rag->request_matrix[process_id][r] > 0) {
            rag->request_matrix[process_id][r] = 0;
        }
    }
    
    /* Mark process as inactive */
    rag->processes[process_id].active = false;
    rag->processes[process_id].id = -1;
    rag->process_count--;
    
    return true;
}

Process* rag_get_process(RAG *rag, int process_id) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return NULL;
    if (!rag->processes[process_id].active) return NULL;
    return &rag->processes[process_id];
}

bool rag_set_process_state(RAG *rag, int process_id, ProcessState state) {
    Process *p = rag_get_process(rag, process_id);
    if (!p) return false;
    p->state = state;
    return true;
}

/* ============================================================================
 * Resource Management Functions
 * ============================================================================ */

int rag_add_resource(RAG *rag, const char *name, int instances) {
    if (!rag || !name || instances <= 0) return -1;
    
    /* Find first available slot */
    int slot = -1;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (!rag->resources[i].active) {
            slot = i;
            break;
        }
    }
    
    if (slot == -1) {
        fprintf(stderr, "Error: Maximum resources (%d) reached\n", MAX_RESOURCES);
        return -1;
    }
    
    /* Initialize resource */
    Resource *r = &rag->resources[slot];
    r->id = slot;
    strncpy(r->name, name, MAX_NAME_LENGTH - 1);
    r->name[MAX_NAME_LENGTH - 1] = '\0';
    r->total_instances = instances;
    r->available_instances = instances;
    r->active = true;
    
    /* Clear this resource's columns in matrices */
    for (int p = 0; p < MAX_PROCESSES; p++) {
        rag->request_matrix[p][slot] = 0;
        rag->assignment_matrix[p][slot] = 0;
    }
    
    rag->resource_count++;
    return slot;
}

bool rag_remove_resource(RAG *rag, int resource_id) {
    if (!rag || resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    if (!rag->resources[resource_id].active) return false;
    
    /* Check if resource is currently allocated */
    for (int p = 0; p < MAX_PROCESSES; p++) {
        if (rag->assignment_matrix[p][resource_id] > 0) {
            fprintf(stderr, "Error: Cannot remove resource %d - still allocated\n", resource_id);
            return false;
        }
    }
    
    /* Clear all requests for this resource */
    for (int p = 0; p < MAX_PROCESSES; p++) {
        rag->request_matrix[p][resource_id] = 0;
    }
    
    /* Mark resource as inactive */
    rag->resources[resource_id].active = false;
    rag->resources[resource_id].id = -1;
    rag->resource_count--;
    
    return true;
}

Resource* rag_get_resource(RAG *rag, int resource_id) {
    if (!rag || resource_id < 0 || resource_id >= MAX_RESOURCES) return NULL;
    if (!rag->resources[resource_id].active) return NULL;
    return &rag->resources[resource_id];
}

/* ============================================================================
 * Edge (Request/Assignment) Management Functions
 * ============================================================================ */

bool rag_request_resource(RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    
    Process *p = rag_get_process(rag, process_id);
    Resource *r = rag_get_resource(rag, resource_id);
    
    if (!p || !r) return false;
    
    /* Check if already requesting */
    if (rag->request_matrix[process_id][resource_id] > 0) {
        return true; /* Already requesting */
    }
    
    /* Add request edge */
    rag->request_matrix[process_id][resource_id] = 1;
    
    /* Update process state to waiting */
    p->state = PROCESS_WAITING;
    
    return true;
}

bool rag_cancel_request(RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    
    if (rag->request_matrix[process_id][resource_id] == 0) {
        return false; /* Not requesting */
    }
    
    rag->request_matrix[process_id][resource_id] = 0;
    
    /* Check if process has other pending requests */
    bool has_requests = false;
    for (int r = 0; r < MAX_RESOURCES; r++) {
        if (rag->request_matrix[process_id][r] > 0) {
            has_requests = true;
            break;
        }
    }
    
    if (!has_requests && rag->processes[process_id].active) {
        rag->processes[process_id].state = PROCESS_RUNNING;
    }
    
    return true;
}

bool rag_allocate_resource(RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    
    Process *p = rag_get_process(rag, process_id);
    Resource *r = rag_get_resource(rag, resource_id);
    
    if (!p || !r) return false;
    
    /* Check if resource is available */
    if (r->available_instances <= 0) {
        return false; /* No available instances */
    }
    
    /* Remove request edge if exists */
    rag->request_matrix[process_id][resource_id] = 0;
    
    /* Add assignment edge */
    rag->assignment_matrix[process_id][resource_id]++;
    r->available_instances--;
    
    /* Update process state */
    bool has_requests = false;
    for (int res = 0; res < MAX_RESOURCES; res++) {
        if (rag->request_matrix[process_id][res] > 0) {
            has_requests = true;
            break;
        }
    }
    if (!has_requests) {
        p->state = PROCESS_RUNNING;
    }
    
    return true;
}

bool rag_release_resource(RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    
    if (rag->assignment_matrix[process_id][resource_id] <= 0) {
        return false; /* Not holding this resource */
    }
    
    /* Remove assignment edge */
    rag->assignment_matrix[process_id][resource_id]--;
    
    /* Update resource availability */
    if (rag->resources[resource_id].active) {
        rag->resources[resource_id].available_instances++;
    }
    
    return true;
}

int rag_release_all_resources(RAG *rag, int process_id) {
    if (!rag || process_id < 0 || process_id >= MAX_PROCESSES) return 0;
    
    int released = 0;
    for (int r = 0; r < MAX_RESOURCES; r++) {
        while (rag->assignment_matrix[process_id][r] > 0) {
            rag_release_resource(rag, process_id, r);
            released++;
        }
    }
    
    return released;
}

/* ============================================================================
 * Query Functions
 * ============================================================================ */

bool rag_is_requesting(const RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    return rag->request_matrix[process_id][resource_id] > 0;
}

bool rag_is_holding(const RAG *rag, int process_id, int resource_id) {
    if (!rag) return false;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return false;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return false;
    return rag->assignment_matrix[process_id][resource_id] > 0;
}

int rag_get_held_resources(const RAG *rag, int process_id, int *resources, int max_size) {
    if (!rag || !resources || max_size <= 0) return 0;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return 0;
    
    int count = 0;
    for (int r = 0; r < MAX_RESOURCES && count < max_size; r++) {
        if (rag->assignment_matrix[process_id][r] > 0) {
            resources[count++] = r;
        }
    }
    return count;
}

int rag_get_requested_resources(const RAG *rag, int process_id, int *resources, int max_size) {
    if (!rag || !resources || max_size <= 0) return 0;
    if (process_id < 0 || process_id >= MAX_PROCESSES) return 0;
    
    int count = 0;
    for (int r = 0; r < MAX_RESOURCES && count < max_size; r++) {
        if (rag->request_matrix[process_id][r] > 0) {
            resources[count++] = r;
        }
    }
    return count;
}

int rag_get_holding_processes(const RAG *rag, int resource_id, int *processes, int max_size) {
    if (!rag || !processes || max_size <= 0) return 0;
    if (resource_id < 0 || resource_id >= MAX_RESOURCES) return 0;
    
    int count = 0;
    for (int p = 0; p < MAX_PROCESSES && count < max_size; p++) {
        if (rag->assignment_matrix[p][resource_id] > 0) {
            processes[count++] = p;
        }
    }
    return count;
}

/* ============================================================================
 * Utility Functions
 * ============================================================================ */

void rag_print(const RAG *rag) {
    if (!rag) {
        printf("RAG: NULL\n");
        return;
    }
    
    printf("\n========== Resource Allocation Graph ==========\n");
    printf("Processes: %d, Resources: %d\n\n", rag->process_count, rag->resource_count);
    
    /* Print processes */
    printf("--- Processes ---\n");
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (rag->processes[i].active) {
            const Process *p = &rag->processes[i];
            const char *state_str;
            switch (p->state) {
                case PROCESS_RUNNING:    state_str = "RUNNING"; break;
                case PROCESS_WAITING:    state_str = "WAITING"; break;
                case PROCESS_BLOCKED:    state_str = "BLOCKED"; break;
                case PROCESS_TERMINATED: state_str = "TERMINATED"; break;
                default: state_str = "UNKNOWN";
            }
            printf("  P%d: %s (priority=%d, state=%s)\n", 
                   p->id, p->name, p->priority, state_str);
        }
    }
    
    /* Print resources */
    printf("\n--- Resources ---\n");
    for (int i = 0; i < MAX_RESOURCES; i++) {
        if (rag->resources[i].active) {
            const Resource *r = &rag->resources[i];
            printf("  R%d: %s (total=%d, available=%d)\n",
                   r->id, r->name, r->total_instances, r->available_instances);
        }
    }
    
    /* Print request edges */
    printf("\n--- Request Edges (Process -> Resource) ---\n");
    bool has_requests = false;
    for (int p = 0; p < MAX_PROCESSES; p++) {
        if (!rag->processes[p].active) continue;
        for (int r = 0; r < MAX_RESOURCES; r++) {
            if (rag->request_matrix[p][r] > 0) {
                printf("  P%d (%s) --requests--> R%d (%s)\n",
                       p, rag->processes[p].name,
                       r, rag->resources[r].name);
                has_requests = true;
            }
        }
    }
    if (!has_requests) printf("  (none)\n");
    
    /* Print assignment edges */
    printf("\n--- Assignment Edges (Resource -> Process) ---\n");
    bool has_assignments = false;
    for (int p = 0; p < MAX_PROCESSES; p++) {
        if (!rag->processes[p].active) continue;
        for (int r = 0; r < MAX_RESOURCES; r++) {
            if (rag->assignment_matrix[p][r] > 0) {
                printf("  R%d (%s) --assigned--> P%d (%s) [%d instance(s)]\n",
                       r, rag->resources[r].name,
                       p, rag->processes[p].name,
                       rag->assignment_matrix[p][r]);
                has_assignments = true;
            }
        }
    }
    if (!has_assignments) printf("  (none)\n");
    
    printf("================================================\n\n");
}

void rag_get_stats(const RAG *rag, int *total_processes, int *total_resources,
                   int *total_requests, int *total_assignments) {
    if (!rag) {
        if (total_processes) *total_processes = 0;
        if (total_resources) *total_resources = 0;
        if (total_requests) *total_requests = 0;
        if (total_assignments) *total_assignments = 0;
        return;
    }
    
    if (total_processes) *total_processes = rag->process_count;
    if (total_resources) *total_resources = rag->resource_count;
    
    int requests = 0, assignments = 0;
    for (int p = 0; p < MAX_PROCESSES; p++) {
        for (int r = 0; r < MAX_RESOURCES; r++) {
            requests += rag->request_matrix[p][r];
            assignments += rag->assignment_matrix[p][r];
        }
    }
    
    if (total_requests) *total_requests = requests;
    if (total_assignments) *total_assignments = assignments;
}
