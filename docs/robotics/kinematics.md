# Mobile Rover Kinematics and Odometry

Tamimystic OS incorporates a deterministic kinematic transformation library capable of mapping high-level velocity commands ($\vec{v} = [v_x, v_y, \omega_z]^T$) into discrete wheel angular velocities and integrating real-time encoder pulses into high-precision Cartesian odometry poses.

---

## 1. Differential Drive Kinematics

Differential drive is the foundational non-holonomic mobile robotics configuration consisting of two independently driven coaxial wheels of radius $r$ separated by track baseline distance $L$.

```
                    ^ v_x (Forward)
                    |
              +-----------+
      Wheel L |   ROBOT   | Wheel R
       [---]  |   CHASSIS |  [---]
       (w_L)  +-----------+  (w_R)
              <-----L----->
```

### Inverse Kinematics (Target Twist to Wheel Velocities)
Given a target linear forward velocity $v_x$ (m/s) and angular rotational velocity $\omega_z$ (rad/s):

$$\omega_L = \frac{v_x - \frac{L}{2} \cdot \omega_z}{r}$$

$$\omega_R = \frac{v_x + \frac{L}{2} \cdot \omega_z}{r}$$

Where:
- $\omega_L, \omega_R$ are left and right wheel angular velocities in $\text{rad/s}$.
- $r$ is wheel radius in meters (default $0.0325\text{ m}$ for 65mm wheels).
- $L$ is wheelbase separation distance in meters (default $0.145\text{ m}$).

### Forward Kinematics (Wheel Speeds to Robot Twist)
Given measured left and right wheel angular velocities $\omega_L$ and $\omega_R$:

$$v_x = \frac{r \cdot (\omega_R + \omega_L)}{2}$$

$$\omega_z = \frac{r \cdot (\omega_R - \omega_L)}{L}$$

---

## 2. Mecanum Omnidirectional Holonomic Kinematics

Mecanum wheels feature passive rollers angled at $45^\circ$ relative to the wheel circumference. By adjusting individual wheel speeds, the robot can achieve holonomic omnidirectional translation in any planar direction ($v_x, v_y$) while simultaneously rotating ($\omega_z$).

```
             +-----------------------+
      W1 (FL)| //                 \\ | W2 (FR)
             |                       |
             |       (v_x, v_y)      |
             |           (+)         |
             |                       |
      W3 (RL)| \\                 // | W4 (RR)
             +-----------------------+
             <-----------2a---------->
             ^                       ^
             |-----------2b----------|
```

Where:
- $a = \frac{L_x}{2}$ is half the longitudinal wheelbase length.
- $b = \frac{L_y}{2}$ is half the lateral track width.
- $r$ is wheel radius.

### Mecanum Inverse Kinematics Matrix:
$$\begin{bmatrix} \omega_1 \\ \omega_2 \\ \omega_3 \\ \omega_4 \end{bmatrix} = \frac{1}{r} \begin{bmatrix} 1 & -1 & -(a + b) \\ 1 & 1 & (a + b) \\ 1 & 1 & -(a + b) \\ 1 & -1 & (a + b) \end{bmatrix} \begin{bmatrix} v_x \\ v_y \\ \omega_z \end{bmatrix}$$

Expanding into individual wheel scalar equations:
$$\omega_{\text{FL}} = \frac{1}{r} \cdot \left(v_x - v_y - (a + b)\cdot \omega_z\right)$$

$$\omega_{\text{FR}} = \frac{1}{r} \cdot \left(v_x + v_y + (a + b)\cdot \omega_z\right)$$

$$\omega_{\text{RL}} = \frac{1}{r} \cdot \left(v_x + v_y - (a + b)\cdot \omega_z\right)$$

$$\omega_{\text{RR}} = \frac{1}{r} \cdot \left(v_x - v_y + (a + b)\cdot \omega_z\right)$$

---

## 3. Ackermann Steering Geometry (Car-like Vehicles)

For high-speed automotive rover chassis, Tamimystic OS implements pure Ackermann steering geometry to ensure all four wheels trace concentric arcs around a single Instantaneous Center of Rotation (ICR), eliminating tire scrub.

```
                   ICR (Instantaneous Center of Rotation)
                    *
                   / \
                  /   \
                 /  R  \
                /       \
               /         \
        [---] /           \ [---]  Front Steered Wheels
        (\delta_i)       (\delta_o)
              +-----------+
              |           |
              |     L     | Wheelbase
              |           |
              +-----------+
        [---]               [---]  Rear Drive Wheels
              <-----W-----> Track
```

