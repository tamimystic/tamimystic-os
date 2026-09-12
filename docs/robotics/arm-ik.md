# 6-DOF Robotic Arm Inverse Kinematics

Tamimystic OS features a real-time robotic arm manipulation engine (`os_arm`) capable of computing Forward Kinematics (FK) and solving 6-DOF Inverse Kinematics (IK) in sub-millisecond execution times directly on Core 1 of the ESP32-S3.

---

## Kinematic Model & Denavit-Hartenberg (DH) Parameters

The standard 6-DOF robotic arm geometry consists of six revolute joints:
1. **$J_1$ (Base Yaw)**: Rotates about the vertical $Z_0$ axis.
2. **$J_2$ (Shoulder Pitch)**: Elevates the upper arm.
3. **$J_3$ (Elbow Pitch)**: Articulates the forearm.
4. **$J_4$ (Wrist Pitch)**: Adjusts pitch of the end-effector.
5. **$J_5$ (Wrist Roll)**: Axial rotation of the end-effector.
6. **$J_6$ (Gripper / End-Effector)**: Tool actuation.

```mermaid
graph TD
    Base["Base (Origin O_0)"] -->|Joint 1: Yaw (theta_1)| Shoulder["Shoulder (Joint 2: theta_2)"]
    Shoulder -->|Link L1: a_2| Elbow["Elbow (Joint 3: theta_3)"]
    Elbow -->|Link L2: a_3| Wrist["Wrist Center (Joint 4: theta_4)"]
    Wrist -->|Link L3: d_6| Tool["End-Effector / Gripper (Joint 5 & 6)"]
```

### Standard DH Parameter Table:

| Link $i$ | Joint Variable $\theta_i$ | Link Twist $\alpha_i$ | Link Length $a_i$ | Link Offset $d_i$ |
|---|---|---|---|---|
| **Link 1** | $\theta_1$ | $+90^\circ$ ($+\pi/2$) | $0\text{ mm}$ | $d_1 = 105\text{ mm}$ (Base Height) |
| **Link 2** | $\theta_2$ | $0^\circ$ | $a_2 = 140\text{ mm}$ (Upper Arm) | $0\text{ mm}$ |
| **Link 3** | $\theta_3$ | $0^\circ$ | $a_3 = 140\text{ mm}$ (Forearm) | $0\text{ mm}$ |
| **Link 4** | $\theta_4$ | $+90^\circ$ ($+\pi/2$) | $0\text{ mm}$ | $0\text{ mm}$ |
| **Link 5** | $\theta_5$ | $-90^\circ$ ($-\pi/2$) | $0\text{ mm}$ | $0\text{ mm}$ |
| **Link 6** | $\theta_6$ | $0^\circ$ | $0\text{ mm}$ | $d_6 = 85\text{ mm}$ (End-Effector Length) |

The individual homogeneous transformation matrix $A_i^{i-1}$ is defined as:
$$A_i^{i-1} = \begin{bmatrix} \cos\theta_i & -\sin\theta_i\cos\alpha_i & \sin\theta_i\sin\alpha_i & a_i\cos\theta_i \\ \sin\theta_i & \cos\theta_i\cos\alpha_i & -\sin\theta_i\sin\alpha_i & a_i\sin\theta_i \\ 0 & \sin\alpha_i & \cos\alpha_i & d_i \\ 0 & 0 & 0 & 1 \end{bmatrix}$$

The total forward kinematic transformation of the end-effector relative to base frame:
$$T_0^6 = A_1^0 \cdot A_2^1 \cdot A_3^2 \cdot A_4^3 \cdot A_5^4 \cdot A_6^5$$

---

## Inverse Kinematics: Analytical Decoupling Algorithm

