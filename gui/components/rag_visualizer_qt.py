"""
RAG Visualizer Component (PyQt6)

Provides visual representation of the Resource Allocation Graph using QGraphicsView.
Features smooth animations, glow effects, and interactive node dragging.
"""

import math
from PyQt6.QtWidgets import (
    QWidget, QVBoxLayout, QHBoxLayout, QGraphicsView, QGraphicsScene,
    QGraphicsEllipseItem, QGraphicsRectItem, QGraphicsLineItem,
    QGraphicsTextItem, QGraphicsDropShadowEffect, QPushButton,
    QLabel, QButtonGroup, QRadioButton, QFrame, QGraphicsItem
)
from PyQt6.QtCore import Qt, QRectF, QPointF, QLineF, pyqtSignal, QTimer
from PyQt6.QtGui import (
    QPen, QBrush, QColor, QPainter, QFont, QPainterPath,
    QRadialGradient, QLinearGradient
)

import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from utils.theme_qt import COLORS, get_font
from utils.graph_layout import GraphLayout


class ProcessNode(QGraphicsEllipseItem):
    """Process node (circle) with subtle glow effect"""
    
    def __init__(self, pid, name, x, y, radius=28, is_deadlocked=False):
        super().__init__(-radius, -radius, radius * 2, radius * 2)
        self.pid = pid
        self.name = name
        self.radius = radius
        self.is_deadlocked = is_deadlocked
        
        self.setPos(x, y)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemSendsGeometryChanges, True)
        self.setAcceptHoverEvents(True)
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        self.setZValue(10)
        
        self._setup_appearance()
        self._create_label()
        
    def _setup_appearance(self):
        """Setup node appearance with clean style"""
        color = QColor(COLORS['deadlock'] if self.is_deadlocked else COLORS['node_process'])
        
        # Simple solid fill
        self.setBrush(QBrush(color))
        self.setPen(QPen(color.lighter(120), 2))
        
        # Subtle shadow
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(color)
        shadow.setBlurRadius(12 if self.is_deadlocked else 8)
        shadow.setOffset(0, 2)
        self.setGraphicsEffect(shadow)
        
    def _create_label(self):
        """Create text label for the node"""
        self.label = QGraphicsTextItem(self)
        self.label.setPlainText(f"P{self.pid}")
        self.label.setFont(get_font('body', bold=True))
        self.label.setDefaultTextColor(QColor(COLORS['text_white']))
        
        # Center the label
        rect = self.label.boundingRect()
        self.label.setPos(-rect.width() / 2, -rect.height() / 2)
        
    def set_deadlocked(self, is_deadlocked):
        """Update deadlock state"""
        if self.is_deadlocked != is_deadlocked:
            self.is_deadlocked = is_deadlocked
            self._setup_appearance()
            
    def itemChange(self, change, value):
        """Handle position changes for edge updates"""
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged:
            scene = self.scene()
            if scene and hasattr(scene, 'update_edges'):
                scene.update_edges()
        return super().itemChange(change, value)
        
    def hoverEnterEvent(self, event):
        self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().hoverEnterEvent(event)
        
    def hoverLeaveEvent(self, event):
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().hoverLeaveEvent(event)


