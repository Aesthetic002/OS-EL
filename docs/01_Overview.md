# Project Overview

## Introduction
**OS-EL (Operating System Experimental Lab)** is a specialized educational module designed to demonstrate **Deadlock Detection and Recovery** concepts in operating systems. It provides a robust C-based backend that implements the **Resource Allocation Graph (RAG)** model and a DFS-based cycle detection algorithm, coupled with a Python/Qt frontend for visualization.

This project allows students and developers to visualize how deadlocks form, how they are detected by the OS, and the impact of various recovery strategies.

## Key Features
-   **Resource Allocation Graph (RAG)**: Full implementation of processes, resources, request edges, and assignment edges.
-   **Cycle Detection**: Depth-First Search (DFS) algorithm to identify cycles (deadlocks) in the graph.
-   **Wait-For Graph**: capability to transform RAG into a Wait-For Graph (WFG) for simplified analysis.
-   **Recovery Strategies**: Multiple strategies to resolve deadlocks:
    -   Process Termination (All, One-by-One).
    -   Resource Preemption.
    -   Rollback (simulated).
-   **Simulation Scenarios**: Pre-configured scenarios like *Dining Philosophers* and *Circular Wait*.
-   **Dual Interface**:
    -   **CLI**: Interactive command-line interface for direct control.
    -   **API**: JSON-based API over stdin/stdout for GUI integration.

## Directory Structure
-   `src/`: Core C implementation files (`rag.c`, `cycle_detector.c`, etc.).
-   `include/`: Header files defining the core structures and APIs.
-   `gui/`: Python-based Qt GUI application.
-   `main.c`: Entry point for the CLI and API server.
-   `Makefile`: Build configuration.

## Quick Start (Build & Run)

### Prerequisites
-   **Compiler**: GCC or MinGW (for Windows).
-   **Make**: GNU Make or `mingw32-make`.
-   **Python**: Python 3.x (for GUI).
-   **PyQt6**: `pip install PyQt6` (for GUI).

### Building the Backend
Open a terminal in the project root (`OS-EL/`) and run:
```powershell
mingw32-make
```
This produces the executable (e.g., `bin/deadlock.exe` or `huffman.exe` depending on legacy naming).

### Running the CLI
```powershell
.\bin\deadlock.exe
```

### Running the GUI
```powershell
python gui\gui_qt.py
```
*Note: The GUI automatically launches the backend executable in API mode.*
