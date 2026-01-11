"""
OS-EL Deadlock Detection & Recovery GUI (PyQt6)

Main application entry point for the PyQt6-based GUI.
"""

import sys
import os

from PyQt6.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QSplitter, QTabWidget, QLabel, QMessageBox, QStatusBar,
    QMenuBar, QMenu, QFrame
)
from PyQt6.QtCore import Qt, QTimer
from PyQt6.QtGui import QAction, QFont, QIcon

# Add parent and current directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from backend_interface import BackendInterface
from utils.theme_qt import apply_theme, COLORS, get_font
from components.rag_visualizer_qt import RAGVisualizer
from components.control_panel_qt import ControlPanel
from components.deadlock_panel_qt import DeadlockPanel


class DeadlockGUI(QMainWindow):
    """Main GUI application for deadlock detection and recovery"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("OS-EL | Deadlock Detection & Recovery")
        self.setMinimumSize(1200, 800)
        
        # Center on screen
        screen = QApplication.primaryScreen().geometry()
        x = (screen.width() - 1400) // 2
        y = (screen.height() - 900) // 2
        self.setGeometry(x, y, 1400, 900)
        
        # Initialize backend
        self._init_backend()
        
        # Create UI
        self._create_menu()
        self._create_central_widget()
        self._create_status_bar()
        
        # Initial refresh
        QTimer.singleShot(500, self._refresh_all)
        
    def _init_backend(self):
        """Initialize backend connection"""
        try:
            self.backend = BackendInterface()
            self.backend.start()
            self.backend_connected = True
        except Exception as e:
            self.backend_connected = False
            QMessageBox.critical(self, "Backend Error",
                f"Failed to start backend:\n{e}\n\n"
                "Please ensure the backend executable is built:\n"
                "  mingw32-make\n\n"
                "The GUI will run in offline mode.")
            # Create a dummy backend for testing
            self.backend = None
            
    def _create_menu(self):
        """Create menu bar"""
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("File")
        
        reset_action = QAction("Reset RAG", self)
        reset_action.setShortcut("Ctrl+R")
        reset_action.triggered.connect(self._reset_rag)
        file_menu.addAction(reset_action)
        
        file_menu.addSeparator()
        
        exit_action = QAction("Exit", self)
        exit_action.setShortcut("Ctrl+Q")
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
        # View menu
        view_menu = menubar.addMenu("View")
        
        refresh_action = QAction("Refresh All", self)
        refresh_action.setShortcut("F5")
        refresh_action.triggered.connect(self._refresh_all)
        view_menu.addAction(refresh_action)
        
        # Help menu
        help_menu = menubar.addMenu("Help")
        
        about_action = QAction("About", self)
        about_action.triggered.connect(self._show_about)
        help_menu.addAction(about_action)
        
    def _create_central_widget(self):
        """Create main content area"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        layout = QHBoxLayout(central_widget)
        layout.setContentsMargins(16, 16, 16, 16)
        layout.setSpacing(12)
        
        # Create splitter for resizable panels
        splitter = QSplitter(Qt.Orientation.Horizontal)
        layout.addWidget(splitter)
        
        # Left panel: RAG Visualizer (takes more space)
        if self.backend:
            self.rag_visualizer = RAGVisualizer(self.backend)
        else:
            self.rag_visualizer = QLabel("Backend not connected")
            self.rag_visualizer.setAlignment(Qt.AlignmentFlag.AlignCenter)
            self.rag_visualizer.setStyleSheet(f"""
                background-color: {COLORS['bg_secondary']};
                border: 1px solid {COLORS['border']};
                border-radius: 8px;
                color: {COLORS['text_muted']};
                font-size: 14pt;
            """)
        splitter.addWidget(self.rag_visualizer)
        
        # Right panel: Controls container
        right_panel = QWidget()
        right_layout = QVBoxLayout(right_panel)
        right_layout.setContentsMargins(0, 0, 0, 0)
        right_layout.setSpacing(0)
        
        # Tab widget (main tabs)
        self.tabs = QTabWidget()
        right_layout.addWidget(self.tabs)
        
        if self.backend:
            # Control Panel
            self.control_panel = ControlPanel(self.backend)
            self.control_panel.updated.connect(self._refresh_all)
            self.tabs.addTab(self.control_panel, "Control")
            
            # Deadlock Panel
            self.deadlock_panel = DeadlockPanel(self.backend)
            self.deadlock_panel.updated.connect(self._refresh_all)
            self.tabs.addTab(self.deadlock_panel, "Deadlock")
        else:
            placeholder = QLabel("Backend not connected.\nPlease build the backend and restart.")
            placeholder.setAlignment(Qt.AlignmentFlag.AlignCenter)
            placeholder.setStyleSheet(f"color: {COLORS['text_muted']};")
            self.tabs.addTab(placeholder, "Offline")
        
        splitter.addWidget(right_panel)
        
        # Set splitter sizes (70% visualizer, 30% controls)
        splitter.setSizes([700, 300])
        
    def _create_status_bar(self):
        """Create status bar"""
        status_bar = QStatusBar()
        self.setStatusBar(status_bar)
        
        # Connection status
        if self.backend_connected:
            status_label = QLabel("● Connected")
            status_label.setStyleSheet(f"color: {COLORS['success']}; padding: 4px 8px;")
        else:
            status_label = QLabel("○ Disconnected")
            status_label.setStyleSheet(f"color: {COLORS['error']}; padding: 4px 8px;")
        status_bar.addWidget(status_label)
        
        # Spacer
        status_bar.addPermanentWidget(QLabel(""))
        
        # Version
        version_label = QLabel("OS-EL v1.0")
        version_label.setStyleSheet(f"color: {COLORS['text_muted']}; padding: 4px 8px;")
        status_bar.addPermanentWidget(version_label)
        
    def _refresh_all(self):
        """Refresh all panels"""
        if self.backend:
            if hasattr(self, 'rag_visualizer') and hasattr(self.rag_visualizer, 'refresh'):
                self.rag_visualizer.refresh()
            if hasattr(self, 'control_panel'):
                self.control_panel.refresh()
                
    def _reset_rag(self):
        """Reset the RAG to empty state"""
        if not self.backend:
            return
            
        reply = QMessageBox.question(self, "Confirm Reset",
            "Reset the Resource Allocation Graph?\n"
            "This will clear all processes, resources, and edges.",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
            
        if reply == QMessageBox.StandardButton.Yes:
            try:
                self.backend.rag_reset()
                self._refresh_all()
            except Exception as e:
                QMessageBox.critical(self, "Error", f"Failed to reset: {e}")
                
    def _show_about(self):
        """Show about dialog"""
        QMessageBox.about(self, "About OS-EL",
            "<h2>OS-EL</h2>"
            "<p><b>Deadlock Detection & Recovery System</b></p>"
            "<p>An Operating System Experimental Lab module for understanding "
            "deadlock detection and recovery using Resource Allocation Graphs.</p>"
            "<hr>"
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>Visual RAG display with interactive nodes</li>"
            "<li>DFS-based cycle detection</li>"
            "<li>Multiple recovery strategies</li>"
            "<li>Test scenarios</li>"
            "</ul>"
            "<p><small>Technologies: C backend, PyQt6 GUI</small></p>")
            
    def closeEvent(self, event):
        """Handle window close"""
        if self.backend:
            try:
                self.backend.stop()
            except:
                pass
        event.accept()


def main():
    """Main entry point"""
    # Create application
    app = QApplication(sys.argv)
    app.setApplicationName("OS-EL Deadlock Detection")
    
    # Apply dark theme
    apply_theme(app)
    
    # Create and show main window
    window = DeadlockGUI()
    window.show()
    
    # Run event loop
    sys.exit(app.exec())


if __name__ == "__main__":
    main()
