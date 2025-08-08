#!/usr/bin/env python3
"""
TUBA Demo Script
Demonstrates the OSC functionality without requiring GUI
"""

import time
import threading
from pythonosc import udp_client, dispatcher
from pythonosc.osc_server import BlockingOSCUDPServer

def demo_osc_communication():
    """Demonstrate OSC send/receive like the TUBA app would use"""
    
    print("🎛️  TUBA OSC Communication Demo")
    print("=" * 50)
    
    # Set up message logging
    received_messages = []
    
    def log_message(address, *args):
        received_messages.append((address, args))
        print(f"📡 OSC RECV: {address} {args}")
    
    # Create dispatcher and server (simulating TUBA receiving messages)
    disp = dispatcher.Dispatcher()
    disp.set_default_handler(log_message)
    
    # Start server on port 9001 (TUBA's default receive port)
    server = BlockingOSCUDPServer(("127.0.0.1", 9001), disp)
    server_thread = threading.Thread(target=server.serve_forever, daemon=True)
    server_thread.start()
    print("🎧 OSC Server started on port 9001 (TUBA receive)")
    
    # Create client (simulating TUBA sending to remote device on port 7001)
    client = udp_client.SimpleUDPClient("127.0.0.1", 7001)
    print("📤 OSC Client configured for port 7001 (TUBA send)")
    
    print("\n🎹 Simulating TUBA control changes...")
    
    # Simulate various control changes that TUBA would send
    controls = [
        ("/2/volume", 0.75, "Master Volume to 75%"),
        ("/2/gain", 0.5, "Mic 1 Gain to 50%"),
        ("/2/eqGain1", 0.6, "Bass EQ to 60%"),
        ("/2/eqGain2", 0.45, "Mid EQ to 45%"),
        ("/2/eqGain3", 0.8, "Treble EQ to 80%"),
        ("/2/eqEnable", 1.0, "EQ Enabled"),
        ("/1/busOutput", 1.0, "Select Output Bus"),
        ("/2/track+", 1.0, "Next Track")
    ]
    
    for pattern, value, description in controls:
        try:
            client.send_message(pattern, value)
            print(f"📺 OSC SEND: {pattern} {value} ({description})")
            time.sleep(0.2)  # Small delay between messages
        except Exception as e:
            print(f"❌ Failed to send {pattern}: {e}")
    
    print("\n📥 Now simulating responses from remote device...")
    
    # Simulate responses that TUBA would receive
    response_client = udp_client.SimpleUDPClient("127.0.0.1", 9001)
    
    responses = [
        ("/2/trackname", "Main", "Channel name response"),
        ("/2/volume", 0.75, "Volume confirmation"),
        ("/2/eqEnable", 1.0, "EQ enable confirmation"),
        ("/1/busOutput", 1.0, "Bus selection confirmation")
    ]
    
    for pattern, value, description in responses:
        try:
            response_client.send_message(pattern, value)
            print(f"📻 Remote SEND: {pattern} {value} ({description})")
            time.sleep(0.2)
        except Exception as e:
            print(f"❌ Failed to send response {pattern}: {e}")
    
    # Wait a bit for all messages to be processed
    time.sleep(0.5)
    
    # Cleanup
    server.shutdown()
    print("\n🔌 OSC Server stopped")
    
    print(f"\n📊 Summary: {len(received_messages)} messages received")
    print("✅ OSC communication demo complete!")
    print("\nThis demonstrates the same OSC protocol that TUBA uses to")
    print("communicate with audio mixing devices like TotalMix.")

if __name__ == "__main__":
    try:
        demo_osc_communication()
    except KeyboardInterrupt:
        print("\n⏹️  Demo interrupted by user")
    except Exception as e:
        print(f"\n❌ Demo failed: {e}")
        import traceback
        traceback.print_exc()