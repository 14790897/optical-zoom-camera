# Optical Zoom Camera Controller
ESP32C3-based dual stepper motor controller with web interface for camera optical zoom control. The system provides HTTP API endpoints and a responsive web UI for controlling two A4988 stepper motor drivers.

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively
- Bootstrap and build the repository:
  - Install PlatformIO: `pip3 install platformio`
  - Add to PATH: `export PATH=$PATH:$HOME/.local/bin`
  - Create secrets.h: Copy secrets.h template and configure WiFi credentials
  - Build: `pio run` -- takes 3-5 minutes for initial setup with platform download. NEVER CANCEL. Set timeout to 20+ minutes.
  - Upload to device: `pio run -t upload` -- takes 2-3 minutes. NEVER CANCEL. Set timeout to 10+ minutes.
  - Monitor serial: `pio device monitor -b 115200`
- Build for filesystem (web interface): `pio run -t uploadfs` -- uploads data/index.html to device. Takes 1-2 minutes.
- Clean build: `pio run -t clean` -- takes 30 seconds.

## Required Configuration
- Create `secrets.h` in project root with the following content:
```cpp
#ifndef SECRETS_H
#define SECRETS_H

// WiFi Configuration  
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// mDNS Configuration
#define MDNS_HOST_NAME "camera-lens"  

// OTA Update Configuration
#define OTA_PASSWORD "admin123"

#endif // SECRETS_H
```

## Project Structure
- `platformio.ini`: PlatformIO configuration for ESP32C3 AirM2M board
- `src/main_web.cpp`: Main firmware with web server and motor control (493 lines)
- `main.cpp`: Simple standalone motor test code (58 lines)
- `data/index.html`: Web interface HTML/CSS/JS (655 lines)
- `secrets.h`: WiFi and OTA credentials (create from template above)

## Build Targets and Timing
- **CRITICAL**: Always use long timeouts for PlatformIO commands as they can take significant time on first run
- `pio run`: Initial build takes 3-5 minutes (platform/library download). Subsequent builds 1-2 minutes. NEVER CANCEL.
- `pio run -t upload`: Firmware upload takes 2-3 minutes. NEVER CANCEL. Set timeout to 10+ minutes.
- `pio run -t uploadfs`: Filesystem upload takes 1-2 minutes. NEVER CANCEL. Set timeout to 10+ minutes.
- `pio device monitor`: Real-time serial monitoring, runs indefinitely until stopped.

## Testing and Validation
- **Local Web Interface Testing**: Test HTML before upload with: `cd data && python3 -m http.server 8081` then verify page loads
- **Device Web Interface Testing**: After upload, access web interface at device IP or http://camera-lens.local
- **Manual Testing Scenarios**: 
  1. Test motor control: Use web buttons to run motors clockwise/counterclockwise for different durations
  2. Test API endpoints: Send HTTP GET requests to `/motor1/cw/5`, `/motor2/ccw/10`, `/both?d1=cw&d2=ccw&t=3`
  3. Test status endpoint: GET `/status` should return JSON with motor states
  4. Test system reboot: GET `/reboot` should restart ESP32
  5. Test speed control: Use `/set/speed/500` to change motor speed
- **Serial Monitor Validation**: Watch serial output at 115200 baud for motor state logs and WiFi connection status
- Always test both individual motor control and dual motor operation
- Verify web interface loads correctly and shows proper status updates
- **Configuration Validation**: Always ensure secrets.h is properly configured before building

## Hardware Configuration  
- ESP32C3 AirM2M Core board
- Dual A4988 stepper motor drivers
- Motor 1: DIR=GPIO2, STEP=GPIO3, EN=GPIO12
- Motor 2: DIR=GPIO10, STEP=GPIO6, EN=GPIO18  
- 2000 steps per revolution (1.8° step angle motors)
- Default step delay: 1000μs (adjustable via web interface)

## Network Configuration
- WiFi connection required (configured in secrets.h)
- mDNS hostname: camera-lens.local (configurable)
- Web server on port 80
- OTA updates available after initial USB flash
- CORS enabled for cross-origin development

## Development Workflow
- **Primary development**: Edit src/main_web.cpp for firmware changes
- **Web interface**: Edit data/index.html for UI changes, then run `pio run -t uploadfs`
- **Configuration**: Modify platformio.ini for build settings
- Always monitor serial output during development: `pio device monitor -b 115200`
- **OTA Development**: After initial USB flash, use: `pio run -t upload --upload-port camera-lens.local`

## Common API Endpoints
The device exposes these HTTP endpoints:
- `/motor1/cw/{duration}` - Motor 1 clockwise for {duration} seconds
- `/motor1/ccw/{duration}` - Motor 1 counterclockwise  
- `/motor2/cw/{duration}` - Motor 2 clockwise
- `/motor2/ccw/{duration}` - Motor 2 counterclockwise
- `/both?d1={dir}&d2={dir}&t={time}` - Control both motors simultaneously
- `/stop/motor1` - Stop motor 1
- `/stop/motor2` - Stop motor 2  
- `/stop/both` - Stop both motors
- `/set/speed/{delay}` - Set step delay in microseconds
- `/status` - Get JSON status of both motors
- `/reboot` - Restart ESP32
- `/` - Serve web interface

## Troubleshooting
- **Build failures**: Ensure secrets.h exists and has valid syntax
- **Upload failures**: Check USB connection, try different baud rates in platformio.ini
- **WiFi connection issues**: Verify credentials in secrets.h, check serial monitor for connection status  
- **Web interface not loading**: Run `pio run -t uploadfs` to upload HTML files
- **Motor not moving**: Check wiring, power supply, and enable pin states (active low)
- **mDNS not resolving**: Use IP address directly, check network mDNS support

## Common Command Outputs
The following are outputs from frequently run commands. Reference them instead of running bash commands to save time.

### Repository Structure
```
ls -la
.
..
.git/
.github/
.gitignore
.vscode/
AGENTS.md
airm2m.json
data/
lib/
main.cpp
official.json
platformio.ini
secrets.h
src/
test/
```

### PlatformIO Environment
```
pio system info
--------------------------  ---------------------------------------------
PlatformIO Core             6.1.18+
Python                      3.12+
Platform                    Linux/Windows/macOS
File System Encoding        utf-8
```

### File Encoding
Always use UTF-8 encoding for all files to prevent corruption, especially with Chinese characters in comments.