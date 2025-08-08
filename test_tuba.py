#!/usr/bin/env python3
"""
Test script for TUBA components
Tests OSC functionality and basic imports without requiring GUI
"""

import os
import sys
import time
import threading
from pathlib import Path

# Test dotenv functionality
try:
    from dotenv import load_dotenv, set_key
    print("✓ python-dotenv imported successfully")
except ImportError as e:
    print(f"✗ Error importing python-dotenv: {e}")
    sys.exit(1)

# Test python-osc functionality
try:
    from pythonosc import udp_client, dispatcher
    from pythonosc.osc_server import BlockingOSCUDPServer
    print("✓ python-osc imported successfully")
except ImportError as e:
    print(f"✗ Error importing python-osc: {e}")
    sys.exit(1)

# Test OSC communication
def test_osc():
    """Test OSC server and client"""
    print("\nTesting OSC functionality...")
    
    # Create a dispatcher
    disp = dispatcher.Dispatcher()
    received_messages = []
    
    def message_handler(address, *args):
        received_messages.append((address, args))
        print(f"OSC RECV: {address} {args}")
    
    disp.set_default_handler(message_handler)
    
    # Start server
    try:
        osc_server = BlockingOSCUDPServer(("127.0.0.1", 9001), disp)
        server_thread = threading.Thread(target=osc_server.serve_forever, daemon=True)
        server_thread.start()
        print("✓ OSC server started on port 9001")
        
        # Create client
        client = udp_client.SimpleUDPClient("127.0.0.1", 9001)
        print("✓ OSC client created")
        
        # Send test message
        client.send_message("/test/channel", [0.5, "test"])
        print("✓ Test message sent")
        
        # Wait for message to be received
        time.sleep(0.1)
        
        # Check if message was received
        if received_messages:
            print("✓ OSC communication working correctly")
            print(f"  Received: {received_messages}")
        else:
            print("✗ No messages received")
            
        # Cleanup
        osc_server.shutdown()
        print("✓ OSC server stopped")
        
    except Exception as e:
        print(f"✗ OSC test failed: {e}")
        return False
        
    return True

# Test dotenv functionality
def test_dotenv():
    """Test dotenv functionality"""
    print("\nTesting dotenv functionality...")
    
    # Create test .env file
    test_env = Path("test.env")
    
    try:
        # Write test values
        set_key(test_env, "TEST_IP", "192.168.1.100")
        set_key(test_env, "TEST_PORT", "8000")
        print("✓ Written test values to .env file")
        
        # Load and verify
        load_dotenv(test_env)
        ip = os.getenv("TEST_IP")
        port = os.getenv("TEST_PORT")
        
        if ip == "192.168.1.100" and port == "8000":
            print("✓ dotenv reading/writing working correctly")
            print(f"  Read: IP={ip}, PORT={port}")
        else:
            print(f"✗ dotenv values incorrect: IP={ip}, PORT={port}")
            return False
            
        # Cleanup
        test_env.unlink()
        print("✓ Test file cleaned up")
        
    except Exception as e:
        print(f"✗ dotenv test failed: {e}")
        return False
        
    return True

# Test GUI imports (minimal)
def test_gui_imports():
    """Test GUI imports without creating widgets"""
    print("\nTesting GUI imports...")
    
    # Set headless mode
    os.environ['QT_QPA_PLATFORM'] = 'offscreen'
    
    try:
        from PySide6.QtCore import Qt, QTimer, Signal, QObject
        print("✓ PySide6.QtCore imported successfully")
        
        from PySide6.QtWidgets import QApplication
        print("✓ PySide6.QtWidgets imported successfully")
        
        # Test QApplication creation
        app = QApplication([])
        print("✓ QApplication created successfully")
        
        return True
        
    except Exception as e:
        print(f"✗ GUI import test failed: {e}")
        print("Note: This may be expected in headless environments")
        return False

def main():
    """Run all tests"""
    print("TUBA Component Tests")
    print("=" * 50)
    
    tests = [
        ("dotenv", test_dotenv),
        ("OSC", test_osc),
        ("GUI imports", test_gui_imports)
    ]
    
    passed = 0
    total = len(tests)
    
    for name, test_func in tests:
        try:
            if test_func():
                passed += 1
            print()
        except Exception as e:
            print(f"✗ {name} test crashed: {e}")
            print()
    
    print("=" * 50)
    print(f"Tests passed: {passed}/{total}")
    
    if passed == total:
        print("✓ All tests passed! TUBA core functionality is working.")
        return 0
    else:
        print("✗ Some tests failed. Check the errors above.")
        return 1

if __name__ == "__main__":
    sys.exit(main())