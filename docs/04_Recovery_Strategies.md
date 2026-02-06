# Recovery Strategies

Once a deadlock is detected, the OS must recover to restore system stability. This module implements several recovery strategies defined in `recovery.h`.

## 1. Process Termination
This is the most common and simplest method. We kill one or more processes to break the cycle.

### Strategies
-   **`RECOVERY_TERMINATE_ALL`**: Terminates *all* processes involved in the deadlock.
    -   *Pros*: Fast, guaranteed resolution.
    -   *Cons*: Maximum data loss.
-   **`RECOVERY_TERMINATE_ONE`**: Terminates a single "victim" process.
    -   *Pros*: Minimal data loss if lucky.
    -   *Cons*: May not resolve deadlock if multiple cycles exist (requires iterative check).
-   **`RECOVERY_TERMINATE_ITERATIVE`**: Repeatedly terminates the "best" victim until the deadlock is gone.

### Selection Criteria
When choosing a victim, we use the following criteria (`SelectionCriteria`):
1.  **`SELECT_LOWEST_PRIORITY`**: Kill the least important process.
2.  **`SELECT_FEWEST_RESOURCES`**: Kill the process holding the least resources (cheaper to restart).
3.  **`SELECT_MOST_RESOURCES`**: Kill the "greedy" process (frees up many resources).
4.  **`SELECT_SHORTEST_RUNTIME`**: Kill the youngest process.

## 2. Resource Preemption
**`RECOVERY_PREEMPT_RESOURCES`**
Instead of killing the process, we forcibly take a resource away from it.
-   **Mechanism**: The OS de-allocates resource $R$ from Process $P$ and gives it to the waiting process.
-   ** Challenge**: The victim process $P$ is now in an inconsistent state. It must be rolled back or paused.

## 3. Rollback
**`RECOVERY_ROLLBACK`**
This is a simulated feature in OS-EL. Real OSs use checkpointing.
-   **Mechanism**: The system reverts a process to a safe state (e.g., before it requested the deadlock-causing resource).
-   **Implementation**: In this lab, "Rollback" effectively releases all resources held by the process and resets its state to `WAITING` or `READY`, simulating a restart of the transaction.

## Recovery Configuration
Configuring recovery involves setting the `RecoveryConfig` struct:
```c
RecoveryConfig config;
config.strategy = RECOVERY_TERMINATE_ITERATIVE;
config.selection = SELECT_LOWEST_PRIORITY;
config.max_terminations = 5; // Safety limit
```
