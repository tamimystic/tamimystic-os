# Architecture Overview

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
