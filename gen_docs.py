import os

docs = {
    "docs/index.md": """# Welcome to Tamimystic OS

**Tamimystic OS** is an advanced, hardware-agnostic operating system designed natively for edge devices, with a primary focus on the **ESP32-S3 (N16R8)** architecture. 

## Vision and Philosophy

Historically, microcontroller firmware development has been rigidly tied to hardware configurations. Pins, sensors, and execution loops are traditionally hardcoded, requiring a complete recompilation and flashing cycle for every minor hardware adjustment.

Tamimystic OS introduces the philosophy of **Flash Once, Configure Infinitely**. The core objective is to decouple the hardware from the software logic. By flashing the operating system binary only once, developers and engineers can dynamically route hardware pins, swap peripherals, and update business logic in real-time without ever invoking a C++ compiler again.

## Core Capabilities

1.  **Dual-Target Architecture**: The OS is structured with a strict separation of concerns, allowing the core scheduler and application layers to compile natively on Windows and Linux for rapid simulation, while maintaining an ESP-IDF cross-compilation pipeline for the physical ESP32-S3 hardware.
2.  **Pre-emptive Task Scheduling**: Built upon FreeRTOS, the OS provides true multithreading. Background tasks, network events, and hardware interrupts are processed concurrently without blocking user-level application execution.
3.  **Universal Hardware Abstraction Layer (HAL)**: A completely dynamic driver matrix abstracts UART, I2C, SPI, and PWM. Hardware peripherals are bound to pins at runtime via non-volatile storage (NVS) configurations.
4.  **Edge Intelligence Engine**: A highly optimized C++ implementation of TensorFlow Lite Micro allows the OS to execute neural network inferences (Vision and Audio) directly on the edge device, utilizing the ESP32-S3 vector instructions.
5.  **MicroPython Runtime Execution**: Business logic and robotic control sequences are written in Python. The OS safely interprets these scripts on a dedicated core, providing bindings to the underlying C++ HAL and AI Engine.
6.  **Local Configuration Dashboard**: The OS spawns an asynchronous HTTP server on boot, providing a local web interface for network configuration, hardware routing, and Over-The-Air (OTA) Python script uploads.

Explore the documentation to understand the architecture, API references, and deployment procedures.
""",

    "docs/getting-started/installation.md": """# Installation Guide

The installation process for Tamimystic OS is designed to be fully reproducible. The continuous integration pipeline automatically compiles the system binaries upon every verified commit.

## Obtaining the Firmware Binaries

1. Navigate to the GitHub Actions page of the repository.
2. Select the latest successful build artifact labeled `tamimystic_os_firmware`.
3. Extract the downloaded archive. The archive contains the following critical binary files:
    *   `bootloader.bin`: The second-stage bootloader responsible for initializing flash and PSRAM.
    *   `partition-table.bin`: The memory map defining the boundaries for NVS, OTA, and Virtual File Systems (SPIFFS).
    *   `tamimystic_os.bin`: The core operating system executable.

## Flashing the Device (Windows)

The standard procedure utilizes the Espressif Flash Download Tool.

1.  Download the official Espressif Flash Download Tool.
2.  Launch the application and select **ESP32-S3** as the Target Chip and **Develop** as the WorkMode.
3.  Load the binaries into the flashing queue and assign their explicit memory offsets:
    *   `bootloader.bin` at offset `0x0`
    *   `partition-table.bin` at offset `0x8000`
    *   `tamimystic_os.bin` at offset `0x10000`
4.  Configure the hardware parameters to match the ESP32-S3 N16R8 specification:
    *   SPI SPEED: **80MHz**
    *   SPI MODE: **QIO**
    *   FLASH SIZE: **16MB**
5.  Select the corresponding COM port, set the baud rate to `460800`, and initiate the flashing process.

## Flashing via Command Line (esptool.py)

For automated environments, the `esptool.py` command-line utility is recommended.

```bash
esptool.py -p COM3 -b 460800 --before default_reset --after hard_reset --chip esp32s3 write_flash --flash_mode qio --flash_size 16MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 tamimystic_os.bin
```

Upon successful flashing, issue a hardware reset to the board to begin the initialization sequence.
""",

    "docs/getting-started/first-boot.md": """# First Boot and Configuration

Upon receiving a hard reset after flashing, Tamimystic OS initiates its primary boot sequence. This document details the expected behavior and configuration steps required to bring the system online.

## Initialization Sequence

When the device powers on, the following sequence occurs internally:
1.  **Hardware Verification**: The OS validates the presence and integrity of the 8MB PSRAM and 16MB Flash memory.
2.  **Virtual File System (VFS) Mounting**: The SPIFFS partition is mounted. If this is the first boot, the OS automatically formats the partition.
3.  **Non-Volatile Storage (NVS) Loading**: The OS reads the `os_config` registry to determine network states and peripheral bindings.
4.  **Network Initialization**: If no valid Wi-Fi credentials exist in the NVS, the OS defaults to Access Point (AP) mode.

## Accessing the Configuration Dashboard

1.  Open the Wi-Fi settings on your host machine (PC or Smartphone).
2.  Scan for a network SSID named **TamimysticOS_Setup** (or similar fallback SSID).
3.  Connect to the network.
4.  Open a modern web browser and navigate to the default gateway IP address (typically `http://192.168.4.1`).

## Network and Peripheral Configuration

The Web Dashboard serves as the primary interface for system configuration.

*   **Station Mode Setup**: Enter your local router's SSID and Password to transition the device from AP mode to Station mode. Upon saving, the OS will attempt to connect to the provided network and assign itself a local IP via DHCP.
*   **Hardware Routing**: Navigate to the peripherals tab. Here, you can map physical ESP32-S3 pins to logical OS functions. For example, you can map Pin 18 and 19 to `MotorDriver_Left`. The OS writes this mapping to NVS, ensuring it persists across reboots.
*   **Application Uploads**: Use the Over-The-Air (OTA) file manager to upload your Python scripts (`main.py`) directly to the device's Virtual File System.
""",

    "docs/architecture/overview.md": """# Architecture Overview

Tamimystic OS is structured into five distinct, strictly enforced layers. This modular architecture ensures that high-level application logic remains completely isolated from low-level hardware constraints.

## 1. Hardware Abstraction Layer (HAL)
The foundation of the OS. The HAL abstracts away vendor-specific SDKs (such as ESP-IDF). It utilizes a Dynamic Pin Multiplexing system, allowing developers to route I2C, SPI, PWM, and UART protocols to physical pins at runtime via a configuration matrix, entirely eliminating the need for hardcoded pin definitions.

## 2. Core Kernel and Resource Manager
Built upon FreeRTOS, the core kernel manages thread execution and memory boundaries. It strictly isolates the 8MB PSRAM into dedicated sectors: Camera Framebuffers, AI Tensor Arenas, and MicroPython Heap memory. The kernel also pins tasks to specific CPU cores (Core 0 for networking and OS tasks, Core 1 for AI inference and application logic) to maximize throughput.

## 3. Intelligence Engine (Edge AI)
A bare-metal C++ implementation of TensorFlow Lite Micro. This engine is highly optimized for the ESP32-S3 vector instructions. It intercepts Direct Memory Access (DMA) streams from the camera buffer, performs object detection or facial recognition matrices, and pushes the resultant vectors to the Event Bus.

## 4. Application Layer (MicroPython)
The user-facing execution environment. The OS embeds a MicroPython interpreter that runs dynamically uploaded `.py` scripts. Custom C++ bindings bridge the Python interpreter to the HAL and AI engines, allowing Python scripts to invoke low-level hardware commands and read AI inference results with near-native execution speeds.

## 5. Middleware and Connectivity
The system hosts an asynchronous HTTP server and WebSockets implementation. This layer handles the Web Dashboard, REST API configurations, and Over-The-Air (OTA) firmware and file system updates.
""",

    "docs/architecture/scheduler.md": """# Core Scheduler and Event Bus

Tamimystic OS utilizes a deterministic, preemptive task scheduler built on FreeRTOS to guarantee real-time execution capabilities.

## Dual-Core Task Pinning

The ESP32-S3 features an Xtensa Dual-Core 32-bit LX7 microprocessor. Tamimystic OS explicitly pins tasks to specific cores to prevent race conditions and maximize computational bandwidth.

*   **Core 0 (System Core)**: Reserved for highly asynchronous, non-blocking tasks. This includes the Wi-Fi stack, Bluetooth Low Energy (BLE) stack, asynchronous HTTP server, and the Virtual File System (VFS) read/write operations.
*   **Core 1 (Application Core)**: Dedicated strictly to computationally heavy, blocking, or user-defined operations. This includes the TensorFlow Lite Micro inference pipeline, image processing algorithms, and the MicroPython runtime environment.

## The Event Bus Architecture

Communication between the isolated tasks and cores is managed through a thread-safe Event Bus. Instead of direct function calls across boundaries, tasks publish events to the bus, and interested subsystems subscribe to specific event topics.

*   **Decoupling**: The Wi-Fi manager does not need to know the state of the motor driver. It simply publishes a `WIFI_CONNECTED` event. The web server, subscribed to this event, will then initialize the HTTP endpoints.
*   **Data Integrity**: Data payloads passed through the Event Bus are deeply copied to prevent memory corruption when transferring pointers between contexts.
""",

    "docs/architecture/hal.md": """# Universal Hardware Abstraction Layer

The Hardware Abstraction Layer (HAL) is the bridge between the Tamimystic OS C++ core and the physical electrical interfaces of the microcontroller.

## Design Principle: Abstraction over Implementation

The OS core never interacts directly with ESP-IDF specific functions (e.g., `ledc_timer_config` or `uart_driver_install`). Instead, it calls generic OS abstractions (e.g., `os_hal_pwm_init` or `os_hal_uart_read`).

This design provides two massive advantages:
1.  **Portability**: The operating system can be ported to different microcontrollers (e.g., STM32, RP2040) simply by rewriting the underlying HAL implementations, without changing a single line of OS core logic.
2.  **Native Simulation**: When compiled on Windows or Linux, the HAL intercepts hardware calls and redirects them to the terminal console (e.g., printing `[PWM] Set Pin 18 to 50%` instead of actually toggling a physical pin). This allows developers to test OS logic entirely in software.

## Dynamic Multiplexing

Unlike traditional firmware where `MOTOR_PIN` is defined as a macro `#define MOTOR_PIN 18` at compile time, Tamimystic OS resolves hardware mapping at runtime.

The system boot sequence reads a JSON-like configuration from the Non-Volatile Storage (NVS). The HAL then dynamically initializes the internal multiplexers to route the requested internal signals (like PWM generators or I2C buses) to the physical pins specified by the user through the Web Dashboard.
""",

    "docs/features/ai-engine.md": """# Edge AI Engine

Tamimystic OS integrates a highly efficient Edge AI pipeline designed to execute quantized neural networks locally, without reliance on cloud processing.

## TensorFlow Lite Micro Integration

The core intelligence layer is powered by TensorFlow Lite Micro. The engine is written purely in C++ to avoid the overhead of the MicroPython runtime during matrix multiplications and convolutions.

## Memory Management for AI

Deep learning models require significant contiguous memory, known as the Tensor Arena. 
The OS allocates the Tensor Arena exclusively within the 8MB external PSRAM. To prevent fragmentation, this allocation occurs immediately during the boot sequence, prior to the initialization of the web server or application layers.

## Inference Pipeline

The standard inference workflow operates as follows:
1.  **Data Acquisition**: The camera module captures a frame using Direct Memory Access (DMA), placing the raw bytes directly into PSRAM.
2.  **Preprocessing**: The image is down-sampled and color-converted to match the input tensor requirements of the loaded model (e.g., 96x96 RGB).
3.  **Execution**: The TFLite interpreter executes the model graph.
4.  **Post-processing**: The output tensor (representing bounding boxes, confidence scores, or classifications) is parsed.
5.  **Event Publishing**: The final structured result is broadcasted to the internal OS Event Bus, making it immediately available to the MicroPython application layer.
""",

    "docs/features/micropython.md": """# MicroPython Integration

To democratize robotics and IoT development, Tamimystic OS embeds a complete MicroPython interpreter. This allows users to write their business logic in high-level Python, bypassing the complexities of C++ memory management and cross-compilation.

## Execution Environment

The MicroPython interpreter is compiled as a static library and linked into the Tamimystic OS binary. When the OS finishes its hardware and network boot sequences, it spawns a dedicated FreeRTOS task on Core 1 to execute the Python Virtual Machine (VM).

## C++ to Python Bindings

The true power of this integration lies in the custom bindings. We have exposed the OS's internal C++ APIs to the Python runtime.

When a user writes the following Python code:
```python
import os_motor
os_motor.set_speed(1, 100)
```
The Python VM intercepts the `set_speed` call and invokes the underlying C++ HAL function. This provides the execution simplicity of Python with the raw I/O performance of C++.

## Virtual File System (VFS)

Python scripts are stored in the SPIFFS (SPI Flash File System) partition. Users upload their scripts via the Web Dashboard. The OS automatically searches for a `main.py` file in the VFS on boot and executes it as the entry point for the user's application.
""",

    "docs/features/robotics.md": """# Robotics Control Subsystem

Tamimystic OS provides a specialized subsystem dedicated entirely to kinematics and actuator control, abstracting the complex mathematics required for robotics.

## Actuator Support

The OS provides native, highly optimized drivers for various actuators:
*   **DC Motors**: Controlled via standard H-Bridge configurations. The OS abstracts the dual-PWM or PWM+Direction pin logic into a simple `set_velocity(percentage)` API.
*   **Servo Motors**: High-resolution PWM generation guarantees precise angular positioning.
*   **Stepper Motors**: Integrated step-generation logic allows for precise rotational control without blocking the main execution threads.

## Closed-Loop Control

For advanced robotics, open-loop control is insufficient. The OS includes an embedded Proportional-Integral-Derivative (PID) controller matrix.
By routing encoder feedback pins to the OS hardware counters, the system can calculate real-time RPM or positional data. The PID matrix computes the required PWM adjustments autonomously at high frequencies, ensuring accurate trajectories despite variable physical loads.
""",

    "docs/development/build.md": """# Build from Source

For core developers contributing to the C++ layers of Tamimystic OS, setting up the build environment is critical.

## Prerequisites

Tamimystic OS utilizes the Espressif IoT Development Framework (ESP-IDF) version 5.2.

1.  Download and install ESP-IDF v5.2 following the official Espressif documentation for your host OS (Windows/Linux/macOS).
2.  Ensure the ESP-IDF tools are properly exported to your system's PATH.

## Project Compilation

Navigate to the project root directory and execute the standard build commands:

```bash
idf.py set-target esp32s3
idf.py build
```

## Native Compilation (Simulation)

Tamimystic OS features a dual-target build system. You can compile the OS natively on your host machine to test business logic and architectural changes without physical hardware.

Using CMake and a standard C++ compiler (like GCC or MinGW):
```bash
mkdir build_native
cd build_native
cmake .. -DTARGET_NATIVE=ON
make
```
This generates an executable that simulates the OS scheduler and intercepts hardware calls.
""",

    "docs/development/cicd.md": """# Continuous Integration and Deployment

Tamimystic OS enforces strict CI/CD pipelines to guarantee software stability. All code pushed to the repository is subject to automated verification.

## GitHub Actions Workflow

The primary pipeline is defined in `.github/workflows/build.yml`.

1.  **Trigger**: The workflow executes on every push to the `main` branch or any Pull Request.
2.  **Environment Provisioning**: A fresh Ubuntu environment is provisioned, and the official `espressif/esp-idf-ci-action` container is initialized.
3.  **Compilation Verification**: The entire operating system is cross-compiled for the ESP32-S3 target. Any compilation errors, syntax faults, or strict warning violations (`-Werror`) will immediately fail the build.
4.  **Artifact Generation**: Upon successful compilation, the workflow extracts the critical `.bin` files (`bootloader.bin`, `partition-table.bin`, `tamimystic_os.bin`) and archives them.
5.  **Distribution**: The generated zip artifact is securely uploaded to the GitHub Actions run page, making it immediately available for end-users to download and deploy.

This automated pipeline ensures that the `main` branch always represents a verified, functional state of the operating system.
"""
}

for path, content in docs.items():
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

print("Documentation generated successfully.")
