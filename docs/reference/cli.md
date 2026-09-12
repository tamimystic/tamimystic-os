# Serial CLI Command Reference (aeron>)

Tamimystic OS features a dedicated interactive command-line interface accessible over the primary UART console at **115200 baud** (8 data bits, no parity, 1 stop bit).

The prompt displays `aeron>` when the shell is active and ready to process commands.

---

## Interactive Shell Overview

```text
=======================================================
       TAMIMYSTIC OS - ESP32-S3 ULTRA PRO MAX          
=======================================================
[BOOT] Starting system bring-up sequence...
[INFO] Starting background services & CLI...
aeron> help
```

---

## Master Command Index

| Command Category | Command Syntax | Arguments & Parameters | Description |
|---|---|---|---|
| **System** | `help` | None | Lists all registered commands and syntax descriptions. |
| | `reboot` | None | Triggers hardware restart. |
| **Networking** | `wifi` | `<ssid> <password>` | Connects to a Wi-Fi access point in Station mode. |
| **Pin Matrix** | `pin show` | None | Displays complete peripheral-to-GPIO mapping table. |
| | `pin set` | `<function_name> <gpio>` | Reassigns a pin function and saves to NVS. |
| | `pin reset` | None | Restores default hardware pin assignments. |
| **Plug & Play** | `pnp scan` | None | Executes active I2C bus address sweep ($0x08 - 0x77$). |
| | `pnp list` | None | Lists all auto-discovered sensors and status. |
| **Robotics** | `robot mode` | `<diff \| mecanum \| arm \| balance>` | Switches active robotics topology. |
| | `robot move` | `<linear_pct> <angular_pct>` | Commands differential rover speed ($-100$ to $+100\%$). |
| | `robot strafe` | `<vx> <vy> <omega>` | Commands holonomic Mecanum velocity vector. |
| | `robot arm` | `<j1> <j2> <j3> <j4> <j5> <j6>` | Sets robotic arm joint angles ($0^\circ$ to $180^\circ$). |
| | `robot ik` | `<x> <y> <z> [pitch] [grip]` | Computes and moves arm tip to Cartesian target (cm). |
| | `robot stop` | None | Engages emergency brake and halts all actuators. |
| | `robot resume` | None | Releases emergency stop. |
| | `robot status` | None | Outputs real-time kinematics telemetry JSON. |
| **Edge AI & Vision** | `ai status` | None | Outputs model FPS, inference latency, and detection list. |
| | `ai model` | `<person \| object \| lane \| gesture>` | Switches active quantized neural network model. |
| | `ai track` | `<on \| off>` | Toggles autonomous visual target tracking loop. |
| | `camera status`| None | Checks DVP driver state and PSRAM allocation. |
| | `camera snap` | None | Captures a snapshot frame into memory. |
| **Scripting & Apps**| `python eval`| `"<python_code>"` | Evaluates raw Python expression string. |
| | `python run` | `"<filename.py>"` | Executes a saved Python script from LittleFS VFS. |
| | `python stop`| None | Terminates executing Python script. |
| | `wasm run` | `"<filename.wasm>"` | Executes WebAssembly bytecode in sandboxed VM. |
| **Storage & OTA** | `storage ls` | None | Lists all files, sizes, and paths in LittleFS VFS. |
| | `storage df` | None | Displays flash storage usage and free space in KB. |
| | `storage rm` | `"<filename>"` | Deletes a file from flash storage. |
| | `ota status` | None | Inspects active partition slot, state, and rollback. |
| **micro-ROS (ROS2)**| `ros2 status`| None | Checks connection state, agent IP, and message counts. |
| | `ros2 connect`| `<ip> <port> <domain_id>` | Connects to micro-ROS agent over UDP. |
| | `ros2 disconnect`| None | Disconnects from micro-ROS agent. |
| **2D LiDAR & SLAM** | `slam status`| None | Displays map explored cells, pose, and scan stats. |
| | `slam nav` | `<target_x_cm> <target_y_cm>` | Initiates A* path planning to coordinates. |
| | `slam clear` | None | Clears occupancy grid map and resets odometry. |
| | `slam cancel`| None | Aborts active navigation waypoint trajectory. |
| | `slam lidar` | `<sim \| rplidar \| ld19>` | Switches active LiDAR driver interface. |
| **Audio Edge AI** | `audio status`| None | Checks I2S state, microphone dB, and KWS status. |
| | `audio say` | `"<phrase>"` | Synthesizes speech phrase via I2S DAC. |
| | `audio tone` | `<freq_hz> <duration_ms>` | Generates PCM square/sine tone. |
| | `audio beep` | `<pattern_id (1-4)>` | Plays preset audio melody or alert chime. |
| | `audio kws` | `<on \| off>` | Enables/disables Keyword Spotting neural listener. |
| | `audio cmd` | `"<keyword_string>"` | Simulates voice keyword command execution. |
| | `audio volume`| `<0-100>` | Adjusts master audio output volume percentage. |
| **ESP-NOW Swarm** | `espnow status`| None | Displays MAC, channel, swarm role, and RF latency. |
| | `espnow peers` | None | Lists paired mesh robots, RSSI, and last poses. |
| | `espnow swarm` | `<leader\|follower\|off> [slot] [spacing]` | Configures multi-robot swarm role and formation. |
| | `espnow remote`| `<on \| off>` | Toggles wireless gamepad receiver listening. |
| | `espnow send` | `<dest_mac> <message>` | Transmits raw payload to peer over ESP-NOW. |

