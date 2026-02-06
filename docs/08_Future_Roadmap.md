# Future Roadmap

This document outlines the planned improvements and experimental features for the OS-EL project.

## Short-Term Goals (v1.1)

### 1. Banker's Algorithm Implementation
-   **Current Status**: Stubbed definitions exists (`SCENARIO_BANKER_SAFE`), but logic is unimplemented.
-   **Goal**: Implement the Banker's safety algorithm to *prevent* unsafe allocations before they happen, rather than just detecting deadlocks after the fact.

### 2. Enhanced Recovery Stats
-   **Goal**: Better analytics for recovery strategies. Measure "cost" in terms of lost CPU cycles (simulated by process runtime attribute) rather than just process count.

## Mid-Term Goals (v2.0)

### 1. Multithreaded Detection
-   **Goal**: Parallelize the cycle detection algorithm for very large graphs ($N > 1000$).
-   **Method**: Split the graph into sub-graphs or use parallel DFS.

### 2. WebAssembly (Wasm) Port
-   **Goal**: Compile the C backend to Wasm using Emscripten.
-   **Benefit**: Run the entire simulation in a web browser, removing the need for local compilation and Python/Qt.

## Long-Term Vision

### 1. Distributed Deadlock Detection
-   **Concept**: Simulate distributed systems where the RAG is split across multiple nodes (processes communicating over a network).
-   **Challenge**: detecting global cycles without a central coordinator (Chandy-Misra-Haas algorithm).

### 2. Real-Time Visualization
-   Integrate with a more advanced visualization library (like D3.js via the Wasm port) for smoother, interactive graph manipulations.
