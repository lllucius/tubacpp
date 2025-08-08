#!/usr/bin/env python3
"""
TUBA - Totalmix UBA (Ugly But Accessible)
Python/PySide6 version of the mixer control panel

Converted from C++ wxWidgets version to use:
- PySide6 for cross-platform GUI
- python-osc for OSC communication
- python-dotenv for settings management
- Enhanced accessibility for screen readers
"""

import sys
import os
import logging
import threading
import time
from typing import Dict, Any, Optional
from pathlib import Path

# Set up environment for headless operation if needed
if 'DISPLAY' not in os.environ and 'QT_QPA_PLATFORM' not in os.environ:
    os.environ['QT_QPA_PLATFORM'] = 'offscreen'

try:
    from PySide6.QtWidgets import (
        QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout, 
        QSlider, QLineEdit, QLabel, QPushButton, QCheckBox, QDialog,
        QFormLayout, QDialogButtonBox, QGroupBox, QGridLayout,
        QMessageBox, QSpacerItem, QSizePolicy
    )
    from PySide6.QtCore import Qt, QTimer, Signal, QObject, QThread
    from PySide6.QtGui import QAction, QKeySequence, QAccessible, QFont
except ImportError as e:
    print(f"Error importing PySide6: {e}")
    print("Please install PySide6: pip install PySide6")
    print("Note: In headless environments, you may need system packages for Qt dependencies")
    sys.exit(1)

try:
    from pythonosc import udp_client, dispatcher
    from pythonosc.osc_server import BlockingOSCUDPServer
    from pythonosc.osc_message import OscMessage
except ImportError as e:
    print(f"Error importing python-osc: {e}")
    print("Please install python-osc: pip install python-osc")
    sys.exit(1)

try:
    from dotenv import load_dotenv, set_key
except ImportError as e:
    print(f"Error importing python-dotenv: {e}")
    print("Please install python-dotenv: pip install python-dotenv")
    sys.exit(1)

# Configure logging for OSC traffic
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)


class OSCHandler(QObject):
    """Handles OSC communication in a separate thread"""
    
    value_received = Signal(str, float)  # pattern, value
    string_received = Signal(str, str)   # pattern, string
    
    def __init__(self, local_port: int):
        super().__init__()
        self.local_port = local_port
        self.server = None
        self.client = None
        self.server_thread = None
        
    def start_server(self):
        """Start OSC server in a separate thread"""
        try:
            disp = dispatcher.Dispatcher()
            disp.set_default_handler(self._handle_message)
            
            self.server = BlockingOSCUDPServer(("0.0.0.0", self.local_port), disp)
            self.server_thread = threading.Thread(target=self.server.serve_forever, daemon=True)
            self.server_thread.start()
            logger.info(f"OSC server started on port {self.local_port}")
            
        except Exception as e:
            logger.error(f"Failed to start OSC server: {e}")
            
    def stop_server(self):
        """Stop OSC server"""
        if self.server:
            self.server.shutdown()
            if self.server_thread:
                self.server_thread.join(timeout=1.0)
            logger.info("OSC server stopped")
            
    def set_client(self, ip: str, port: int):
        """Set up OSC client for sending messages"""
        try:
            self.client = udp_client.SimpleUDPClient(ip, port)
            logger.info(f"OSC client configured for {ip}:{port}")
        except Exception as e:
            logger.error(f"Failed to configure OSC client: {e}")
            
    def _handle_message(self, address: str, *args):
        """Handle incoming OSC messages"""
        logger.info(f"OSC RECV: {address} {args}")
        
        if args:
            if isinstance(args[0], (int, float)):
                self.value_received.emit(address, float(args[0]))
            elif isinstance(args[0], str):
                self.string_received.emit(address, args[0])
                
    def send_message(self, address: str, *args):
        """Send OSC message"""
        if self.client:
            try:
                self.client.send_message(address, args)
                logger.info(f"OSC SEND: {address} {args}")
            except Exception as e:
                logger.error(f"Failed to send OSC message: {e}")
        else:
            logger.warning("OSC client not configured")


