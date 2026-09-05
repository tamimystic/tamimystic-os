# Wheeled Kinematics: Differential and Mecanum 4WD

This guide explains the mathematics, control theory, and practical programming of wheeled mobile robots in Tamimystic OS.

---

## 1. Differential Drive Kinematics (2WD / 4WD Rover)

A differential drive robot consists of two independently driven wheels with a shared wheelbase $L$.

```text
       [Left Wheel: v_L]
           |---------|
                |
                +-----> Linear Velocity: v_x
                |       Angular Velocity: omega
           |---------|
       [Right Wheel: v_R]
              <-- L -->
```

### Mathematical Equations
Given a commanded linear speed $v_x \in [-100, 100]\%$ and angular turning rate $\omega \in [-100, 100]\%$:

$$v_L = v_x - \frac{\omega \cdot L}{2}, \quad v_R = v_x + \frac{\omega \cdot L}{2}$$

### Practical Example
* **Drive straight at 70%**: $v_x = 70, \omega = 0 \implies v_L = 70\%, v_R = 70\%$
* **Pivot right in place**: $v_x = 0, \omega = 50 \implies v_L = -50\%, v_R = 50\%$
* **Smooth gentle left curve**: $v_x = 60, \omega = -20 \implies v_L = 40\%, v_R = 80\%$

```bash
# Set mode to differential
aeron> robot mode diff

# Command straight motion
aeron> robot move 70 0

# Command spin turn
aeron> robot move 0 50
```

---

## 2. Mecanum 4WD Kinematics (Omnidirectional / Holonomic)

Mecanum wheels have passive rollers oriented at a $45^\circ$ angle along their circumference. This allows the robot to move in **any direction instantly without turning** (strafing sideways, diagonal motion, and rotating simultaneously).

```text
       [FL: 45° \\] ------------- [FR: 45° //]
            |                            |
            |       ^ v_x (Forward)      |
            |       |                    |
            |   <---+---> v_y (Strafe)   |
            |       |                    |
            |      ( ) omega (Turn)      |
            |                            |
       [RL: 45° //] ------------- [RR: 45° \\]
```

### Mathematical Equations
For a commanded 3-DOF twist vector $(v_x, v_y, \omega)$:

$$\begin{aligned}
v_{FL} &= v_x - v_y - \omega \cdot \frac{L_x + L_y}{2} \\
v_{FR} &= v_x + v_y + \omega \cdot \frac{L_x + L_y}{2} \\
v_{RL} &= v_x + v_y - \omega \cdot \frac{L_x + L_y}{2} \\
v_{RR} &= v_x - v_y + \omega \cdot \frac{L_x + L_y}{2}
\end{aligned}$$

### Control Modes in Mecanum
1. **Pure Forward/Reverse**: $v_x = \pm 80, v_y = 0, \omega = 0 \implies$ all 4 wheels rotate in same direction.
2. **Pure Sideways Strafe (Right)**: $v_x = 0, v_y = 60, \omega = 0 \implies FL = -60, FR = +60, RL = +60, RR = -60$.
3. **Diagonal Traversal ($45^\circ$)**: $v_x = 50, v_y = 50, \omega = 0 \implies FL = 0, FR = 100, RL = 100, RR = 0$.
4. **Spin on Axis**: $v_x = 0, v_y = 0, \omega = 40 \implies$ Left wheels reverse, Right wheels forward.

```bash
# Switch to Mecanum mode
aeron> robot mode mecanum

# Strafe pure right at 60%
aeron> robot strafe 0 60 0

# Move diagonally forward-left at 50%
aeron> robot strafe 50 -50 0

# Full holonomic maneuver: Drive forward 40% while strafing right 30% and turning 15%
aeron> robot strafe 40 30 15
```

---

## Python Code Example

```python
import tamimystic

# 1. Drive forward
tamimystic.robot.move(linear_speed=60, angular_speed=0)
tamimystic.delay(1500)

# 2. Stop
tamimystic.robot.stop()
```
