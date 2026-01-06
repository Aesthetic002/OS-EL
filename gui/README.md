# OS-EL GUI

Python GUI for the OS-EL Deadlock Detection & Recovery System.

## Requirements

- Python 3.6 or higher
- Tkinter (usually included with Python)
- Built backend (`bin/deadlock.exe`)

## Installation

No additional installation required! All dependencies are Python built-ins.

## Running the GUI

1. First, build the C backend:
   ```powershell
   mingw32-make
   ```

2. Run the GUI:
   ```powershell
   python gui\gui.py
   ```

   Or from the gui directory:
   ```powershell
   cd gui
   python gui.py
   ```

## Features

### Resource Allocation Graph Visualizer
- Interactive visual representation of processes and resources
- Multiple layout algorithms (force-directed, circular, hierarchical)
- Real-time updates when operations are performed
- Visual highlighting of deadlocked processes/resources
- Draggable nodes for custom layouts

### Control Panel
- **Process Management**: Add, remove, and list processes
- **Resource Management**: Add, remove, and list resources
- **Edge Operations**: Request, allocate, and release resources

### Deadlock Detection & Recovery
- One-click deadlock detection
- Visual status indicators
- Multiple recovery strategies:
  - Terminate All
  - Terminate Lowest Priority
  - Terminate One Process
  - Iterative Termination
  - Preempt Resources
  - Rollback
- Strategy recommendation engine

### Simulation Scenarios
- **Simple Deadlock**: Two-process deadlock scenario
- **Circular Wait**: Configurable n-process circular wait
- **Dining Philosophers**: Classic synchronization problem
- **Random Scenario**: Randomly generated deadlock scenarios
- Event logging system
- Auto-detect and auto-recover options

## Usage Tips

1. **Start Simple**: Begin with the "Simple Deadlock" scenario to understand the basics
2. **Visualize**: Use the RAG visualizer to see the state of the system
3. **Experiment**: Try different recovery strategies to see their effects
4. **Step Through**: Use "Single Tick" in simulations to observe step-by-step execution
5. **Refresh**: Click "Refresh" buttons to update visualizations after changes

## Troubleshooting

### Backend Connection Error
- Ensure the backend is built: `mingw32-make`
- Check that `bin/deadlock.exe` exists
- Try rebuilding: `mingw32-make clean` then `mingw32-make`

### Tkinter Not Found
- Windows: Reinstall Python with "tcl/tk and IDLE" option
- Linux: `sudo apt-get install python3-tk`
- Mac: Usually pre-installed

## Architecture

```
gui/
├── gui.py                 # Main application
├── backend_interface.py   # JSON API communication
├── components/
│   ├── rag_visualizer.py  # RAG visualization
│   ├── control_panel.py   # Process/resource controls
│   ├── deadlock_panel.py  # Detection and recovery
│   └── simulation_panel.py # Simulation scenarios
└── utils/
    ├── theme.py           # UI theming
    └── graph_layout.py    # Layout algorithms
```

## License

Part of the OS-EL project.