# Building Tamimystic OS from Source

Tamimystic OS is architected for dual-target compilation:
1. **Target Hardware (ESP32-S3-N16R8)**: Cross-compiled via the Espressif ESP-IDF v5.2+ toolchain with full hardware driver integration and SIMD vector acceleration.
2. **Native Desktop Simulator (PC - Windows / Linux / macOS)**: Compiled via native GCC / Clang / MSVC to simulate sensor auto-discovery, kinematics, SLAM, and Web APIs directly on your workstation without physical hardware.

---

## 1. Building for Target Hardware (ESP32-S3)

### Step 1: Install ESP-IDF v5.2 Toolchain
Follow the official [Espressif ESP-IDF Installation Guide](https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32s3/get-started/) to install ESP-IDF v5.2 LTS.

Activate the environment variables in your terminal:
```bash
# On Linux / macOS:
. $HOME/esp/esp-idf/export.sh

# On Windows (PowerShell):
. $HOME/esp/esp-idf/export.ps1
```

### Step 2: Clone the Repository
```bash
git clone https://github.com/tamimystic/tamimystic-os.git
cd tamimystic-os
```

### Step 3: Set Hardware Target
```bash
idf.py set-target esp32s3
```

### Step 4: Build Firmware Binary Suite
```bash
idf.py build
```

The build system invokes CMake and Ninja to compile all kernel components with strict `-Wall -Werror=all` flags. Output binaries are generated in the `build/` directory:
- `build/bootloader/bootloader.bin`
- `build/partition_table/partition-table.bin`
- `build/ota_data_initial.bin`
- `build/tamimystic-os.bin`

### Step 5: Flash and Open Serial Monitor
```bash
# Flash at 921600 baud and launch serial monitor
idf.py -p COM4 -b 921600 flash monitor
```

To exit the serial monitor, press `Ctrl + ]`.

---

## Key Hardware Configuration Flags (`sdkconfig.defaults`)

The project relies on specific hardware configurations optimized for the ESP32-S3-N16R8:

| Configuration Variable | Value | Purpose |
|---|---|---|
| `CONFIG_ESP32S3_SPIRAM_SUPPORT` | `y` | Enables external SPI PSRAM support. |
| `CONFIG_SPIRAM_MODE_OCT` | `y` | Configures 8-line Octal SPI high-speed PSRAM mode. |
| `CONFIG_SPIRAM_SPEED_80M` | `y` | Sets PSRAM clock frequency to 80 MHz. |
| `CONFIG_SPIRAM_USE_MALLOC` | `y` | Allows dynamic allocations $> 4\text{ KB}$ to automatically route to PSRAM. |
| `CONFIG_FREERTOS_HZ` | `1000` | Sets FreeRTOS tick rate to 1000 Hz ($1\text{ ms}$) for deterministic control. |
| `CONFIG_COMPILER_OPTIMIZATION_PERF` | `y` | Enables `-O2` compiler optimization for high SIMD throughput. |

---

## 2. Building the Native Desktop Simulator (PC Environment)

The native simulator allows rapid algorithm development, automated unit testing, and UI preview without requiring an ESP32 board.

### Building on Windows (MinGW / Make):
```powershell
# Create build directory and generate Makefiles
cmake -B build -G "MinGW Makefiles"

# Compile simulator executable
cmake --build build -j8

# Run Simulator
.\build\tamimystic_os_sim.exe
```

### Building on Linux / macOS (GCC / Clang):
```bash
# Generate build files
cmake -B build

# Compile simulator binary
cmake --build build -j$(nproc)

# Run Simulator
./build/tamimystic_os_sim
```

### Simulator Features:
- **Simulated I2C Bus**: Emulates MPU-6050, VL53L0X, and BME280 sensor data.
- **Simulated Kinematics Rover**: Simulates motor encoders and updates Cartesian odometry in real time.
- **Simulated 2D LiDAR**: Synthesizes a virtual $4\text{m} \times 4\text{m}$ room with obstacles for testing SLAM and $A^*$ path planning.
- **Local HTTP Web Server**: Starts the Web Dashboard at `http://localhost:8080`.

---

## 3. Flash Memory and Static Analysis Profiling

To inspect memory consumption and ensure internal SRAM buffers remain within bounds:

```bash
# Display overall partition and RAM memory usage
idf.py size

# Display component-by-component memory breakdown
idf.py size-components

# Display detailed per-symbol memory map
idf.py size-files
```
