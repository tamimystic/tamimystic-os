# 5-Minute Quickstart

This hands-on quickstart guide walks you through executing five core capabilities of Tamimystic OS in under five minutes.

---

## Tutorial 1: System Telemetry and Hardware Pin Toggling

Let's verify system health and toggle the onboard status LED using all three interaction methods (CLI, HTTP REST, and MicroPython).

### Method A: Serial CLI
```bash
# Check CPU, heap, and PSRAM status
tamimystic> status

# Blink the status LED (connected to default GPIO 48)
tamimystic> pin set status_led 48
tamimystic> gpio write 48 1
tamimystic> gpio write 48 0
```

### Method B: HTTP REST API (from your computer's terminal)
```bash
# Query system JSON telemetry
curl http://192.168.4.1/api/status

# Expected JSON response:
# {
#   "device": "ESP32-S3-N16R8",
#   "uptime_sec": 342,
#   "cpu_freq_mhz": 240,
#   "psram_total_kb": 8192,
#   "psram_free_kb": 7842,
#   "internal_free_kb": 384,
#   "wifi_mode": "AP+STA",
#   "ros2_status": "DISCONNECTED"
# }
```

### Method C: Embedded MicroPython Script
```python
import tamimystic
import time

print("Tamimystic OS Python Engine Active!")
print("Free PSRAM:", tamimystic.system.get_free_psram(), "bytes")

# Blink Status LED 5 times
for i in range(5):
    tamimystic.pin.write(48, 1)
    time.sleep(0.2)
    tamimystic.pin.write(48, 0)
    time.sleep(0.2)
```

---

## Tutorial 2: Closed-Loop Robotics Differential Drive

Drive your rover chassis with precise linear and angular velocities using the internal 1000 Hz PID motion controller.

### Method A: Serial CLI
```bash
# Set chassis configuration to Differential Drive
tamimystic> motion type diff

# Set maximum velocity limits (0.8 m/s linear, 1.5 rad/s angular)
tamimystic> motion set_limits 0.8 1.5

# Drive forward at 0.4 m/s with 0.1 rad/s curve
tamimystic> motion drive 0.4 0.1

# Stop motors immediately
tamimystic> motion stop
```

### Method B: HTTP REST API
```bash
# Send drive command (linear: 0.5 m/s, angular: 0.0 rad/s)
curl -X POST "http://192.168.4.1/api/motion/drive?linear=0.5&angular=0.0"

# Emergency Stop
curl -X POST "http://192.168.4.1/api/motion/stop"
```

### Method C: Embedded MicroPython Script
```python
import tamimystic
import time

# Configure Differential Drive kinematics
tamimystic.motion.set_type("diff")

# Drive forward for 2.0 seconds
tamimystic.motion.drive(0.3, 0.0)
time.sleep(2.0)

# Rotate clockwise for 1.0 second
tamimystic.motion.drive(0.0, 1.2)
time.sleep(1.0)

# Stop
tamimystic.motion.stop()
```

---

## Tutorial 3: Reading Plug-and-Play IMU and Laser Distance Sensors

Tamimystic OS automatically detects I2C peripherals during startup. Let's inspect live pitch/roll data from an MPU-6050 and millimetric distance from a VL53L0X.

### Method A: Serial CLI
```bash
# Trigger an I2C scan to list connected devices
tamimystic> sensor scan

# Output:
# [I2C Scan] Address 0x29: VL53L0X Laser ToF Sensor [ONLINE]
# [I2C Scan] Address 0x68: MPU-6050 6-DOF IMU [ONLINE]

# Query current IMU reading
tamimystic> sensor imu

# Output:
# IMU Acceleration: X=0.02g, Y=-0.01g, Z=0.98g
# IMU Gyroscope:    Roll=0.4 deg, Pitch=-1.2 deg, Yaw=45.8 deg
```

