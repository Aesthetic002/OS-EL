"""
Control Panel Component

Provides interface for managing processes, resources, and edges.
"""

import tkinter as tk
from tkinter import ttk, messagebox
from utils.theme import COLORS, FONTS, apply_button_style


class ControlPanel(ttk.Frame):
    """Control panel for process/resource management"""
    
    def __init__(self, parent, backend, on_update=None):
        """
        Initialize control panel
        
        Args:
            parent: Parent widget
            backend: BackendInterface instance
            on_update: Callback function when data is updated
        """
        super().__init__(parent)
        self.backend = backend
        self.on_update = on_update
        
        self._create_widgets()
    
    def _create_widgets(self):
        """Create control panel widgets"""
        # Create notebook for tabs
        notebook = ttk.Notebook(self)
        notebook.pack(fill='both', expand=True)
        
        # Process tab
        process_tab = tk.Frame(notebook, bg=COLORS['bg_primary'])
        notebook.add(process_tab, text="Processes")
        self._create_process_controls(process_tab)
        
        # Resource tab
        resource_tab = tk.Frame(notebook, bg=COLORS['bg_primary'])
        notebook.add(resource_tab, text="Resources")
        self._create_resource_controls(resource_tab)
        
        # Edge operations tab
        edge_tab = tk.Frame(notebook, bg=COLORS['bg_primary'])
        notebook.add(edge_tab, text="Edge Operations")
        self._create_edge_controls(edge_tab)
    
    def _create_process_controls(self, parent):
        """Create process management controls"""
        # Add process section
        add_frame = tk.LabelFrame(parent, text="Add Process", bg=COLORS['bg_primary'],
                                 font=FONTS['header_small'], padx=10, pady=10)
        add_frame.pack(fill='x', padx=10, pady=5)
        
        tk.Label(add_frame, text="Name:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=0, column=0, sticky='w', pady=3)
        self.process_name_entry = tk.Entry(add_frame, font=FONTS['body'], width=20)
        self.process_name_entry.grid(row=0, column=1, pady=3, padx=5)
        
        tk.Label(add_frame, text="Priority:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=1, column=0, sticky='w', pady=3)
        self.process_priority_entry = tk.Entry(add_frame, font=FONTS['body'], width=20)
        self.process_priority_entry.grid(row=1, column=1, pady=3, padx=5)
        self.process_priority_entry.insert(0, "50")
        
        add_btn = tk.Button(add_frame, text="Add Process", command=self._add_process)
        add_btn.grid(row=2, column=0, columnspan=2, pady=5)
        apply_button_style(add_btn, 'primary')
        
        # Process list section
        list_frame = tk.LabelFrame(parent, text="Process List", bg=COLORS['bg_primary'],
                                  font=FONTS['header_small'], padx=10, pady=10)
        list_frame.pack(fill='both', expand=True, padx=10, pady=5)
        
        # Treeview
        tree_frame = tk.Frame(list_frame, bg=COLORS['bg_primary'])
        tree_frame.pack(fill='both', expand=True)
        
        scrollbar = ttk.Scrollbar(tree_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.process_tree = ttk.Treeview(tree_frame, columns=('ID', 'Name', 'Priority', 'State'),
                                        show='headings', yscrollcommand=scrollbar.set)
        self.process_tree.heading('ID', text='ID')
        self.process_tree.heading('Name', text='Name')
        self.process_tree.heading('Priority', text='Priority')
        self.process_tree.heading('State', text='State')
        
        self.process_tree.column('ID', width=50)
        self.process_tree.column('Name', width=150)
        self.process_tree.column('Priority', width=80)
        self.process_tree.column('State', width=100)
        
        self.process_tree.pack(fill='both', expand=True)
        scrollbar.config(command=self.process_tree.yview)
        
        # Remove button
        remove_btn = tk.Button(list_frame, text="Remove Selected", command=self._remove_process)
        remove_btn.pack(pady=5)
        apply_button_style(remove_btn, 'danger')
        
        # Refresh button
        refresh_btn = tk.Button(list_frame, text="Refresh", command=self._refresh_processes)
        refresh_btn.pack(pady=5)
        apply_button_style(refresh_btn, 'secondary')
    
    def _create_resource_controls(self, parent):
        """Create resource management controls"""
        # Add resource section
        add_frame = tk.LabelFrame(parent, text="Add Resource", bg=COLORS['bg_primary'],
                                 font=FONTS['header_small'], padx=10, pady=10)
        add_frame.pack(fill='x', padx=10, pady=5)
        
        tk.Label(add_frame, text="Name:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=0, column=0, sticky='w', pady=3)
        self.resource_name_entry = tk.Entry(add_frame, font=FONTS['body'], width=20)
        self.resource_name_entry.grid(row=0, column=1, pady=3, padx=5)
        
        tk.Label(add_frame, text="Instances:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=1, column=0, sticky='w', pady=3)
        self.resource_instances_entry = tk.Entry(add_frame, font=FONTS['body'], width=20)
        self.resource_instances_entry.grid(row=1, column=1, pady=3, padx=5)
        self.resource_instances_entry.insert(0, "1")
        
        add_btn = tk.Button(add_frame, text="Add Resource", command=self._add_resource)
        add_btn.grid(row=2, column=0, columnspan=2, pady=5)
        apply_button_style(add_btn, 'primary')
        
        # Resource list section
        list_frame = tk.LabelFrame(parent, text="Resource List", bg=COLORS['bg_primary'],
                                  font=FONTS['header_small'], padx=10, pady=10)
        list_frame.pack(fill='both', expand=True, padx=10, pady=5)
        
        # Treeview
        tree_frame = tk.Frame(list_frame, bg=COLORS['bg_primary'])
        tree_frame.pack(fill='both', expand=True)
        
        scrollbar = ttk.Scrollbar(tree_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.resource_tree = ttk.Treeview(tree_frame, columns=('ID', 'Name', 'Total', 'Available'),
                                         show='headings', yscrollcommand=scrollbar.set)
        self.resource_tree.heading('ID', text='ID')
        self.resource_tree.heading('Name', text='Name')
        self.resource_tree.heading('Total', text='Total')
        self.resource_tree.heading('Available', text='Available')
        
        self.resource_tree.column('ID', width=50)
        self.resource_tree.column('Name', width=150)
        self.resource_tree.column('Total', width=80)
        self.resource_tree.column('Available', width=100)
        
        self.resource_tree.pack(fill='both', expand=True)
        scrollbar.config(command=self.resource_tree.yview)
        
        # Remove button
        remove_btn = tk.Button(list_frame, text="Remove Selected", command=self._remove_resource)
        remove_btn.pack(pady=5)
        apply_button_style(remove_btn, 'danger')
        
        # Refresh button
        refresh_btn = tk.Button(list_frame, text="Refresh", command=self._refresh_resources)
        refresh_btn.pack(pady=5)
        apply_button_style(refresh_btn, 'secondary')
    
    def _create_edge_controls(self, parent):
        """Create edge operation controls"""
        # Request resource
        request_frame = tk.LabelFrame(parent, text="Request Resource", bg=COLORS['bg_primary'],
                                     font=FONTS['header_small'], padx=10, pady=10)
        request_frame.pack(fill='x', padx=10, pady=5)
        
        tk.Label(request_frame, text="Process ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=0, column=0, sticky='w', pady=3)
        self.request_process_entry = tk.Entry(request_frame, font=FONTS['body'], width=15)
        self.request_process_entry.grid(row=0, column=1, pady=3, padx=5)
        
        tk.Label(request_frame, text="Resource ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=1, column=0, sticky='w', pady=3)
        self.request_resource_entry = tk.Entry(request_frame, font=FONTS['body'], width=15)
        self.request_resource_entry.grid(row=1, column=1, pady=3, padx=5)
        
        request_btn = tk.Button(request_frame, text="Request", command=self._request_resource)
        request_btn.grid(row=2, column=0, columnspan=2, pady=5)
        apply_button_style(request_btn, 'primary')
        
        # Allocate resource
        allocate_frame = tk.LabelFrame(parent, text="Allocate Resource", bg=COLORS['bg_primary'],
                                      font=FONTS['header_small'], padx=10, pady=10)
        allocate_frame.pack(fill='x', padx=10, pady=5)
        
        tk.Label(allocate_frame, text="Process ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=0, column=0, sticky='w', pady=3)
        self.allocate_process_entry = tk.Entry(allocate_frame, font=FONTS['body'], width=15)
        self.allocate_process_entry.grid(row=0, column=1, pady=3, padx=5)
        
        tk.Label(allocate_frame, text="Resource ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=1, column=0, sticky='w', pady=3)
        self.allocate_resource_entry = tk.Entry(allocate_frame, font=FONTS['body'], width=15)
        self.allocate_resource_entry.grid(row=1, column=1, pady=3, padx=5)
        
        allocate_btn = tk.Button(allocate_frame, text="Allocate", command=self._allocate_resource)
        allocate_btn.grid(row=2, column=0, columnspan=2, pady=5)
        apply_button_style(allocate_btn, 'primary')
        
        # Release resource
        release_frame = tk.LabelFrame(parent, text="Release Resource", bg=COLORS['bg_primary'],
                                     font=FONTS['header_small'], padx=10, pady=10)
        release_frame.pack(fill='x', padx=10, pady=5)
        
        tk.Label(release_frame, text="Process ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=0, column=0, sticky='w', pady=3)
        self.release_process_entry = tk.Entry(release_frame, font=FONTS['body'], width=15)
        self.release_process_entry.grid(row=0, column=1, pady=3, padx=5)
        
        tk.Label(release_frame, text="Resource ID:", bg=COLORS['bg_primary'],
                font=FONTS['body']).grid(row=1, column=0, sticky='w', pady=3)
        self.release_resource_entry = tk.Entry(release_frame, font=FONTS['body'], width=15)
        self.release_resource_entry.grid(row=1, column=1, pady=3, padx=5)
        
        release_btn = tk.Button(release_frame, text="Release", command=self._release_resource)
        release_btn.grid(row=2, column=0, columnspan=2, pady=5)
        apply_button_style(release_btn, 'primary')
        
        release_all_btn = tk.Button(release_frame, text="Release All for Process",
                                    command=self._release_all)
        release_all_btn.grid(row=3, column=0, columnspan=2, pady=5)
        apply_button_style(release_all_btn, 'secondary')
    
    # ============================================================================
    # Process Operations
    # ============================================================================
    
    def _add_process(self):
        """Add a new process"""
        name = self.process_name_entry.get().strip()
        priority_str = self.process_priority_entry.get().strip()
        
        if not name:
            messagebox.showerror("Error", "Please enter a process name")
            return
        
        try:
            priority = int(priority_str)
        except ValueError:
            messagebox.showerror("Error", "Priority must be a number")
            return
        
        try:
            response = self.backend.add_process(name, priority)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"Process '{name}' added successfully")
                self.process_name_entry.delete(0, tk.END)
                self._refresh_processes()
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to add process: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to add process: {e}")
    
    def _remove_process(self):
        """Remove selected process"""
        selection = self.process_tree.selection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a process to remove")
            return
        
        item = self.process_tree.item(selection[0])
        pid = int(item['values'][0])
        
        if not messagebox.askyesno("Confirm", f"Remove process P{pid}?"):
            return
        
        try:
            response = self.backend.remove_process(pid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", "Process removed successfully")
                self._refresh_processes()
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to remove process: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to remove process: {e}")
    
    def _refresh_processes(self):
        """Refresh process list"""
        self.process_tree.delete(*self.process_tree.get_children())
        
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
                    self.process_tree.insert('', 'end', values=(
                        process['id'],
                        process['name'],
                        process['priority'],
                        process['state'].upper()
                    ))
        except Exception as e:
            print(f"Error refreshing processes: {e}")
    
    # ============================================================================
    # Resource Operations
    # ============================================================================
    
    def _add_resource(self):
        """Add a new resource"""
        name = self.resource_name_entry.get().strip()
        instances_str = self.resource_instances_entry.get().strip()
        
        if not name:
            messagebox.showerror("Error", "Please enter a resource name")
            return
        
        try:
            instances = int(instances_str)
            if instances < 1:
                raise ValueError("Instances must be at least 1")
        except ValueError as e:
            messagebox.showerror("Error", f"Invalid instances: {e}")
            return
        
        try:
            response = self.backend.add_resource(name, instances)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"Resource '{name}' added successfully")
                self.resource_name_entry.delete(0, tk.END)
                self._refresh_resources()
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to add resource: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to add resource: {e}")
    
    def _remove_resource(self):
        """Remove selected resource"""
        selection = self.resource_tree.selection()
        if not selection:
            messagebox.showwarning("Warning", "Please select a resource to remove")
            return
        
        item = self.resource_tree.item(selection[0])
        rid = int(item['values'][0])
        
        if not messagebox.askyesno("Confirm", f"Remove resource R{rid}?"):
            return
        
        try:
            response = self.backend.remove_resource(rid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", "Resource removed successfully")
                self._refresh_resources()
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to remove resource: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to remove resource: {e}")
    
    def _refresh_resources(self):
        """Refresh resource list"""
        self.resource_tree.delete(*self.resource_tree.get_children())
        
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
                    self.resource_tree.insert('', 'end', values=(
                        resource['id'],
                        resource['name'],
                        resource['total_instances'],
                        resource['available_instances']
                    ))
        except Exception as e:
            print(f"Error refreshing resources: {e}")
    
    # ============================================================================
    # Edge Operations
    # ============================================================================
    
    def _request_resource(self):
        """Create a request edge"""
        try:
            pid = int(self.request_process_entry.get())
            rid = int(self.request_resource_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Please enter valid process and resource IDs")
            return
        
        try:
            response = self.backend.request_resource(pid, rid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"P{pid} requested R{rid}")
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to request resource: {e}")
    
    def _allocate_resource(self):
        """Allocate a resource to a process"""
        try:
            pid = int(self.allocate_process_entry.get())
            rid = int(self.allocate_resource_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Please enter valid process and resource IDs")
            return
        
        try:
            response = self.backend.allocate_resource(pid, rid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"R{rid} allocated to P{pid}")
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to allocate resource: {e}")
    
    def _release_resource(self):
        """Release a resource from a process"""
        try:
            pid = int(self.release_process_entry.get())
            rid = int(self.release_resource_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Please enter valid process and resource IDs")
            return
        
        try:
            response = self.backend.release_resource(pid, rid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"P{pid} released R{rid}")
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to release resource: {e}")
    
    def _release_all(self):
        """Release all resources for a process"""
        try:
            pid = int(self.release_process_entry.get())
        except ValueError:
            messagebox.showerror("Error", "Please enter a valid process ID")
            return
        
        try:
            response = self.backend.release_all(pid)
            if response and response.get('status') == 'success':
                messagebox.showinfo("Success", f"All resources released from P{pid}")
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to release resources: {e}")
    
    def refresh(self):
        """Refresh all lists"""
        self._refresh_processes()
        self._refresh_resources()
