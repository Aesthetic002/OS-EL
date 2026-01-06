"""
Backend Interface for OS-EL Deadlock Detection System

This module manages communication with the C backend executable via subprocess
and provides a Python API for sending JSON commands and receiving responses.
"""

import subprocess
import json
import threading
import queue
import os
import sys


class BackendInterface:
    """Interface to communicate with the C backend via JSON API"""
    
    def __init__(self, executable_path=None):
        """
        Initialize the backend interface
        
        Args:
            executable_path: Path to the deadlock.exe executable
                           If None, will look in ../bin/deadlock.exe
        """
        if executable_path is None:
            # Default to bin/deadlock.exe relative to project root
            script_dir = os.path.dirname(os.path.abspath(__file__))
            project_root = os.path.dirname(script_dir)
            executable_path = os.path.join(project_root, 'bin', 'deadlock.exe')
        
        self.executable_path = executable_path
        self.process = None
        self.response_queue = queue.Queue()
        self.reader_thread = None
        self.running = False
        
    def start(self):
        """Start the backend process"""
        if self.running:
            return
        
        if not os.path.exists(self.executable_path):
            raise FileNotFoundError(f"Backend executable not found: {self.executable_path}")
        
        # Start the backend process with --api flag
        self.process = subprocess.Popen(
            [self.executable_path, '--api'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        
        self.running = True
        
        # Start reader thread to handle responses
        self.reader_thread = threading.Thread(target=self._read_responses, daemon=True)
        self.reader_thread.start()
        
    def stop(self):
        """Stop the backend process"""
        if not self.running:
            return
        
        try:
            # Send shutdown command
            self.send_command({"command": "shutdown"}, wait_response=False)
        except:
            pass
        
        self.running = False
        
        if self.process:
            try:
                self.process.stdin.close()
                self.process.wait(timeout=2)
            except:
                self.process.kill()
            self.process = None
    
    def _read_responses(self):
        """Background thread to read responses from backend"""
        while self.running and self.process:
            try:
                line = self.process.stdout.readline()
                if not line:
                    break
                
                # Try to parse as JSON
                try:
                    response = json.loads(line.strip())
                    self.response_queue.put(response)
                except json.JSONDecodeError:
                    # Not JSON, might be debug output
                    pass
            except Exception as e:
                if self.running:
                    self.response_queue.put({"status": "error", "message": str(e)})
                break
    
    def send_command(self, command_dict, wait_response=True, timeout=5.0):
        """
        Send a command to the backend and optionally wait for response
        
        Args:
            command_dict: Dictionary containing the command and parameters
            wait_response: Whether to wait for and return the response
            timeout: Timeout in seconds for waiting for response
            
        Returns:
            Response dictionary if wait_response=True, None otherwise
        """
        if not self.running or not self.process:
            raise RuntimeError("Backend not running")
        
        # Send command as JSON
        json_str = json.dumps(command_dict)
        try:
            self.process.stdin.write(json_str + '\n')
            self.process.stdin.flush()
        except Exception as e:
            raise RuntimeError(f"Failed to send command: {e}")
        
        if not wait_response:
            return None
        
        # Wait for response - skip any "ready" or non-command responses
        import time
        start_time = time.time()
        while time.time() - start_time < timeout:
            try:
                response = self.response_queue.get(timeout=0.5)
                # Skip initialization messages
                if response.get('status') == 'ready':
                    continue
                return response
            except queue.Empty:
                continue
        
        raise TimeoutError(f"No response received within {timeout} seconds")
    
    # ============================================================================
    # RAG Operations
    # ============================================================================
    
    def rag_init(self):
        """Initialize the RAG"""
        return self.send_command({"command": "rag_init"})
    
    def rag_reset(self):
        """Reset the RAG to empty state"""
        return self.send_command({"command": "rag_reset"})
    
    def rag_get_state(self):
        """Get the current RAG state"""
        return self.send_command({"command": "rag_get_state"})
    
    # ============================================================================
    # Process Operations
    # ============================================================================
    
    def add_process(self, name, priority=50):
        """Add a new process"""
        return self.send_command({
            "command": "add_process",
            "name": name,
            "priority": priority
        })
    
    def remove_process(self, process_id):
        """Remove a process"""
        return self.send_command({
            "command": "remove_process",
            "process_id": process_id
        })
    
    def list_processes(self):
        """List all processes"""
        return self.send_command({"command": "list_processes"})
    
    def get_process(self, process_id):
        """Get details of a specific process"""
        return self.send_command({
            "command": "get_process",
            "process_id": process_id
        })
    
    # ============================================================================
    # Resource Operations
    # ============================================================================
    
    def add_resource(self, name, instances=1):
        """Add a new resource"""
        return self.send_command({
            "command": "add_resource",
            "name": name,
            "instances": instances
        })
    
    def remove_resource(self, resource_id):
        """Remove a resource"""
        return self.send_command({
            "command": "remove_resource",
            "resource_id": resource_id
        })
    
    def list_resources(self):
        """List all resources"""
        return self.send_command({"command": "list_resources"})
    
    def get_resource(self, resource_id):
        """Get details of a specific resource"""
        return self.send_command({
            "command": "get_resource",
            "resource_id": resource_id
        })
    
    # ============================================================================
    # Edge Operations
    # ============================================================================
    
    def request_resource(self, process_id, resource_id):
        """Create a request edge from process to resource"""
        return self.send_command({
            "command": "request_resource",
            "process_id": process_id,
            "resource_id": resource_id
        })
    
    def allocate_resource(self, process_id, resource_id):
        """Allocate resource to process (create assignment edge)"""
        return self.send_command({
            "command": "allocate_resource",
            "process_id": process_id,
            "resource_id": resource_id
        })
    
    def release_resource(self, process_id, resource_id):
        """Release resource from process"""
        return self.send_command({
            "command": "release_resource",
            "process_id": process_id,
            "resource_id": resource_id
        })
    
    def release_all(self, process_id):
        """Release all resources held by a process"""
        return self.send_command({
            "command": "release_all",
            "process_id": process_id
        })
    
    # ============================================================================
    # Detection Operations
    # ============================================================================
    
    def detect_deadlock(self):
        """Detect deadlock in the current RAG"""
        return self.send_command({"command": "detect_deadlock"})
    
    def detect_all_cycles(self):
        """Detect all cycles in the RAG"""
        return self.send_command({"command": "detect_all_cycles"})
    
    def is_process_deadlocked(self, process_id):
        """Check if a specific process is deadlocked"""
        return self.send_command({
            "command": "is_process_deadlocked",
            "process_id": process_id
        })
    
    def get_wait_for_graph(self):
        """Get the wait-for graph"""
        return self.send_command({"command": "get_wait_for_graph"})
    
    # ============================================================================
    # Recovery Operations
    # ============================================================================
    
    def recover(self, strategy=1, criteria=1):
        """
        Recover from deadlock
        
        Args:
            strategy: Recovery strategy (1-6)
                     1: Terminate All
                     2: Terminate Lowest Priority
                     3: Terminate One
                     4: Iterative Termination
                     5: Preempt Resources
                     6: Rollback
            criteria: Selection criteria (1-3)
                     1: Lowest Priority
                     2: Fewest Resources
                     3: Youngest Process
        """
        return self.send_command({
            "command": "recover",
            "strategy": strategy,
            "criteria": criteria
        })
    
    def recommend_strategy(self):
        """Get recommended recovery strategy"""
        return self.send_command({"command": "recommend_strategy"})
    
    # ============================================================================
    # Simulation Operations
    # ============================================================================
    
    def sim_init(self):
        """Initialize simulation"""
        return self.send_command({"command": "sim_init"})
    
    def sim_load_scenario(self, scenario):
        """
        Load a simulation scenario
        
        Args:
            scenario: Scenario number (0-3)
                     0: Simple Deadlock
                     1: Circular Wait
                     2: Dining Philosophers
                     3: Random
        """
        return self.send_command({
            "command": "sim_load_scenario",
            "scenario": scenario
        })
    
    def sim_start(self):
        """Start simulation"""
        return self.send_command({"command": "sim_start"})
    
    def sim_pause(self):
        """Pause simulation"""
        return self.send_command({"command": "sim_pause"})
    
    def sim_resume(self):
        """Resume simulation"""
        return self.send_command({"command": "sim_resume"})
    
    def sim_stop(self):
        """Stop simulation"""
        return self.send_command({"command": "sim_stop"})
    
    def sim_tick(self, auto_detect=False, auto_recover=False):
        """Execute one simulation tick"""
        return self.send_command({
            "command": "sim_tick",
            "auto_detect": auto_detect,
            "auto_recover": auto_recover
        })
    
    def sim_get_state(self):
        """Get current simulation state"""
        return self.send_command({"command": "sim_get_state"})
    
    # ============================================================================
    # System Operations
    # ============================================================================
    
    def ping(self):
        """Ping the backend to check if it's responsive"""
        return self.send_command({"command": "ping"})
    
    def get_version(self):
        """Get backend API version"""
        return self.send_command({"command": "get_version"})
