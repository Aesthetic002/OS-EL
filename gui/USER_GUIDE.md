# OS-EL: Deadlock Detection & Recovery System - GUI Guide

## Overview

**OS-EL** (Operating System Experimental Lab) is a comprehensive **Deadlock Detection and Recovery System** designed to help understand and visualize deadlock scenarios in operating systems. It uses **Resource Allocation Graphs (RAG)** and **DFS-based cycle detection** to identify deadlocks, with multiple recovery strategies.

## Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                        Python GUI (Tkinter)                         │
├───────────────┬──────────────────┬──────────────────────────────────┤
│ RAG Visualizer│  Control Panel   │    Simulation Panel              │
│               │  Deadlock Panel  │                                  │
└───────┬───────┴────────┬─────────┴───────────────┬──────────────────┘
        │                │                         │
        └────────────────┼─────────────────────────┘
                         │  JSON via stdin/stdout
        ┌────────────────▼─────────────────────────┐
        │       C Backend (deadlock.exe --api)     │
        ├──────────────────────────────────────────┤
        │  • RAG Data Structures                   │
        │  • Cycle Detection (DFS)                 │
        │  • Recovery Strategies                   │
        │  • Simulation Engine                     │
        └──────────────────────────────────────────┘
```

## How to Run

1. **Build the Backend** (if not already built):
   ```bash
   cd c:\Ganesh\Projects\OS-EL
   mingw32-make
   ```

2. **Run the GUI**:
   ```bash
   python gui\gui.py
   ```

## GUI Panels Explained

### 1. Resource Allocation Graph (RAG) Visualizer (Left Panel)

The **visual heart** of the application. It displays:
- **Processes** (circles): Represented as P0, P1, P2...
- **Resources** (rectangles): Represented as R0, R1, R2... with instance counts
- **Request Edges** (dashed arrows): Process → Resource (process wants resource)
- **Assignment Edges** (solid arrows): Resource → Process (resource allocated to process)
- **Deadlocked nodes**: Shown in red color

**Features**:
- **Drag nodes** to rearrange the graph
- **Layout options**: Force-Directed, Circular, or Hierarchical
- **Refresh** button to update from backend

### 2. Control Panel (Middle Panel - Top)

Manages **manual** process and resource operations via tabs:

#### Processes Tab
- **Add Process**: Create a new process with name and priority (0-100)
- **Process List**: View all active processes with ID, name, priority, and state
- **Remove Selected**: Delete a selected process

#### Resources Tab
- **Add Resource**: Create a resource with name and instance count
- **Resource List**: View all resources with total and available instances
- **Remove Selected**: Delete a selected resource

#### Edge Operations Tab
- **Request Resource**: Create a request edge (Process ID → Resource ID)
- **Allocate Resource**: Convert request to allocation (give resource to process)
- **Release Resource**: Process releases a specific resource
- **Release All**: Process releases all held resources

### 3. Deadlock Panel (Middle Panel - Bottom)

**Detection and Recovery** controls:

#### Detection Section
- **Detect Deadlock**: Run DFS-based cycle detection on the RAG
- Displays: Cycle count, deadlocked processes, deadlocked resources

#### Recovery Section
Choose a **strategy**:
- **Terminate All**: Kill all deadlocked processes
- **Terminate Lowest Priority**: Kill lowest priority process in cycle
- **Terminate One**: Kill one process based on criteria
- **Iterative Termination**: Keep terminating until deadlock resolved
- **Preempt Resources**: Force release resources
- **Rollback**: Rollback process state

Choose **selection criteria**:
- Lowest Priority
- Fewest Resources
- Youngest Process

### 4. Simulation Panel (Right Panel)

Pre-built **deadlock scenarios** for learning:

#### Load Scenario
- **Simple Deadlock**: Two processes in circular wait
- **Circular Wait**: N processes each holding one resource and waiting for the next
- **Dining Philosophers**: Classic deadlock problem with N philosophers and N forks
- **Random Scenario**: Randomly generated processes/resources with potential deadlock

#### Simulation Controls
- **Start/Pause/Stop**: Control simulation execution
- **Single Tick**: Step through simulation one event at a time
- **Auto-Detect Deadlock**: Automatically check for deadlock each tick
- **Auto-Recover**: Automatically apply recovery when deadlock detected

#### Auto-Run Section
- **Speed (ms/tick)**: Control simulation speed (default 500ms)
- **Auto-Run**: Automatically step through simulation with visual updates

#### Event Log
- Real-time log of simulation events, deadlock detection, and recovery actions

## How Deadlock Detection Works

### Resource Allocation Graph (RAG)

A **RAG** is a directed graph where:
- **Nodes**: Processes (circles) and Resources (rectangles)
- **Request Edge** (P → R): Process P is waiting for Resource R
- **Assignment Edge** (R → P): Resource R is held by Process P

### Cycle Detection (DFS)

The system uses **Depth-First Search** to detect cycles:
1. Build a wait-for graph from the RAG
2. Run DFS from each unvisited node
3. If we encounter a node already in the current path → **Cycle detected!**
4. A cycle in the wait-for graph indicates **deadlock**

### Example: Simple Deadlock

```
P0 holds R0, wants R1
P1 holds R1, wants R0

