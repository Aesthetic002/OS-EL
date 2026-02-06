# Simulation Engine

The simulation module (`simulator.c`) allows users to run pre-defined scenarios to observe deadlock behavior in a controlled environment.

## Simulation State
The state tracks the entire lifecycle of a simulation run:
```c
typedef struct {
    RAG rag;                        // Snapshot of graph
    SimulationScenario scenario;    // Current scenario ID
    int current_tick;               // Time step
    bool running;                   // Status
    SimulationEvent events[1024];   // Log history
} SimulationState;
```

## Scenarios
The system includes several built-in scenarios:

### 1. Simple Deadlock (`SCENARIO_SIMPLE_DEADLOCK`)
A classic AB-BA deadlock.
-   **Process P1** holds A, requests B.
-   **Process P2** holds B, requests A.
-   *Outcome*: Immediate deadlock.

### 2. Circular Wait (`SCENARIO_CIRCULAR_WAIT`)
A ring of $N$ processes.
-   Each Process $P_i$ holds Resource $R_i$.
-   Each Process $P_i$ requests Resource $R_{(i+1) \% N}$.
-   *Outcome*: Deadlock involving all $N$ processes.

### 3. Dining Philosophers (`SCENARIO_DINING_PHILOSOPHERS`)
Simulates the classic problem with $N$ philosophers (processes) and $N$ forks (resources).
-   Strict ordering is NOT imposed here, purposely creating a deadlock scenario where all philosophers pick up their left fork simultaneously.

### 4. Random Scenario (`SCENARIO_RANDOM`)
Generates a chaotic environment for stress testing.
-   Randomly creates processes and resources.
-   Randomly assigns requests and allocations.
-   *Usage*: Good for testing the robustness of the cycle detection algorithm.

## Event System
The simulation records events to a log, which can be replayed or displayed by the GUI.
-   **Events**: `PROCESS_CREATE`, `RESOURCE_REQUEST`, `RESOURCE_ALLOCATE`, `DEADLOCK_DETECTED`, `RECOVERY_STARTED`.
-   **Logging**: All events are timestamped with the `current_tick`.
