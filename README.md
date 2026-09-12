# Tamimystic OS (ESP32-S3-N16R8)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3--N16R8-red.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Flash](https://img.shields.io/badge/Flash-16MB%20Quad%2FOctal-green.svg)]()
[![PSRAM](https://img.shields.io/badge/PSRAM-8MB%20Octal-orange.svg)]()
[![Build Simulation](https://img.shields.io/badge/Native%20Simulation-POSIX%2FWindows-brightgreen.svg)]()

**Tamimystic OS** is an enterprise-grade, deterministic, dual-core embedded operating system designed specifically for the **ESP32-S3-N16R8** (16MB Flash, 8MB Octal PSRAM, Xtensa Dual-Core LX7 @ 240MHz). It transforms the ESP32-S3 into an autonomous robotics brain, edge AI vision processor, 2D LiDAR SLAM navigator, micro-ROS distributed node, voice-controlled assistant, and live in-browser development environment.

* **Documentation Website**: [https://tamimystic.github.io/tamimystic-os/](https://tamimystic.github.io/tamimystic-os/)
* **Source Repository**: [https://github.com/tamimystic/tamimystic-os](https://github.com/tamimystic/tamimystic-os)

The system features a **dual-target architecture**: it compiles directly to bare-metal hardware via ESP-IDF v5.2+, or runs natively on Windows/Linux as a high-fidelity desktop simulator for rapid algorithm prototyping.

---

## System Architecture Matrix

```
+-----------------------------------------------------------------------------------------+
|                                    TAMIMYSTIC OS CORE                                   |
+-----------------------------------------------------------------------------------------+
|  In-Browser Web IDE & Dashboard |   Live DVP Video Stream      | Dual-Bank A/B OTA      |
+---------------------------------+------------------------------+------------------------+
|  MicroPython Virtual Engine     |   Multi-Drive Kinematics     | MobileNet SIMD AI      |
+---------------------------------+------------------------------+------------------------+
|  Dynamic Software Pin Matrix    |   6-DOF Robotic Arm IK       | micro-ROS (XRCE-DDS)   |
+---------------------------------+------------------------------+------------------------+
|  2D LiDAR SLAM & A* Navigation  |   I2S Audio DSP & KWS Voice  | ESP-NOW Swarm Mesh     |
+---------------------------------+------------------------------+------------------------+
|  4.375MB LittleFS VFS Flash     |   PnP Sensor Auto-Discovery  | Dual-Core 240MHz RTOS  |
+-----------------------------------------------------------------------------------------+
```

---

## Core Subsystems Overview

### 1. Dynamic Software Pin Matrix & PnP Auto-Discovery (`os_pnp`)
- **PnP Auto-Discovery**: Automatically sweeps the I2C bus (`0x08` to `0x77`) on startup and registers drivers without firmware modification.
  - **Sensors**: MPU-6050 (6-DOF IMU), VL53L0X (Laser Distance), BME280 (Environmental), SSD1306 (128x64 OLED), PCA9685 (16-Ch PWM Servo), ADS1115 (16-bit ADC).
- **Dynamic Pin Matrix**: Remap any peripheral pin (I2C SDA/SCL, Motor PWM, Encoders, I2S Audio, LEDs) at runtime via CLI, REST API, or Web UI with persistent NVS backing.
- **Hardware Protection**: Prevents accidental re-assignment of Octal Flash/PSRAM lines (GPIO 26-37) and strapping pins (GPIO 0, 45, 46).

### 2. Universal Robotics & Multi-Drive Kinematics (`os_robotics`)
- **Kinematics Engine**:
  - **Differential Drive**: Forward and angular velocity decomposition $(v, \omega) \to (\omega_L, \omega_R)$.
  - **Mecanum Omnidirectional (4WD)**: 4-wheel independent vector decomposition for 360-degree holonomic strafing.
  - **Ackermann Steering**: Automotive geometry with coordinated steering angles.
  - **6-DOF Robotic Arm Analytical IK/FK**: Closed-form analytical Inverse Kinematics solver converting Cartesian coordinates $(X, Y, Z, \text{Roll}, \text{Pitch})$ into joint angles in $< 0.8\text{ ms}$.
- **1000 Hz Hard Real-Time Motion Loop on Core 1**:
  - Dual closed-loop PID velocity controller with feed-forward and anti-windup.
  - Fail-safe communication timeout watchdog (500ms) and software/hardware Emergency Stop (E-STOP).

### 3. Native ROS 2 & micro-ROS Integration (`os_ros2`)
- Native **eProsima Micro XRCE-DDS** client running on Core 0 over UDP (port 8888) or Serial UART.
- Interoperates with ROS 2 Jazzy, Iron, and Humble.
- Standard Topics: `/cmd_vel`, `/odom`, `/scan`, `/imu/data_raw`, `/joint_states`, `/tf`, `/diagnostics`.

### 4. 2D LiDAR Driver, SLAM & Path Planning (`os_slam`)
- High-speed UART driver for 360-degree 2D LiDAR scanners (RPLiDAR A1/A2, LD06, YDLIDAR).
- **2D Occupancy Grid Map**: $80 \times 80$ discrete grid cells with Bayesian log-odds probability updates and Bresenham raycasting in 8MB PSRAM.
- **Global Path Planning**: $A^*$ (A-Star) search algorithm with Euclidean heuristics and obstacle inflation.

### 5. Edge AI, DVP Camera & Visual Tracking (`os_ai`, `os_tracking`)
- **DVP Parallel Camera**: Supports OV2640, OV3660, and OV5640 sensors with double-buffering in 8MB Octal PSRAM.
- **Xtensa LX7 PIE Vector SIMD Acceleration**: Runs quantized INT8 neural models at up to 55+ FPS.
- **Models**: MobileNet-V2 Object Classifier, Person Detector, BlazeFace, Hand Gesture Recognizer.
- **Autonomous Tracking**: Closed-loop visual servoing PID controller for Color Blobs, AprilTags, and Neural Bounding Boxes.

### 6. Audio Edge AI, Voice Control & Speech Synthesis (`os_audio`)
- **Digital I2S Interface**: Knowles INMP441 MEMS microphone and MAX98357A 3.2W Class-D amplifier.
- **Real-Time DSP**: 16 kHz 16-bit PCM windowing with Mel-Frequency Cepstral Coefficients (MFCC) feature extraction.
- **Keyword Spotting (KWS)**: On-device 1D-CNN recognizing *"Hey Mystic"*, *"Forward"*, *"Backward"*, *"Stop"*, *"Turn Left"*, *"Turn Right"*.
- **Voice Feedback**: Embedded WAV audio synthesizer for spoken alerts.

### 7. ESP-NOW Mesh Swarm Robotics (`os_espnow`)
- Ultra-low-latency ($< 4\text{ ms}$) connectionless 2.4 GHz RF protocol operating concurrently with Wi-Fi.
- Multi-robot Leader-Follower formation kinematics with automatic geometric coordinate offsets.
- Handheld wireless controller bridge for low-latency remote teleoperation.

### 8. Bluetooth Low Energy (BLE 5.0) & Web Bluetooth (`os_ble`)
- High-efficiency NimBLE GATT server with custom 128-bit Robotics Service UUID.
- Exposes 20Hz real-time motion telemetry, 6-DOF arm control, and bidirectional joystick twist.
- Zero-install pairing directly via Web Bluetooth API in Google Chrome and Microsoft Edge.

### 9. Automated Testing & Subsystem Benchmarks (`tests/`, `os_cli`)
- 13 comprehensive CTest component test suites with 100% pass verification on host and CI.
- Serial CLI benchmark runner (`bench [all|ik|kinematics|slam]`) measuring microsecond latency and ops/sec throughput.
- Integrated GitHub Actions automated workflow for continuous regression prevention.

### 10. MicroPython Sandboxed Engine & In-Browser Web IDE (`os_apps`, `os_web`)
- Embedded MicroPython runtime with 512 KB dedicated PSRAM heap.
- Comprehensive `tamimystic` standard library (`system`, `pin`, `motion`, `arm`, `sensor`, `camera`, `ai`, `slam`, `espnow`, `ble`, `audio`).
- In-browser code editor with live WebSocket REPL console and 4.375 MB LittleFS VFS.

### 11. Dual-Bank A/B OTA Firmware Updates (`os_ota`)
- Symmetrical 4.5 MB application partitions (`ota_0` and `ota_1`) with automated bootloader rollback protection.
- Web-based drag-and-drop firmware flashing and REST binary streaming.

---

## 16 MB Flash Partition Table (`partitions.csv`)

| Partition Name | Type | Subtype | Offset | Size | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `nvs` | `data` | `nvs` | `0x009000` | 24 KB | Non-Volatile Configuration & Pin Matrix |
| `otadata` | `data` | `ota` | `0x00F000` | 8 KB | Dual-Bank boot switching state |
| `phy_init` | `data` | `phy` | `0x011000` | 4 KB | RF calibration data |
| `ota_0` | `app` | `ota_0` | `0x020000` | 4.5 MB | Primary Executable Firmware Application |
| `ota_1` | `app` | `ota_1` | `0x4A0000` | 4.5 MB | Secondary Rollback Firmware Application |
| `model` | `data` | `undefined` | `0x920000` | 2.5 MB | INT8 Quantized Deep Learning Models |
| `storage` | `data` | `littlefs` | `0xBA0000` | 4.375 MB | LittleFS Virtual File System |

---

## Interactive Serial CLI Shell (`tamimystic>`)

Connect via USB Serial at **115200 baud**:

```bash
# Display system health and memory stats
tamimystic> status

# Drive differential chassis
tamimystic> motion drive 0.5 0.0

# Solve 6-DOF Inverse Kinematics for target coordinate
tamimystic> arm ik 150.0 0.0 120.0 0.0 30.0 0.0

# Start 2D LiDAR SLAM
tamimystic> slam start

# Run single-shot AI neural classification
tamimystic> ai run

# Broadcast swarm velocity vector over ESP-NOW
tamimystic> espnow swarm 0.4 0.1
```

---

## Getting Started

### 1. Compiling and Running on Native PC Simulation
```bash
# Clone repository
git clone https://github.com/tamimystic/tamimystic-os.git
cd tamimystic-os

# Build simulator with CMake
cmake -B build
cmake --build build

# Launch Simulator (Runs Web Dashboard at http://localhost:8080)
./build/tamimystic_os_sim
```

### 2. Flashing to ESP32-S3-N16R8 Hardware
```bash
# Configure target
idf.py set-target esp32s3

# Build firmware
idf.py build

# Flash and launch serial monitor
idf.py -p COM4 -b 921600 flash monitor
```

---

## License
Tamimystic OS is licensed under the [MIT License](LICENSE).
Developed by **Tamimystic**.