### Method B: HTTP REST API
```bash
# Fetch all sensor readings in unified JSON format
curl http://192.168.4.1/api/sensors

# Expected response:
# {
#   "imu": {"roll": 0.4, "pitch": -1.2, "yaw": 45.8, "accel_z": 0.98},
#   "distance_mm": 452,
#   "environment": {"temp_c": 26.4, "humidity_pct": 55.2, "pressure_hpa": 1013.25}
# }
```

### Method C: Autonomous Obstacle Avoidance Script (MicroPython)
```python
import tamimystic
import time

print("Starting autonomous collision avoidance routine...")

while True:
    dist = tamimystic.sensor.get_distance_mm()
    
    if dist < 200:  # Obstacle detected closer than 20 cm
        print("Obstacle detected at", dist, "mm! Reversing and turning...")
        tamimystic.motion.drive(-0.2, 0.0)
        time.sleep(0.5)
        tamimystic.motion.drive(0.0, 1.5)  # Spin right
        time.sleep(0.8)
    else:
        tamimystic.motion.drive(0.3, 0.0)  # Move forward safely
        
    time.sleep(0.05)  # 20 Hz loop rate
```

---

## Tutorial 4: Live DVP Camera Stream and Edge AI Neural Inference

Stream high-frequency video directly to your browser and execute INT8 vector accelerated object classification on Core 1.

### Step 1: Open Video Stream in Browser
Open `http://192.168.4.1/api/camera/stream` in your browser. The embedded HTTP engine continuously streams an MJPEG video feed at up to 30 FPS.

### Step 2: Trigger Neural Network Inference via CLI
```bash
tamimystic> ai run

# Output:
# [AI Engine] Captured Frame: 160x120 RGB565 (PSRAM 0x3d800000)
# [AI Engine] Running MobileNet-V2 INT8 SIMD Model...
# [AI Engine] Inference Time: 17.8 ms (56.1 FPS throughput)
# [AI Engine] Top Prediction: "Person" (Confidence: 93.8%)
# [AI Engine] Bounding Box: [x=42, y=18, w=76, h=95]
```

### Step 3: Trigger Neural Inference via REST API
```bash
curl http://192.168.4.1/api/ai/classify

# Response:
# {
#   "model": "mobilenet_v2_int8",
#   "latency_ms": 17.8,
#   "top_class": "person",
#   "confidence": 0.938,
#   "bbox": {"x": 42, "y": 18, "w": 76, "h": 95}
# }
```

---

## Tutorial 5: Multi-Node ESP-NOW Swarm Mesh Broadcasting

Broadcast synchronization pulses across nearby Tamimystic OS nodes without requiring a Wi-Fi router.

### Step 1: Query Swarm Radio State
```bash
tamimystic> espnow status

# Output:
# [ESP-NOW] State: ACTIVE | Channel: 1 | Local MAC: 34:85:18:2B:64:90
# [ESP-NOW] Swarm Mode: LEADER | Active Peers: 2
# [ESP-NOW] Packets TX: 1,420 | Packets RX: 2,840 | Errors: 0
```

### Step 2: Broadcast Swarm Velocity Command
```bash
# Broadcast swarm velocity (linear: 0.4 m/s, angular: 0.1 rad/s) to all follower nodes
tamimystic> espnow swarm 0.4 0.1

# Send a custom payload string to broadcast address (FF:FF:FF:FF:FF:FF)
tamimystic> espnow send FF:FF:FF:FF:FF:FF "FORMATION_WEDGE_SYNC"
```

---

## Summary and Next Steps

You have now exercised:
1. GPIO pin matrix configuration and hardware telemetry
2. 1000 Hz closed-loop differential drive robotics kinematics
3. I2C sensor bus auto-discovery and distance polling
4. Low-latency DVP camera streaming and on-device SIMD AI inference
5. Zero-latency 2.4 GHz ESP-NOW swarm mesh communication

Explore the detailed architecture in the **[Robotics Control Engine](file:///I:/tamimystic-os/docs/robotics/overview.md)** and **[Dynamic Pin Matrix](file:///I:/tamimystic-os/docs/hardware/pin-matrix.md)**.
