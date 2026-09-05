# 5-Minute Quickstart Tutorial

This hands-on tutorial walks you through your very first Tamimystic OS project: writing a Python script to blink the onboard status LED, reading a live distance sensor, and driving a robot motor.

---

## What We Will Accomplish
1. Configure an onboard LED pin.
2. Read a distance sensor using the native Python API.
3. Command the robot platform to drive forward and auto-brake when an obstacle is detected.

---

## Step 1: Open the Web Python IDE

1. Connect your computer to the same Wi-Fi network as your ESP32-S3.
2. Navigate to `http://<your-device-ip>/` in your web browser.
3. Scroll down to the **Dynamic Python IDE and VFS Storage** section.

---

## Step 2: Paste the Quickstart Script

Paste the following script directly into the in-browser code editor:

```python
import tamimystic

print("=== Tamimystic OS Autonomous Rover Script Started ===")

# 1. Turn ON Status LED (Default GPIO 48 on ESP32-S3)
tamimystic.gpio.write(pin=48, value=1)
print("[LED] Status LED ON")

# 2. Check front obstacle distance via VL53L0X / Ultrasonic
dist = tamimystic.sensor.read_distance()
print("Front Obstacle Distance:", dist, "cm")

# 3. Autonomous Driving Decision
if dist > 20.0:
    print("[DRIVE] Path Clear! Commanding Rover Forward at 50% Speed...")
    tamimystic.robot.move(linear_speed=50, angular_speed=0)
    
    # Drive for 2 seconds
    tamimystic.delay(2000)
    
    print("[DRIVE] Stopping Rover.")
    tamimystic.robot.stop()
else:
    print("[SAFETY] Obstacle too close (< 20cm)! Refusing to drive forward.")
    tamimystic.robot.stop()

# 4. Turn OFF Status LED
tamimystic.gpio.write(pin=48, value=0)
print("[LED] Status LED OFF")
print("=== Script Completed Successfully! ===")
```

---

## Step 3: Run and Watch the Live Output

1. Click the **Run Script** button in the Web IDE (or run `python eval "<code...>"` in the serial CLI).
2. Look at the **Console Output** window below the editor:

```text
=== Tamimystic OS Autonomous Rover Script Started ===
[GPIO] Pin 48 -> 1
[LED] Status LED ON
Front Obstacle Distance: 24.8 cm
[DRIVE] Path Clear! Commanding Rover Forward at 50% Speed...
[ROBOT] Velocity commanded: Vx=50%, W=0%
[DRIVE] Stopping Rover.
[ROBOT] Emergency Stopped.
[GPIO] Pin 48 -> 0
[LED] Status LED OFF
=== Script Completed Successfully! ===
```

You have now programmed an autonomous robotics decision loop directly from your browser without compiling a single line of C++!

---

## Step 4: Make It Run on Boot (autorun.py)

If you want your ESP32-S3 robot to execute this script automatically whenever the battery is turned on:
1. In the Web IDE, enter `autorun.py` as the filename.
2. Click **Save File to Flash VFS**.
3. Next time your ESP32-S3 powers on, Tamimystic OS will detect `autorun.py` in the 6.8MB flash storage and execute it autonomously!
