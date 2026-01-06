"""
Deadlock Panel Component

Provides interface for deadlock detection and recovery.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import json
from utils.theme import COLORS, FONTS, apply_button_style


class DeadlockPanel(ttk.Frame):
    """Deadlock detection and recovery panel"""
    
    def __init__(self, parent, backend, on_update=None):
        """
        Initialize deadlock panel
        
        Args:
            parent: Parent widget
            backend: BackendInterface instance
            on_update: Callback function when data is updated
        """
        super().__init__(parent)
        self.backend = backend
        self.on_update = on_update
        self.detection_result = None
        
        self._create_widgets()
    
    def _create_widgets(self):
        """Create deadlock panel widgets"""
        # Detection section
        detection_frame = tk.LabelFrame(self, text="Deadlock Detection", 
                                       bg=COLORS['bg_primary'],
                                       font=FONTS['header'], padx=10, pady=10)
        detection_frame.pack(fill='both', expand=True, padx=10, pady=5)
        
        # Detect button
        btn_frame = tk.Frame(detection_frame, bg=COLORS['bg_primary'])
        btn_frame.pack(fill='x', pady=5)
        
        detect_btn = tk.Button(btn_frame, text="🔍 Detect Deadlock",
                              command=self._detect_deadlock)
        detect_btn.pack(side='left', padx=5)
        apply_button_style(detect_btn, 'primary')
        
        # Status indicator
        status_frame = tk.Frame(detection_frame, bg=COLORS['bg_primary'])
        status_frame.pack(fill='x', pady=5)
        
        tk.Label(status_frame, text="Status:", bg=COLORS['bg_primary'],
                font=FONTS['body_small']).pack(side='left', padx=5)
        
        self.status_label = tk.Label(status_frame, text="Not checked",
                                     bg=COLORS['bg_secondary'],
                                     font=FONTS['body'], padx=10, pady=5,
                                     relief='solid', borderwidth=1)
        self.status_label.pack(side='left', padx=5)
        
        # Results display
        results_frame = tk.Frame(detection_frame, bg=COLORS['bg_primary'])
        results_frame.pack(fill='both', expand=True, pady=5)
        
        # Results text area
        scrollbar = tk.Scrollbar(results_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.results_text = tk.Text(results_frame, height=10, font=FONTS['mono'],
                                    bg=COLORS['bg_secondary'], fg=COLORS['text_primary'],
                                    relief='solid', borderwidth=1, wrap='word',
                                    yscrollcommand=scrollbar.set)
        self.results_text.pack(fill='both', expand=True)
        scrollbar.config(command=self.results_text.yview)
        
        # Recovery section
        recovery_frame = tk.LabelFrame(self, text="Deadlock Recovery",
                                      bg=COLORS['bg_primary'],
                                      font=FONTS['header'], padx=10, pady=10)
        recovery_frame.pack(fill='both', padx=10, pady=5)
        
        # Strategy selection
        strategy_frame = tk.Frame(recovery_frame, bg=COLORS['bg_primary'])
        strategy_frame.pack(fill='x', pady=5)
        
        tk.Label(strategy_frame, text="Recovery Strategy:", bg=COLORS['bg_primary'],
                font=FONTS['body']).pack(side='left', padx=5)
        
        self.strategy_var = tk.StringVar(value="2")
        strategies = [
            ("Terminate All", "1"),
            ("Terminate Lowest Priority", "2"),
            ("Terminate One", "3"),
            ("Iterative Termination", "4"),
            ("Preempt Resources", "5"),
            ("Rollback", "6")
        ]
        
        strategy_dropdown = ttk.Combobox(strategy_frame, textvariable=self.strategy_var,
                                        values=[s[1] + ": " + s[0] for s in strategies],
                                        state='readonly', width=30)
        strategy_dropdown.current(1)
        strategy_dropdown.pack(side='left', padx=5)
        
        # Criteria selection
        criteria_frame = tk.Frame(recovery_frame, bg=COLORS['bg_primary'])
        criteria_frame.pack(fill='x', pady=5)
        
        tk.Label(criteria_frame, text="Selection Criteria:", bg=COLORS['bg_primary'],
                font=FONTS['body']).pack(side='left', padx=5)
        
        self.criteria_var = tk.StringVar(value="1")
        criteria = [
            ("Lowest Priority", "1"),
            ("Fewest Resources", "2"),
            ("Youngest Process", "3")
        ]
        
        criteria_dropdown = ttk.Combobox(criteria_frame, textvariable=self.criteria_var,
                                        values=[c[1] + ": " + c[0] for c in criteria],
                                        state='readonly', width=30)
        criteria_dropdown.current(0)
        criteria_dropdown.pack(side='left', padx=5)
        
        # Recommend and Recover buttons
        btn_frame2 = tk.Frame(recovery_frame, bg=COLORS['bg_primary'])
        btn_frame2.pack(fill='x', pady=5)
        
        recommend_btn = tk.Button(btn_frame2, text="💡 Recommend Strategy",
                                 command=self._recommend_strategy)
        recommend_btn.pack(side='left', padx=5)
        apply_button_style(recommend_btn, 'secondary')
        
        recover_btn = tk.Button(btn_frame2, text="🔧 Recover",
                               command=self._recover)
        recover_btn.pack(side='left', padx=5)
        apply_button_style(recover_btn, 'primary')
        
        # Recovery results
        recovery_results_frame = tk.Frame(recovery_frame, bg=COLORS['bg_primary'])
        recovery_results_frame.pack(fill='both', expand=True, pady=5)
        
        scrollbar2 = tk.Scrollbar(recovery_results_frame)
        scrollbar2.pack(side='right', fill='y')
        
        self.recovery_text = tk.Text(recovery_results_frame, height=8, font=FONTS['mono'],
                                    bg=COLORS['bg_secondary'], fg=COLORS['text_primary'],
                                    relief='solid', borderwidth=1, wrap='word',
                                    yscrollcommand=scrollbar2.set)
        self.recovery_text.pack(fill='both', expand=True)
        scrollbar2.config(command=self.recovery_text.yview)
    
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
                
                # Update status label
                if deadlock_detected:
                    self.status_label.config(text="⚠ DEADLOCK DETECTED",
                                           bg=COLORS['error'],
                                           fg=COLORS['text_white'])
                else:
                    self.status_label.config(text="✓ No Deadlock (Safe)",
                                           bg=COLORS['success'],
                                           fg=COLORS['text_white'])
                
                # Display results
                self.results_text.delete('1.0', tk.END)
                
                if deadlock_detected:
                    self.results_text.insert(tk.END, "⚠ DEADLOCK DETECTED!\n\n", 'error')
                    self.results_text.insert(tk.END, f"Cycle Count: {data.get('cycle_count', 0)}\n\n")
                    
                    deadlocked_processes = data.get('deadlocked_processes', [])
                    if deadlocked_processes:
                        self.results_text.insert(tk.END, "Deadlocked Processes:\n")
                        for pid in deadlocked_processes:
                            self.results_text.insert(tk.END, f"  • Process P{pid}\n")
                        self.results_text.insert(tk.END, "\n")
                    
                    deadlocked_resources = data.get('deadlocked_resources', [])
                    if deadlocked_resources:
                        self.results_text.insert(tk.END, "Deadlocked Resources:\n")
                        for rid in deadlocked_resources:
                            self.results_text.insert(tk.END, f"  • Resource R{rid}\n")
                else:
                    self.results_text.insert(tk.END, "✓ No deadlock detected.\n\n", 'success')
                    self.results_text.insert(tk.END, "The system is in a safe state.\n")
                    self.results_text.insert(tk.END, "All processes can potentially complete.\n")
                
                # Tag configurations for colored text
                self.results_text.tag_config('error', foreground=COLORS['error'], font=FONTS['header_small'])
                self.results_text.tag_config('success', foreground=COLORS['success'], font=FONTS['header_small'])
                
                # Trigger update
                if self.on_update:
                    self.on_update()
                    
            else:
                messagebox.showerror("Error", f"Failed to detect deadlock: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to detect deadlock: {e}")
    
    def _recommend_strategy(self):
        """Get recommended recovery strategy"""
        try:
            response = self.backend.recommend_strategy()
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                strategy_id = data.get('strategy', 2)
                strategy_name = data.get('name', 'Unknown')
                
                messagebox.showinfo("Recommended Strategy",
                                  f"Recommended recovery strategy:\n\n"
                                  f"Strategy {strategy_id}: {strategy_name}\n\n"
                                  f"This has been automatically selected for you.")
                
                # Set the strategy
                self.strategy_var.set(str(strategy_id))
            else:
                messagebox.showerror("Error", f"Failed to get recommendation: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to get recommendation: {e}")
    
    def _recover(self):
        """Execute recovery"""
        if not self.detection_result or not self.detection_result.get('deadlock_detected', False):
            if not messagebox.askyesno("No Deadlock",
                                      "No deadlock was detected in the last check.\n"
                                      "Do you want to proceed with recovery anyway?"):
                return
        
        # Extract strategy and criteria
        strategy_str = self.strategy_var.get().split(':')[0]
        criteria_str = self.criteria_var.get().split(':')[0]
        
        try:
            strategy = int(strategy_str)
            criteria = int(criteria_str)
        except ValueError:
            messagebox.showerror("Error", "Invalid strategy or criteria selection")
            return
        
        try:
            response = self.backend.recover(strategy, criteria)
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                # Display recovery results
                self.recovery_text.delete('1.0', tk.END)
                
                success = data.get('success', False)
                if success:
                    self.recovery_text.insert(tk.END, "✓ Recovery Successful!\n\n", 'success')
                else:
                    self.recovery_text.insert(tk.END, "⚠ Recovery Attempted\n\n", 'warning')
                
                self.recovery_text.insert(tk.END, f"Processes Terminated: {data.get('processes_terminated', 0)}\n")
                self.recovery_text.insert(tk.END, f"Resources Preempted: {data.get('resources_preempted', 0)}\n")
                self.recovery_text.insert(tk.END, f"Iterations: {data.get('iterations', 0)}\n\n")
                
                summary = data.get('summary', 'No summary available')
                self.recovery_text.insert(tk.END, f"Summary:\n{summary}\n")
                
                # Tag configurations
                self.recovery_text.tag_config('success', foreground=COLORS['success'], font=FONTS['header_small'])
                self.recovery_text.tag_config('warning', foreground=COLORS['warning'], font=FONTS['header_small'])
                
                messagebox.showinfo("Recovery Complete", "Recovery operation completed.\nCheck the results below for details.")
                
                # Trigger update
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to recover: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to recover: {e}")
    
    def refresh(self):
        """Refresh detection (optional auto-refresh)"""
        # Could be called periodically if needed
        pass
