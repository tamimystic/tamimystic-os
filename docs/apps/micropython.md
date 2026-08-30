# 🐍 MicroPython Runtime & Native OS API Reference

Tamimystic OS embeds a lightweight MicroPython interpreter that enables users to write robot logic, automation scripts, and sensor loops in high-level Python with zero C++ compilation.

---

## 📚 The `tamimystic` Native Module Reference

The `tamimystic` module exposes real-time bindings to the underlying C++ OS kernel.

```python
import tamimystic
```

---

### 1. Robotics Control (`tamimystic.robot.*`)

#### `tamimystic.robot.move(linear_speed, angular_speed)`
Commands differential or holonomic linear movement.
* `linear_speed`: Forward/Reverse speed in percent ($-100$ to $100$).
* `angular_speed`: Turning rate in percent ($-100$ Left to $+100$ Right).

```python
# Drive forward at 60% speed
tamimystic.robot.move(60, 0)
```

#### `tamimystic.robot.arm(j1, j2, j3, j4, j5, j6)`
Directly sets the 6 joint angles of an articulated robotic arm.
* `j1` (Base Yaw): $0^\circ - 180^\circ$
* `j2` (Shoulder Pitch): $0^\circ - 180^\circ$
* `j3` (Elbow Pitch): $0^\circ - 180^\circ$
* `j4` (Wrist Pitch): $0^\circ - 180^\circ$
* `j5` (Wrist Roll): $0^\circ - 180^\circ$
* `j6` (Gripper Claw): $0\% - 100\%$

```python
# Set arm to ready pose
tamimystic.robot.arm(90, 45, 90, 90, 90, 0)
```

#### `tamimystic.robot.ik(x, y, z)`
Computes and applies the closed-form Inverse Kinematics solution for target Cartesian coordinates in centimeters.

```python
# Move arm tip to coordinate (15cm, 5cm, 10cm)
tamimystic.robot.ik(15.0, 5.0, 10.0)
```

#### `tamimystic.robot.stop()`
Engages the Emergency Stop, immediately halting all motors and servos.

```python
tamimystic.robot.stop()
```

---

### 2. Sensor Readings (`tamimystic.sensor.*`)

#### `tamimystic.sensor.read_distance()`
Returns the live distance in centimeters from the active VL53L0X Laser ToF or Ultrasonic sensor.

```python
dist = tamimystic.sensor.read_distance()
print("Current obstacle distance:", dist, "cm")
```

---

### 3. Hardware GPIO Control (`tamimystic.gpio.*`)

#### `tamimystic.gpio.write(pin, value)`
Sets the digital state of any safe user GPIO pin.
* `pin`: GPIO number (e.g., `48` for Status LED).
* `value`: `1` (HIGH / 3.3V) or `0` (LOW / 0V).

```python
# Turn on status LED on GPIO 48
tamimystic.gpio.write(48, 1)
```

---

### 4. Non-Blocking System Delay (`tamimystic.delay`)

#### `tamimystic.delay(ms)`
Yields execution to background FreeRTOS tasks (networking, AI, safety control loop) for the specified duration.

```python
# Sleep for 1.5 seconds
tamimystic.delay(1500)
```

---

## 💻 CLI Python Commands

```bash
# Execute Python one-liner directly from serial CLI
aeron> python eval "tamimystic.gpio.write(48, 1)"

# Run a saved script from the 6.8MB Flash VFS
aeron> python run "my_robot.py"

# Stop currently executing Python script
aeron> python stop
```
