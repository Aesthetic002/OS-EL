"""
OS-EL Deadlock Detection & Recovery GUI

Main application entry point for the Tkinter-based GUI.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from backend_interface import BackendInterface
from components.rag_visualizer import RAGVisualizer
from components.control_panel import ControlPanel
from components.deadlock_panel import DeadlockPanel
from components.simulation_panel import SimulationPanel
from utils.theme import COLORS, FONTS


class DeadlockGUI:
    """Main GUI application for deadlock detection and recovery"""
    
    def __init__(self, root):
        """
        Initialize the GUI
        
        Args:
            root: Tkinter root window
        """
        self.root = root
        self.root.title("OS-EL: Deadlock Detection & Recovery System")
        self.root.geometry("1400x900")
        self.root.configure(bg=COLORS['bg_primary'])
        
        # Initialize backend
        self.backend = None
        self._init_backend()
        
        # Create UI
        self._create_menu()
        self._create_widgets()
        
        # Initial refresh
        self.refresh_all()
        
        # Handle window close
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
    
    def _init_backend(self):
        """Initialize backend connection"""
        try:
            print("Initializing backend...")
            self.backend = BackendInterface()
            print(f"Backend executable path: {self.backend.executable_path}")
            print(f"Executable exists: {os.path.exists(self.backend.executable_path)}")
            
            print("Starting backend process...")
            self.backend.start()
            print("Backend process started")
            
            # Test connection with timeout
            print("Testing connection with ping...")
            import time
            time.sleep(0.5)  # Give backend time to initialize
            
            response = self.backend.ping()
            print(f"Ping response: {response}")
            
            if response and response.get('status') == 'success':
                print("✓ Backend connected successfully")
            else:
                raise Exception(f"Backend ping returned unexpected response: {response}")
                
        except FileNotFoundError as e:
            messagebox.showerror("Backend Error",
                               f"Backend executable not found!\n\n"
                               f"Path: {self.backend.executable_path if self.backend else 'Unknown'}\n\n"
                               f"Please build the backend first:\n"
                               f"  mingw32-make\n\n"
                               f"Error: {e}")
            sys.exit(1)
        except Exception as e:
            import traceback
            error_details = traceback.format_exc()
            messagebox.showerror("Backend Error",
                               f"Failed to connect to backend:\n{e}\n\n"
                               f"Please ensure the backend is built:\n"
                               f"  mingw32-make\n\n"
                               f"And check that bin/deadlock.exe exists.\n\n"
                               f"Details:\n{error_details}")
            sys.exit(1)
    
    def _create_menu(self):
        """Create menu bar"""
        menubar = tk.Menu(self.root)
        self.root.config(menu=menubar)
        
        # File menu
        file_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="File", menu=file_menu)
        file_menu.add_command(label="Reset RAG", command=self._reset_rag)
        file_menu.add_separator()
        file_menu.add_command(label="Exit", command=self._on_close)
        
        # View menu
        view_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="View", menu=view_menu)
        view_menu.add_command(label="Refresh All", command=self.refresh_all)
        
        # Help menu
        help_menu = tk.Menu(menubar, tearoff=0)
        menubar.add_cascade(label="Help", menu=help_menu)
        help_menu.add_command(label="About", command=self._show_about)
    
    def _create_widgets(self):
        """Create main widgets"""
        # Header
        header = tk.Frame(self.root, bg=COLORS['primary'], height=60)
        header.pack(fill='x')
        header.pack_propagate(False)
        
        tk.Label(header, text="OS-EL: Deadlock Detection & Recovery System",
                font=FONTS['header_large'], bg=COLORS['primary'],
                fg=COLORS['text_white']).pack(side='left', padx=20, pady=15)
        
        # Main content area with PanedWindow for flexible resizing
        main_paned = tk.PanedWindow(self.root, orient='horizontal', 
                                    sashrelief='raised', sashwidth=8,
                                    bg=COLORS['bg_primary'])
        main_paned.pack(fill='both', expand=True, padx=5, pady=5)
        
        # Left column: RAG Visualizer (largest)
        left_frame = tk.Frame(main_paned, bg=COLORS['bg_primary'])
        
        vis_label = tk.Label(left_frame, text="Resource Allocation Graph",
                            font=FONTS['header'], bg=COLORS['bg_primary'],
                            fg=COLORS['text_primary'])
        vis_label.pack(anchor='w', padx=5, pady=5)
        
        self.visualizer = RAGVisualizer(left_frame, self.backend)
        self.visualizer.pack(fill='both', expand=True)
        
        main_paned.add(left_frame, minsize=300, stretch='always')
        
        # Right paned window for control/deadlock and simulation
        right_paned = tk.PanedWindow(main_paned, orient='horizontal',
                                     sashrelief='raised', sashwidth=8,
                                     bg=COLORS['bg_primary'])
        
        # Middle column: Control Panel and Deadlock Panel (in vertical pane)
        middle_frame = tk.Frame(right_paned, bg=COLORS['bg_primary'])
        
        # Create vertical paned window for control and deadlock panels
        middle_paned = tk.PanedWindow(middle_frame, orient='vertical',
                                      sashrelief='raised', sashwidth=5,
                                      bg=COLORS['bg_primary'])
        middle_paned.pack(fill='both', expand=True)
        
        # Control panel (top)
        control_container = tk.Frame(middle_paned, bg=COLORS['bg_primary'])
        control_label = tk.Label(control_container, text="Control Panel",
                                font=FONTS['header'], bg=COLORS['bg_primary'],
                                fg=COLORS['text_primary'])
        control_label.pack(anchor='w', padx=5, pady=5)
        
        self.control_panel = ControlPanel(control_container, self.backend,
                                         on_update=self.refresh_all)
        self.control_panel.pack(fill='both', expand=True)
        middle_paned.add(control_container, minsize=150, stretch='always')
        
        # Deadlock panel (bottom)
        deadlock_container = tk.Frame(middle_paned, bg=COLORS['bg_primary'])
        self.deadlock_panel = DeadlockPanel(deadlock_container, self.backend,
                                           on_update=self.refresh_all)
        self.deadlock_panel.pack(fill='both', expand=True)
        middle_paned.add(deadlock_container, minsize=100, stretch='always')
        
        right_paned.add(middle_frame, minsize=300, stretch='always')
        
        # Right column: Simulation Panel
        right_frame = tk.Frame(right_paned, bg=COLORS['bg_primary'])
        
        sim_label = tk.Label(right_frame, text="Simulation",
                            font=FONTS['header'], bg=COLORS['bg_primary'],
                            fg=COLORS['text_primary'])
        sim_label.pack(anchor='w', padx=5, pady=5)
        
        self.simulation_panel = SimulationPanel(right_frame, self.backend,
                                               on_update=self.refresh_all)
        self.simulation_panel.pack(fill='both', expand=True)
        
        right_paned.add(right_frame, minsize=350, stretch='always')
        
        main_paned.add(right_paned, minsize=700, stretch='always')
        
        # Status bar
        status_bar = tk.Frame(self.root, bg=COLORS['bg_secondary'], height=25)
        status_bar.pack(fill='x', side='bottom')
        status_bar.pack_propagate(False)
        
        self.status_label = tk.Label(status_bar, text="Ready",
                                     font=FONTS['body_small'],
                                     bg=COLORS['bg_secondary'],
                                     fg=COLORS['text_secondary'], anchor='w')
        self.status_label.pack(side='left', padx=10)
        
        tk.Label(status_bar, text="Backend: Connected",
                font=FONTS['body_small'], bg=COLORS['bg_secondary'],
                fg=COLORS['success']).pack(side='right', padx=10)
    
    def refresh_all(self):
        """Refresh all panels"""
        try:
            self.status_label.config(text="Refreshing...")
            self.root.update_idletasks()
            
            self.visualizer.refresh()
            self.control_panel.refresh()
            
            self.status_label.config(text="Ready")
        except Exception as e:
            self.status_label.config(text=f"Error: {e}")
            print(f"Error refreshing: {e}")
    
    def _reset_rag(self):
        """Reset the RAG to empty state"""
        if messagebox.askyesno("Confirm Reset",
                              "This will remove all processes, resources, and edges.\n"
                              "Are you sure?"):
            try:
                self.backend.rag_reset()
                messagebox.showinfo("Success", "RAG has been reset")
                self.refresh_all()
            except Exception as e:
                messagebox.showerror("Error", f"Failed to reset RAG: {e}")
    
    def _show_about(self):
        """Show about dialog"""
        about_text = """
OS-EL: Deadlock Detection & Recovery System
Version 1.0.0

A comprehensive system for understanding and managing
deadlocks in operating systems using Resource Allocation
Graphs (RAG) and DFS-based cycle detection.

Features:
• Visual RAG representation
• Multiple deadlock detection algorithms
• Recovery strategies (termination, preemption, rollback)
• Pre-built simulation scenarios
• Interactive process/resource management

Developed for Operating System Experimental Lab
        """
        
        messagebox.showinfo("About OS-EL", about_text.strip())
    
    def _on_close(self):
        """Handle window close"""
        if messagebox.askyesno("Confirm Exit", "Are you sure you want to exit?"):
            try:
                if self.backend:
                    self.backend.stop()
            except:
                pass
            self.root.destroy()


def main():
    """Main entry point"""
    root = tk.Tk()
    app = DeadlockGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
