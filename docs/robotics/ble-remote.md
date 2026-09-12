# Bluetooth Low Energy (BLE 5.0) and Web Bluetooth

Tamimystic OS incorporates a lightweight, high-performance **Bluetooth Low Energy (BLE 5.0) Subsystem** (`os_ble`) powered by the open-source **NimBLE** stack. This allows direct, zero-install smartphone, tablet, and laptop control using the standard **Web Bluetooth API** in Google Chrome or Microsoft Edge without requiring native app store downloads or local Wi-Fi router infrastructure.

---

## Architectural Workflow & NimBLE GATT Server

The BLE subsystem runs a dedicated GATT (Generic Attribute Profile) Server on **Core 0**, exposing a custom 128-bit Robotics Service:

```mermaid
graph LR
    subgraph ClientDevices["Client Controller (Android / iOS / PC)"]
        Browser["Web Browser (Google Chrome / Edge)"]
        WebBLE["Web Bluetooth API (navigator.bluetooth)"]
        Browser --> WebBLE
    end

    subgraph BLEStack["ESP32-S3 BLE 5.0 Stack (NimBLE on Core 0)"]
        GAP["GAP Advertising (Device Name: 'Tamimystic-Bot', +9 dBm)"]
        GATT["GATT Server (Service UUID: 19B10000-...-1214)"]
        ChrTwist["Char 0xFF01: Joystick Twist (Write Without Response)"]
        ChrArm["Char 0xFF02: 6-DOF Arm Joints (Write)"]
        ChrTelem["Char 0xFF03: System Telemetry (Notify @ 20Hz)"]
        ChrSens["Char 0xFF04: Sensor Stream (Notify @ 20Hz)"]
        ChrCmd["Char 0xFF05: System Command & E-STOP (Write/Read)"]
        
        GATT --> ChrTwist
        GATT --> ChrArm
        GATT --> ChrTelem
        GATT --> ChrSens
        GATT --> ChrCmd
    end

    subgraph Core1RealTime["Core 1: Real-Time Robotics Engine"]
        Motion["1000Hz Motion Controller (Diff / Mecanum)"]
        Arm["6-DOF Inverse Kinematics Engine"]
    end

    WebBLE <-->|2.4 GHz BLE 5.0 Link (< 15ms Latency)| GATT
    ChrTwist -->|Real-Time Velocity Setpoint| Motion
    ChrArm -->|Target Joint Angles| Arm
    Motion -->|Odometry Feedback| ChrTelem
```

---

## 128-Bit GATT Service & Characteristic Specifications

Tamimystic OS defines the base UUID `19B1xxxx-E8F2-537E-4F6C-D104768A1214`:

| Characteristic Identifier | 128-Bit UUID | Properties | Data Payload Format | Description |
|---|---|---|---|---|
| **Robotics Service** | `19B10000-E8F2-537E-4F6C-D104768A1214` | Primary Service | N/A | Parent container service for all robotics attributes. |
| **Joystick Twist** | `19B10001-E8F2-537E-4F6C-D104768A1214` | Write Without Response / Write | `BleTwistPacket` (7 bytes) | Real-time $(v_x, v_y, \omega_z)$ normalized velocity setpoints. |
| **6-DOF Robotic Arm** | `19B10002-E8F2-537E-4F6C-D104768A1214` | Write | `BleArmPacket` (14 bytes) | Target joint angles ($\theta_1 \dots \theta_6$) and gripper position. |
| **System Telemetry** | `19B10003-E8F2-537E-4F6C-D104768A1214` | Read / Notify (20 Hz) | `BleTelemetryPacket` (28 bytes) | Battery %, CPU temp, uptime, PSRAM, odometry $(x, y, \theta)$. |
| **Sensor Feed** | `19B10004-E8F2-537E-4F6C-D104768A1214` | Read / Notify (20 Hz) | `BleSensorPacket` (16 bytes) | Live IMU roll/pitch/yaw angles, ToF laser distance in mm. |
| **System Commands** | `19B10005-E8F2-537E-4F6C-D104768A1214` | Write / Read | Byte array | Emergency Stop (`0xFF`), Clear E-Stop (`0x00`), Reboot. |

---

## Binary Packet Structures

### 1. Joystick Twist Command Packet (7 Bytes):
```c
struct BleTwistPacket {
    int8_t linear_x_pct;   // -100 to +100 (%) -> mapped to +/- 0.8 m/s
    int8_t linear_y_pct;   // -100 to +100 (%) -> mapped to +/- 0.8 m/s (for Mecanum strafe)
    int8_t angular_z_pct;  // -100 to +100 (%) -> mapped to +/- 2.0 rad/s
    uint8_t buttons;       // Bit 0: E-Stop, Bit 1: Arm Home, Bit 2: Turbo Boost
    uint32_t sequence;     // Packet counter for jitter and loss detection
};
```