class ResourceNode(QGraphicsRectItem):
    """Resource node (rectangle) with clean style"""
    
    def __init__(self, rid, name, total, available, x, y, size=48, is_deadlocked=False):
        half = size / 2
        super().__init__(-half, -half, size, size)
        self.rid = rid
        self.name = name
        self.total = total
        self.available = available
        self.size = size
        self.is_deadlocked = is_deadlocked
        
        self.setPos(x, y)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsMovable, True)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemSendsGeometryChanges, True)
        self.setAcceptHoverEvents(True)
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        self.setZValue(10)
        
        self._setup_appearance()
        self._create_label()
        
    def _setup_appearance(self):
        """Setup node appearance with clean style"""
        color = QColor(COLORS['deadlock'] if self.is_deadlocked else COLORS['node_resource'])
        
        # Simple solid fill
        self.setBrush(QBrush(color))
        self.setPen(QPen(color.lighter(120), 2))
        
        # Subtle shadow
        shadow = QGraphicsDropShadowEffect()
        shadow.setColor(color)
        shadow.setBlurRadius(12 if self.is_deadlocked else 8)
        shadow.setOffset(0, 2)
        self.setGraphicsEffect(shadow)
        
    def _create_label(self):
        """Create text label for the node"""
        self.label = QGraphicsTextItem(self)
        self.label.setPlainText(f"R{self.rid}\n{self.available}/{self.total}")
        self.label.setFont(get_font('small', bold=True))
        self.label.setDefaultTextColor(QColor(COLORS['text_white']))
        
        # Center the label
        rect = self.label.boundingRect()
        self.label.setPos(-rect.width() / 2, -rect.height() / 2)
        
    def set_deadlocked(self, is_deadlocked):
        """Update deadlock state"""
        if self.is_deadlocked != is_deadlocked:
            self.is_deadlocked = is_deadlocked
            self._setup_appearance()
            
    def update_info(self, total, available):
        """Update resource instance counts"""
        self.total = total
        self.available = available
        self.label.setPlainText(f"R{self.rid}\n{self.available}/{self.total}")
        rect = self.label.boundingRect()
        self.label.setPos(-rect.width() / 2, -rect.height() / 2)
            
    def itemChange(self, change, value):
        """Handle position changes for edge updates"""
        if change == QGraphicsItem.GraphicsItemChange.ItemPositionHasChanged:
            scene = self.scene()
            if scene and hasattr(scene, 'update_edges'):
                scene.update_edges()
        return super().itemChange(change, value)
        
    def hoverEnterEvent(self, event):
        self.setCursor(Qt.CursorShape.ClosedHandCursor)
        super().hoverEnterEvent(event)
        
    def hoverLeaveEvent(self, event):
        self.setCursor(Qt.CursorShape.OpenHandCursor)
        super().hoverLeaveEvent(event)


class EdgeItem(QGraphicsLineItem):
    """Edge between nodes (request or assignment)"""
    
    def __init__(self, from_node, to_node, edge_type='request', label=''):
        super().__init__()
        self.from_node = from_node
        self.to_node = to_node
        self.edge_type = edge_type
        self.edge_label = label
        
        self.setZValue(5)
        self._setup_appearance()
        self._create_label()
        self.update_position()
        
    def _setup_appearance(self):
        """Setup edge appearance based on type"""
        if self.edge_type == 'request':
            color = QColor(COLORS['edge_request'])
            pen = QPen(color, 2, Qt.PenStyle.DashLine)
        else:  # assignment
            color = QColor(COLORS['edge_assignment'])
            pen = QPen(color, 2, Qt.PenStyle.SolidLine)
            
        self.setPen(pen)
        self.color = color
        
    def _create_label(self):
        """Create label for the edge"""
        self.label = QGraphicsTextItem(self)
        self.label.setPlainText(self.edge_label)
        self.label.setFont(get_font('tiny'))
        self.label.setDefaultTextColor(self.color)
        
    def update_position(self):
        """Update edge position based on node positions"""
        if not self.from_node or not self.to_node:
            return
            
        start = self.from_node.scenePos()
        end = self.to_node.scenePos()
        
        # Calculate direction
        dx = end.x() - start.x()
        dy = end.y() - start.y()
        length = math.sqrt(dx * dx + dy * dy)
        
        if length < 1:
            return
            
        # Normalize
        dx /= length
        dy /= length
        
        # Get node radii
        from_radius = getattr(self.from_node, 'radius', 24)
        to_radius = getattr(self.to_node, 'radius', 24)
        if hasattr(self.to_node, 'size'):
            to_radius = self.to_node.size / 2
        if hasattr(self.from_node, 'size'):
            from_radius = self.from_node.size / 2
        
        # Adjust start and end points to node edges
        start_x = start.x() + dx * from_radius
        start_y = start.y() + dy * from_radius
        end_x = end.x() - dx * to_radius
        end_y = end.y() - dy * to_radius
        
        self.setLine(start_x, start_y, end_x, end_y)
        
        # Position label at midpoint
        mid_x = (start_x + end_x) / 2
        mid_y = (start_y + end_y) / 2 - 10
        rect = self.label.boundingRect()
        self.label.setPos(mid_x - rect.width() / 2, mid_y - rect.height() / 2)
        
    def _draw_arrow_head(self, x, y, dx, dy):
        """Draw arrow head at the end of the edge"""
        pass