---

## Detailed Command Documentation and Examples

### 1. System & Connectivity Commands

#### `wifi <ssid> <password>`
Connects Tamimystic OS to a local 2.4GHz Wi-Fi access point:
```text
aeron> wifi LabRouter SecretPassword123
[SYS] Connecting to Wi-Fi SSID: LabRouter...
[SYS] Wi-Fi Connected! IP Address: 192.168.1.145
[WEB] Starting ESP32 HTTP Server on Port 80...
```

---

### 2. Hardware & Pin Matrix Commands

#### `pin show`
Outputs the current dynamic software pin mapping table:
```text
aeron> pin show
=== Tamimystic OS Dynamic Pin Matrix ===
  Function            GPIO    Status
  -----------------------------------------
  I2C0_SDA            GPIO4   Active (Safe)
  I2C0_SCL            GPIO5   Active (Safe)
  PWM_MOTOR_L_FWD     GPIO1   Active (Safe)
  PWM_MOTOR_L_REV     GPIO2   Active (Safe)
  PWM_MOTOR_R_FWD     GPIO42  Active (Safe)
  PWM_MOTOR_R_REV     GPIO41  Active (Safe)
  I2S_MIC_DIN         GPIO40  Active (Safe)
  I2S_DAC_DOUT        GPIO39  Active (Safe)
  I2S_BCLK            GPIO41  Active (Safe)
  I2S_WS              GPIO42  Active (Safe)
```

#### `pin set <func_name> <gpio>`
Reassigns a peripheral pin function dynamically:
```text
aeron> pin set I2C0_SDA 6
[PIN] Function I2C0_SDA remapped: GPIO4 -> GPIO6
[NVS] Pin configuration persisted to flash.
```

---

### 3. Plug & Play Hardware Commands

#### `pnp scan`
Performs an active address sweep across the I2C bus:
```text
aeron> pnp scan
[PNP] Scanning I2C Bus on SDA=GPIO4, SCL=GPIO5...
[PNP] Found Device at 0x68 -> MPU-6050 6-Axis IMU (Driver Loaded)
[PNP] Found Device at 0x29 -> VL53L0X Laser ToF Distance Sensor (Driver Loaded)
[PNP] Found Device at 0x40 -> PCA9685 16-Channel PWM Servo Controller (Driver Loaded)
[PNP] Scan complete. 3 devices active.
```

---

### 4. Robotics Kinematics Commands

#### `robot mode <diff | mecanum | arm | balance>`
Switches the active kinematic transformation matrix:
```text
aeron> robot mode mecanum
[ROBOT] Mode changed to: Mecanum Holonomic 4WD
```

#### `robot move <linear_spd> <angular_spd>`
Commands differential rover velocity ($-100$ to $+100\%$):
```text
aeron> robot move 60 20
[ROBOT] Twist set: vx=60.0 cm/s, omega=20.0 deg/s
```

