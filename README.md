# OS-EL: Deadlock Detection & Recovery Module

An Operating System Experimental Lab module for understanding deadlock detection and recovery using Resource Allocation Graphs (RAG) with DFS-based cycle detection.

## Overview

- Resource Allocation Graph (RAG) data structure
- DFS-based cycle detection for deadlock detection
- Multiple recovery strategies (termination, preemption, rollback)
- Simulation scenarios (Simple Deadlock, Circular Wait, Dining Philosophers)
- Interactive CLI interface
- JSON API for Python GUI integration

## Project Structure

```
OS-EL/
├── include/
│   ├── rag.h              # Resource Allocation Graph header
│   ├── cycle_detector.h   # DFS-based cycle detection header
│   ├── recovery.h         # Recovery strategies header
│   ├── simulator.h        # Simulation scenarios header
│   └── api.h              # JSON API header
├── src/
│   ├── rag.c              # RAG implementation
│   ├── cycle_detector.c   # Cycle detection implementation
│   ├── recovery.c         # Recovery strategies implementation
│   ├── simulator.c        # Simulation implementation
│   └── api.c              # JSON API implementation
├── tests/
│   └── test_deadlock.c    # Unit tests
├── main.c                 # CLI demo program
├── Makefile               # Build configuration
└── README.md              # This file
```

## Build
Compile:
```powershell
mingw32-make
```

Run CLI:
```powershell
.\bin\deadlock.exe
```

Run GUI:
```powershell
pip install PyQt6
python gui\gui_qt.py
```

## GUI Features

The project includes a modern PyQt6-based GUI with:

- **Visual RAG Display**: Interactive graph visualization with multiple layout algorithms
- **Process/Resource Management**: Easy-to-use controls for managing system entities
- **Deadlock Detection**: Real-time detection with visual highlighting
- **Recovery Strategies**: Multiple configurable recovery options
- **Simulation Scenarios**: Pre-built scenarios including Simple Deadlock, Dining Philosophers, etc.
- **Event Logging**: Track simulation progress and system events

See [gui/README.md](gui/README.md) for detailed GUI documentation.

## Quick Start

1. Build the backend:
   ```powershell
   mingw32-make
   ```

2. Run the GUI:
   ```powershell
   pip install PyQt6
   python gui\gui_qt.py
   ```

3. Try the "Simple Deadlock" scenario from the Simulation panel
4. Click "Detect Deadlock" to see the detection in action
5. Experiment with different recovery strategies

