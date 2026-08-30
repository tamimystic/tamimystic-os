# Universal Robotics Brain & Kinematics Engine

Tamimystic OS provides a dedicated real-time robotics engine executing on **Core 1 @ 50 Hz**, abstracting complex kinematics, multi-actuator coordination, and safety control loops into a unified control interface.

---

## 🦾 Supported Robot Topologies

1. **Differential Rover (2WD / 4WD)**:
   - Forward/Reverse and Angular Turning.
   - Kinematics equations:
     $$v_L = v_x - \frac{\omega \cdot L}{2}, \quad v_R = v_x + \frac{\omega \cdot L}{2}$$
2. **Mecanum 4WD (Omnidirectional / Holonomic)**:
   - Independent 4-wheel omnidirectional strafing, diagonal traversal, and spinning on axis without changing heading.
   - Kinematics equations:
     $$\begin{aligned}
     v_{FL} &= v_x - v_y - \omega \cdot \frac{L_x + L_y}{2} \\
     v_{FR} &= v_x + v_y + \omega \cdot \frac{L_x + L_y}{2} \\
     v_{RL} &= v_x + v_y - \omega \cdot \frac{L_x + L_y}{2} \\
     v_{RR} &= v_x - v_y + \omega \cdot \frac{L_x + L_y}{2}
     \end{aligned}$$
3. **6-DOF Robotic Arm (Analytical IK & FK Solver)**:
   - Joint array: Base Yaw ($J_1$), Shoulder Pitch ($J_2$), Elbow Pitch ($J_3$), Wrist Pitch ($J_4$), Wrist Roll ($J_5$), Gripper ($J_6$).
   - Closed-form analytical inverse kinematics solver executing in $<1\text{ ms}$ on ESP32-S3.
4. **Self-Balancing Inverted Pendulum Robot**:
   - 200Hz complementary filter with MPU-6050 accelerometer & gyroscope integration.

---

## 🎛️ Actuator & Multi-Channel Servo Subsystem

- **PCA9685 16-Channel I2C Expander**:
  - Auto-probed on I2C address `0x40`.
  - Configured at 50Hz (20ms period) with 12-bit PWM resolution (4096 steps, ~4.88µs/step).
- **ESP32-S3 High-Resolution LEDC PWM Fallback**:
  - Automatically activates onboard GPIO PWM pins if PCA9685 is not connected.

---

## 🛡️ Real-Time Safety & Virtual Proximity Bumper

- **Virtual Proximity Bumper**: Continuously monitors VL53L0X Time-of-Flight / HC-SR04 ultrasonic distance sensor readings. If distance $< 15\text{ cm}$ while driving forward, the 50Hz safety loop automatically engages the emergency brake to prevent physical collisions.
- **Hardware & Software E-Stop**: Instantly shuts down all H-bridges and sets motor duties to 0% with sub-millisecond response time.

---

## 💻 CLI & API Usage

```bash
# Set robot mode to Mecanum 4WD
aeron> robot mode mecanum

# Drive rover with linear speed 60% and turning rate 20%
aeron> robot move 60 20

# Omnidirectional strafe: Vx=50%, Vy=-30%, Omega=10%
aeron> robot strafe 50 -30 10

# Inverse Kinematics command to 6-DOF arm: Move end-effector to (X=15cm, Y=10cm, Z=12cm, Pitch=0°, Gripper=80%)
aeron> robot ik 15 10 12 0 80

# Emergency Stop
aeron> robot stop
aeron> robot resume
```
