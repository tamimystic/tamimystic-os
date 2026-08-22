# Core Scheduler and Event Bus

Tamimystic OS utilizes a deterministic, preemptive task scheduler built on FreeRTOS to guarantee real-time execution capabilities.

## Dual-Core Task Pinning

The ESP32-S3 features an Xtensa Dual-Core 32-bit LX7 microprocessor. Tamimystic OS explicitly pins tasks to specific cores to prevent race conditions and maximize computational bandwidth.

*   **Core 0 (System Core)**: Reserved for highly asynchronous, non-blocking tasks. This includes the Wi-Fi stack, Bluetooth Low Energy (BLE) stack, asynchronous HTTP server, and the Virtual File System (VFS) read/write operations.
*   **Core 1 (Application Core)**: Dedicated strictly to computationally heavy, blocking, or user-defined operations. This includes the TensorFlow Lite Micro inference pipeline, image processing algorithms, and the MicroPython runtime environment.

## The Event Bus Architecture

Communication between the isolated tasks and cores is managed through a thread-safe Event Bus. Instead of direct function calls across boundaries, tasks publish events to the bus, and interested subsystems subscribe to specific event topics.

*   **Decoupling**: The Wi-Fi manager does not need to know the state of the motor driver. It simply publishes a `WIFI_CONNECTED` event. The web server, subscribed to this event, will then initialize the HTTP endpoints.
*   **Data Integrity**: Data payloads passed through the Event Bus are deeply copied to prevent memory corruption when transferring pointers between contexts.
