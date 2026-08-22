# Welcome to Tamimystic OS

![Tamimystic OS](https://img.shields.io/badge/Platform-ESP32--S3-blue?style=for-the-badge&logo=espressif)
![Status](https://img.shields.io/badge/Status-Ultra_Pro_Max-success?style=for-the-badge)

**Tamimystic OS** is an end-to-end, ultra-pro-max professional operating system built natively for edge devices, specifically targeting the **ESP32-S3 (N16R8)**.

## 🚀 Vision

Designed for robotics, edge AI, and IoT, Tamimystic OS bridges the gap between low-level hardware constraints and high-level software paradigms. 

The philosophy is simple: **Flash Once, Configure Infinitely.**
Instead of recompiling C++ code every time you want to change a pin or update logic, Tamimystic OS provides a Universal HAL, a MicroPython Application Engine, and a Local Web Dashboard. You just upload `.py` apps and configure settings from your browser!

## ✨ Core Features

* **Dual-Target Architecture**: Compiles natively on Windows/Linux for rapid simulation, and cross-compiles for ESP32-S3.
* **Pre-emptive FreeRTOS Scheduler**: True multithreading with prioritized background tasks and a unified Event Bus.
* **Universal HAL**: Hot-swappable driver layer (UART, I2C, SPI, PWM) that completely abstracts away ESP-IDF.
* **Edge AI Engine**: Built-in TensorFlow Lite Micro pipeline for object detection and audio processing.
* **MicroPython App Layer**: Write your business logic in Python. The OS executes it safely without blocking the core scheduler.
* **Web-based Dashboard**: Configure Wi-Fi, upload Python apps, and monitor system health locally via the built-in HTTP server.

---

Ready to get started? Head over to the [Installation Guide](getting-started/installation.md).
