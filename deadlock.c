#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

// Structure to hold system state
typedef struct {
    int num_processes;
    int num_resources;
    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int available[MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];
    bool finished[MAX_PROCESSES];
    char process_names[MAX_PROCESSES][20];
    char resource_names[MAX_RESOURCES][20];
} SystemState;

// Function prototypes
void initializeSystem(SystemState *state);
void calculateNeed(SystemState *state);
bool isSafeState(SystemState *state, int safeSequence[]);
bool requestResources(SystemState *state, int processId, int request[]);
void releaseResources(SystemState *state, int processId);
void displaySystemState(SystemState *state);
void displayMatrices(SystemState *state);
void saveStateToFile(SystemState *state, const char *filename);
void loadStateFromFile(SystemState *state, const char *filename);
int detectDeadlock(SystemState *state);
void generateReport(SystemState *state);

int main() {
    SystemState state;
    int choice, processId, i;
    int request[MAX_RESOURCES];
    int safeSeq[MAX_PROCESSES];
    char filename[100];
    
    printf("=== Advanced Deadlock Avoidance System ===\n");
    printf("Implementing Banker's Algorithm with Dynamic Resource Management\n\n");
    
    while (1) {
        printf("\n--- MAIN MENU ---\n");
        printf("1. Initialize New System\n");
        printf("2. Display System State\n");
        printf("3. Request Resources (Dynamic)\n");
        printf("4. Release Resources\n");
        printf("5. Check Safe State\n");
        printf("6. Detect Deadlock\n");
        printf("7. Save State to File\n");
        printf("8. Load State from File\n");
        printf("9. Generate Detailed Report\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                initializeSystem(&state);
                calculateNeed(&state);
                printf("\nSystem initialized successfully!\n");
                break;
                
            case 2:
                displaySystemState(&state);
                break;
                
            case 3:
                printf("\nEnter Process ID (0-%d): ", state.num_processes - 1);
                scanf("%d", &processId);
                
                if (processId < 0 || processId >= state.num_processes) {
                    printf("Invalid process ID!\n");
                    break;
                }
                
                printf("Enter resource request for process %s:\n", state.process_names[processId]);
                for (i = 0; i < state.num_resources; i++) {
                    printf("%s: ", state.resource_names[i]);
                    scanf("%d", &request[i]);
                }
                
                if (requestResources(&state, processId, request)) {
                    printf("\n✓ Request GRANTED! Resources allocated.\n");
                    calculateNeed(&state);
                } else {
                    printf("\n✗ Request DENIED! Would lead to unsafe state.\n");
                }
                break;
                
            case 4:
                printf("\nEnter Process ID to release resources (0-%d): ", state.num_processes - 1);
                scanf("%d", &processId);
                
                if (processId < 0 || processId >= state.num_processes) {
                    printf("Invalid process ID!\n");
                    break;
                }
                
                releaseResources(&state, processId);
                calculateNeed(&state);
                printf("\n✓ Resources released successfully!\n");
                break;
                
            case 5:
                if (isSafeState(&state, safeSeq)) {
                    printf("\n✓ System is in SAFE STATE!\n");
                    printf("Safe Sequence: ");
                    for (i = 0; i < state.num_processes; i++) {
                        printf("%s", state.process_names[safeSeq[i]]);
                        if (i < state.num_processes - 1) printf(" -> ");
                    }
                    printf("\n");
                } else {
                    printf("\n✗ System is in UNSAFE STATE! Deadlock possible.\n");
                }
                break;
                
            case 6:
                detectDeadlock(&state);
                break;
                
            case 7:
                printf("\nEnter filename to save state: ");
                scanf("%s", filename);
                saveStateToFile(&state, filename);
                printf("State saved to %s\n", filename);
                break;
                
            case 8:
                printf("\nEnter filename to load state: ");
                scanf("%s", filename);
                loadStateFromFile(&state, filename);
                calculateNeed(&state);
                printf("State loaded from %s\n", filename);
                break;
                
            case 9:
                generateReport(&state);
                break;
                
            case 0:
                printf("\nExiting system. Goodbye!\n");
                return 0;
                
            default:
                printf("\nInvalid choice! Please try again.\n");
        }
    }
    
    return 0;
}

