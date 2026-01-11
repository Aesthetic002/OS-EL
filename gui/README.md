# OS-EL GUI (PyQt6)

Modern Python GUI for the OS-EL Deadlock Detection & Recovery System.

## Requirements

- Python 3.8 or higher
- PyQt6
- Built backend (`bin/deadlock.exe`)

## Installation

```powershell
pip install PyQt6
```

## Running the GUI

1. First, build the C backend:
   ```powershell
   mingw32-make
   ```

2. Run the GUI:
   ```powershell
   python gui\gui_qt.py
   ```

   Or from the gui directory:
   ```powershell
   cd gui
   python gui_qt.py
   ```

## Features

### Modern Dark Theme
- Sleek dark color palette with indigo/purple accents
- Smooth animations and glow effects
- Responsive layout with resizable panels

### Resource Allocation Graph Visualizer
- QGraphicsView-based interactive visualization
- Gradient-filled nodes with glow effects
- Multiple layout algorithms (force-directed, circular, hierarchical)
- Real-time updates when operations are performed
- Visual highlighting of deadlocked processes/resources (red glow)
- Draggable nodes for custom layouts
- Zoom and pan support

### Control Panel
- **Processes Tab**: Add, remove, and list processes with priority
- **Resources Tab**: Add, remove, and list resources with instance counts
- **Edge Operations Tab**: Request, allocate, and release resources

### Deadlock Detection & Recovery
- One-click deadlock detection
- Visual status indicators (green/red)
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
- Event logging with timestamps
- Auto-detect and auto-recover options
- Auto-run with adjustable speed slider

## Usage Tips

1. **Start Simple**: Begin with the "Simple Deadlock" scenario to understand the basics
2. **Visualize**: Use the RAG visualizer to see the state of the system
3. **Experiment**: Try different recovery strategies to see their effects
4. **Step Through**: Use "Tick" in simulations to observe step-by-step execution
5. **Auto-Run**: Use the speed slider and Auto-Run for continuous simulation

## Troubleshooting

### Backend Connection Error
- Ensure the backend is built: `mingw32-make`
- Check that `bin/deadlock.exe` exists
- Try rebuilding: `mingw32-make clean` then `mingw32-make`

### PyQt6 Installation Issues
- Update pip: `pip install --upgrade pip`
- Install PyQt6: `pip install PyQt6`
- On some systems: `pip install PyQt6-Qt6`

## Architecture

```
gui/
├── gui_qt.py                  # Main PyQt6 application
├── backend_interface.py       # JSON API communication
├── components/
│   ├── rag_visualizer_qt.py   # QGraphicsView RAG visualization
│   ├── control_panel_qt.py    # Process/resource controls
│   ├── deadlock_panel_qt.py   # Detection and recovery
│   └── simulation_panel_qt.py # Simulation scenarios
└── utils/
    ├── theme_qt.py            # Dark theme styling
    └── graph_layout.py        # Layout algorithms
```

## License

Part of the OS-EL project.