# Universal Hardware Abstraction Layer

The Hardware Abstraction Layer (HAL) is the bridge between the Tamimystic OS C++ core and the physical electrical interfaces of the microcontroller.

## Design Principle: Abstraction over Implementation

The OS core never interacts directly with ESP-IDF specific functions (e.g., `ledc_timer_config` or `uart_driver_install`). Instead, it calls generic OS abstractions (e.g., `os_hal_pwm_init` or `os_hal_uart_read`).

This design provides two massive advantages:
1.  **Portability**: The operating system can be ported to different microcontrollers (e.g., STM32, RP2040) simply by rewriting the underlying HAL implementations, without changing a single line of OS core logic.
2.  **Native Simulation**: When compiled on Windows or Linux, the HAL intercepts hardware calls and redirects them to the terminal console (e.g., printing `[PWM] Set Pin 18 to 50%` instead of actually toggling a physical pin). This allows developers to test OS logic entirely in software.

## Dynamic Multiplexing

Unlike traditional firmware where `MOTOR_PIN` is defined as a macro `#define MOTOR_PIN 18` at compile time, Tamimystic OS resolves hardware mapping at runtime.

The system boot sequence reads a JSON-like configuration from the Non-Volatile Storage (NVS). The HAL then dynamically initializes the internal multiplexers to route the requested internal signals (like PWM generators or I2C buses) to the physical pins specified by the user through the Web Dashboard.