void initializeSystem(SystemState *state) {
    int i, j;
    
    printf("\n--- System Initialization ---\n");
    printf("Enter number of processes: ");
    scanf("%d", &state->num_processes);
    printf("Enter number of resource types: ");
    scanf("%d", &state->num_resources);
    
    // Input process names
    printf("\nEnter process names:\n");
    for (i = 0; i < state->num_processes; i++) {
        printf("Process %d: ", i);
        scanf("%s", state->process_names[i]);
    }
    
    // Input resource names
    printf("\nEnter resource names:\n");
    for (i = 0; i < state->num_resources; i++) {
        printf("Resource %d: ", i);
        scanf("%s", state->resource_names[i]);
    }
    
    // Input Allocation Matrix
    printf("\n--- Allocation Matrix ---\n");
    for (i = 0; i < state->num_processes; i++) {
        printf("Process %s: ", state->process_names[i]);
        for (j = 0; j < state->num_resources; j++) {
            scanf("%d", &state->allocation[i][j]);
        }
    }
    
    // Input Maximum Matrix
    printf("\n--- Maximum Matrix ---\n");
    for (i = 0; i < state->num_processes; i++) {
        printf("Process %s: ", state->process_names[i]);
        for (j = 0; j < state->num_resources; j++) {
            scanf("%d", &state->maximum[i][j]);
        }
    }
    
    // Input Available Resources
    printf("\n--- Available Resources ---\n");
    for (i = 0; i < state->num_resources; i++) {
        printf("%s: ", state->resource_names[i]);
        scanf("%d", &state->available[i]);
    }
    
    // Initialize finished array
    for (i = 0; i < state->num_processes; i++) {
        state->finished[i] = false;
    }
}

void calculateNeed(SystemState *state) {
    int i, j;
    for (i = 0; i < state->num_processes; i++) {
        for (j = 0; j < state->num_resources; j++) {
            state->need[i][j] = state->maximum[i][j] - state->allocation[i][j];
        }
    }
}

