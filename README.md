# TUBA - Totalmix UBA (Python Edition)

**Converted from C++ to Python** - A mixer control panel that uses OSC (Open Sound Control) to communicate with remote audio devices.

## Original C++ Version

The original C++ application used wxWidgets and Windows-specific accessibility features. This Python version provides the same functionality with cross-platform compatibility.

## Features

- **Cross-platform GUI** using PySide6 (Qt6)
- **OSC Communication** using python-osc for reliable UDP messaging
- **Accessibility Support** for screen readers like JAWS
- **Configurable Settings** with persistent storage via dotenv
- **Console Logging** of all OSC traffic for debugging
- **Channel Controls** with both sliders and text input
- **Real-time Updates** from remote OSC server

## Requirements

- Python 3.8+
- PySide6 (Qt6 for Python)
- python-osc (OSC protocol support)
- python-dotenv (Configuration management)

## Installation

1. Clone or download this repository
2. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

## Usage

### Running the Application

```bash
# Using the launcher (recommended)
python run_tuba.py

# Or directly
python tuba.py
```

### Configuration

The application uses a `.env` file for configuration:

```env
REMOTE_IP=127.0.0.1      # IP address of remote OSC server
REMOTE_PORT=7001         # Port where OSC messages are sent
LOCAL_PORT=9001          # Port where OSC messages are received
```

You can modify these settings through the Settings dialog (accessible via the Settings button or Ctrl+S).

### Controls

The mixer panel includes:

**Gain Channels:**
- Mic 1 Volume & Gain
- Mic 2 Volume & Gain  
- MIDI Volume
- Master Volume
- Slave Volume

**EQ Channels:**
- Bass
- Mid
- Treble
- EQ Enable checkbox

Each channel has:
- A slider for visual adjustment (0-1000 range)
- A text input box for precise value entry
- Accessible names and descriptions for screen readers

### Accessibility

The application is designed to be fully accessible:
- All controls have proper accessible names and descriptions
- Keyboard navigation support (Tab, Arrow keys)
- Screen reader compatibility (tested with JAWS)
- Menu shortcuts (Ctrl+S for Settings, Ctrl+Q to Quit)

## OSC Protocol

The application communicates using the OSC (Open Sound Control) protocol:

- **Outgoing**: Control changes are sent to `REMOTE_IP:REMOTE_PORT`
- **Incoming**: Status updates received on `LOCAL_PORT`
- **Logging**: All OSC traffic is logged to the console

### OSC Message Format

Control messages follow this pattern:
```
/2/volume <float_value>     # Volume controls
/2/gain <float_value>       # Gain controls  
/2/eqGain1 <float_value>    # Bass EQ
/2/eqGain2 <float_value>    # Mid EQ
/2/eqGain3 <float_value>    # Treble EQ
/2/eqEnable <float_value>   # EQ enable (0.0 or 1.0)
```

## Development

### Project Structure

```
tubacpp/
├── tuba.py              # Main Python application
├── requirements.txt     # Python dependencies  
├── run_tuba.py         # Launcher script
├── test_tuba.py        # Component tests
├── .env                # Configuration file
├── README.md           # This documentation
└── tuba/               # Original C++ source
    ├── tuba.cpp        # Original main application
    ├── tuba.h          # Original headers
    ├── channel.cpp     # Channel management
    ├── channel.h       # Channel headers
    ├── oscpkt.h        # OSC implementation
    └── udp.h           # UDP communication
```

### Testing

Run the test suite to verify functionality:

```bash
python test_tuba.py
```

This tests:
- OSC communication (server/client)
- Configuration management (dotenv)
- GUI imports (when display available)

## Conversion Notes

### Changes from C++ Version

1. **GUI Framework**: wxWidgets → PySide6
2. **OSC Library**: Custom implementation → python-osc
3. **Configuration**: wxConfig → python-dotenv
4. **Accessibility**: Windows-specific → Cross-platform Qt accessibility
5. **Logging**: Custom debug → Python logging module

### Maintained Features

- Same OSC message patterns and protocol
- Identical channel layout and controls
- Same accessibility focus
- Equivalent functionality for screen readers

## Troubleshooting

### No Display Available

If running on a headless system, the launcher will automatically run in test mode:
```bash
python run_tuba.py  # Will run tests instead of GUI
```

### OSC Connection Issues

1. Check firewall settings for UDP ports
2. Verify remote server is running and accessible
3. Review console logs for OSC traffic
4. Test with OSC debugging tools

### Missing Dependencies

Install system packages for Qt/PySide6:
```bash
# Ubuntu/Debian
sudo apt-get install python3-pyside6

# Or use pip
pip install PySide6 python-osc python-dotenv
```

## License

Converted from the original TUBA project. Please respect the original licensing terms.

## Contributing

When contributing:
1. Maintain accessibility features
2. Follow existing code style
3. Test on multiple platforms
4. Document OSC protocol changes
5. Update accessibility descriptions for new controls