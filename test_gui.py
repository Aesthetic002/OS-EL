"""
Test script to verify GUI can start
"""
import sys
import os

# Add gui directory to path
gui_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'gui')
sys.path.insert(0, gui_dir)

print(f"GUI directory: {gui_dir}")
print(f"Python path: {sys.path[0]}")

# Test imports
try:
    from backend_interface import BackendInterface
    print("✓ backend_interface imported successfully")
    
    # Test backend path
    bi = BackendInterface()
    print(f"✓ Backend path: {bi.executable_path}")
    print(f"✓ Backend exists: {os.path.exists(bi.executable_path)}")
    
    # Test starting backend
    print("\nStarting backend...")
    bi.start()
    print("✓ Backend started")
    
    # Test ping
    response = bi.ping()
    print(f"✓ Ping response: {response}")
    
    # Stop backend
    bi.stop()
    print("✓ Backend stopped")
    
    print("\n✓✓✓ All tests passed! GUI should work.")
    
except Exception as e:
    print(f"\n✗ Error: {e}")
    import traceback
    traceback.print_exc()