bool isSafeState(SystemState *state, int safeSequence[]) {
    int work[MAX_RESOURCES];
    bool finish[MAX_PROCESSES];
    int i, j, k, count = 0;
    
    // Initialize work and finish
    for (i = 0; i < state->num_resources; i++) {
        work[i] = state->available[i];
    }
    
    for (i = 0; i < state->num_processes; i++) {
        finish[i] = false;
    }
    
    // Find safe sequence
    while (count < state->num_processes) {
        bool found = false;
        
        for (i = 0; i < state->num_processes; i++) {
            if (!finish[i]) {
                bool canAllocate = true;
                
                // Check if needs can be satisfied
                for (j = 0; j < state->num_resources; j++) {
                    if (state->need[i][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }
                
                if (canAllocate) {
                    // Simulate resource release
                    for (k = 0; k < state->num_resources; k++) {
                        work[k] += state->allocation[i][k];
                    }
                    
                    safeSequence[count++] = i;
                    finish[i] = true;
                    found = true;
                }
            }
        }
        
        // If no process found, system is unsafe
        if (!found) {
            return false;
        }
    }
    
    return true;
}

bool requestResources(SystemState *state, int processId, int request[]) {
    int i;
    int tempAvailable[MAX_RESOURCES];
    int tempAllocation[MAX_PROCESSES][MAX_RESOURCES];
    int tempNeed[MAX_PROCESSES][MAX_RESOURCES];
    int safeSeq[MAX_PROCESSES];
    
    // Step 1: Check if request <= need
    for (i = 0; i < state->num_resources; i++) {
        if (request[i] > state->need[processId][i]) {
            printf("Error: Process has exceeded its maximum claim!\n");
            return false;
        }
    }
    
    // Step 2: Check if request <= available
    for (i = 0; i < state->num_resources; i++) {
        if (request[i] > state->available[i]) {
            printf("Resources not available. Process must wait.\n");
            return false;
        }
    }
    
    // Step 3: Pretend to allocate (save current state)
    for (i = 0; i < state->num_resources; i++) {
        tempAvailable[i] = state->available[i];
    }
    
    for (i = 0; i < state->num_processes; i++) {
        for (int j = 0; j < state->num_resources; j++) {
            tempAllocation[i][j] = state->allocation[i][j];
            tempNeed[i][j] = state->need[i][j];
        }
    }
    
    // Modify state temporarily
    for (i = 0; i < state->num_resources; i++) {
        state->available[i] -= request[i];
        state->allocation[processId][i] += request[i];
        state->need[processId][i] -= request[i];
    }
    
    // Step 4: Check if resulting state is safe
    if (isSafeState(state, safeSeq)) {
        // Safe state - keep the allocation
        return true;
    } else {
        // Unsafe state - rollback
        for (i = 0; i < state->num_resources; i++) {
            state->available[i] = tempAvailable[i];
        }
        
        for (i = 0; i < state->num_processes; i++) {
            for (int j = 0; j < state->num_resources; j++) {
                state->allocation[i][j] = tempAllocation[i][j];
                state->need[i][j] = tempNeed[i][j];
            }
        }
        
        return false;
    }
}

void releaseResources(SystemState *state, int processId) {
    int i;
    
    printf("\nReleasing resources from process %s:\n", state->process_names[processId]);
    
    for (i = 0; i < state->num_resources; i++) {
        printf("%s: %d units released\n", state->resource_names[i], state->allocation[processId][i]);
        state->available[i] += state->allocation[processId][i];
        state->allocation[processId][i] = 0;
    }
}

void displaySystemState(SystemState *state) {
    int i, j;
    
    printf("\n========== CURRENT SYSTEM STATE ==========\n");
    printf("Processes: %d | Resources: %d\n\n", state->num_processes, state->num_resources);
    
    // Allocation Matrix
    printf("--- Allocation Matrix ---\n");
    printf("%-10s", "Process");
    for (j = 0; j < state->num_resources; j++) {
        printf("%-8s", state->resource_names[j]);
    }
    printf("\n");
    
    for (i = 0; i < state->num_processes; i++) {
        printf("%-10s", state->process_names[i]);
        for (j = 0; j < state->num_resources; j++) {
            printf("%-8d", state->allocation[i][j]);
        }
        printf("\n");
    }
    
    // Maximum Matrix
    printf("\n--- Maximum Matrix ---\n");
    printf("%-10s", "Process");
    for (j = 0; j < state->num_resources; j++) {
        printf("%-8s", state->resource_names[j]);
    }
    printf("\n");
    
    for (i = 0; i < state->num_processes; i++) {
        printf("%-10s", state->process_names[i]);
        for (j = 0; j < state->num_resources; j++) {
            printf("%-8d", state->maximum[i][j]);
        }
        printf("\n");
    }
    
    // Need Matrix
    printf("\n--- Need Matrix ---\n");
    printf("%-10s", "Process");
    for (j = 0; j < state->num_resources; j++) {
        printf("%-8s", state->resource_names[j]);
    }
    printf("\n");
    
    for (i = 0; i < state->num_processes; i++) {
        printf("%-10s", state->process_names[i]);
        for (j = 0; j < state->num_resources; j++) {
            printf("%-8d", state->need[i][j]);
        }
        printf("\n");
    }
    
    // Available Resources
    printf("\n--- Available Resources ---\n");
    for (i = 0; i < state->num_resources; i++) {
        printf("%s: %d  ", state->resource_names[i], state->available[i]);
    }
    printf("\n==========================================\n");
}

int detectDeadlock(SystemState *state) {
    int safeSeq[MAX_PROCESSES];
    
    printf("\n--- Deadlock Detection ---\n");
    
    if (isSafeState(state, safeSeq)) {
        printf("✓ No deadlock detected. System is in safe state.\n");
        return 0;
    } else {
        printf("✗ DEADLOCK DETECTED! System is in unsafe state.\n");
        printf("Processes potentially involved in deadlock:\n");
        
        // Identify processes that cannot proceed
        int work[MAX_RESOURCES];
        bool finish[MAX_PROCESSES];
        int i, j;
        
        for (i = 0; i < state->num_resources; i++) {
            work[i] = state->available[i];
        }
        
        for (i = 0; i < state->num_processes; i++) {
            finish[i] = false;
        }
        
        // Find processes that can finish
        bool changed = true;
        while (changed) {
            changed = false;
            for (i = 0; i < state->num_processes; i++) {
                if (!finish[i]) {
                    bool canProceed = true;
                    for (j = 0; j < state->num_resources; j++) {
                        if (state->need[i][j] > work[j]) {
                            canProceed = false;
                            break;
                        }
                    }
                    
                    if (canProceed) {
                        for (j = 0; j < state->num_resources; j++) {
                            work[j] += state->allocation[i][j];
                        }
                        finish[i] = true;
                        changed = true;
                    }
                }
            }
        }
        
        // Print processes in deadlock
        for (i = 0; i < state->num_processes; i++) {
            if (!finish[i]) {
                printf("  - %s (needs: ", state->process_names[i]);
                for (j = 0; j < state->num_resources; j++) {
                    printf("%s:%d ", state->resource_names[j], state->need[i][j]);
                }
                printf(")\n");
            }
        }
        
        return 1;
    }
}

void saveStateToFile(SystemState *state, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file for writing!\n");
        return;
    }
    
    int i, j;
    
    fprintf(file, "%d %d\n", state->num_processes, state->num_resources);
    
    // Save process names
    for (i = 0; i < state->num_processes; i++) {
        fprintf(file, "%s\n", state->process_names[i]);
    }
    
    // Save resource names
    for (i = 0; i < state->num_resources; i++) {
        fprintf(file, "%s\n", state->resource_names[i]);
    }
    
    // Save allocation
    for (i = 0; i < state->num_processes; i++) {
        for (j = 0; j < state->num_resources; j++) {
            fprintf(file, "%d ", state->allocation[i][j]);
        }
        fprintf(file, "\n");
    }
    
    // Save maximum
    for (i = 0; i < state->num_processes; i++) {
        for (j = 0; j < state->num_resources; j++) {
            fprintf(file, "%d ", state->maximum[i][j]);
        }
        fprintf(file, "\n");
    }
    
    // Save available
    for (i = 0; i < state->num_resources; i++) {
        fprintf(file, "%d ", state->available[i]);
    }
    
    fclose(file);
}

void loadStateFromFile(SystemState *state, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file for reading!\n");
        return;
    }
    
    int i, j;
    
    fscanf(file, "%d %d", &state->num_processes, &state->num_resources);
    
    // Load process names
    for (i = 0; i < state->num_processes; i++) {
        fscanf(file, "%s", state->process_names[i]);
    }
    
    // Load resource names
    for (i = 0; i < state->num_resources; i++) {
        fscanf(file, "%s", state->resource_names[i]);
    }
    
    // Load allocation
    for (i = 0; i < state->num_processes; i++) {
        for (j = 0; j < state->num_resources; j++) {
            fscanf(file, "%d", &state->allocation[i][j]);
        }
    }
    
    // Load maximum
    for (i = 0; i < state->num_processes; i++) {
        for (j = 0; j < state->num_resources; j++) {
            fscanf(file, "%d", &state->maximum[i][j]);
        }
    }
    
    // Load available
    for (i = 0; i < state->num_resources; i++) {
        fscanf(file, "%d", &state->available[i]);
    }
    
    fclose(file);
}

