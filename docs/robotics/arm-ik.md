# 6-DOF Robotic Arm and Analytical Inverse Kinematics

Tamimystic OS embeds a closed-form, real-time **Analytical Inverse Kinematics (IK)** and **Forward Kinematics (FK)** solver specifically formulated for 6-DOF articulated robotic arms.

---

## Robotic Arm Kinematic Model

A 6-DOF robotic manipulator is modeled with standard link lengths ($L_1 = 10\text{ cm}, L_2 = 12\text{ cm}, L_3 = 8\text{ cm}$):

```text
                  (J4: Wrist Pitch) --- (J5: Wrist Roll) === [J6: Gripper]
                        /                                     End-Effector (X, Y, Z)
                       /  L3 (Forearm)
                      /
            (J3: Elbow Pitch)
                   /
                  /
                 /  L2 (Upper Arm)
                /
      (J2: Shoulder Pitch)
            |
            |  L1 (Base Column)
            |
      (J1: Base Yaw)
   =================== (Ground Plane)
```

---

## Analytical Inverse Kinematics Equations

Given a target 3D end-effector coordinate $(X, Y, Z)$ in centimeters and pitch angle $\theta_p$:

### 1. Base Yaw ($J_1$)
$$J_1 = \text{atan2}(Y, X) \times \frac{180^\circ}{\pi}$$

### 2. Radial Distance and Elevation ($r, z'$)
$$r = \sqrt{X^2 + Y^2} - L_3 \cos(\theta_p), \quad z' = Z - L_1 - L_3 \sin(\theta_p)$$
$$D = \sqrt{r^2 + z'^2}$$

### 3. Elbow Pitch ($J_3$) via Law of Cosines
$$\cos(J_3) = \frac{L_2^2 + L_3^2 - D^2}{2 L_2 L_3} \implies J_3 = \text{acos}\left(\text{clamp}\left(\frac{L_2^2 + L_3^2 - D^2}{2 L_2 L_3}, -1, 1\right)\right)$$

### 4. Shoulder Pitch ($J_2$)
$$J_2 = \text{atan2}(z', r) + \text{atan2}(L_3 \sin(J_3), L_2 + L_3 \cos(J_3))$$

### 5. Wrist Pitch ($J_4$)
$$J_4 = \theta_p - (J_2 + J_3)$$

Execution Performance: Because the solver uses pure analytical closed-form trigonometry (instead of heavy numerical iterations or matrix inversions), the ESP32-S3 computes the complete 6-joint solution in **less than 0.2 milliseconds**!

---

## PCA9685 16-Channel Servo Mapping

When a PCA9685 I2C module is detected at address `0x40`, the joints are routed automatically:

| PCA9685 Channel | Joint Name | Physical Range | Default Home Angle |
|---|---|---|---|
| **Channel 0** | $J_1$: Base Yaw (Pan) | $0^\circ - 180^\circ$ | **$90^\circ$** (Center) |
| **Channel 1** | $J_2$: Shoulder Pitch | $0^\circ - 180^\circ$ | **$45^\circ$** |
| **Channel 2** | $J_3$: Elbow Pitch | $0^\circ - 180^\circ$ | **$90^\circ$** |
| **Channel 3** | $J_4$: Wrist Pitch | $0^\circ - 180^\circ$ | **$90^\circ$** |
| **Channel 4** | $J_5$: Wrist Roll | $0^\circ - 180^\circ$ | **$90^\circ$** |
| **Channel 5** | $J_6$: Gripper Claw | $0\% - 100\%$ ($0^\circ - 90^\circ$) | **$0\%$** (Closed) |

---

## CLI and Python Examples

### Direct Joint Control:
```bash
# Set individual joint angles: Base=90°, Shoulder=45°, Elbow=90°, WristP=90°, WristR=90°, Gripper=50%
aeron> robot arm 90 45 90 90 90 50
```

### Cartesian Inverse Kinematics Control:
```bash
# Move arm tip to X=15cm, Y=10cm, Z=12cm, Pitch=0°, Gripper=100% (Open)
aeron> robot ik 15 10 12 0 100
```

### Python Scripting:
```python
import tamimystic

# Pick and Place Sequence
print("1. Moving arm to pick target (X=15, Y=10, Z=5)...")
tamimystic.robot.ik(x=15.0, y=10.0, z=5.0)
tamimystic.delay(1000)

print("2. Closing gripper to grasp object...")
# J1=90, J2=45, J3=90, J4=90, J5=90, Gripper=100%
tamimystic.robot.arm(90, 45, 90, 90, 90, 100)
tamimystic.delay(500)

print("3. Lifting object to Z=15cm...")
tamimystic.robot.ik(x=15.0, y=10.0, z=15.0)
```
