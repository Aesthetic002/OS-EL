"""
Deadlock Panel Component (PyQt6)

Provides interface for deadlock detection and recovery.
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGroupBox, QLabel,
    QPushButton, QComboBox, QTextEdit, QFrame, QMessageBox, QSpinBox
)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QFont, QColor, QTextCharFormat

import sys
import os
import json
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from utils.theme_qt import COLORS, get_font


class StatusIndicator(QFrame):
    """Minimal status indicator widget"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setFixedHeight(40)
        self._setup_ui()
        self.set_status("unknown")
        
    def _setup_ui(self):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(16, 8, 16, 8)
        layout.setSpacing(10)
        
        self.icon_label = QLabel()
        self.icon_label.setFont(get_font('h2', bold=True))
        self.icon_label.setFixedWidth(24)
        self.icon_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.icon_label)
        
        self.text_label = QLabel()
        self.text_label.setFont(get_font('body', bold=True))
        layout.addWidget(self.text_label)
        
        layout.addStretch()
        
    def set_status(self, status):
        """Set status: 'deadlock', 'safe', or 'unknown'"""
        if status == 'deadlock':
            self.setStyleSheet(f"""
                QFrame {{
                    background-color: {COLORS['error']};
                    border-radius: 6px;
                }}
                QLabel {{
                    color: white;
                    background: transparent;
                }}
            """)
            self.icon_label.setText("⚠")
            self.text_label.setText("Deadlock Detected")
        elif status == 'safe':
            self.setStyleSheet(f"""
                QFrame {{
                    background-color: {COLORS['success']};
                    border-radius: 6px;
                }}
                QLabel {{
                    color: white;
                    background: transparent;
                }}
            """)
            self.icon_label.setText("✓")
            self.text_label.setText("System Safe")
        else:
            self.setStyleSheet(f"""
                QFrame {{
                    background-color: {COLORS['bg_tertiary']};
                    border-radius: 6px;
                }}
                QLabel {{
                    color: {COLORS['text_muted']};
                    background: transparent;
                }}
            """)
            self.icon_label.setText("○")
            self.text_label.setText("Not checked")