### Steering Angle Equations:
$$\tan(\delta_i) = \frac{L}{R - \frac{W}{2}}$$

$$\tan(\delta_o) = \frac{L}{R + \frac{W}{2}}$$

$$\cot(\delta_o) - \cot(\delta_i) = \frac{W}{L}$$

Where:
- $\delta_i, \delta_o$ are inner and outer front wheel steering angles.
- $L$ is longitudinal wheelbase.
- $W$ is lateral track width.
- $R$ is turn radius from the vehicle centerline to ICR.

---

## 4. Real-Time Odometry Dead Reckoning

The motion engine integrates quadrature encoder ticks at 1000 Hz using 2nd-order Runge-Kutta / Midpoint integration to update global robot pose $(x, y, \theta)$:

### Discrete-Time Pose Update Equations:
Given encoder tick delta counts $\Delta N_L$ and $\Delta N_R$ over time step $\Delta t$:

$$\Delta d_L = \frac{2\pi \cdot r \cdot \Delta N_L}{\text{CPR}}$$

$$\Delta d_R = \frac{2\pi \cdot r \cdot \Delta N_R}{\text{CPR}}$$

$$\Delta d = \frac{\Delta d_R + \Delta d_L}{2}$$

$$\Delta \theta = \frac{\Delta d_R - \Delta d_L}{L}$$

Midpoint orientation angle $\theta_{\text{mid}} = \theta_k + \frac{\Delta \theta}{2}$.

Global Cartesian coordinates at step $k+1$:
$$x_{k+1} = x_k + \Delta d \cdot \cos\left(\theta_{\text{mid}}\right)$$

$$y_{k+1} = y_k + \Delta d \cdot \sin\left(\theta_{\text{mid}}\right)$$

$$\theta_{k+1} = \text{atan2}\left(\sin(\theta_k + \Delta \theta), \cos(\theta_k + \Delta \theta)\right)$$

> [!NOTE]
> Angle normalization using `atan2(sin, cos)` ensures heading $\theta$ remains bound strictly in the range $[-\pi, +\pi]$ radians without unbounded accumulator wrap-around errors.

---

## 5. Velocity Profiling & S-Curve Clamping

To prevent wheel slippage and motor current surges, target velocity setpoints undergo trapezoidal acceleration limiting:

$$v_{\text{cmd}}(t + \Delta t) = \text{clamp}\left(v_{\text{target}}, v_{\text{cmd}}(t) - a_{\max}\Delta t, v_{\text{cmd}}(t) + a_{\max}\Delta t\right)$$

Where $a_{\max}$ is the user-configured linear acceleration limit (default $1.5\text{ m/s}^2$).

---

## MicroPython Motion API

```python
import tamimystic
import time

# Set chassis kinematic model: 'diff', 'mecanum', 'ackermann', 'skid'
tamimystic.motion.set_type("diff")

# Set physical dimensions (Wheel Radius: 0.033m, Track: 0.150m)
tamimystic.motion.set_geometry(wheel_radius=0.033, track_width=0.150)

# Set acceleration limit (1.2 m/s^2)
tamimystic.motion.set_accel_limit(1.2)

# Drive forward at 0.4 m/s with 0.0 rad/s angular
tamimystic.motion.drive(0.4, 0.0)
time.sleep(2.0)

# Read live odometry pose
pose = tamimystic.motion.get_pose()
print(f"Robot Position -> X: {pose['x']:.3f} m, Y: {pose['y']:.3f} m, Theta: {pose['theta']:.2f} rad")

# Reset odometry to origin (0, 0, 0)
tamimystic.motion.reset_odometry()
```

---

## Serial CLI and REST API Reference

### Serial CLI:
```bash
# Set chassis type
tamimystic> motion type diff

# Drive rover (linear m/s, angular rad/s)
tamimystic> motion drive 0.5 0.1

# Holonomic drive (vx, vy, omega)
tamimystic> motion holonomic 0.3 -0.2 0.0

# Query current odometry
tamimystic> motion odom

# Reset odometry
tamimystic> motion odom_reset
```

### HTTP REST API:
```bash
# Standard drive command
POST /api/motion/drive?linear=0.5&angular=0.1

# Holonomic mecanum drive command
POST /api/motion/holonomic?vx=0.3&vy=-0.2&omega=0.0

# Fetch live odometry
GET /api/motion/odom
```
