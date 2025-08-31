# LVML Demo

## What is LVML?

**LVML** is a powerful combination of **LVGL XML** and **web server integration** that revolutionizes embedded UI development. This approach is similar to the World Wide Web (WWW)  concept but uses LVGL XML for IoT displays, enabling dynamic UI updates without firmware recompilation.

## 🎯 The LVML Concept

LVML combines the best of both worlds:
1. **LVGL's XML system** - Clean, declarative UI definitions
2. **Web server integration** - Remote UI management and updates
3. **Dynamic loading** - Runtime UI changes without firmware updates
4. **Scalable architecture** - Manage multiple devices from central servers

This creates a **web-like development workflow** for embedded systems, where developers can:
- Write UIs in **readable XML** instead of verbose C code
- **Serve UIs from web servers** using familiar HTTP protocols
- **Update device interfaces remotely** without touching firmware
- **Scale to many devices** with centralized UI management

## 🚀 Why LVML Matters

Traditional embedded UI development requires:
- Writing UI code in C/C++
- Recompiling firmware for every UI change
- Uploading new firmware to devices
- Managing UI code scattered across multiple projects

**LVML eliminates these pain points** by providing:
- **Declarative UI definition** in XML format
- **Remote UI updates** via HTTP
- **No firmware recompilation** for UI changes
- **Centralized UI management** for multiple devices

## 🎯 Demo Goals

This project serves as a **practical example** of LVGL's XML system, showing developers how to:
1. **Write LVGL UIs in XML format** - Clean, readable, maintainable
2. **Integrate with web servers** - Remote UI management and updates
3. **Create dynamic, updatable interfaces** - Change UI without touching firmware
4. **Leverage LVGL's component system** - Reusable UI components

## 🚀 Key Demonstrations

### 1. LVGL XML Format
- **Component-based UI definition** using XML syntax
- **Event binding** with `<lv_event-call_function>` elements
- **Styling and layout** using LVGL's XML attributes
- **Nested object hierarchy** for complex UI structures

### 2. Web Server Integration
- **HTTP-based UI loading** from remote server
- **Dynamic content delivery** without firmware updates
- **Real-time UI modifications** by updating XML files on server
- **Scalable UI management** for multiple devices

### 3. Runtime UI Updates
- **Hot-swappable screens** loaded on-demand
- **Memory-efficient UI switching** with proper cleanup
- **Event-driven navigation** between different XML-defined screens

## 🏗️ Architecture

### Hardware
- **Board**: ESP32-S3 Box 3
- **Display**: 320x240 TFT with ILI9342 driver
- **Touch**: GT911 capacitive touch controller
- **Memory**: 16MB PSRAM for efficient buffer management

### Software Stack
- **Framework**: Arduino + PlatformIO
- **Graphics**: LVGL 9.3.0
- **UI Definition**: LVGL XML Component System
- **Network**: WiFi + HTTP client for dynamic content loading

## 📁 Project Structure

```
├── src/
│   └── main.cpp              # Main firmware code
├── lvml_web/                 # XML UI definitions
│   ├── main.xml             # Initial screen with Next button
│   └── step1.xml            # Second screen with navigation
├── include/                  # Configuration headers
│   ├── lv_conf.h            # LVGL configuration
│   ├── WifiConfig.h         # WiFi credentials
│   └── GT911_Setup.h        # Touch controller setup
├── boards/                   # Board-specific configurations
├── platformio.ini           # PlatformIO configuration
└── README.md                # This file
```

## 🔧 Setup Instructions

### Prerequisites
- PlatformIO IDE or CLI
- ESP32-S3 Box 3 development board
- WiFi network access
- HTTP server to host XML files

### 1. Clone the Repository
```bash
git clone git@github.com:liusida/LVML-demo.git
cd LVML-demo
```

### 2. Configure WiFi
Create `include/WifiConfig.h`:
```cpp
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#define WIFI_SSID "Your_WiFi_SSID"
#define WIFI_PASSWORD "Your_WiFi_Password"

#endif
```

### 3. Configure HTTP Server
Set up a web server (e.g., Python, Node.js, or any HTTP server) to serve the XML files from the `lvml_web/` directory.

Example Python server:
```bash
cd lvml_web
python -m http.server 8866
```

### 4. Build and Upload
```bash
# Build the project
pio run

# Upload to ESP32-S3 Box 3
pio run --target upload

# Monitor serial output
pio device monitor
```

## 📱 UI Screens

### Main Screen (`main.xml`)
- Simple interface with a "Next" button
- Demonstrates basic LVGL XML structure
- Includes event binding for dynamic navigation

### Step 1 Screen (`step1.xml`)
- More complex layout with multiple buttons
- Shows nested object structure
- Demonstrates styling and positioning

## 🔌 Dynamic Loading System

The firmware implements a dynamic UI loading system:

1. **Initial Load**: Loads `main.xml` from the web server
2. **Screen Navigation**: Dynamically loads new screens based on user interaction
3. **Memory Management**: Properly cleans up previous screens before loading new ones
4. **Error Handling**: Graceful fallback if XML loading fails

## 🚀 Features

- **Dynamic UI Loading**: Load UI definitions from web servers at runtime
- **Touch Support**: Full capacitive touch integration with GT911 controller
- **WiFi Connectivity**: Seamless network integration for remote UI updates
- **Memory Efficient**: Optimized memory usage with PSRAM support
- **Cross-Platform**: Works with any HTTP server and client devices

## 🔍 Technical Details

### LVGL XML Parsing
- Custom XML parser for LVGL component system
- Event binding and callback management
- Dynamic object creation and destruction
- Memory-efficient screen switching

### Network Layer
- HTTP client implementation for XML fetching
- Automatic retry mechanisms for network failures
- Configurable timeout and error handling
- Support for both HTTP and HTTPS (with proper certificates)

## 🤝 Contributing

We welcome contributions! Please feel free to:
- Report bugs and issues
- Suggest new features
- Submit pull requests
- Improve documentation

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- **LVGL Team** for the excellent graphics library and XML system
- **ESP32 Community** for the robust hardware platform
- **PlatformIO** for the excellent development environment

---

**LVML Demo** - Bringing web-like development to embedded displays! 🚀