#### `robot arm <j1> <j2> <j3> <j4> <j5> <j6>`
Commands direct 6-DOF joint angles in degrees ($0^\circ - 180^\circ$):
```text
aeron> robot arm 90 45 90 90 90 50
[ROBOT:ARM] Joints updated: J1=90.0, J2=45.0, J3=90.0, J4=90.0, J5=90.0, Grip=50.0%
```

#### `robot ik <x> <y> <z> [pitch] [gripper]`
Executes analytical Inverse Kinematics to position the end-effector in Cartesian space (coordinates in cm):
```text
aeron> robot ik 18.0 0.0 12.0 0.0 80.0
[ROBOT:IK] Target Cartesian: (18.0, 0.0, 12.0 cm)
[ROBOT:IK] Solution Found -> J1=90.0°, J2=42.1°, J3=95.8°, J4=-47.9°, J5=90.0°, Grip=80%
```

---

### 5. Edge AI & Computer Vision Commands

#### `ai status`
```text
aeron> ai status
=== Edge AI Neural Pipeline Telemetry ===
  Active Model    : MobileNet-V2 Person Detector
  Inference Speed : 22.4 FPS (18 ms latency)
  SIMD Vector Opt : ESP-NN Active
  Visual Tracking : ENABLED (Target Locked)
  Detection [0]   : Person (94.8% confidence) @ [X: 0.52, Y: 0.48, W: 0.28, H: 0.62]
```

#### `ai model <person | object | lane | gesture>`
```text
aeron> ai model object
[AI] Switched Neural Model to: COCO-80 Multi-Object Detector
```

---

### 6. micro-ROS (ROS 2) Commands

#### `ros2 connect <agent_ip> <agent_port> <domain_id>`
Connects to an external `micro-ros-agent`:
```text
aeron> ros2 connect 192.168.1.100 8888 0
[ROS2] Initializing Micro XRCE-DDS Client...
[ROS2] Connected to Agent @ 192.168.1.100:8888 (Domain 0)
[ROS2] 6 Topics Active: /cmd_vel, /odom, /joint_states, /imu/data, /scan, /camera
```

---

### 7. 2D LiDAR & SLAM Commands

#### `slam nav <target_x_cm> <target_y_cm>`
Plans and follows an optimal collision-free path to target coordinates:
```text
aeron> slam nav 150.0 100.0
[SLAM] Target: (150.0, 100.0 cm)
[SLAM] A* Path Computed: 28 Waypoints Generated.
[SLAM] Waypoint navigation tracking active.
```

---

### 8. Audio Edge AI Commands

#### `audio say "<phrase>"`
```text
aeron> audio say "Tamimystic OS initialized. Mission ready."
[AUDIO:TTS] Synthesizing Speech: "Tamimystic OS initialized. Mission ready."
[AUDIO:DAC] Tone Output: 587 Hz (60 ms)
```

#### `audio beep <pattern_id>`
* `1`: Boot Melody (C5 -> E5 -> G5)
* `2`: Obstacle Warning Beep
* `3`: Keyword Recognized Chime
* `4`: Emergency Stop Alarm Buzzer
```text
aeron> audio beep 3
[AUDIO:DAC] Tone Output: 1046 Hz (120 ms)
```

---

### 9. ESP-NOW Mesh Swarm Commands

#### `espnow status`
```text
aeron> espnow status
=== ESP-NOW 2.4GHz Radio & Swarm Mesh Status ===
  MAC Address      : 24:DC:C3:98:45:A0
  Wi-Fi Channel    : 1
  Swarm Role       : FOLLOWER
  Formation        : TRIANGLE
  Follower Slot    : 1
  Formation Spacing: 60.0 cm
  Remote Control   : ACTIVE (Listening)
  Packets Sent     : 1420
  Packets Received : 1418
  Average Latency  : 2.1 ms
  Active Peers     : 2
```

#### `espnow swarm <leader | follower | off> [slot] [spacing_cm]`
```text
aeron> espnow swarm follower 1 60.0
[ESPNOW] Swarm Role Changed: FOLLOWER | Slot: 1 | Spacing: 60 cm
```