void generateReport(SystemState *state) {
    int safeSeq[MAX_PROCESSES];
    int i, j;
    
    printf("\n========== SYSTEM ANALYSIS REPORT ==========\n");
    printf("Generated at runtime\n\n");
    
    printf("System Configuration:\n");
    printf("  Total Processes: %d\n", state->num_processes);
    printf("  Total Resource Types: %d\n\n", state->num_resources);
    
    // Resource utilization
    printf("Resource Utilization:\n");
    for (i = 0; i < state->num_resources; i++) {
        int total_allocated = 0;
        for (j = 0; j < state->num_processes; j++) {
            total_allocated += state->allocation[j][i];
        }
        int total = total_allocated + state->available[i];
        float utilization = (total > 0) ? (float)total_allocated / total * 100 : 0;
        printf("  %s: %.2f%% utilized (%d/%d)\n", 
               state->resource_names[i], utilization, total_allocated, total);
    }
    
    printf("\nSafety Analysis:\n");
    if (isSafeState(state, safeSeq)) {
        printf("  Status: SAFE ✓\n");
        printf("  Safe Sequence: ");
        for (i = 0; i < state->num_processes; i++) {
            printf("%s", state->process_names[safeSeq[i]]);
            if (i < state->num_processes - 1) printf(" -> ");
        }
        printf("\n");
        printf("  Deadlock Risk: LOW\n");
    } else {
        printf("  Status: UNSAFE ✗\n");
        printf("  Safe Sequence: NONE\n");
        printf("  Deadlock Risk: HIGH\n");
    }
    
    printf("\nProcess Details:\n");
    for (i = 0; i < state->num_processes; i++) {
        printf("  %s:\n", state->process_names[i]);
        printf("    Allocated: ");
        for (j = 0; j < state->num_resources; j++) {
            printf("%s:%d ", state->resource_names[j], state->allocation[i][j]);
        }
        printf("\n    Remaining Need: ");
        for (j = 0; j < state->num_resources; j++) {
            printf("%s:%d ", state->resource_names[j], state->need[i][j]);
        }
        printf("\n");
    }
    
    printf("===========================================\n");
}
