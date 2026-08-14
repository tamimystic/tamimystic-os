# Tamimystic OS

Tamimystic OS is an advanced, production-grade operating system designed for edge devices, robotics, and artificial intelligence applications. Built primarily for the ESP32 architecture, it provides a robust, modular framework that decouples hardware interactions from core logic, enabling seamless simulation on native desktop environments (Windows/POSIX) prior to hardware deployment.

## Key Features

- **Hardware Abstraction Layer (HAL)**
  A universal driver framework for GPIO, PWM, I2C, and other peripherals, allowing code to run unmodified on both ESP32 hardware and native desktop simulations.

- **Dynamic Execution Engine**
  Unlike traditional embedded systems with hardcoded logic, Tamimystic OS dynamically loads AI models (TFLite Micro) and WebAssembly (WASM) applications directly from its Virtual File System (VFS).

- **Web-Based OTA & Dashboard**
  Includes a fully responsive, modern web dashboard for system monitoring, robotics teleoperation, and Over-The-Air (OTA) file management, enabling users to upload models and applications seamlessly.

- **Non-Volatile Storage (NVS)**
  Persistent configuration management ensuring system settings, Wi-Fi credentials, and application states are safely retained across reboots.

- **Dual Target Build System**
  Engineered with CMake to support direct compilation via ESP-IDF for ESP32 microcontrollers, or via MinGW/GCC for local PC simulation and testing.

## System Architecture

The OS is divided into several highly modular components:
- **os_core**: The central event bus and system initialization logic.
- **os_hal**: The Hardware Abstraction Layer for GPIO, PWM, and I2C.
- **os_storage**: Virtual File System integration (SPIFFS for ESP32, local directory for Native).
- **os_config**: Persistent configuration using NVS.
- **os_network**: Wi-Fi station management and networking utilities.
- **os_web**: HTTP server and WebSocket host for the user dashboard and API.
- **os_robotics**: Motor control, sensor fusion, and robotics algorithms.
- **os_ai**: Edge inference engine utilizing TensorFlow Lite Micro.
- **os_apps**: WebAssembly runtime for executing third-party modules.

## Building the OS

### Native Simulation (Windows/Linux)
You can compile and run the operating system as a native application on your PC.
1. Create a `build` directory.
2. Run `cmake ..` and `cmake --build .`
3. Execute the resulting binary (e.g., `tamimystic_os_sim.exe`). The system will emulate a network and start the web dashboard on `http://localhost:8080`.

### ESP32 Hardware Deployment
To deploy on an actual ESP32-S3:
1. Setup the ESP-IDF framework.
2. Ensure your target is set to `esp32s3`.
3. Build and flash using the standard `idf.py build flash monitor` command.

## License
Tamimystic OS is proprietary software developed for advanced robotics and edge computing research. All rights reserved.
