# 📷 Optical Zoom Camera Control System

An ESP32-based dual stepper motor control system designed specifically for camera optical zoom and focus functions. Features web interface remote control, OTA wireless updates, and real-time status monitoring.

## ✨ Features

- 🎯 **Dual Stepper Motor Control** - Independent control of two A4988 stepper motor drivers
- 🌐 **Web Remote Control** - Responsive web interface supporting both mobile and desktop access
- 📡 **WiFi Connectivity** - Wireless control via WiFi network
- 🔄 **OTA Updates** - Wireless firmware updates without cable connection
- ⚡ **Real-time Control** - Adjustable speed, direction, and duration
- 🖥️ **Status Monitoring** - Real-time display of motor status and remaining time
- 📱 **Cross-platform Compatible** - Device access via mDNS domain names

## 🛠️ Hardware Requirements

### Main Controller
- **ESP32-C3** development board (AirM2M CORE ESP32C3 recommended)
- Built-in WiFi and Bluetooth functionality

### Stepper Motor Drivers
- **2 x A4988** stepper motor drivers
- **2 x NEMA 17** stepper motors (1.8° step angle)
- 12V/24V power adapter (according to motor specifications)

### Wiring Diagram

#### Motor 1 (Zoom Control)
```
ESP32-C3    ->   A4988 #1
GPIO2       ->   DIR (Direction)
GPIO3       ->   STEP (Step Pulse)
GPIO12      ->   EN (Enable, active low)
```

#### Motor 2 (Focus Control)
```
ESP32-C3    ->   A4988 #2
GPIO10      ->   DIR (Direction)
GPIO6       ->   STEP (Step Pulse)
GPIO18      ->   EN (Enable, active low)
```

#### A4988 Power Connections
```
VDD    -> 3.3V (Logic Power)
VMOT   -> 12V/24V (Motor Power)
GND    -> GND (Common Ground)
```

## 🚀 Quick Start

### 1. Environment Setup

Install [PlatformIO](https://platformio.org/) development environment:

```bash
# Using VSCode plugin (recommended)
# Or install via command line
pip install platformio
```

### 2. WiFi Configuration

Copy the example configuration file and modify WiFi settings:

```bash
cp src/secrets.h.example src/secrets.h
```

Then edit the `src/secrets.h` file:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Configuration
#define WIFI_SSID "Your_WiFi_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"

// mDNS Hostname
#define MDNS_HOST_NAME "camera-lens"

// OTA Update Password
#define OTA_PASSWORD "admin123"

#endif
```

### 3. Build and Upload

```bash
# Build project
pio run

# Upload firmware to device
pio run -t upload

# Monitor serial output
pio device monitor
```

### 4. Access Web Interface

After successful firmware upload, the device will automatically connect to WiFi and start the web server:

- **IP Address Access**: `http://device_ip_address`
- **Domain Access**: `http://camera-lens.local`

Check the serial monitor for the device's IP address.

## 🖥️ Web Interface Usage

### Main Functions

1. **Independent Motor Control**
   - Clockwise/Counterclockwise rotation
   - Adjustable duration (1-60 seconds)
   - One-click stop function

2. **Dual Motor Coordination**
   - Simultaneous control of both motors
   - Support for different direction combinations

3. **Parameter Settings**
   - Adjust step speed (speed control)
   - Real-time effect

4. **System Management**
   - View running status
   - OTA update information
   - System reboot

### API Endpoints

#### Motor Control
```
GET /motor1/cw?t=5    # Motor 1 clockwise for 5 seconds
GET /motor1/ccw?t=3   # Motor 1 counterclockwise for 3 seconds
GET /motor2/cw?t=10   # Motor 2 clockwise for 10 seconds
GET /motor2/ccw?t=8   # Motor 2 counterclockwise for 8 seconds
```

#### Stop Control
```
GET /motor1/stop      # Stop motor 1
GET /motor2/stop      # Stop motor 2
GET /stop/all         # Stop all motors
```

#### Parameter Settings
```
GET /set/speed/800    # Set step interval to 800 microseconds
```

#### Status Query
```
GET /status           # Get motor status JSON
```

## 🔧 Development Guide

### Project Structure

```
optical-zoom-camera/
├── src/
│   └── main_web.cpp          # Main program (Web version)
├── data/
│   └── index.html            # Web interface
├── main.cpp                  # Basic version (Serial control only)
├── platformio.ini            # PlatformIO configuration
├── airm2m.json              # AirM2M board definition
├── official.json            # ESP32-C3 official board definition
└── README.md                # Project documentation
```

### Build Options

Configure the following in `platformio.ini`:

```ini
# Debug level
build_flags = -DCORE_DEBUG_LEVEL=3

# Enable OTA functionality
build_flags = -DENABLE_OTA=1

# File system type
board_build.filesystem = littlefs
```

### Custom Configuration

Modify the following parameters in code:

```cpp
// Stepper motor parameters
#define STEPS_PER_REV 2000    // Steps per revolution
#define STEP_DELAY 1000       // Step interval (microseconds)

// Pin definitions
#define DIR_PIN_1 2           // Motor 1 direction pin
#define STEP_PIN_1 3          // Motor 1 step pin
// ... other pin configurations
```

## 📡 OTA Wireless Updates

### Enable OTA Updates

1. Uncomment OTA configuration in `platformio.ini`:

```ini
upload_protocol = espota
upload_port = camera-lens.local
upload_flags = --auth=admin123
```

2. Initial firmware upload requires USB connection
3. Subsequent updates can be done wirelessly:

```bash
pio run -t upload
```

### Update Steps

1. Ensure device is online and accessible
2. Run `pio run -t upload`
3. Enter OTA password: `admin123`
4. Wait for update completion

## 🔍 Troubleshooting

### Common Issues

1. **WiFi Connection Failed**
   - Check WiFi configuration in `secrets.h`
   - Verify WiFi signal strength and password

2. **Motors Not Rotating**
   - Check power connections (12V/24V supply)
   - Verify A4988 driver wiring
   - Confirm enable pin status (active low)

3. **Web Interface Inaccessible**
   - Check device IP address (serial monitor output)
   - Try mDNS domain: `camera-lens.local`
   - Verify firewall settings

4. **OTA Update Failed**
   - Check network connection stability
   - Confirm OTA password is correct
   - Restart device and retry

### Debug Information

Enable debug output for detailed information:

```bash
pio device monitor -b 115200
```

## 📋 Technical Specifications

- **Microcontroller**: ESP32-C3 (160MHz, 4MB Flash)
- **Stepper Motors**: Dual A4988 drivers
- **Motor Parameters**: NEMA 17, 1.8° step angle, 2000 steps/revolution
- **WiFi**: 802.11 b/g/n 2.4GHz
- **Power Requirements**: 5V USB + 12V/24V motor power
- **Control Precision**: 0.18° minimum step angle
- **Web Server**: ESPAsyncWebServer asynchronous processing

## 🤝 Contributing

Issues and Pull Requests are welcome!

1. Fork this project
2. Create feature branch: `git checkout -b feature/new-feature`
3. Commit changes: `git commit -m 'Add some feature'`
4. Push branch: `git push origin feature/new-feature`
5. Submit Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## 🆘 Support

- 📧 Email: [Project Email]
- 🐛 Bug Reports: [GitHub Issues](https://github.com/14790897/optical-zoom-camera/issues)
- 💬 Discussions: [GitHub Discussions](https://github.com/14790897/optical-zoom-camera/discussions)

---

*Providing precise stepper motor control solutions for camera optical systems* 📸