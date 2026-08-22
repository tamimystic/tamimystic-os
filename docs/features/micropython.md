# MicroPython Integration

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
