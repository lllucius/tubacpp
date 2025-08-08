#!/usr/bin/env python3
"""
TUBA Launcher Script
Handles environment setup and provides fallback for headless environments
"""

import sys
import os
from pathlib import Path

def check_display():
    """Check if we have a display available"""
    return 'DISPLAY' in os.environ or sys.platform == 'win32' or sys.platform == 'darwin'

def install_dependencies():
    """Install required dependencies if missing"""
    try:
        import subprocess
        
        # Check if requirements.txt exists
        if Path("requirements.txt").exists():
            print("Installing dependencies from requirements.txt...")
            result = subprocess.run([sys.executable, "-m", "pip", "install", "-r", "requirements.txt"], 
                                 capture_output=True, text=True)
            if result.returncode != 0:
                print(f"Warning: Failed to install dependencies: {result.stderr}")
            else:
                print("Dependencies installed successfully!")
        
    except Exception as e:
        print(f"Warning: Could not install dependencies: {e}")

def main():
    """Main launcher"""
    print("TUBA - Totalmix UBA (Python Edition)")
    print("=" * 50)
    
    # Install dependencies if needed
    install_dependencies()
    
    # Check display availability
    has_display = check_display()
    
    if not has_display:
        print("No display detected. Running in test mode...")
        print("For full GUI operation, run on a system with a display.")
        print()
        
        # Run tests instead of GUI
        try:
            import test_tuba
            return test_tuba.main()
        except ImportError:
            print("Test file not found. Core functionality test:")
            
            # Basic test
            try:
                from dotenv import load_dotenv
                from pythonosc import udp_client
                print("✓ Core dependencies available")
                print("✓ TUBA should work with a display")
            except ImportError as e:
                print(f"✗ Missing dependencies: {e}")
                return 1
                
        return 0
    
    else:
        print("Display detected. Starting GUI application...")
        
        # Try to import and run the main application
        try:
            from tuba import main as tuba_main
            return tuba_main()
            
        except ImportError as e:
            print(f"Error importing TUBA: {e}")
            print("GUI libraries may not be available on this system.")
            return 1
        except Exception as e:
            print(f"Error starting TUBA: {e}")
            return 1

if __name__ == "__main__":
    sys.exit(main())