class ChannelControl(QWidget):
    """A single channel control with slider and text input"""
    
    value_changed = Signal(str, float)  # channel_name, value
    
    def __init__(self, name: str, parent=None):
        super().__init__(parent)
        self.name = name
        self.setup_ui()
        
    def setup_ui(self):
        layout = QHBoxLayout(self)
        layout.setContentsMargins(5, 5, 5, 5)
        
        # Label
        label = QLabel(self.name)
        label.setMinimumWidth(120)
        label.setAccessibleName(f"{self.name} channel")
        layout.addWidget(label)
        
        # Slider
        self.slider = QSlider(Qt.Horizontal)
        self.slider.setMinimum(0)
        self.slider.setMaximum(1000)
        self.slider.setValue(0)
        self.slider.setAccessibleName(f"{self.name} slider")
        self.slider.setAccessibleDescription(f"Adjust {self.name} level from 0 to 1000")
        self.slider.valueChanged.connect(self._on_slider_changed)
        layout.addWidget(self.slider, 1)  # Stretch factor 1
        
        # Text input
        self.text_input = QLineEdit()
        self.text_input.setText("0")
        self.text_input.setMaximumWidth(80)
        self.text_input.setAccessibleName(f"{self.name} value input")
        self.text_input.setAccessibleDescription(f"Enter {self.name} value directly")
        self.text_input.returnPressed.connect(self._on_text_changed)
        self.text_input.editingFinished.connect(self._on_text_changed)
        layout.addWidget(self.text_input)
        
    def _on_slider_changed(self, value: int):
        """Handle slider value change"""
        self.text_input.setText(str(value))
        normalized_value = value / 1000.0
        self.value_changed.emit(self.name, normalized_value)
        
    def _on_text_changed(self):
        """Handle text input change"""
        try:
            value = int(self.text_input.text())
            value = max(0, min(1000, value))  # Clamp to valid range
            self.slider.setValue(value)
            self.text_input.setText(str(value))
            normalized_value = value / 1000.0
            self.value_changed.emit(self.name, normalized_value)
        except ValueError:
            # Reset to slider value if invalid input
            self.text_input.setText(str(self.slider.value()))
            
    def set_value(self, normalized_value: float):
        """Set value from external source (e.g., OSC)"""
        slider_value = int(normalized_value * 1000)
        slider_value = max(0, min(1000, slider_value))
        self.slider.setValue(slider_value)
        self.text_input.setText(str(slider_value))


