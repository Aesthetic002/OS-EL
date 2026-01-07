"""
Simulation Panel Component

Provides interface for loading and running simulation scenarios.
"""

import tkinter as tk
from tkinter import ttk, messagebox
import json
from utils.theme import COLORS, FONTS, apply_button_style


class SimulationPanel(ttk.Frame):
    """Simulation scenarios and controls panel"""
    
    def __init__(self, parent, backend, on_update=None):
        """
        Initialize simulation panel
        
        Args:
            parent: Parent widget
            backend: BackendInterface instance
            on_update: Callback function when data is updated
        """
        super().__init__(parent)
        self.backend = backend
        self.on_update = on_update
        self.simulation_running = False
        
        self._create_widgets()
    
    def _create_widgets(self):
        """Create simulation panel widgets"""
        # Scenario selection
        scenario_frame = tk.LabelFrame(self, text="Load Scenario",
                                      bg=COLORS['bg_primary'],
                                      font=FONTS['header'], padx=10, pady=10)
        scenario_frame.pack(fill='x', padx=10, pady=5)
        
        # Simple scenarios
        simple_frame = tk.Frame(scenario_frame, bg=COLORS['bg_primary'])
        simple_frame.pack(fill='x', pady=5)
        
        simple_btn = tk.Button(simple_frame, text="Simple Deadlock",
                              command=lambda: self._load_scenario(0))
        simple_btn.pack(side='left', padx=5)
        apply_button_style(simple_btn, 'primary')
        
        # Circular wait
        circular_frame = tk.Frame(scenario_frame, bg=COLORS['bg_primary'])
        circular_frame.pack(fill='x', pady=5)
        
        tk.Label(circular_frame, text="Circular Wait (n processes):",
                bg=COLORS['bg_primary'], font=FONTS['body']).pack(side='left', padx=5)
        
        self.circular_n_entry = tk.Entry(circular_frame, font=FONTS['body'], width=5)
        self.circular_n_entry.insert(0, "5")
        self.circular_n_entry.pack(side='left', padx=5)
        
        circular_btn = tk.Button(circular_frame, text="Load Circular Wait",
                                command=lambda: self._load_scenario(1))
        circular_btn.pack(side='left', padx=5)
        apply_button_style(circular_btn, 'primary')
        
        # Dining philosophers
        philo_frame = tk.Frame(scenario_frame, bg=COLORS['bg_primary'])
        philo_frame.pack(fill='x', pady=5)
        
        tk.Label(philo_frame, text="Dining Philosophers (n):",
                bg=COLORS['bg_primary'], font=FONTS['body']).pack(side='left', padx=5)
        
        self.philo_n_entry = tk.Entry(philo_frame, font=FONTS['body'], width=5)
        self.philo_n_entry.insert(0, "5")
        self.philo_n_entry.pack(side='left', padx=5)
        
        philo_btn = tk.Button(philo_frame, text="Load Dining Philosophers",
                             command=lambda: self._load_scenario(2))
        philo_btn.pack(side='left', padx=5)
        apply_button_style(philo_btn, 'primary')
        
        # Random scenario
        random_frame = tk.Frame(scenario_frame, bg=COLORS['bg_primary'])
        random_frame.pack(fill='x', pady=5)
        
        tk.Label(random_frame, text="Random Scenario:",
                bg=COLORS['bg_primary'], font=FONTS['body']).pack(side='left', padx=5)
        
        tk.Label(random_frame, text="Processes:",
                bg=COLORS['bg_primary'], font=FONTS['body_small']).pack(side='left', padx=2)
        self.random_p_entry = tk.Entry(random_frame, font=FONTS['body'], width=5)
        self.random_p_entry.insert(0, "5")
        self.random_p_entry.pack(side='left', padx=2)
        
        tk.Label(random_frame, text="Resources:",
                bg=COLORS['bg_primary'], font=FONTS['body_small']).pack(side='left', padx=2)
        self.random_r_entry = tk.Entry(random_frame, font=FONTS['body'], width=5)
        self.random_r_entry.insert(0, "3")
        self.random_r_entry.pack(side='left', padx=2)
        
        random_btn = tk.Button(random_frame, text="Load Random",
                              command=lambda: self._load_scenario(3))
        random_btn.pack(side='left', padx=5)
        apply_button_style(random_btn, 'primary')
        
        # Simulation controls
        control_frame = tk.LabelFrame(self, text="Simulation Controls",
                                     bg=COLORS['bg_primary'],
                                     font=FONTS['header'], padx=10, pady=10)
        control_frame.pack(fill='x', padx=10, pady=5)
        
        # Control buttons
        btn_frame = tk.Frame(control_frame, bg=COLORS['bg_primary'])
        btn_frame.pack(fill='x', pady=5)
        
        self.start_btn = tk.Button(btn_frame, text="▶ Start",
                                   command=self._start_simulation)
        self.start_btn.pack(side='left', padx=5)
        apply_button_style(self.start_btn, 'primary')
        
        self.pause_btn = tk.Button(btn_frame, text="⏸ Pause",
                                   command=self._pause_simulation, state='disabled')
        self.pause_btn.pack(side='left', padx=5)
        apply_button_style(self.pause_btn, 'secondary')
        
        self.stop_btn = tk.Button(btn_frame, text="⏹ Stop",
                                  command=self._stop_simulation, state='disabled')
        self.stop_btn.pack(side='left', padx=5)
        apply_button_style(self.stop_btn, 'danger')
        
        tick_btn = tk.Button(btn_frame, text="⏭ Single Tick",
                            command=self._tick_simulation)
        tick_btn.pack(side='left', padx=5)
        apply_button_style(tick_btn, 'secondary')
        
        # Auto options
        auto_frame = tk.Frame(control_frame, bg=COLORS['bg_primary'])
        auto_frame.pack(fill='x', pady=5)
        
        self.auto_detect_var = tk.BooleanVar(value=True)
        tk.Checkbutton(auto_frame, text="Auto-Detect Deadlock",
                      variable=self.auto_detect_var, bg=COLORS['bg_primary'],
                      font=FONTS['body']).pack(side='left', padx=5)
        
        self.auto_recover_var = tk.BooleanVar(value=False)
        tk.Checkbutton(auto_frame, text="Auto-Recover",
                      variable=self.auto_recover_var, bg=COLORS['bg_primary'],
                      font=FONTS['body']).pack(side='left', padx=5)
        
        # Event log
        log_frame = tk.LabelFrame(self, text="Event Log",
                                 bg=COLORS['bg_primary'],
                                 font=FONTS['header'], padx=10, pady=10)
        log_frame.pack(fill='both', expand=True, padx=10, pady=5)
        
        scrollbar = tk.Scrollbar(log_frame)
        scrollbar.pack(side='right', fill='y')
        
        self.log_text = tk.Text(log_frame, height=15, font=FONTS['mono'],
                               bg=COLORS['bg_secondary'], fg=COLORS['text_primary'],
                               relief='solid', borderwidth=1, wrap='word',
                               yscrollcommand=scrollbar.set)
        self.log_text.pack(fill='both', expand=True)
        scrollbar.config(command=self.log_text.yview)
        
        # Clear button
        clear_btn = tk.Button(log_frame, text="Clear Log",
                             command=lambda: self.log_text.delete('1.0', tk.END))
        clear_btn.pack(pady=5)
        apply_button_style(clear_btn, 'secondary')
        
        # Auto-run controls
        autorun_frame = tk.LabelFrame(self, text="Auto-Run Simulation",
                                     bg=COLORS['bg_primary'],
                                     font=FONTS['header'], padx=10, pady=10)
        autorun_frame.pack(fill='x', padx=10, pady=5)
        
        speed_frame = tk.Frame(autorun_frame, bg=COLORS['bg_primary'])
        speed_frame.pack(fill='x', pady=5)
        
        tk.Label(speed_frame, text="Speed (ms/tick):", bg=COLORS['bg_primary'],
                font=FONTS['body']).pack(side='left', padx=5)
        self.speed_entry = tk.Entry(speed_frame, font=FONTS['body'], width=8)
        self.speed_entry.insert(0, "500")
        self.speed_entry.pack(side='left', padx=5)
        
        autorun_btn_frame = tk.Frame(autorun_frame, bg=COLORS['bg_primary'])
        autorun_btn_frame.pack(fill='x', pady=5)
        
        self.autorun_btn = tk.Button(autorun_btn_frame, text="▶ Auto-Run",
                                    command=self._toggle_autorun)
        self.autorun_btn.pack(side='left', padx=5)
        apply_button_style(self.autorun_btn, 'primary')
        
        self.autorun_active = False
        self.autorun_job = None
    
    def _load_scenario(self, scenario_id):
        """Load a simulation scenario"""
        try:
            # Initialize simulation first
            self.backend.sim_init()
            
            # Load the scenario
            response = self.backend.sim_load_scenario(scenario_id)
            if response and response.get('status') == 'success':
                scenario_names = ["Simple Deadlock", "Circular Wait", "Dining Philosophers", "Random"]
                scenario_name = scenario_names[scenario_id] if scenario_id < len(scenario_names) else f"Scenario {scenario_id}"
                
                self._log(f"✓ Loaded scenario: {scenario_name}")
                messagebox.showinfo("Success", f"Scenario '{scenario_name}' loaded successfully")
                
                # Get simulation state and copy to main RAG
                sim_state = self.backend.sim_get_state()
                if sim_state and sim_state.get('status') == 'success':
                    # The simulation creates its own RAG, we need to also update the main RAG
                    # For now, just refresh the visualizer
                    pass
                
                if self.on_update:
                    self.on_update()
            else:
                self._log(f"✗ Failed to load scenario {scenario_id}")
                messagebox.showerror("Error", f"Failed to load scenario: {response.get('message', 'Unknown error')}")
        except Exception as e:
            self._log(f"✗ Error loading scenario: {e}")
            messagebox.showerror("Error", f"Failed to load scenario: {e}")
    
    def _start_simulation(self):
        """Start the simulation"""
        try:
            response = self.backend.sim_start()
            if response and response.get('status') == 'success':
                self.simulation_running = True
                self._log("▶ Simulation started")
                self.start_btn.config(state='disabled')
                self.pause_btn.config(state='normal')
                self.stop_btn.config(state='normal')
            else:
                messagebox.showerror("Error", f"Failed to start: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to start simulation: {e}")
    
    def _pause_simulation(self):
        """Pause the simulation"""
        try:
            response = self.backend.sim_pause()
            if response and response.get('status') == 'success':
                self._log("⏸ Simulation paused")
                self.pause_btn.config(text="▶ Resume", command=self._resume_simulation)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to pause: {e}")
    
    def _resume_simulation(self):
        """Resume the simulation"""
        try:
            response = self.backend.sim_resume()
            if response and response.get('status') == 'success':
                self._log("▶ Simulation resumed")
                self.pause_btn.config(text="⏸ Pause", command=self._pause_simulation)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to resume: {e}")
    
    def _stop_simulation(self):
        """Stop the simulation"""
        try:
            response = self.backend.sim_stop()
            if response and response.get('status') == 'success':
                self.simulation_running = False
                self._log("⏹ Simulation stopped")
                self.start_btn.config(state='normal')
                self.pause_btn.config(state='disabled', text="⏸ Pause", command=self._pause_simulation)
                self.stop_btn.config(state='disabled')
        except Exception as e:
            messagebox.showerror("Error", f"Failed to stop: {e}")
    
    def _tick_simulation(self):
        """Execute one simulation tick"""
        try:
            auto_detect = self.auto_detect_var.get()
            auto_recover = self.auto_recover_var.get()
            
            response = self.backend.sim_tick(auto_detect, auto_recover)
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                tick = data.get('current_tick', 0)
                running = data.get('running', False)
                deadlock = data.get('deadlock_occurred', False)
                
                status = "running" if running else "ended"
                deadlock_str = " [DEADLOCK]" if deadlock else ""
                
                self._log(f"⏭ Tick {tick}: {status}{deadlock_str}")
                
                if not running:
                    self._log("⏹ Simulation completed")
                    self.simulation_running = False
                    self.start_btn.config(state='normal')
                    self.pause_btn.config(state='disabled')
                    self.stop_btn.config(state='disabled')
                
                if self.on_update:
                    self.on_update()
            else:
                messagebox.showerror("Error", f"Failed to tick: {response.get('message', 'Unknown error')}")
        except Exception as e:
            messagebox.showerror("Error", f"Failed to execute tick: {e}")
    
    def _log(self, message):
        """Add message to event log"""
        self.log_text.insert(tk.END, message + "\n")
        self.log_text.see(tk.END)
    
    def _toggle_autorun(self):
        """Toggle auto-run mode"""
        if self.autorun_active:
            # Stop auto-run
            self.autorun_active = False
            if self.autorun_job:
                self.after_cancel(self.autorun_job)
                self.autorun_job = None
            self.autorun_btn.config(text="▶ Auto-Run")
            self._log("⏹ Auto-run stopped")
        else:
            # Start auto-run
            self.autorun_active = True
            self.autorun_btn.config(text="⏹ Stop Auto-Run")
            self._log("▶ Auto-run started")
            self._auto_step()
    
    def _auto_step(self):
        """Execute one auto-step and schedule the next"""
        if not self.autorun_active:
            return
        
        try:
            auto_detect = self.auto_detect_var.get()
            auto_recover = self.auto_recover_var.get()
            
            response = self.backend.sim_tick(auto_detect, auto_recover)
            if response and response.get('status') == 'success':
                data = response.get('data', {})
                if isinstance(data, str):
                    data = json.loads(data)
                
                tick = data.get('current_tick', 0)
                running = data.get('running', False)
                deadlock = data.get('deadlock_occurred', False)
                
                status = "running" if running else "ended"
                deadlock_str = " [DEADLOCK!]" if deadlock else ""
                
                self._log(f"⏭ Tick {tick}: {status}{deadlock_str}")
                
                # Update visualization
                if self.on_update:
                    self.on_update()
                
                if not running or deadlock:
                    # Simulation ended or deadlock occurred
                    self.autorun_active = False
                    self.autorun_btn.config(text="▶ Auto-Run")
                    if deadlock:
                        self._log("⚠ Deadlock detected! Auto-run stopped.")
                    else:
                        self._log("⏹ Simulation completed")
                    return
                
                # Schedule next step
                try:
                    speed = int(self.speed_entry.get())
                    if speed < 100:
                        speed = 100
                except ValueError:
                    speed = 500
                
                self.autorun_job = self.after(speed, self._auto_step)
            else:
                self._log(f"✗ Auto-step failed: {response.get('message', 'Unknown')}")
                self.autorun_active = False
                self.autorun_btn.config(text="▶ Auto-Run")
        except Exception as e:
            self._log(f"✗ Auto-step error: {e}")
            self.autorun_active = False
            self.autorun_btn.config(text="▶ Auto-Run")