RAG:
  R0 ──→ P0 ──→ R1 ──→ P1 ──→ R0  (CYCLE!)
```

## Recovery Strategies Explained

| Strategy | Description | Best For |
|----------|-------------|----------|
| Terminate All | Kills all deadlocked processes | Quick resolution, acceptable data loss |
| Terminate Lowest | Kills lowest priority first | Priority-based systems |
| Terminate One | Kills one selected process | Minimal disruption |
| Iterative | Keeps terminating until resolved | Complex multi-cycle deadlocks |
| Preempt Resources | Forces resource release | When process state can be saved |
| Rollback | Reverts process to checkpoint | Systems with checkpointing |

## Typical Workflow

### Manual Exploration
1. Add some processes (P0, P1, P2)
2. Add some resources (R0, R1)
3. Allocate resources to processes
4. Create request edges to form a cycle
5. Click "Detect Deadlock" to find the cycle
6. Choose a recovery strategy and execute

### Simulation Mode
1. Click "Load Dining Philosophers" with 5 philosophers
2. The RAG will show 5 philosophers and 5 forks
3. Set speed to 500ms and click "Auto-Run"
4. Watch as philosophers pick up forks and eventually deadlock
5. Check "Auto-Recover" to see automatic resolution

## Understanding the Visualization

| Element | Shape | Color | Meaning |
|---------|-------|-------|---------|
| Process | Circle | Blue | Normal process |
| Process | Circle | Red | Deadlocked process |
| Resource | Rectangle | Green | Normal resource |
| Resource | Rectangle | Red | Involved in deadlock |
| Request Edge | Dashed Arrow | Orange | P waiting for R |
| Assignment Edge | Solid Arrow | Blue | R held by P |

## Keyboard Shortcuts

- **Drag nodes**: Click and drag to reposition
- **Resize panels**: Drag the dividers between panels

## Troubleshooting

### "Failed to connect to backend"
- Ensure `bin/deadlock.exe` exists (run `mingw32-make`)
- Close any other instances of the GUI

### Processes/Resources not appearing
- Click "Refresh" button in the RAG visualizer
- Check the Control Panel tabs for the lists

### Simulation not updating graph
- The graph auto-updates on each tick
- Try clicking "Refresh" if needed

## Educational Goals

This system helps understand:
1. **What is a deadlock?** - Visual representation of circular wait
2. **How to detect deadlocks** - DFS cycle detection algorithm
3. **Recovery trade-offs** - Different strategies with different costs
4. **Classic problems** - Dining Philosophers, Circular Wait
5. **Resource Allocation Graphs** - Industry-standard representation