class SettingsDialog(QDialog):
    """Settings dialog for OSC configuration"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        self.setWindowTitle("TUBA Settings")
        self.setModal(True)
        self.setup_ui()
        self.load_settings()
        
    def setup_ui(self):
        layout = QVBoxLayout(self)
        
        # Form layout
        form_layout = QFormLayout()
        
        # Remote IP
        self.remote_ip_edit = QLineEdit()
        self.remote_ip_edit.setAccessibleName("Remote IP address")
        self.remote_ip_edit.setAccessibleDescription("IP address of the remote OSC server")
        form_layout.addRow("Remote IP:", self.remote_ip_edit)
        
        # Remote Port
        self.remote_port_edit = QLineEdit()
        self.remote_port_edit.setAccessibleName("Remote port")
        self.remote_port_edit.setAccessibleDescription("UDP port of the remote OSC server")
        form_layout.addRow("Remote Port:", self.remote_port_edit)
        
        # Local Port
        self.local_port_edit = QLineEdit()
        self.local_port_edit.setAccessibleName("Local port")
        self.local_port_edit.setAccessibleDescription("Local UDP port for receiving OSC messages")
        form_layout.addRow("Local Port:", self.local_port_edit)
        
        layout.addLayout(form_layout)
        
        # Buttons
        button_box = QDialogButtonBox(QDialogButtonBox.Ok | QDialogButtonBox.Cancel)
        button_box.accepted.connect(self.accept)
        button_box.rejected.connect(self.reject)
        layout.addWidget(button_box)
        
    def load_settings(self):
        """Load settings from environment"""
        self.remote_ip_edit.setText(os.getenv("REMOTE_IP", "127.0.0.1"))
        self.remote_port_edit.setText(os.getenv("REMOTE_PORT", "7001"))
        self.local_port_edit.setText(os.getenv("LOCAL_PORT", "9001"))
        
    def save_settings(self):
        """Save settings to .env file"""
        env_path = Path(".env")
        
        set_key(env_path, "REMOTE_IP", self.remote_ip_edit.text())
        set_key(env_path, "REMOTE_PORT", self.remote_port_edit.text())
        set_key(env_path, "LOCAL_PORT", self.local_port_edit.text())
        
    def get_settings(self) -> tuple:
        """Get current settings values"""
        return (
            self.remote_ip_edit.text(),
            int(self.remote_port_edit.text()),
            int(self.local_port_edit.text())
        )


class TubaMainWindow(QMainWindow):
    """Main window for TUBA mixer control panel"""
    
    def __init__(self):
        super().__init__()
        self.setWindowTitle("TUBA - Totalmix UBA")
        self.setMinimumSize(800, 600)
        
        # Load environment settings
        load_dotenv()
        
        # Initialize OSC handler
        local_port = int(os.getenv("LOCAL_PORT", "9001"))
        self.osc_handler = OSCHandler(local_port)
        self.osc_handler.value_received.connect(self.handle_osc_value)
        self.osc_handler.string_received.connect(self.handle_osc_string)
        
        # Configure OSC client
        remote_ip = os.getenv("REMOTE_IP", "127.0.0.1")
        remote_port = int(os.getenv("REMOTE_PORT", "7001"))
        self.osc_handler.set_client(remote_ip, remote_port)
        
        # Channel controls dictionary
        self.channels: Dict[str, ChannelControl] = {}
        
        # Values storage for pattern matching
        self.values: Dict[str, float] = {}
        
        # Setup UI and start services
        self.setup_ui()
        self.setup_accessibility()
        self.start_osc_services()
        self.request_initial_values()
        
    def setup_ui(self):
        """Setup the main UI"""
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        main_layout = QVBoxLayout(central_widget)
        main_layout.setContentsMargins(10, 10, 10, 10)
        main_layout.setSpacing(10)
        
        # Create channel groups
        self.create_gain_channels(main_layout)
        self.create_eq_channels(main_layout)
        
        # Add spacer to push settings button to bottom
        spacer = QSpacerItem(20, 40, QSizePolicy.Minimum, QSizePolicy.Expanding)
        main_layout.addItem(spacer)
        
        # Settings button (bottom right)
        settings_layout = QHBoxLayout()
        settings_spacer = QSpacerItem(40, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)
        settings_layout.addItem(settings_spacer)
        
        self.settings_button = QPushButton("Settings")
        self.settings_button.setAccessibleName("Settings button")
        self.settings_button.setAccessibleDescription("Open settings dialog to configure OSC connection")
        self.settings_button.clicked.connect(self.open_settings)
        settings_layout.addWidget(self.settings_button)
        
        main_layout.addLayout(settings_layout)
        
    def create_gain_channels(self, parent_layout: QVBoxLayout):
        """Create gain channel controls"""
        gain_group = QGroupBox("Gain Channels")
        gain_group.setAccessibleName("Gain channels group")
        gain_layout = QVBoxLayout(gain_group)
        
        # Gain channels (before EQ as per requirements)
        gain_channels = [
            "Mic 1 Volume", "Mic 1 Gain",
            "Mic 2 Volume", "Mic 2 Gain", 
            "MIDI", "Master", "Slave"
        ]
        
        for channel_name in gain_channels:
            channel = ChannelControl(channel_name)
            channel.value_changed.connect(self.handle_channel_value_changed)
            self.channels[channel_name] = channel
            gain_layout.addWidget(channel)
            
        parent_layout.addWidget(gain_group)
        
    def create_eq_channels(self, parent_layout: QVBoxLayout):
        """Create EQ channel controls"""
        eq_group = QGroupBox("EQ Channels")
        eq_group.setAccessibleName("EQ channels group")
        eq_layout = QVBoxLayout(eq_group)
        
        # EQ channels
        eq_channels = ["Bass", "Mid", "Treble"]
        
        for channel_name in eq_channels:
            channel = ChannelControl(channel_name)
            channel.value_changed.connect(self.handle_channel_value_changed)
            self.channels[channel_name] = channel
            eq_layout.addWidget(channel)
            
        # EQ Enable checkbox
        self.eq_enable = QCheckBox("Enable EQ")
        self.eq_enable.setAccessibleName("EQ Enable")
        self.eq_enable.setAccessibleDescription("Enable or disable equalizer")
        self.eq_enable.stateChanged.connect(self.handle_eq_enable_changed)
        eq_layout.addWidget(self.eq_enable)
        
        parent_layout.addWidget(eq_group)
        
    def setup_accessibility(self):
        """Setup accessibility features for screen readers"""
        # Set accessible properties for the main window
        self.setAccessibleName("TUBA Mixer Control Panel")
        self.setAccessibleDescription("Audio mixer control panel with OSC communication")
        
        # Enable focus policy for keyboard navigation
        self.setFocusPolicy(Qt.StrongFocus)
        
        # Add keyboard shortcuts
        self.create_menu_bar()
        
    def create_menu_bar(self):
        """Create menu bar with keyboard shortcuts"""
        menubar = self.menuBar()
        
        # File menu
        file_menu = menubar.addMenu("&File")
        
        settings_action = QAction("&Settings...", self)
        settings_action.setShortcut(QKeySequence("Ctrl+S"))
        settings_action.setAccessibleDescription("Open settings dialog")
        settings_action.triggered.connect(self.open_settings)
        file_menu.addAction(settings_action)
        
        file_menu.addSeparator()
        
        exit_action = QAction("E&xit", self)
        exit_action.setShortcut(QKeySequence("Ctrl+Q"))
        exit_action.setAccessibleDescription("Exit application")
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
    def start_osc_services(self):
        """Start OSC server and client services"""
        self.osc_handler.start_server()
        
    def request_initial_values(self):
        """Request current values from remote server on startup"""
        # Send initialization messages similar to the C++ version
        QTimer.singleShot(1000, self._send_initialization_messages)
        
    def _send_initialization_messages(self):
        """Send OSC messages to request current state"""
        # Request bus information
        self.osc_handler.send_message("/1/busPlayback", 1.0)
        self.osc_handler.send_message("/1/busInput", 1.0) 
        self.osc_handler.send_message("/1/busOutput", 1.0)
        
        # Request channel information for specific channels
        channels_to_request = [
            ("2", "Input", "Mic 1"),
            ("2", "Input", "SPDIF"), 
            ("2", "Output", "Main"),
            ("2", "Output", "Speaker B")
        ]
        
        for page, bus, channel in channels_to_request:
            QTimer.singleShot(2000, lambda p=page, b=bus, c=channel: self._request_channel_info(p, b, c))
            
    def _request_channel_info(self, page: str, bus: str, channel: str):
        """Request information for a specific channel"""
        # Send messages to select the channel and get its current values
        self.osc_handler.send_message(f"/{page}/bus{bus}", 1.0)
        # Add slight delay between messages
        QTimer.singleShot(100, lambda: self.osc_handler.send_message("/2/track-", 1.0))
        
    def handle_channel_value_changed(self, channel_name: str, value: float):
        """Handle when user changes a channel value"""
        # Map channel names to OSC patterns similar to C++ version
        osc_mapping = {
            "Slave": ("/2/volume", "Speaker B", "Output"),
            "Master": ("/2/volume", "Main", "Output"), 
            "Mic 1 Volume": ("/2/volume", "Mic 1", "Input"),
            "Mic 1 Gain": ("/2/gain", "Mic 1", "Input"),
            "Mic 2 Volume": ("/2/volume", "Mic 2", "Input"),
            "Mic 2 Gain": ("/2/gain", "Mic 2", "Input"),
            "MIDI": ("/2/volume", "SPDIF", "Input"),
            "Bass": ("/2/eqGain1", "Main", "Output"),
            "Mid": ("/2/eqGain2", "Main", "Output"), 
            "Treble": ("/2/eqGain3", "Main", "Output")
        }
        
        if channel_name in osc_mapping:
            pattern, target, bus = osc_mapping[channel_name]
            # Send the OSC message with the channel selection and value
            self._send_channel_fader(pattern, target, bus, value)
            
    def _send_channel_fader(self, pattern: str, target: str, bus: str, value: float):
        """Send OSC fader message for a specific channel"""
        # First select the correct bus and channel, then send the value
        self.osc_handler.send_message(f"/2/bus{bus}", 1.0)
        QTimer.singleShot(50, lambda: self.osc_handler.send_message(pattern, value))
        
    def handle_eq_enable_changed(self, state: int):
        """Handle EQ enable checkbox change"""
        enabled = state == Qt.Checked
        # Send EQ enable message for both Main and Speaker B channels
        self.osc_handler.send_message("/2/eqEnable", 1.0 if enabled else 0.0)
        
    def handle_osc_value(self, address: str, value: float):
        """Handle incoming OSC value messages"""
        # Store the value for pattern matching
        self.values[address] = value
        
        # Handle specific patterns similar to C++ version
        if "/2/volume" in address:
            self._update_volume_controls(value)
        elif "/2/gain" in address:
            self._update_gain_controls(value)
        elif "/2/eqGain" in address:
            self._update_eq_controls(address, value)
        elif "/2/eqEnable" in address:
            self.eq_enable.setChecked(value != 0.0)
            
    def _update_volume_controls(self, value: float):
        """Update volume controls based on current channel context"""
        # This would need more sophisticated channel tracking like the C++ version
        # For now, just log the received value
        logger.info(f"Volume update received: {value}")
        
    def _update_gain_controls(self, value: float):
        """Update gain controls based on current channel context"""
        logger.info(f"Gain update received: {value}")
        
    def _update_eq_controls(self, address: str, value: float):
        """Update EQ controls"""
        if "eqGain1" in address and "Bass" in self.channels:
            self.channels["Bass"].set_value(value)
        elif "eqGain2" in address and "Mid" in self.channels:
            self.channels["Mid"].set_value(value)
        elif "eqGain3" in address and "Treble" in self.channels:
            self.channels["Treble"].set_value(value)
            
    def handle_osc_string(self, address: str, value: str):
        """Handle incoming OSC string messages"""
        logger.info(f"String message: {address} = {value}")
        
    def open_settings(self):
        """Open settings dialog"""
        dialog = SettingsDialog(self)
        if dialog.exec() == QDialog.Accepted:
            dialog.save_settings()
            
            # Reconfigure OSC with new settings
            remote_ip, remote_port, local_port = dialog.get_settings()
            
            # Restart OSC services with new settings
            self.osc_handler.stop_server()
            self.osc_handler = OSCHandler(local_port)
            self.osc_handler.value_received.connect(self.handle_osc_value)
            self.osc_handler.string_received.connect(self.handle_osc_string)
            self.osc_handler.set_client(remote_ip, remote_port)
            self.osc_handler.start_server()
            
            QMessageBox.information(self, "Settings", "Settings saved. OSC services restarted.")
            
    def closeEvent(self, event):
        """Handle application close"""
        self.osc_handler.stop_server()
        event.accept()


def main():
    """Main entry point"""
    app = QApplication(sys.argv)
    
    # Set application properties for accessibility
    app.setApplicationName("TUBA")
    app.setApplicationDisplayName("TUBA - Totalmix UBA")
    app.setApplicationVersion("2.0")
    app.setOrganizationName("TUBA Project")
    
    # Create and show main window
    window = TubaMainWindow()
    window.show()
    
    # Set focus to the first control for screen reader accessibility
    if window.channels:
        first_channel = next(iter(window.channels.values()))
        first_channel.slider.setFocus()
    
    return app.exec()


if __name__ == "__main__":
    sys.exit(main())