class DeadlockPanel(QWidget):
    """Deadlock detection and recovery panel"""
    
    updated = pyqtSignal()
    
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self.detection_result = None
        self._setup_ui()
        
    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setSpacing(12)
        layout.setContentsMargins(8, 12, 8, 8)
        
        # Load Scenario section
        scenario_group = QGroupBox("Quick Test")
        scenario_layout = QVBoxLayout(scenario_group)
        scenario_layout.setSpacing(10)
        
        circular_row = QHBoxLayout()
        circular_row.setSpacing(12)
        
        circular_label = QLabel("Circular Wait")
        circular_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        circular_row.addWidget(circular_label)
        
        self.circular_n_input = QSpinBox()
        self.circular_n_input.setRange(3, 8)
        self.circular_n_input.setValue(4)
        self.circular_n_input.setFixedWidth(70)
        circular_row.addWidget(self.circular_n_input)
        
        proc_label = QLabel("processes")
        proc_label.setStyleSheet(f"color: {COLORS['text_muted']};")
        circular_row.addWidget(proc_label)
        circular_row.addStretch()
        
        load_btn = QPushButton("Load")
        load_btn.setFixedWidth(70)
        load_btn.clicked.connect(self._load_scenario)
        circular_row.addWidget(load_btn)
        
        scenario_layout.addLayout(circular_row)
        layout.addWidget(scenario_group)
        
        # Detection section
        detection_group = QGroupBox("Detection")
        detection_layout = QVBoxLayout(detection_group)
        detection_layout.setSpacing(12)
        
        # Status indicator
        self.status_indicator = StatusIndicator()
        detection_layout.addWidget(self.status_indicator)
        
        # Detect button
        detect_btn = QPushButton("Run Detection")
        detect_btn.clicked.connect(self._detect_deadlock)
        detection_layout.addWidget(detect_btn)
        
        # Results text
        self.results_text = QTextEdit()
        self.results_text.setReadOnly(True)
        self.results_text.setMinimumHeight(70)
        self.results_text.setMaximumHeight(90)
        self.results_text.setPlaceholderText("Detection results will appear here...")
        detection_layout.addWidget(self.results_text)
        
        layout.addWidget(detection_group)
        
        # Recovery section
        recovery_group = QGroupBox("Recovery")
        recovery_layout = QVBoxLayout(recovery_group)
        recovery_layout.setSpacing(12)
        
        # Strategy selection
        strategy_row = QHBoxLayout()
        strategy_row.setSpacing(12)
        
        strategy_label = QLabel("Strategy")
        strategy_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        strategy_label.setFixedWidth(60)
        strategy_row.addWidget(strategy_label)
        
        self.strategy_combo = QComboBox()
        self.strategy_combo.addItems([
            "Terminate All",
            "Terminate Lowest Priority",
            "Terminate One",
            "Iterative Termination",
            "Preempt Resources",
            "Rollback"
        ])
        self.strategy_combo.setCurrentIndex(1)
        strategy_row.addWidget(self.strategy_combo)
        recovery_layout.addLayout(strategy_row)
        
        # Buttons
        btn_row = QHBoxLayout()
        btn_row.setSpacing(8)
        
        recommend_btn = QPushButton("Recommend")
        recommend_btn.setProperty("secondary", True)
        recommend_btn.clicked.connect(self._recommend_strategy)
        btn_row.addWidget(recommend_btn)
        
        recover_btn = QPushButton("Execute Recovery")
        recover_btn.clicked.connect(self._recover)
        btn_row.addWidget(recover_btn)
        
        recovery_layout.addLayout(btn_row)
        
        # Recovery results
        self.recovery_text = QTextEdit()
        self.recovery_text.setReadOnly(True)
        self.recovery_text.setMinimumHeight(50)
        self.recovery_text.setMaximumHeight(70)
        self.recovery_text.setPlaceholderText("Recovery results will appear here...")
        recovery_layout.addWidget(self.recovery_text)
        
        layout.addWidget(recovery_group)
        layout.addStretch()
        
    def _load_scenario(self):
        """Load circular wait scenario"""
        try:
            self.backend.sim_init()
            response = self.backend.sim_load_scenario(1)  # 1 = Circular Wait
            if response and response.get('status') == 'success':
                n = self.circular_n_input.value()
                self.results_text.clear()
                self.results_text.setPlainText(f"Loaded circular wait with {n} processes")
                self.status_indicator.set_status("unknown")
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to load: {e}")
        
    def _detect_deadlock(self):
        """Detect deadlock in current RAG"""
        try:
            response = self.backend.detect_deadlock()
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                self.detection_result = data
                deadlock_detected = data.get('deadlock_detected', False)
                
                # Update status indicator
                self.status_indicator.set_status('deadlock' if deadlock_detected else 'safe')
                
                # Display results
                self.results_text.clear()
                
                if deadlock_detected:
                    self.results_text.setStyleSheet(f"color: {COLORS['error']};")
                    self.results_text.append(f"Cycles found: {data.get('cycle_count', 0)}")
                    
                    deadlocked_processes = data.get('deadlocked_processes', [])
                    if deadlocked_processes:
                        pids = ", ".join([f"P{pid}" for pid in deadlocked_processes])
                        self.results_text.append(f"Involved: {pids}")
                else:
                    self.results_text.setStyleSheet(f"color: {COLORS['success']};")
                    self.results_text.append("No deadlock detected")
                    self.results_text.append("System is in a safe state")
                
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to detect: {e}")
            
    def _recommend_strategy(self):
        """Get recommended recovery strategy"""
        try:
            response = self.backend.recommend_strategy()
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                strategy_id = data.get('strategy', 2)
                self.strategy_combo.setCurrentIndex(strategy_id - 1)
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed: {e}")
            
    def _recover(self):
        """Execute recovery"""
        if not self.detection_result or not self.detection_result.get('deadlock_detected', False):
            reply = QMessageBox.question(self, "No Deadlock",
                "No deadlock detected. Proceed anyway?",
                QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            if reply != QMessageBox.StandardButton.Yes:
                return
        
        strategy = self.strategy_combo.currentIndex() + 1
        
        try:
            response = self.backend.recover(strategy, 1)
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                self.recovery_text.clear()
                
                success = data.get('success', False)
                if success:
                    self.recovery_text.setStyleSheet(f"color: {COLORS['success']};")
                    self.recovery_text.append("Recovery successful")
                else:
                    self.recovery_text.setStyleSheet(f"color: {COLORS['warning']};")
                    self.recovery_text.append("Recovery attempted")
                
                terminated = data.get('processes_terminated', 0)
                if terminated > 0:
                    self.recovery_text.append(f"Terminated: {terminated} process(es)")
                
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed: {e}")
            
    def refresh(self):
        """Refresh detection"""
        pass
