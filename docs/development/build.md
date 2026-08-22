# Build from Source

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
