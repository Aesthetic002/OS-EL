"""
Control Panel Component (PyQt6)

Provides interface for managing processes, resources, and edges.
"""

from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QTabWidget, QGroupBox,
    QLabel, QLineEdit, QSpinBox, QPushButton, QTableWidget,
    QTableWidgetItem, QHeaderView, QMessageBox, QFrame, QSizePolicy
)
from PyQt6.QtCore import Qt, pyqtSignal
from PyQt6.QtGui import QFont

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from utils.theme_qt import COLORS, get_font


class ControlPanel(QWidget):
    """Control panel for process/resource management"""
    
    updated = pyqtSignal()
    
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        self._setup_ui()
        
    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(8, 8, 8, 8)
        layout.setSpacing(0)
        
        # Tab widget for sub-tabs (set objectName for styling)
        self.tabs = QTabWidget()
        self.tabs.setObjectName("subTabs")
        layout.addWidget(self.tabs)
        
        # Process tab
        process_tab = self._create_process_tab()
        self.tabs.addTab(process_tab, "Processes")
        
        # Resource tab
        resource_tab = self._create_resource_tab()
        self.tabs.addTab(resource_tab, "Resources")
        
        # Edge tab
        edge_tab = self._create_edge_tab()
        self.tabs.addTab(edge_tab, "Edges")
        
    def _create_process_tab(self):
        """Create process management tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(12)
        layout.setContentsMargins(8, 12, 8, 8)
        
        # Add Process section
        add_group = QGroupBox("Add Process")
        add_layout = QVBoxLayout(add_group)
        add_layout.setSpacing(12)
        
        # Name row
        name_row = QHBoxLayout()
        name_row.setSpacing(12)
        name_label = QLabel("Name")
        name_label.setFixedWidth(60)
        name_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.process_name_input = QLineEdit()
        self.process_name_input.setPlaceholderText("Enter process name...")
        name_row.addWidget(name_label)
        name_row.addWidget(self.process_name_input)
        add_layout.addLayout(name_row)
        
        # Priority row
        priority_row = QHBoxLayout()
        priority_row.setSpacing(12)
        priority_label = QLabel("Priority")
        priority_label.setFixedWidth(60)
        priority_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.process_priority_input = QSpinBox()
        self.process_priority_input.setRange(0, 100)
        self.process_priority_input.setValue(50)
        self.process_priority_input.setFixedWidth(100)
        priority_row.addWidget(priority_label)
        priority_row.addWidget(self.process_priority_input)
        priority_row.addStretch()
        add_layout.addLayout(priority_row)
        
        # Add button
        add_btn = QPushButton("Add Process")
        add_btn.clicked.connect(self._add_process)
        add_layout.addWidget(add_btn)
        
        layout.addWidget(add_group)
        
        # Process List section
        list_group = QGroupBox("Process List")
        list_layout = QVBoxLayout(list_group)
        list_layout.setSpacing(12)
        
        # Table
        self.process_table = QTableWidget()
        self.process_table.setColumnCount(4)
        self.process_table.setHorizontalHeaderLabels(["ID", "Name", "Priority", "State"])
        self.process_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        self.process_table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
        self.process_table.setAlternatingRowColors(True)
        self.process_table.verticalHeader().setVisible(False)
        self.process_table.setShowGrid(False)
        list_layout.addWidget(self.process_table)
        
        # Buttons
        btn_row = QHBoxLayout()
        btn_row.setSpacing(8)
        
        remove_btn = QPushButton("Remove Selected")
        remove_btn.setProperty("danger", True)
        remove_btn.clicked.connect(self._remove_process)
        btn_row.addWidget(remove_btn)
        
        refresh_btn = QPushButton("Refresh")
        refresh_btn.setProperty("secondary", True)
        refresh_btn.clicked.connect(self._refresh_processes)
        btn_row.addWidget(refresh_btn)
        
        list_layout.addLayout(btn_row)
        layout.addWidget(list_group)
        
        return tab
        
    def _create_resource_tab(self):
        """Create resource management tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(12)
        layout.setContentsMargins(8, 12, 8, 8)
        
        # Add Resource section
        add_group = QGroupBox("Add Resource")
        add_layout = QVBoxLayout(add_group)
        add_layout.setSpacing(12)
        
        # Name row
        name_row = QHBoxLayout()
        name_row.setSpacing(12)
        name_label = QLabel("Name")
        name_label.setFixedWidth(60)
        name_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.resource_name_input = QLineEdit()
        self.resource_name_input.setPlaceholderText("Enter resource name...")
        name_row.addWidget(name_label)
        name_row.addWidget(self.resource_name_input)
        add_layout.addLayout(name_row)
        
        # Instances row
        instances_row = QHBoxLayout()
        instances_row.setSpacing(12)
        instances_label = QLabel("Instances")
        instances_label.setFixedWidth(60)
        instances_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.resource_instances_input = QSpinBox()
        self.resource_instances_input.setRange(1, 100)
        self.resource_instances_input.setValue(1)
        self.resource_instances_input.setFixedWidth(100)
        instances_row.addWidget(instances_label)
        instances_row.addWidget(self.resource_instances_input)
        instances_row.addStretch()
        add_layout.addLayout(instances_row)
        
        # Add button
        add_btn = QPushButton("Add Resource")
        add_btn.clicked.connect(self._add_resource)
        add_layout.addWidget(add_btn)
        
        layout.addWidget(add_group)
        
        # Resource List section
        list_group = QGroupBox("Resource List")
        list_layout = QVBoxLayout(list_group)
        list_layout.setSpacing(12)
        
        # Table
        self.resource_table = QTableWidget()
        self.resource_table.setColumnCount(4)
        self.resource_table.setHorizontalHeaderLabels(["ID", "Name", "Total", "Available"])
        self.resource_table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)
        self.resource_table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
        self.resource_table.setAlternatingRowColors(True)
        self.resource_table.verticalHeader().setVisible(False)
        self.resource_table.setShowGrid(False)
        list_layout.addWidget(self.resource_table)
        
        # Buttons
        btn_row = QHBoxLayout()
        btn_row.setSpacing(8)
        
        remove_btn = QPushButton("Remove Selected")
        remove_btn.setProperty("danger", True)
        remove_btn.clicked.connect(self._remove_resource)
        btn_row.addWidget(remove_btn)
        
        refresh_btn = QPushButton("Refresh")
        refresh_btn.setProperty("secondary", True)
        refresh_btn.clicked.connect(self._refresh_resources)
        btn_row.addWidget(refresh_btn)
        
        list_layout.addLayout(btn_row)
        layout.addWidget(list_group)
        
        return tab
        
    def _create_edge_tab(self):
        """Create edge operations tab"""
        tab = QWidget()
        layout = QVBoxLayout(tab)
        layout.setSpacing(12)
        layout.setContentsMargins(8, 12, 8, 8)
        
        # Request Resource section
        request_group = QGroupBox("Request Resource")
        request_layout = QVBoxLayout(request_group)
        request_layout.setSpacing(10)
        
        req_row1 = QHBoxLayout()
        req_row1.setSpacing(12)
        pid_label = QLabel("Process ID")
        pid_label.setFixedWidth(80)
        pid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.request_pid_input = QSpinBox()
        self.request_pid_input.setRange(0, 999)
        self.request_pid_input.setFixedWidth(100)
        req_row1.addWidget(pid_label)
        req_row1.addWidget(self.request_pid_input)
        req_row1.addStretch()
        request_layout.addLayout(req_row1)
        
        req_row2 = QHBoxLayout()
        req_row2.setSpacing(12)
        rid_label = QLabel("Resource ID")
        rid_label.setFixedWidth(80)
        rid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.request_rid_input = QSpinBox()
        self.request_rid_input.setRange(0, 999)
        self.request_rid_input.setFixedWidth(100)
        req_row2.addWidget(rid_label)
        req_row2.addWidget(self.request_rid_input)
        req_row2.addStretch()
        request_layout.addLayout(req_row2)
        
        request_btn = QPushButton("Request")
        request_btn.clicked.connect(self._request_resource)
        request_layout.addWidget(request_btn)
        
        layout.addWidget(request_group)
        
        # Allocate Resource section
        allocate_group = QGroupBox("Allocate Resource")
        allocate_layout = QVBoxLayout(allocate_group)
        allocate_layout.setSpacing(10)
        
        alloc_row1 = QHBoxLayout()
        alloc_row1.setSpacing(12)
        alloc_pid_label = QLabel("Process ID")
        alloc_pid_label.setFixedWidth(80)
        alloc_pid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.allocate_pid_input = QSpinBox()
        self.allocate_pid_input.setRange(0, 999)
        self.allocate_pid_input.setFixedWidth(100)
        alloc_row1.addWidget(alloc_pid_label)
        alloc_row1.addWidget(self.allocate_pid_input)
        alloc_row1.addStretch()
        allocate_layout.addLayout(alloc_row1)
        
        alloc_row2 = QHBoxLayout()
        alloc_row2.setSpacing(12)
        alloc_rid_label = QLabel("Resource ID")
        alloc_rid_label.setFixedWidth(80)
        alloc_rid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.allocate_rid_input = QSpinBox()
        self.allocate_rid_input.setRange(0, 999)
        self.allocate_rid_input.setFixedWidth(100)
        alloc_row2.addWidget(alloc_rid_label)
        alloc_row2.addWidget(self.allocate_rid_input)
        alloc_row2.addStretch()
        allocate_layout.addLayout(alloc_row2)
        
        allocate_btn = QPushButton("Allocate")
        allocate_btn.setProperty("success", True)
        allocate_btn.clicked.connect(self._allocate_resource)
        allocate_layout.addWidget(allocate_btn)
        
        layout.addWidget(allocate_group)
        
        # Release Resource section
        release_group = QGroupBox("Release Resource")
        release_layout = QVBoxLayout(release_group)
        release_layout.setSpacing(10)
        
        rel_row1 = QHBoxLayout()
        rel_row1.setSpacing(12)
        rel_pid_label = QLabel("Process ID")
        rel_pid_label.setFixedWidth(80)
        rel_pid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.release_pid_input = QSpinBox()
        self.release_pid_input.setRange(0, 999)
        self.release_pid_input.setFixedWidth(100)
        rel_row1.addWidget(rel_pid_label)
        rel_row1.addWidget(self.release_pid_input)
        rel_row1.addStretch()
        release_layout.addLayout(rel_row1)
        
        rel_row2 = QHBoxLayout()
        rel_row2.setSpacing(12)
        rel_rid_label = QLabel("Resource ID")
        rel_rid_label.setFixedWidth(80)
        rel_rid_label.setStyleSheet(f"color: {COLORS['text_secondary']}; font-weight: 500;")
        self.release_rid_input = QSpinBox()
        self.release_rid_input.setRange(0, 999)
        self.release_rid_input.setFixedWidth(100)
        rel_row2.addWidget(rel_rid_label)
        rel_row2.addWidget(self.release_rid_input)
        rel_row2.addStretch()
        release_layout.addLayout(rel_row2)
        
        rel_btn_row = QHBoxLayout()
        rel_btn_row.setSpacing(8)
        
        release_btn = QPushButton("Release")
        release_btn.clicked.connect(self._release_resource)
        rel_btn_row.addWidget(release_btn)
        
        release_all_btn = QPushButton("Release All")
        release_all_btn.setProperty("secondary", True)
        release_all_btn.clicked.connect(self._release_all)
        rel_btn_row.addWidget(release_all_btn)
        
        release_layout.addLayout(rel_btn_row)
        layout.addWidget(release_group)
        
        layout.addStretch()
        
        return tab
        
    # ========================================================================
    # Process Operations
    # ========================================================================
    
    def _add_process(self):
        """Add a new process"""
        name = self.process_name_input.text().strip()
        priority = self.process_priority_input.value()
        
        if not name:
            QMessageBox.warning(self, "Error", "Please enter a process name")
            return
            
        try:
            response = self.backend.add_process(name, priority)
            if response and response.get('status') == 'success':
                self.process_name_input.clear()
                self._refresh_processes()
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error", 
                    f"Failed to add process: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to add process: {e}")
            
    def _remove_process(self):
        """Remove selected process"""
        selected = self.process_table.selectedItems()
        if not selected:
            QMessageBox.warning(self, "Warning", "Please select a process to remove")
            return
            
        row = selected[0].row()
        pid = int(self.process_table.item(row, 0).text())
        
        reply = QMessageBox.question(self, "Confirm", f"Remove process P{pid}?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        if reply != QMessageBox.StandardButton.Yes:
            return
            
        try:
            response = self.backend.remove_process(pid)
            if response and response.get('status') == 'success':
                self._refresh_processes()
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed to remove process: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to remove process: {e}")
            
    def _refresh_processes(self):
        """Refresh process list"""
        self.process_table.setRowCount(0)
        
        try:
            response = self.backend.list_processes()
            if response and response.get('status') == 'success':
                import json
                data = response.get('data', '[]')
                if isinstance(data, str):
                    processes = json.loads(data)
                else:
                    processes = data
                    
                for process in processes:
                    row = self.process_table.rowCount()
                    self.process_table.insertRow(row)
                    
                    # Center-align items
                    for col, value in enumerate([
                        str(process['id']),
                        process['name'],
                        str(process['priority']),
                        process['state'].upper()
                    ]):
                        item = QTableWidgetItem(value)
                        item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                        self.process_table.setItem(row, col, item)
        except Exception as e:
            print(f"Error refreshing processes: {e}")
            
    # ========================================================================
    # Resource Operations
    # ========================================================================
    
    def _add_resource(self):
        """Add a new resource"""
        name = self.resource_name_input.text().strip()
        instances = self.resource_instances_input.value()
        
        if not name:
            QMessageBox.warning(self, "Error", "Please enter a resource name")
            return
            
        try:
            response = self.backend.add_resource(name, instances)
            if response and response.get('status') == 'success':
                self.resource_name_input.clear()
                self._refresh_resources()
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed to add resource: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to add resource: {e}")
            
    def _remove_resource(self):
        """Remove selected resource"""
        selected = self.resource_table.selectedItems()
        if not selected:
            QMessageBox.warning(self, "Warning", "Please select a resource to remove")
            return
            
        row = selected[0].row()
        rid = int(self.resource_table.item(row, 0).text())
        
        reply = QMessageBox.question(self, "Confirm", f"Remove resource R{rid}?",
            QMessageBox.StandardButton.Yes | QMessageBox.StandardButton.No)
        if reply != QMessageBox.StandardButton.Yes:
            return
            
        try:
            response = self.backend.remove_resource(rid)
            if response and response.get('status') == 'success':
                self._refresh_resources()
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed to remove resource: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to remove resource: {e}")
            
    def _refresh_resources(self):
        """Refresh resource list"""
        self.resource_table.setRowCount(0)
        
        try:
            response = self.backend.list_resources()
            if response and response.get('status') == 'success':
                import json
                data = response.get('data', '[]')
                if isinstance(data, str):
                    resources = json.loads(data)
                else:
                    resources = data
                    
                for resource in resources:
                    row = self.resource_table.rowCount()
                    self.resource_table.insertRow(row)
                    
                    for col, value in enumerate([
                        str(resource['id']),
                        resource['name'],
                        str(resource['total_instances']),
                        str(resource['available_instances'])
                    ]):
                        item = QTableWidgetItem(value)
                        item.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                        self.resource_table.setItem(row, col, item)
        except Exception as e:
            print(f"Error refreshing resources: {e}")
            
    # ========================================================================
    # Edge Operations
    # ========================================================================
    
    def _request_resource(self):
        """Create a request edge"""
        pid = self.request_pid_input.value()
        rid = self.request_rid_input.value()
        
        try:
            response = self.backend.request_resource(pid, rid)
            if response and response.get('status') == 'success':
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to request resource: {e}")
            
    def _allocate_resource(self):
        """Allocate a resource to a process"""
        pid = self.allocate_pid_input.value()
        rid = self.allocate_rid_input.value()
        
        try:
            response = self.backend.allocate_resource(pid, rid)
            if response and response.get('status') == 'success':
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to allocate resource: {e}")
            
    def _release_resource(self):
        """Release a resource from a process"""
        pid = self.release_pid_input.value()
        rid = self.release_rid_input.value()
        
        try:
            response = self.backend.release_resource(pid, rid)
            if response and response.get('status') == 'success':
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to release resource: {e}")
            
    def _release_all(self):
        """Release all resources for a process"""
        pid = self.release_pid_input.value()
        
        try:
            response = self.backend.release_all(pid)
            if response and response.get('status') == 'success':
                self.updated.emit()
            else:
                QMessageBox.warning(self, "Error",
                    f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            QMessageBox.critical(self, "Error", f"Failed to release resources: {e}")
            
    def refresh(self):
        """Refresh all lists"""
        self._refresh_processes()
        self._refresh_resources()
