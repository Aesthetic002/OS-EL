"""
RAG Visualizer Component

Provides visual representation of the Resource Allocation Graph using Tkinter Canvas.
"""

import tkinter as tk
from tkinter import ttk
import math
from utils.theme import COLORS, FONTS
from utils.graph_layout import GraphLayout


class RAGVisualizer(ttk.Frame):
    """Resource Allocation Graph Visualizer"""
    
    def __init__(self, parent, backend):
        """
        Initialize RAG visualizer
        
        Args:
            parent: Parent widget
            backend: BackendInterface instance
        """
        super().__init__(parent)
        self.backend = backend
        
        # Node and edge data
        self.processes = []
        self.resources = []
        self.requests = []  # (process_id, resource_id)
        self.assignments = []  # (resource_id, process_id, count)
        self.deadlocked_processes = set()
        self.deadlocked_resources = set()
        
        # Layout
        self.layout = GraphLayout(width=800, height=600)
        self.positions = {}
        self.node_radius = 25
        
        # Dragging state
        self.dragging = None
        self.drag_start_x = 0
        self.drag_start_y = 0
        
        self._create_widgets()
    
    def _create_widgets(self):
        """Create visualizer widgets"""
        # Toolbar
        toolbar = tk.Frame(self, bg=COLORS['bg_secondary'])
        toolbar.pack(fill='x', padx=5, pady=5)
        
        tk.Label(toolbar, text="Layout:", bg=COLORS['bg_secondary'], 
                font=FONTS['body']).pack(side='left', padx=5)
        
        self.layout_var = tk.StringVar(value="force")
        layouts = [("Force-Directed", "force"), ("Circular", "circular"), 
                  ("Hierarchical", "hierarchical")]
        for text, value in layouts:
            tk.Radiobutton(toolbar, text=text, variable=self.layout_var, 
                          value=value, command=self.relayout,
                          bg=COLORS['bg_secondary'], font=FONTS['body']).pack(side='left', padx=2)
        
        tk.Button(toolbar, text="Refresh", command=self.refresh,
                 bg=COLORS['primary'], fg=COLORS['text_white'],
                 font=FONTS['body'], relief='flat', padx=10).pack(side='left', padx=10)
        
        # Canvas with scrollbars
        canvas_frame = tk.Frame(self)
        canvas_frame.pack(fill='both', expand=True, padx=5, pady=5)
        
        self.canvas = tk.Canvas(canvas_frame, bg=COLORS['bg_primary'], 
                               highlightthickness=1, highlightbackground=COLORS['border'])
        self.canvas.pack(fill='both', expand=True)
        
        # Bind events
        self.canvas.bind('<ButtonPress-1>', self._on_canvas_press)
        self.canvas.bind('<B1-Motion>', self._on_canvas_drag)
        self.canvas.bind('<ButtonRelease-1>', self._on_canvas_release)
        self.canvas.bind('<Configure>', self._on_canvas_configure)
        
        # Legend
        legend_frame = tk.Frame(self, bg=COLORS['bg_secondary'])
        legend_frame.pack(fill='x', padx=5, pady=5)
        
        tk.Label(legend_frame, text="Legend:", bg=COLORS['bg_secondary'],
                font=FONTS['body_small']).pack(side='left', padx=5)
        
        self._add_legend_item(legend_frame, "● Process", COLORS['node_process'])
        self._add_legend_item(legend_frame, "■ Resource", COLORS['node_resource'])
        self._add_legend_item(legend_frame, "→ Request", COLORS['edge_request'])
        self._add_legend_item(legend_frame, "→ Allocation", COLORS['edge_assignment'])
        self._add_legend_item(legend_frame, "● Deadlocked", COLORS['deadlock'])
    
    def _add_legend_item(self, parent, text, color):
        """Add a legend item"""
        frame = tk.Frame(parent, bg=COLORS['bg_secondary'])
        frame.pack(side='left', padx=10)
        
        canvas = tk.Canvas(frame, width=15, height=15, bg=COLORS['bg_secondary'],
                          highlightthickness=0)
        canvas.pack(side='left', padx=2)
        
        if "●" in text:
            canvas.create_oval(2, 2, 13, 13, fill=color, outline=color)
        elif "■" in text:
            canvas.create_rectangle(2, 2, 13, 13, fill=color, outline=color)
        else:
            canvas.create_line(2, 7, 13, 7, fill=color, width=2, arrow=tk.LAST)
        
        tk.Label(frame, text=text.replace("●", "").replace("■", "").replace("→", "").strip(),
                bg=COLORS['bg_secondary'], font=FONTS['body_small']).pack(side='left')
    
    def _on_canvas_configure(self, event):
        """Handle canvas resize"""
        self.layout.width = event.width
        self.layout.height = event.height
        self.relayout()
    
    def _on_canvas_press(self, event):
        """Handle mouse press on canvas"""
        # Check if clicked on a node
        x, y = event.x, event.y
        for node_id, (nx, ny) in self.positions.items():
            distance = math.sqrt((x - nx) ** 2 + (y - ny) ** 2)
            if distance <= self.node_radius:
                self.dragging = node_id
                self.drag_start_x = x
                self.drag_start_y = y
                return
    
    def _on_canvas_drag(self, event):
        """Handle mouse drag on canvas"""
        if self.dragging:
            x, y = event.x, event.y
            dx = x - self.drag_start_x
            dy = y - self.drag_start_y
            
            # Update position
            old_x, old_y = self.positions[self.dragging]
            self.positions[self.dragging] = (old_x + dx, old_y + dy)
            
            self.drag_start_x = x
            self.drag_start_y = y
            
            self.draw()
    
    def _on_canvas_release(self, event):
        """Handle mouse release on canvas"""
        self.dragging = None
    
    def refresh(self):
        """Refresh RAG data from backend"""
        try:
            response = self.backend.rag_get_state()
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    import json
                    data = json.loads(data)
                
                # Update processes and resources
                self.processes = data.get('processes', [])
                self.resources = data.get('resources', [])
                self.requests = [(r['process'], r['resource']) 
                               for r in data.get('requests', [])]
                self.assignments = [(a['resource'], a['process'], a.get('count', 1))
                                  for a in data.get('assignments', [])]
                
                # Update detection status
                try:
                    det_response = self.backend.detect_deadlock()
                    if det_response and det_response.get('status') == 'success':
                        det_data = det_response.get('data', {})
                        if isinstance(det_data, str):
                            det_data = json.loads(det_data)
                        
                        self.deadlocked_processes = set(det_data.get('deadlocked_processes', []))
                        self.deadlocked_resources = set(det_data.get('deadlocked_resources', []))
                except:
                    pass
                
                self.relayout()
        except Exception as e:
            print(f"Error refreshing RAG: {e}")
    
    def relayout(self):
        """Recalculate node positions"""
        layout_type = self.layout_var.get()
        
        # Prepare node lists
        process_ids = [p['id'] for p in self.processes]
        resource_ids = [r['id'] for r in self.resources]
        all_nodes = [('P', pid) for pid in process_ids] + [('R', rid) for rid in resource_ids]
        
        if not all_nodes:
            self.draw()
            return
        
        # Prepare edges for force-directed layout
        edges = []
        for pid, rid in self.requests:
            edges.append((('P', pid), ('R', rid)))
        for rid, pid, _ in self.assignments:
            edges.append((('R', rid), ('P', pid)))
        
        # Calculate layout
        if layout_type == "circular":
            self.positions = self.layout.circular_layout(all_nodes)
        elif layout_type == "hierarchical":
            self.positions = self.layout.hierarchical_layout(process_ids, resource_ids)
        else:  # force-directed
            self.positions = self.layout.force_directed_layout(all_nodes, edges, iterations=50)
        
        self.draw()
    
    def draw(self):
        """Draw the RAG on canvas"""
        self.canvas.delete('all')
        
        # Draw edges first (so they appear behind nodes)
        for pid, rid in self.requests:
            self._draw_edge(('P', pid), ('R', rid), COLORS['edge_request'], 
                          dashed=True, label="REQ")
        
        for rid, pid, count in self.assignments:
            label = f"ALLOC({count})" if count > 1 else "ALLOC"
            self._draw_edge(('R', rid), ('P', pid), COLORS['edge_assignment'], 
                          dashed=False, label=label)
        
        # Draw nodes
        for process in self.processes:
            pid = process['id']
            is_deadlocked = pid in self.deadlocked_processes
            self._draw_process_node(pid, process['name'], is_deadlocked)
        
        for resource in self.resources:
            rid = resource['id']
            is_deadlocked = rid in self.deadlocked_resources
            self._draw_resource_node(rid, resource['name'], 
                                    resource['total_instances'],
                                    resource['available_instances'],
                                    is_deadlocked)
    
    def _draw_process_node(self, pid, name, is_deadlocked=False):
        """Draw a process node (circle)"""
        node_id = ('P', pid)
        if node_id not in self.positions:
            return
        
        x, y = self.positions[node_id]
        r = self.node_radius
        
        color = COLORS['deadlock'] if is_deadlocked else COLORS['node_process']
        
        # Circle
        self.canvas.create_oval(x - r, y - r, x + r, y + r,
                               fill=color, outline=COLORS['border_dark'], width=2)
        
        # Label
        self.canvas.create_text(x, y, text=f"P{pid}\n{name}",
                               fill=COLORS['text_white'], font=FONTS['body_small'])
    
    def _draw_resource_node(self, rid, name, total, available, is_deadlocked=False):
        """Draw a resource node (rectangle)"""
        node_id = ('R', rid)
        if node_id not in self.positions:
            return
        
        x, y = self.positions[node_id]
        r = self.node_radius
        
        color = COLORS['deadlock'] if is_deadlocked else COLORS['node_resource']
        
        # Rectangle
        self.canvas.create_rectangle(x - r, y - r, x + r, y + r,
                                     fill=color, outline=COLORS['border_dark'], width=2)
        
        # Label
        self.canvas.create_text(x, y, text=f"R{rid}\n{name}\n{available}/{total}",
                               fill=COLORS['text_white'], font=FONTS['body_small'])
    
    def _draw_edge(self, from_node, to_node, color, dashed=False, label=""):
        """Draw an edge between two nodes"""
        if from_node not in self.positions or to_node not in self.positions:
            return
        
        x1, y1 = self.positions[from_node]
        x2, y2 = self.positions[to_node]
        
        # Calculate edge start/end on node boundaries
        angle = math.atan2(y2 - y1, x2 - x1)
        r = self.node_radius
        
        x1 = x1 + r * math.cos(angle)
        y1 = y1 + r * math.sin(angle)
        x2 = x2 - r * math.cos(angle)
        y2 = y2 - r * math.sin(angle)
        
        # Draw line
        if dashed:
            self.canvas.create_line(x1, y1, x2, y2, fill=color, width=2,
                                   arrow=tk.LAST, dash=(5, 3))
        else:
            self.canvas.create_line(x1, y1, x2, y2, fill=color, width=2,
                                   arrow=tk.LAST)
        
        # Draw label near the middle
        if label:
            mx = (x1 + x2) / 2
            my = (y1 + y2) / 2
            self.canvas.create_text(mx, my - 10, text=label,
                                   fill=color, font=FONTS['body_small'])
