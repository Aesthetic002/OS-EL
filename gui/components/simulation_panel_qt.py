"""
Simulation Panel Component (PyQt6)

Provides interface for loading simulation scenarios.
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGroupBox, QLabel,
    QPushButton, QSpinBox, QTextEdit, QMessageBox
)
from PyQt6.QtCore import pyqtSignal

import sys
import os
import json
from datetime import datetime
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from utils.theme_qt import COLORS, get_font


class SimulationPanel(QWidget):
    """Simulation scenarios panel"""
    
    updated = pyqtSignal()
    
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self._setup_ui()
        
    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setSpacing(8)
        
        # Scenario selection
        scenario_group = QGroupBox("Load Scenario")
        scenario_layout = QVBoxLayout(scenario_group)
        scenario_layout.setSpacing(8)
        
        # Simple Deadlock
        simple_btn = QPushButton("Simple Deadlock (2 processes)")
        simple_btn.clicked.connect(lambda: self._load_scenario(0))
        scenario_layout.addWidget(simple_btn)
        
        # Circular Wait
        circular_row = QHBoxLayout()
        circular_label = QLabel("Circular Wait:")
        circular_label.setFixedWidth(80)
        circular_row.addWidget(circular_label)
        self.circular_n_input = QSpinBox()
        self.circular_n_input.setRange(3, 8)
        self.circular_n_input.setValue(4)
        self.circular_n_input.setFixedWidth(60)
        circular_row.addWidget(self.circular_n_input)
        circular_row.addWidget(QLabel("processes"))
        circular_row.addStretch()
        circular_btn = QPushButton("Load")
        circular_btn.setFixedWidth(60)
        circular_btn.clicked.connect(lambda: self._load_scenario(1))
        circular_row.addWidget(circular_btn)
        scenario_layout.addLayout(circular_row)
        
        layout.addWidget(scenario_group)
        
        # Step Control
        step_group = QGroupBox("Step Control")
        step_layout = QVBoxLayout(step_group)
        
        tick_btn = QPushButton("Execute Step")
        tick_btn.clicked.connect(self._tick_simulation)
        step_layout.addWidget(tick_btn)
        
        layout.addWidget(step_group)
        
        # Event Log
        log_group = QGroupBox("Event Log")
        log_layout = QVBoxLayout(log_group)
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setMinimumHeight(120)
        log_layout.addWidget(self.log_text)
        
        clear_btn = QPushButton("Clear Log")
        clear_btn.setProperty("secondary", True)
        clear_btn.clicked.connect(lambda: self.log_text.clear())
        log_layout.addWidget(clear_btn)
        
        layout.addWidget(log_group)
        
        # Add stretch to push everything up
        layout.addStretch()
        
    def _load_scenario(self, scenario_id):
        """Load a simulation scenario"""
        try:
            # Initialize simulation first
            self.backend.sim_init()
            
            # Load the scenario
            response = self.backend.sim_load_scenario(scenario_id)
            if response and response.get('status') == 'success':
                scenario_names = ["Simple Deadlock", "Circular Wait"]
                scenario_name = (scenario_names[scenario_id] 
                               if scenario_id < len(scenario_names) 
                               else f"Scenario {scenario_id}")
                
                self._log(f"Loaded: {scenario_name}")
                self.updated.emit()
            else:
                self._log(f"Failed to load scenario")
                QMessageBox.warning(self, "Error",
                    f"Failed to load: {response.get('message', 'Unknown error')}")
        except Exception as e:
            self._log(f"Error: {e}")
            QMessageBox.critical(self, "Error", f"Failed to load scenario: {e}")
            
    def _tick_simulation(self):
        """Execute one simulation tick"""
        try:
            response = self.backend.sim_tick(True, False)
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                tick = data.get('current_tick', 0)
                running = data.get('running', False)
                deadlock = data.get('deadlock_occurred', False)
                
                if deadlock:
                    self._log(f"Step {tick}: DEADLOCK DETECTED")
                elif running:
                    self._log(f"Step {tick}: Running")
                else:
                    self._log(f"Step {tick}: Completed")
                
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to execute step: {e}")
            
    def _log(self, message):
        """Add message to event log with timestamp"""
        timestamp = datetime.now().strftime("%H:%M:%S")
        self.log_text.append(f"[{timestamp}] {message}")
        
    def refresh(self):
        """Refresh panel state"""
        pass