class RAGScene(QGraphicsScene):
    """Custom scene for the RAG visualization"""
    
    def __init__(self):
        super().__init__()
        self.process_nodes = {}  # pid -> ProcessNode
        self.resource_nodes = {}  # rid -> ResourceNode
        self.edges = []  # List of EdgeItem
        
        self.setBackgroundBrush(QBrush(QColor(COLORS['bg_primary'])))
        
    def update_edges(self):
        """Update all edge positions"""
        for edge in self.edges:
            edge.update_position()
            
    def clear_all(self):
        """Clear all nodes and edges"""
        self.clear()
        self.process_nodes = {}
        self.resource_nodes = {}
        self.edges = []


class RAGVisualizer(QWidget):
    """Resource Allocation Graph Visualizer Widget"""
    
    updated = pyqtSignal()
    
    def __init__(self, backend, parent=None):
        super().__init__(parent)
        self.backend = backend
        
        # Data
        self.processes = []
        self.resources = []
        self.requests = []
        self.assignments = []
        self.deadlocked_processes = set()
        self.deadlocked_resources = set()
        
        # Layout
        self.layout_algo = GraphLayout(width=800, height=600)
        self.current_layout = 'force'
        
        self._setup_ui()
        
    def _setup_ui(self):
        """Setup the UI"""
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(8)
        
        # Toolbar
        toolbar = self._create_toolbar()
        layout.addWidget(toolbar)
        
        # Graphics View
        self.scene = RAGScene()
        self.view = QGraphicsView(self.scene)
        self.view.setRenderHint(QPainter.RenderHint.Antialiasing)
        self.view.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
        self.view.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
        self.view.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.view.setResizeAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.view.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self.view.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        layout.addWidget(self.view, 1)
        
        # Legend
        legend = self._create_legend()
        layout.addWidget(legend)
        
    def _create_toolbar(self):
        """Create the toolbar with layout options"""
        toolbar = QFrame()
        toolbar.setStyleSheet(f"""
            QFrame {{
                background-color: {COLORS['bg_secondary']};
                border-radius: 8px;
            }}
        """)
        toolbar_layout = QHBoxLayout(toolbar)
        toolbar_layout.setContentsMargins(16, 10, 16, 10)
        toolbar_layout.setSpacing(12)
        
        # Layout label
        label = QLabel("Layout")
        label.setStyleSheet(f"color: {COLORS['text_muted']}; font-weight: 500;")
        toolbar_layout.addWidget(label)
        
        # Layout radio buttons
        self.layout_group = QButtonGroup(self)
        
        layouts = [
            ("Force", "force"),
            ("Circular", "circular"),
            ("Hierarchical", "hierarchical")
        ]
        
        for text, value in layouts:
            radio = QRadioButton(text)
            radio.setProperty("layout_value", value)
            radio.setStyleSheet(f"""
                QRadioButton {{
                    color: {COLORS['text_secondary']};
                    spacing: 6px;
                    padding: 6px 12px;
                    background-color: {COLORS['bg_tertiary']};
                    border: 1px solid {COLORS['border']};
                    border-radius: 6px;
                }}
                QRadioButton::indicator {{
                    width: 12px;
                    height: 12px;
                    border-radius: 6px;
                    border: 2px solid {COLORS['border_light']};
                    background-color: transparent;
                }}
                QRadioButton::indicator:checked {{
                    background-color: {COLORS['primary']};
                    border-color: {COLORS['primary']};
                }}
                QRadioButton:hover {{
                    color: {COLORS['text_primary']};
                    border-color: {COLORS['primary']};
                }}
            """)
            if value == 'force':
                radio.setChecked(True)
            radio.toggled.connect(self._on_layout_changed)
            self.layout_group.addButton(radio)
            toolbar_layout.addWidget(radio)
            
        toolbar_layout.addStretch()
        
        # Button style for toolbar - consistent height
        btn_style = f"""
            QPushButton {{
                background-color: {COLORS['bg_tertiary']};
                color: {COLORS['text_secondary']};
                border: 1px solid {COLORS['border']};
                border-radius: 6px;
                padding: 6px 14px;
                font-weight: 500;
                min-height: 18px;
            }}
            QPushButton:hover {{
                background-color: {COLORS['bg_hover']};
                color: {COLORS['text_primary']};
                border-color: {COLORS['primary']};
            }}
        """
        
        # Refresh button
        refresh_btn = QPushButton("Refresh")
        refresh_btn.setStyleSheet(btn_style)
        refresh_btn.clicked.connect(self.refresh)
        toolbar_layout.addWidget(refresh_btn)
        
        # Zoom buttons - same style but narrower
        zoom_btn_style = f"""
            QPushButton {{
                background-color: {COLORS['bg_tertiary']};
                color: {COLORS['text_secondary']};
                border: 1px solid {COLORS['border']};
                border-radius: 6px;
                padding: 6px 10px;
                font-weight: 600;
                min-height: 18px;
                min-width: 28px;
            }}
            QPushButton:hover {{
                background-color: {COLORS['bg_hover']};
                color: {COLORS['text_primary']};
                border-color: {COLORS['primary']};
            }}
        """
        
        zoom_in_btn = QPushButton("+")
        zoom_in_btn.setStyleSheet(zoom_btn_style)
        zoom_in_btn.clicked.connect(lambda: self.view.scale(1.2, 1.2))
        toolbar_layout.addWidget(zoom_in_btn)
        
        zoom_out_btn = QPushButton("−")
        zoom_out_btn.setStyleSheet(zoom_btn_style)
        zoom_out_btn.clicked.connect(lambda: self.view.scale(1/1.2, 1/1.2))
        toolbar_layout.addWidget(zoom_out_btn)
        
        reset_btn = QPushButton("Fit")
        reset_btn.setStyleSheet(btn_style)
        reset_btn.clicked.connect(self._fit_view)
        toolbar_layout.addWidget(reset_btn)
        
        return toolbar
        
    def _create_legend(self):
        """Create the legend"""
        legend = QFrame()
        legend.setStyleSheet(f"""
            QFrame {{
                background-color: {COLORS['bg_secondary']};
                border-radius: 8px;
            }}
        """)
        legend_layout = QHBoxLayout(legend)
        legend_layout.setContentsMargins(16, 10, 16, 10)
        legend_layout.setSpacing(24)
        
        # Legend items
        items = [
            ("●", COLORS['node_process'], "Process"),
            ("■", COLORS['node_resource'], "Resource"),
            ("- -", COLORS['edge_request'], "Request"),
            ("—", COLORS['edge_assignment'], "Allocated"),
            ("●", COLORS['deadlock'], "Deadlocked"),
        ]
        
        for symbol, color, text in items:
            item_layout = QHBoxLayout()
            item_layout.setSpacing(6)
            
            symbol_label = QLabel(symbol)
            symbol_label.setStyleSheet(f"color: {color}; font-size: 12pt; font-weight: bold;")
            item_layout.addWidget(symbol_label)
            
            text_label = QLabel(text)
            text_label.setStyleSheet(f"color: {COLORS['text_muted']}; font-size: 9pt;")
            item_layout.addWidget(text_label)
            
            legend_layout.addLayout(item_layout)
            
        legend_layout.addStretch()
        
        return legend
        
    def _on_layout_changed(self, checked):
        """Handle layout change"""
        if checked:
            button = self.sender()
            self.current_layout = button.property("layout_value")
            self._relayout()
            
    def _fit_view(self):
        """Fit the view to show all content"""
        self.view.resetTransform()
        self.view.fitInView(self.scene.sceneRect(), Qt.AspectRatioMode.KeepAspectRatio)
        
    def refresh(self):
        """Refresh RAG data from backend"""
        try:
            response = self.backend.rag_get_state()
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    import json
                    data = json.loads(data)
                
                self.processes = data.get('processes', [])
                self.resources = data.get('resources', [])
                self.requests = [(r['process'], r['resource']) 
                               for r in data.get('requests', [])]
                self.assignments = [(a['resource'], a['process'], a.get('count', 1))
                                  for a in data.get('assignments', [])]
                
                # Get deadlock status
                try:
                    det_response = self.backend.detect_deadlock()
                    if det_response and det_response.get('status') == 'success':
                        det_data = det_response.get('data', {})
                        if isinstance(det_data, str):
                            import json
                            det_data = json.loads(det_data)
                        
                        self.deadlocked_processes = set(det_data.get('deadlocked_processes', []))
                        self.deadlocked_resources = set(det_data.get('deadlocked_resources', []))
                except:
                    pass
                
                self._relayout()
                
        except Exception as e:
            print(f"Error refreshing RAG: {e}")
            
    def _relayout(self):
        """Recalculate positions and redraw"""
        # Update layout dimensions
        self.layout_algo.width = self.view.width() - 40
        self.layout_algo.height = self.view.height() - 40
        
        # Prepare node lists
        process_ids = [p['id'] for p in self.processes]
        resource_ids = [r['id'] for r in self.resources]
        all_nodes = [('P', pid) for pid in process_ids] + [('R', rid) for rid in resource_ids]
        
        if not all_nodes:
            self.scene.clear_all()
            return
        
        # Prepare edges
        edges = []
        for pid, rid in self.requests:
            edges.append((('P', pid), ('R', rid)))
        for rid, pid, _ in self.assignments:
            edges.append((('R', rid), ('P', pid)))
        
        # Calculate positions
        if self.current_layout == 'circular':
            positions = self.layout_algo.circular_layout(all_nodes)
        elif self.current_layout == 'hierarchical':
            positions = self.layout_algo.hierarchical_layout(process_ids, resource_ids)
        else:
            positions = self.layout_algo.force_directed_layout(all_nodes, edges, iterations=50)
        
        # Redraw
        self._draw_graph(positions)
        
    def _draw_graph(self, positions):
        """Draw the graph with the calculated positions"""
        self.scene.clear_all()
        
        # Create process nodes
        for process in self.processes:
            pid = process['id']
            node_id = ('P', pid)
            if node_id in positions:
                x, y = positions[node_id]
                is_deadlocked = pid in self.deadlocked_processes
                node = ProcessNode(pid, process['name'], x, y, is_deadlocked=is_deadlocked)
                self.scene.addItem(node)
                self.scene.process_nodes[pid] = node
        
        # Create resource nodes
        for resource in self.resources:
            rid = resource['id']
            node_id = ('R', rid)
            if node_id in positions:
                x, y = positions[node_id]
                is_deadlocked = rid in self.deadlocked_resources
                node = ResourceNode(
                    rid, resource['name'],
                    resource['total_instances'],
                    resource['available_instances'],
                    x, y, is_deadlocked=is_deadlocked
                )
                self.scene.addItem(node)
                self.scene.resource_nodes[rid] = node
        
        # Create edges
        for pid, rid in self.requests:
            if pid in self.scene.process_nodes and rid in self.scene.resource_nodes:
                edge = EdgeItem(
                    self.scene.process_nodes[pid],
                    self.scene.resource_nodes[rid],
                    'request', ''
                )
                self.scene.addItem(edge)
                self.scene.edges.append(edge)
        
        for rid, pid, count in self.assignments:
            if pid in self.scene.process_nodes and rid in self.scene.resource_nodes:
                edge = EdgeItem(
                    self.scene.resource_nodes[rid],
                    self.scene.process_nodes[pid],
                    'assignment', ''
                )
                self.scene.addItem(edge)
                self.scene.edges.append(edge)
        
        # Fit view
        self.scene.setSceneRect(self.scene.itemsBoundingRect().adjusted(-50, -50, 50, 50))
        
    def resizeEvent(self, event):
        """Handle resize to update layout dimensions"""
        super().resizeEvent(event)
        if self.processes or self.resources:
            # Delay relayout to avoid excessive recalculations
            QTimer.singleShot(100, self._relayout)