### 2. 6-DOF Robotic Arm Command Packet (14 Bytes):
```c
struct BleArmPacket {
    uint16_t joint_angles_deg_x10[6]; // Joints 1..6 (0 to 1800 -> 0.0 to 180.0 degrees)
    uint8_t gripper_pct;              // 0 to 100%
    uint8_t flags;
};
```

### 3. System Telemetry Notification Packet (28 Bytes):
```c
struct BleTelemetryPacket {
    uint8_t battery_pct;
    int8_t cpu_temp_c;
    uint16_t voltage_mv;
    uint32_t uptime_sec;
    uint32_t free_psram_kb;
    uint8_t motion_mode;    // 0: Diff, 1: Mecanum, 2: Ackermann, 3: Arm
    uint8_t safety_flags;   // Bit 0: E-Stop active, Bit 1: Proximity barrier
    float odom_x_m;
    float odom_y_m;
    float odom_theta_rad;
};
```

---

## Web Bluetooth Client Implementation (JavaScript)

Developers can build zero-install companion web apps using standard Web Bluetooth:

```javascript
// Scan and connect to Tamimystic OS BLE Robot
async function connectBleRobot() {
    try {
        const device = await navigator.bluetooth.requestDevice({
            filters: [{ namePrefix: 'Tamimystic' }],
            optionalServices: ['19b10000-e8f2-537e-4f6c-d104768a1214']
        });

        const server = await device.gatt.connect();
        const service = await server.getPrimaryService('19b10000-e8f2-537e-4f6c-d104768a1214');
        
        // Characteristic for sending joystick velocity commands
        const twistChar = await service.getCharacteristic('19b10001-e8f2-537e-4f6c-d104768a1214');

        // Characteristic for receiving live telemetry stream
        const telemChar = await service.getCharacteristic('19b10003-e8f2-537e-4f6c-d104768a1214');
        await telemChar.startNotifications();
        telemChar.addEventListener('characteristicvaluechanged', (event) => {
            const data = event.target.value;
            const batteryPct = data.getUint8(0);
            const cpuTemp = data.getInt8(1);
            console.log(`Live Telemetry -> Battery: ${batteryPct}%, CPU: ${cpuTemp} C`);
        });

        console.log("Connected to Tamimystic OS via Web Bluetooth!");
        return twistChar;
    } catch (error) {
        console.error("Bluetooth Connection Failed:", error);
    }
}

// Transmit velocity command over BLE (Write Without Response)
function sendTwistCommand(twistChar, vx_pct, vy_pct, omega_pct) {
    const buffer = new ArrayBuffer(7);
    const view = new DataView(buffer);
    view.setInt8(0, vx_pct);
    view.setInt8(1, vy_pct);
    view.setInt8(2, omega_pct);
    view.setUint8(3, 0); // buttons
    view.setUint32(4, 0, true); // sequence
    twistChar.writeValueWithoutResponse(buffer);
}
```

---

## MicroPython Bluetooth API

```python
import tamimystic
import time

# Check BLE connection state
status = tamimystic.ble.status()
print("BLE Device Name:", status['device_name'])
print("BLE Advertising State:", status['advertising'])

# Start or stop BLE advertising
tamimystic.ble.adv(True)

# Disconnect active smartphone peer
tamimystic.ble.disconnect()
```

---

## Serial CLI and REST API Reference

### Serial CLI Commands:
```bash
# Query BLE 5.0 GATT Server status
tamimystic> ble status

# Output:
# === BLE 5.0 GATT Server Status ===
#   Device Name:       Tamimystic-Bot
#   State:             ADVERTISING
#   Advertising:       ACTIVE
#   Connected Clients: 0
#   Peer Address:      None
#   Service UUID:      19B10000-E8F2-537E-4F6C-D104768A1214
#   Packets RX:        142 | Packets TX: 840

# Start or stop BLE advertising
tamimystic> ble adv start
tamimystic> ble adv stop

# Disconnect active Bluetooth client
tamimystic> ble disconnect
```

### HTTP REST API Endpoints:

#### 1. Query BLE Server State JSON
```bash
GET /api/ble/status
```
**Response JSON:**
```json
{
  "status": "ADVERTISING",
  "device_name": "Tamimystic-Bot",
  "advertising": true,
  "connected_clients": 0,
  "peer_mac": "",
  "rssi_dbm": -127,
  "packets_rx": 142,
  "packets_tx": 840,
  "service_uuid": "19B10000-E8F2-537E-4F6C-D104768A1214",
  "char_twist_uuid": "19B10001-E8F2-537E-4F6C-D104768A1214",
  "char_arm_uuid": "19B10002-E8F2-537E-4F6C-D104768A1214",
  "char_telemetry_uuid": "19B10003-E8F2-537E-4F6C-D104768A1214",
  "char_sensor_uuid": "19B10004-E8F2-537E-4F6C-D104768A1214"
}
```

#### 2. Configure BLE Advertising
```bash
POST /api/ble/adv?action=start
POST /api/ble/adv?action=stop
```
**Response JSON:**
```json
{
  "status": "ok",
  "advertising": true
}
```
