"""
Graph Layout Algorithms for RAG Visualization

Implements various graph layout algorithms to position nodes in the RAG.
"""

import math
import random


class GraphLayout:
    """Graph layout calculator"""
    
    def __init__(self, width=800, height=600):
        """
        Initialize graph layout
        
        Args:
            width: Canvas width
            height: Canvas height
        """
        self.width = width
        self.height = height
        self.margin = 80  # Margin from edges
    
    def circular_layout(self, nodes):
        """
        Arrange nodes in a circle
        
        Args:
            nodes: List of node IDs
            
        Returns:
            Dictionary mapping node ID to (x, y) position
        """
        positions = {}
        n = len(nodes)
        if n == 0:
            return positions
        
        # Center of the canvas
        cx = self.width / 2
        cy = self.height / 2
        
        # Radius of the circle
        radius = min(self.width, self.height) / 2 - self.margin
        
        # Angle between nodes
        angle_step = 2 * math.pi / n
        
        for i, node_id in enumerate(nodes):
            angle = i * angle_step - math.pi / 2  # Start from top
            x = cx + radius * math.cos(angle)
            y = cy + radius * math.sin(angle)
            positions[node_id] = (x, y)
        
        return positions
    
    def force_directed_layout(self, nodes, edges, iterations=100, k=None):
        """
        Fruchterman-Reingold force-directed layout algorithm
        
        Args:
            nodes: List of node IDs
            edges: List of (source, target) tuples
            iterations: Number of iterations
            k: Optimal distance between nodes (auto-calculated if None)
            
        Returns:
            Dictionary mapping node ID to (x, y) position
        """
        n = len(nodes)
        if n == 0:
            return {}
        
        if n == 1:
            return {nodes[0]: (self.width / 2, self.height / 2)}
        
        # Calculate optimal distance
        if k is None:
            area = (self.width - 2 * self.margin) * (self.height - 2 * self.margin)
            k = math.sqrt(area / n)
        
        # Initialize positions randomly
        positions = {}
        for node_id in nodes:
            x = random.uniform(self.margin, self.width - self.margin)
            y = random.uniform(self.margin, self.height - self.margin)
            positions[node_id] = [x, y]
        
        # Initial temperature
        t = self.width / 10
        dt = t / (iterations + 1)
        
        for iteration in range(iterations):
            # Calculate repulsive forces
            displacements = {node_id: [0.0, 0.0] for node_id in nodes}
            
            # Repulsion between all pairs
            for i, v in enumerate(nodes):
                for w in nodes[i + 1:]:
                    delta_x = positions[v][0] - positions[w][0]
                    delta_y = positions[v][1] - positions[w][1]
                    distance = math.sqrt(delta_x ** 2 + delta_y ** 2)
                    
                    if distance < 0.01:
                        distance = 0.01
                    
                    # Repulsive force
                    fr = k * k / distance
                    
                    displacements[v][0] += (delta_x / distance) * fr
                    displacements[v][1] += (delta_y / distance) * fr
                    displacements[w][0] -= (delta_x / distance) * fr
                    displacements[w][1] -= (delta_y / distance) * fr
            
            # Attractive forces along edges
            for source, target in edges:
                if source not in positions or target not in positions:
                    continue
                
                delta_x = positions[source][0] - positions[target][0]
                delta_y = positions[source][1] - positions[target][1]
                distance = math.sqrt(delta_x ** 2 + delta_y ** 2)
                
                if distance < 0.01:
                    distance = 0.01
                
                # Attractive force
                fa = distance * distance / k
                
                displacements[source][0] -= (delta_x / distance) * fa
                displacements[source][1] -= (delta_y / distance) * fa
                displacements[target][0] += (delta_x / distance) * fa
                displacements[target][1] += (delta_y / distance) * fa
            
            # Limit max displacement to temperature t and prevent from being displaced outside frame
            for node_id in nodes:
                dx = displacements[node_id][0]
                dy = displacements[node_id][1]
                disp = math.sqrt(dx ** 2 + dy ** 2)
                
                if disp > 0.01:
                    positions[node_id][0] += (dx / disp) * min(disp, t)
                    positions[node_id][1] += (dy / disp) * min(disp, t)
                
                # Keep within bounds
                positions[node_id][0] = max(self.margin, min(self.width - self.margin, positions[node_id][0]))
                positions[node_id][1] = max(self.margin, min(self.height - self.margin, positions[node_id][1]))
            
            # Reduce temperature
            t -= dt
        
        # Convert to tuples
        return {node_id: tuple(pos) for node_id, pos in positions.items()}
    
    def hierarchical_layout(self, processes, resources):
        """
        Two-layer hierarchical layout with processes on top and resources on bottom
        
        Args:
            processes: List of process IDs
            resources: List of resource IDs
            
        Returns:
            Dictionary mapping node ID to (x, y) position
        """
        positions = {}
        
        # Top layer: processes
        y_top = self.margin + 50
        if processes:
            x_step = (self.width - 2 * self.margin) / max(len(processes), 1)
            for i, pid in enumerate(processes):
                x = self.margin + (i + 0.5) * x_step
                positions[('P', pid)] = (x, y_top)
        
        # Bottom layer: resources
        y_bottom = self.height - self.margin - 50
        if resources:
            x_step = (self.width - 2 * self.margin) / max(len(resources), 1)
            for i, rid in enumerate(resources):
                x = self.margin + (i + 0.5) * x_step
                positions[('R', rid)] = (x, y_bottom)
        
        return positions
    
    def grid_layout(self, nodes, cols=None):
        """
        Arrange nodes in a grid
        
        Args:
            nodes: List of node IDs
            cols: Number of columns (auto-calculated if None)
            
        Returns:
            Dictionary mapping node ID to (x, y) position
        """
        n = len(nodes)
        if n == 0:
            return {}
        
        if cols is None:
            cols = int(math.ceil(math.sqrt(n)))
        
        rows = int(math.ceil(n / cols))
        
        # Calculate spacing
        x_spacing = (self.width - 2 * self.margin) / max(cols, 1)
        y_spacing = (self.height - 2 * self.margin) / max(rows, 1)
        
        positions = {}
        for i, node_id in enumerate(nodes):
            row = i // cols
            col = i % cols
            x = self.margin + (col + 0.5) * x_spacing
            y = self.margin + (row + 0.5) * y_spacing
            positions[node_id] = (x, y)
        
        return positions
