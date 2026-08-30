# 🛠️ Building from Source (ESP32-S3 & PC Simulation)

Tamimystic OS features a dual-target architecture that can be compiled natively on a PC (Windows / Linux / macOS) for rapid simulation, or cross-compiled with ESP-IDF for physical ESP32-S3-N16R8 hardware.

---

## 🎯 Target 1: Cross-Compiling for ESP32-S3 Hardware

### Prerequisites:
1. **ESP-IDF v5.2 or higher** installed and configured in your environment.
2. CMake 3.16+ and Ninja build system.

### Build Steps:
```bash
# 1. Clone the repository
git clone https://github.com/tamimystic/tamimystic-os.git
cd tamimystic-os

# 2. Set target to ESP32-S3
idf.py set-target esp32s3

# 3. Build firmware
idf.py build

# 4. Flash to ESP32-S3 and open serial monitor
idf.py -p COM3 flash monitor
```

---

## 🖥️ Target 2: Native PC Simulation Build (Windows / Linux)

The Native Simulation build compiles the complete operating system, FreeRTOS task simulation, event bus, kinematics math, neural models, LittleFS VFS, serial CLI, and local web server using your standard host C++17 compiler (GCC / MinGW / Clang / MSVC).

### On Windows (MinGW):
```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
./tamimystic_os_sim.exe
```

### On Linux / macOS (GCC / Clang):
```bash
mkdir build && cd build
cmake ..
make -j4
./tamimystic_os_sim
```

---

## 📁 Repository Directory Structure

```text
tamimystic-os/
├── components/
│   ├── os_hal/          # Universal Hardware Abstraction Layer (GPIO, PWM, I2C, UART)
│   ├── os_core/         # Thread-safe Event Bus and System Dispatcher
│   ├── os_scheduler/    # FreeRTOS Dual-Core Task Scheduler & Native POSIX Thread Pool
│   ├── os_config/       # NVS Persistent Key-Value Configuration Store
│   ├── os_pnp/          # Plug & Play I2C Auto-Discovery Registry & Dynamic Pin Matrix
│   ├── os_robotics/     # Kinematics Engine (2WD, Mecanum, 6-DOF IK/FK, PCA9685)
│   ├── os_ai/           # DVP Camera Driver & TensorFlow Lite Micro SIMD Inference
│   ├── os_storage/      # LittleFS 6.8MB Flash Virtual File System (VFS)
│   ├── os_apps/         # MicroPython & WASM Dynamic Scripting Runners
│   ├── os_cli/          # Serial Interactive Terminal Shell (aeron>)
│   ├── os_network/      # Wi-Fi Station & Captive Network Manager
│   └── os_web/          # Asynchronous HTTP Web Server & Glassmorphism Dashboard
├── docs/                # Comprehensive MkDocs Documentation Markdown Files
├── main/                # System Boot Bring-up Entrypoint (app_main / main)
├── partitions.csv       # 16MB Flash Partition Table (Dual OTA + LittleFS)
├── sdkconfig.defaults   # Hardware configuration defaults for ESP32-S3-N16R8
├── mkdocs.yml           # Material for MkDocs Configuration
└── CMakeLists.txt       # Unified Dual-Target CMake Build Script
```
