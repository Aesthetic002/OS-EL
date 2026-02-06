# Deadlock Detection Algorithm

## Theory
In an Operating System, a deadlock corresponds to a **cycle** in the Resource Allocation Graph (RAG) (for single-instance resources) or a **knot** (for multiple-instance resources). 

This module assumes a strict condition where a cycle implies a potential deadlock, and for single-instance resources, it *guarantees* a deadlock.

## Implementation: Depth-First Search (DFS)
The `cycle_detector.c` module implements a DFS traversal to find cycles.

### The Algorithm
The detection generally runs in $O(V+E)$ time, where $V$ is processes + resources and $E$ is total edges.

```c
bool detect_deadlock(const RAG *rag, DeadlockResult *result) {
    // 1. Mark all nodes as UNVISITED
    // 2. For each Process P in RAG:
    //      If P is UNVISITED:
    //          if dfs(P, path, visited):
    //              Report Deadlock
}
```

### DFS Traversal Logic
When visiting node `u`:
1.  Mark `u` as `VISITING` (currently in recursion stack).
2.  For each neighbor `v` of `u`:
    -   If `v` is `VISITING`: **Cycle Detected!** (Back-edge found).
    -   If `v` is `UNVISITED`: Recursively visit `v`.
3.  Mark `u` as `VISITED` (processing complete).

### Reporting Results
When a cycle is detected, the `DeadlockResult` structure is populated:
-   `deadlock_detected`: Boolean flag.
-   `cycle_count`: Number of distinct cycles found.
-   `deadlocked_processes`: List of Process IDs involved.
-   `deadlocked_resources`: List of Resource IDs involved.

## Wait-For Graph (WFG)
For simpler analysis, the RAG can be collapsed into a **Wait-For Graph**.
-   **Nodes**: Processes only.
-   **Edges**: $P_i \rightarrow P_j$ exists if $P_i$ is waiting for a resource held by $P_j$.

### Usage
The API provides `CMD_GET_WAIT_FOR_GRAPH`, which internally calls `build_wait_for_graph()`. This matrix is useful for higher-level visualizations where resources are abstracted away.
