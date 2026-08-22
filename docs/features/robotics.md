# Robotics Control Subsystem

Tamimystic OS provides a specialized subsystem dedicated entirely to kinematics and actuator control, abstracting the complex mathematics required for robotics.

## Actuator Support

The OS provides native, highly optimized drivers for various actuators:
*   **DC Motors**: Controlled via standard H-Bridge configurations. The OS abstracts the dual-PWM or PWM+Direction pin logic into a simple `set_velocity(percentage)` API.
*   **Servo Motors**: High-resolution PWM generation guarantees precise angular positioning.
*   **Stepper Motors**: Integrated step-generation logic allows for precise rotational control without blocking the main execution threads.

## Closed-Loop Control

For advanced robotics, open-loop control is insufficient. The OS includes an embedded Proportional-Integral-Derivative (PID) controller matrix.
By routing encoder feedback pins to the OS hardware counters, the system can calculate real-time RPM or positional data. The PID matrix computes the required PWM adjustments autonomously at high frequencies, ensuring accurate trajectories despite variable physical loads.
