# Tamimystic OS (ESP32-S3-N16R8)

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-ESP32--S3--N16R8-red.svg)](https://www.espressif.com/en/products/socs/esp32-s3)
[![Flash](https://img.shields.io/badge/Flash-16MB%20Quad%2FOctal-green.svg)]()
[![PSRAM](https://img.shields.io/badge/PSRAM-8MB%20Octal-orange.svg)]()
[![Build Simulation](https://img.shields.io/badge/Native%20Simulation-POSIX%2FWindows-brightgreen.svg)]()

**Tamimystic OS** is an advanced, ultra-modular, standalone Embedded Operating System designed specifically for the **ESP32-S3-N16R8** (16MB Flash, 8MB Octal PSRAM, Xtensa Dual-Core LX7 @ 240MHz). It turns the ESP32-S3 into a plug-and-play robotics brain, edge AI vision computer, and live cloud/web development environment.

* **Documentation Website**: [https://tamimystic.github.io/tamimystic-os/](https://tamimystic.github.io/tamimystic-os/)
* **Source Repository**: [https://github.com/tamimystic/tamimystic-os](https://github.com/tamimystic/tamimystic-os)

The system features a **dual-target architecture**: it compiles directly to bare-metal hardware via ESP-IDF, or runs natively on Windows/Linux as a high-fidelity desktop simulator for rapid algorithm prototyping.

---

## System Highlights & Key Features

```
+-----------------------------------------------------------------------------+
|                             TAMIMYSTIC OS CORE                              |
+-----------------------------------------------------------------------------+
|   In-Browser Web IDE & Teleop   |   Live Vision Canvas   |  Dual-Bank OTA   |
+-----------------------------------------------------------------------------+
|      MicroPython & WASM Engine  |  Multi-Drive Kinematics|  TFLite SIMD AI  |
+-----------------------------------------------------------------------------+
|      Dynamic Pin Matrix (NVS)   |  PCA9685 Servo Driver  | DVP Camera PSRAM |
+-----------------------------------------------------------------------------+
|    6.8MB LittleFS Flash Storage |  PnP Signature Engine  | Dual-Core LX7 RT |
+-----------------------------------------------------------------------------+
```

### 1. Plug & Play Hardware Discovery & Dynamic Pin Matrix (`os_pnp`)
- **Auto-Discovery Engine**: Automatically scans the I2C bus (`0x08`–`0x77`) on boot and registers drivers for over 15+ peripherals with zero manual configuration.
  - **Sensors**: MPU-6050 (6-Axis IMU), BME280/BMP280 (Environmental), VL53L0X (ToF Laser Distance), HMC5883L (Compass), INA219 (Power Monitor).
  - **Actuators & Displays**: PCA9685 (16-Channel 12-bit Servo Driver), SSD1306 / SH1106 (OLED Displays), MCP23017 (GPIO Expander), ADS1115 (16-bit ADC).
- **Dynamic Pin Matrix**: Remap any peripheral pin (I2C SDA/SCL, Motor PWM, Encoders, Camera DVP) at runtime via CLI or Web UI with persistent NVS backing.
- **Hardware Protection**: Automatically shields Octal Flash/PSRAM lines (GPIO 33–37) and strapping pins (GPIO 0, 45, 46) from accidental re-assignment.

### 2. Universal Robotics & Multi-Drive Kinematics Subsystem (`os_robotics`)
- **Kinematics Engine**:
  - **Differential Drive (2WD / 4WD)**: Forward/angular velocity decomposition $(v, \omega) \to (v_L, v_R)$.
  - **Mecanum 4WD**: 4-wheel independent vector decomposition for 360° holonomic and lateral diagonal strafing.
  - **6-DOF Robotic Arm Analytical IK/FK**: Closed-form Inverse Kinematics solver converting $(X, Y, Z)$ Cartesian coordinates into joint angles $(\theta_1, \dots, \theta_4)$ in $< 1\text{ ms}$ with reachability boundary checks.
- **Multi-Channel Servo Controller**: Auto-detects PCA9685 I2C servo expander (`0x40`) with 12-bit precision; automatically falls back to onboard ESP32 LEDC PWM channels if absent.
- **Real-Time 50Hz Safety Loop on Core 1**:
  - Pinned real-time task with FreeRTOS priority.
  - **Virtual Proximity Bumper**: Auto-brakes when ToF laser distance $< 15\text{ cm}$ while driving forward.
  - Watchdog failsafe and software Emergency Stop.

### 3. Edge AI, Live Camera & Vision Autonomy Subsystem (`os_ai`)
- **DVP Parallel Camera Driver**: Supports OV2640, OV5640, and GC0308 sensors with triple-framebuffer allocation in the **8MB Octal PSRAM** (`CAMERA_FB_IN_PSRAM`).
- **TensorFlow Lite Micro with Xtensa SIMD (ESP-NN)**: Runs at **20–22 FPS** with `< 20ms` latency on Core 1.
- **Selectable Neural Models**:
  1. `MobileNet-V2 Person Detector`: Human detection & tracking.
  2. `MobileNet-SSD Object Detector`: Vehicles, traffic cones, balls, and obstacles.
  3. `Autonomous Lane & Line Follower`: Road lane boundary angle & steering calculation.
  4. `Hand Gesture Classifier`: Hand sign recognition (Stop, Forward, Left, Right).
- **Vision-Guided Autonomy**: Autonomous target tracking loop calculates horizontal error $e_x$ from bounding box center and commands `RobotController::setTwist()` to track locked targets automatically.

### 4. Dynamic Scripting Engine (MicroPython & WASM Runner) (`os_apps`)
- **On-the-Fly Execution**: Run Python scripts or WebAssembly modules without recompiling or re-flashing firmware.
- **Tamimystic OS Python API Bindings**:
  ```python
  import tamimystic
  
  # Read distance sensor
  dist = tamimystic.sensor.read_distance()
  print("Distance:", dist, "cm")
  
  # Drive robot
  if dist > 20.0:
      tamimystic.robot.move(40, 0)
  else:
      tamimystic.robot.move(0, 30)
      
  # Cartesian Robotic Arm Control
  tamimystic.robot.ik(14.0, 4.0, 10.0)
  ```
- **Autorun Support**: Automatically executes `/storage/autorun.py` on system boot if present in Flash storage.

### 5. 16MB Custom Partition Scheme & Dual-Bank OTA (`partitions_16mb.csv`)
- **Dual-Bank OTA Updates**: Two 4.5MB application partitions (`app0` and `app1`) with rollback protection.
- **6.8MB LittleFS VFS**: Massive flash storage partition for user scripts, WASM binaries, neural model weights, and web assets.

| Partition Name | Type | Subtype | Offset | Size | Purpose |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `nvs` | `data` | `nvs` | `0x009000` | 64 KB | Configuration & Pin Matrix |
| `otadata` | `data` | `ota` | `0x010000` | 8 KB | Dual-Boot switching state |
| `phy_init` | `data` | `phy` | `0x012000` | 4 KB | RF calibration data |
| `app0` | `app` | `ota_0` | `0x020000` | 4.5 MB | Active Primary Firmware |
| `app1` | `app` | `ota_1` | `0x4A0000` | 4.5 MB | Rollback Secondary Firmware |
| `storage` | `data` | `spiffs` | `0x920000` | 6.8 MB | LittleFS File System |

### 6. Premium In-Browser Web Dashboard & IDE (`os_web`)
Accessible at `http://<device-ip>/` (or `http://localhost:8080` in PC simulation):
- **360° Touch Virtual Joystick**: Real-time rover and mecanum control.
- **Robotic Arm Deck**: 6-axis joint sliders + Cartesian $(X, Y, Z)$ IK solver.
- **In-Browser Python IDE**: Code editor, sample script loader, "Run Script", "Stop", and live stdout terminal.
- **Live Camera Stream & Vision Canvas**: Real-time snapshot feed with dynamic bounding box overlay canvas.
- **Flash File Manager**: Browse, upload, download, and delete files on the 6.8MB storage partition.
- **Dual-Bank OTA Widget**: Displays active boot partition and allows `.bin` firmware upgrades.
- **Plug & Play Table & Dynamic Pin Matrix Remapper**: Configure pins and view connected sensors live.

---

## CLI Shell Reference (`os_cli`)

Tamimystic OS provides an interactive serial REPL shell (`aeron> `) over UART / USB:

| Command | Arguments | Description |
| :--- | :--- | :--- |
| `help` | | List all available commands |
| `sysinfo` | | Display ESP32-S3 CPU, Flash, PSRAM, and Task stats |
| `pnp scan` | | Scan I2C bus for newly connected devices |
| `pnp list` | | List all active auto-detected sensors & drivers |
| `pin list` | | Show current GPIO pin matrix mapping & safety status |
| `pin set` | `<func> <gpio>` | Remap peripheral pin (e.g. `pin set i2c_sda 10`) |
| `pin reset` | | Reset all pin mappings to default NVS config |
| `robot mode` | `<diff\|mecanum\|arm>` | Switch robot kinematics mode |
| `robot move` | `<linear%> <angular%>` | Command differential drive velocity |
| `robot strafe`| `<vx%> <vy%> <w%>` | Command mecanum 4WD holonomic velocity |
| `robot arm` | `<j1> <j2> <j3> <j4> <j5> <j6>` | Set 6-axis arm joint angles (0°–180°) |
| `robot ik` | `<x> <y> <z>` | Solve Inverse Kinematics for target Cartesian point (cm) |
| `robot stop` | | Trigger Emergency Brake |
| `robot resume`| | Release Emergency Brake |
| `robot status`| | Display robot kinematics telemetry |
| `camera snap` | | Capture snapshot from DVP camera |
| `camera status`| | Display camera hardware and PSRAM framebuffer state |
| `ai status` | | Display active neural model, FPS, and SIMD latency |
| `ai model` | `<person\|object\|lane\|gesture>` | Switch active neural model |
| `ai track` | `<on\|off>` | Toggle autonomous vision target tracking |
| `python eval`| `"<code_string>"` | Evaluate inline Python code |
| `python run` | `<file.py>` | Run Python script from 6.8MB Flash storage |
| `python stop`| | Terminate currently running Python script |
| `wasm run` | `<file.wasm>` | Execute WebAssembly binary module |
| `storage ls` | | List all files in LittleFS storage |
| `storage df` | | Show storage capacity, used space, and free space |
| `storage rm` | `<filename>` | Delete a file from flash storage |
| `ota status` | | Display active partition (`app0`/`app1`) and rollback state |

---

## Getting Started

### 1. Running on Native PC Simulation (Windows / Linux / macOS)
You can compile and run the full OS natively on your computer with simulated camera, sensors, robotics kinematics, and web dashboard:

```bash
# Clone the repository
git clone https://github.com/tamimystic/tamimystic-os.git
cd tamimystic-os

# Create build directory and compile with CMake (MinGW / GCC / Clang)
mkdir build
cd build
cmake ..
cmake --build .

# Run simulator
./main/tamimystic_os_sim.exe
```

Open your browser and navigate to `http://localhost:8080` to access the complete Web Dashboard & IDE.

---

### 2. Flashing to ESP32-S3-N16R8 Hardware

#### Prerequisites:
- ESP-IDF v5.0 or later installed and configured (`export.bat` / `export.sh`).
- ESP32-S3-N16R8 development board connected via USB.

#### Build & Flash Commands:
```bash
# Set target to ESP32-S3
idf.py set-target esp32s3

# Build, flash, and monitor
idf.py build
idf.py -p COM_PORT flash monitor
```

---

## Repository Structure

```
tamimystic-os/
├── CMakeLists.txt              # Top-level dual-target CMake configuration
├── partitions_16mb.csv         # 16MB Flash partition table (Dual OTA + 6.8MB Storage)
├── components/
│   ├── os_hal/                 # Hardware Abstraction Layer (ESP32-S3 & Native POSIX)
│   ├── os_core/                # System bring-up, lifecycle, and event bus
│   ├── os_scheduler/           # Dual-core FreeRTOS & Native thread scheduler
│   ├── os_config/              # NVS key-value persistent storage
│   ├── os_storage/             # 6.8MB LittleFS VFS & File Manager
│   ├── os_network/             # Wi-Fi Station & AP connection manager
│   ├── os_pnp/                 # Plug & Play signature database & Pin Matrix
│   ├── os_robotics/            # Kinematics engine, PCA9685 driver & 50Hz safety loop
│   ├── os_ai/                  # Camera DVP driver, TFLite SIMD & vision autonomy
│   ├── os_apps/                # MicroPython & WASM dynamic scripting runtime
│   ├── os_cli/                 # Serial REPL CLI shell
│   └── os_web/                 # High-performance HTTP server, Web IDE & Dashboard
├── docs/                       # Material for MkDocs documentation files
├── main/
│   ├── main.cpp                # System entry point
│   └── CMakeLists.txt          # Main component definition
├── ROADMAP.md                  # Comprehensive technical roadmap
└── mkdocs.yml                  # MkDocs documentation configuration
```

---

## License
Tamimystic OS is licensed under the [MIT License](LICENSE).
Developed by **Tamimystic**.
