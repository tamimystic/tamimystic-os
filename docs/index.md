# Welcome to Tamimystic OS

**Tamimystic OS** is a next-generation, high-performance edge robotics and artificial intelligence operating system designed natively for the **ESP32-S3-N16R8** (Xtensa® dual-core 32-bit LX7 @ 240MHz, 16MB Quad-SPI Flash, 8MB Octal-SPI PSRAM).

🌐 **Official Repository**: [github.com/tamimystic/tamimystic-os](https://github.com/tamimystic/tamimystic-os)  
📖 **Documentation Website**: [tamimystic.github.io/tamimystic-os](https://tamimystic.github.io/tamimystic-os/)

---

## 🎯 Vision and Philosophy: "Flash Once, Configure Infinitely"

Historically, microcontroller firmware development has been rigidly tied to hardware configurations. Pins, sensors, and execution loops are traditionally hardcoded, requiring a complete recompilation and flashing cycle for every minor hardware adjustment.

Tamimystic OS introduces the philosophy of **Flash Once, Configure Infinitely**. The core objective is to decouple the hardware from the software logic. By flashing the operating system binary only once, developers and roboticists can dynamically route hardware pins, hot-swap peripherals, deploy MicroPython / WASM scripts in real-time, and control autonomous robotics without ever invoking a C++ compiler again.

---

## 🏗️ System Architecture

```mermaid
graph TD
    subgraph App Layer
        A1[In-Browser Web Python IDE]
        A2[MicroPython / WASM Engine]
        A3[REST API & Web Dashboard]
    end

    subgraph OS Brain
        B1[Universal Robotics Kinematics 2WD/Mecanum/6-DOF IK]
        B2[Edge AI & Vision Engine MobileNet + ESP-NN]
        B3[Event Bus & Dual-Core Scheduler Core 0/Core 1]
    end

    subgraph Hardware Abstraction & PnP
        C1[Plug & Play I2C Auto-Discovery Registry]
        C2[Dynamic Software Pin Matrix NVS Persisted]
        C3[Universal HAL UART / I2C / PWM / GPIO]
    end

    subgraph Physical Target ESP32-S3-N16R8
        D1[16MB Flash: Dual OTA 4.5MB + 6.8MB LittleFS VFS]
        D2[8MB Octal PSRAM: Camera Triple Buffer + AI Arena]
        D3[Xtensa Dual-Core 240MHz + SIMD Vector Instructions]
    end

    App Layer --> OS Brain
    OS Brain --> Hardware Abstraction & PnP
    Hardware Abstraction & PnP --> Physical Target ESP32-S3-N16R8
```

---

## 🚀 Key Feature Modules

| Subsystem | Highlights |
|---|---|
| **Universal Robotics Brain** | Differential 2WD/4WD, Mecanum 4WD, 6-DOF Arm Analytical IK/FK ($<1\text{ ms}$), PCA9685 16-channel servo driver, 50Hz Real-Time Core 1 loop, 15cm Virtual Proximity Auto-Brake. |
| **Edge AI & Computer Vision** | DVP Camera Pipeline with 8MB Octal PSRAM triple buffering, TensorFlow Lite Micro with Xtensa SIMD (ESP-NN), 4 onboard models (Person, Objects, Road Lane, Gestures), Autonomous Visual Tracking. |
| **Plug & Play Hardware** | 15+ Sensor/Actuator signature database (MPU-6050, BME280, SSD1306, VL53L0X, PCA9685), Dynamic Pin Matrix with NVS persistence and Octal PSRAM pin protection (GPIO 33-37). |
| **Scripting & Flash VFS** | Native MicroPython interpreter with `tamimystic.*` API, WASM micro-runtime, 6.8MB LittleFS flash storage partition, In-Browser Web Python IDE, Dual-Bank OTA. |
| **Universal HAL & Web UI** | Fully asynchronous HTTP Server on Port 80, glassmorphism dashboard, serial CLI (`aeron>`), Dual-target PC simulation & ESP-IDF hardware build. |