To achieve sub-millisecond execution on the embedded microcontroller without iterative numeric divergence, Tamimystic OS utilizes **Kinematic Decoupling** (Pieper's Solution for spherical wrist manipulators):

### Step 1: Spherical Wrist Center Position Computation
Given desired end-effector position $\vec{p}_e = [x_e, y_e, z_e]^T$ and approach unit vector $\hat{a}$ (derived from desired target Pitch $\phi$ and Roll $\psi$):

$$\vec{p}_w = \vec{p}_e - d_6 \cdot \hat{a} = \begin{bmatrix} x_w \\ y_w \\ z_w \end{bmatrix}$$

### Step 2: Joint 1 (Base Yaw) Solution:
$$\theta_1 = \text{atan2}(y_w, x_w)$$

### Step 3: Planar 2-DOF Elbow and Shoulder ($\theta_2, \theta_3$) Solutions:
Projecting into the arm vertical plane:
$$r = \sqrt{x_w^2 + y_w^2}$$

$$s = z_w - d_1$$

$$D = \frac{r^2 + s^2 - a_2^2 - a_3^2}{2 \cdot a_2 \cdot a_3}$$

If $|D| > 1.0$, the target coordinate lies outside the mechanical arm workspace envelope.

Elbow angle $\theta_3$ (Elbow-up configuration):
$$\theta_3 = \text{atan2}\left(\sqrt{1 - D^2}, D\right)$$

Shoulder angle $\theta_2$:
$$\theta_2 = \text{atan2}(s, r) - \text{atan2}\left(a_3 \sin\theta_3, a_2 + a_3 \cos\theta_3\right)$$

### Step 4: Wrist Orientation ($\theta_4, \theta_5, \theta_6$) Solutions:
Joint angles $\theta_4, \theta_5, \theta_6$ are extracted directly from the remaining rotation matrix:
$$R_{3}^6 = (R_0^3)^T \cdot R_{\text{target}}$$

---

## Smooth Trajectory Interpolation (Quintic 5th-Order Polynomial)

To eliminate jerky servo motion and mechanical vibration, joint transitions follow 5th-order polynomials:

$$\theta(t) = a_0 + a_1 t + a_2 t^2 + a_3 t^3 + a_4 t^4 + a_5 t^5$$

Subject to boundary conditions:
- Initial state: $\theta(0) = \theta_0, \quad \dot{\theta}(0) = 0, \quad \ddot{\theta}(0) = 0$
- Final state: $\theta(t_f) = \theta_f, \quad \dot{\theta}(t_f) = 0, \quad \ddot{\theta}(t_f) = 0$

Analytical coefficient solution:
$$a_0 = \theta_0, \quad a_1 = 0, \quad a_2 = 0$$

$$a_3 = \frac{10(\theta_f - \theta_0)}{t_f^3}, \quad a_4 = \frac{-15(\theta_f - \theta_0)}{t_f^4}, \quad a_5 = \frac{6(\theta_f - \theta_0)}{t_f^5}$$

---

## Hardware PWM Pulse Width Mapping

Servo joint angles $[-90^\circ, +90^\circ]$ or $[0^\circ, 180^\circ]$ are mapped linearly into hardware pulse widths:

$$\text{Pulse}(\mu\text{s}) = 500\mu\text{s} + \left(\frac{\theta - \theta_{\min}}{\theta_{\max} - \theta_{\min}}\right) \cdot (2500\mu\text{s} - 500\mu\text{s})$$

On the 12-bit PCA9685 ($50\text{ Hz}$ period $= 20000\mu\text{s}$, 4096 counts):
$$\text{PCA\_Ticks} = \text{round}\left(\frac{\text{Pulse}(\mu\text{s})}{20000} \cdot 4096\right)$$

---

## MicroPython Robotic Arm API

```python
import tamimystic
import time

# Move arm to Cartesian coordinates (X=150mm, Y=0mm, Z=120mm, Pitch=30 deg)
success = tamimystic.arm.move_to(x=150.0, y=0.0, z=120.0, pitch=30.0, roll=0.0, duration_sec=1.5)
if success:
    print("Arm reached target pose successfully")
else:
    print("Target position unreachable (outside workspace envelope)")

# Directly set individual joint angles (degrees)
tamimystic.arm.set_joints([0.0, 45.0, -30.0, 15.0, 0.0])

# Open gripper claw (0% = fully open, 100% = fully closed)
tamimystic.arm.set_gripper(0)
time.sleep(1.0)
tamimystic.arm.set_gripper(100)
```

---

## Serial CLI and REST API Reference

### Serial CLI:
```bash
# Solve and move to Cartesian coordinate (x, y, z, roll, pitch, yaw)
tamimystic> arm ik 150.0 0.0 120.0 0.0 30.0 0.0

# Set joint angles directly (j1..j6 in degrees)
tamimystic> arm joints 0 45 -30 15 0 90

# Set gripper position percentage (0 - 100)
tamimystic> arm gripper 80

# Query current arm forward kinematics pose
tamimystic> arm status
```

### HTTP REST API:
```bash
# Move to Cartesian target
POST /api/arm/ik?x=150.0&y=0.0&z=120.0&pitch=30.0&roll=0.0

# Set individual joint angles
POST /api/arm/joints?j1=0&j2=45&j3=-30&j4=15&j5=0&j6=90

# Set Gripper
POST /api/arm/gripper?percent=80

# Query Arm Pose
GET /api/arm/status
```
