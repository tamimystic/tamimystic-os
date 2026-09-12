# Serial CLI Command Reference (tamimystic>)

Tamimystic OS features a dedicated interactive command-line interface accessible over the primary UART console at **115200 baud** (8 data bits, no parity, 1 stop bit).

The prompt displays `tamimystic>` when the shell is active and ready to process commands.

---

## Interactive Shell Overview

```text
=======================================================
       TAMIMYSTIC OS - ESP32-S3 ULTRA PRO MAX          
=======================================================
[BOOT] Starting system bring-up sequence...
[INFO] Starting background services & CLI...
tamimystic> help
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
| **Filesystem & VFS**| `storage ls` | None | Lists all files and directories in LittleFS partition. |
| | `storage cat`| `<filename>` | Reads and displays file text content. |
| | `storage rm` | `<filename>` | Deletes target file from flash storage. |
| | `storage df` | None | Outputs storage partition capacity, used bytes, and free space. |
| **ROS 2 micro-ROS**| `ros2 status`| None | Displays micro-ROS Agent connection state and topic stats. |
| | `ros2 connect`| `<agent_ip> [port] [domain]` | Connects to remote XRCE-DDS Agent. |
| | `ros2 disconnect` | None | Gracefully severs micro-ROS agent connection. |
| **2D LiDAR SLAM** | `slam status`| None | Outputs current robot pose $(x, y, \theta)$ and mapped cells. |
| | `slam map` | None | Renders ASCII visualization of the 80x80 occupancy grid. |
| | `slam nav` | `<target_x_cm> <target_y_cm>` | Executes $A^*$ path planning to destination. |
| | `slam clear` | None | Wipes current occupancy grid map. |
| | `slam cancel`| None | Aborts active navigation trajectory. |
| **Audio Edge AI** | `audio status`| None | Displays I2S DMA state, KWS neural status, and volume. |
| | `audio cmd` | `<command_name>` | Manually triggers a recognized voice action. |
| | `audio say` | `"<phrase>"` | Synthesizes custom speech output over I2S speaker. |
| | `audio beep` | `[pattern_1-4]` | Plays programmed acoustic alert chime. |
| | `audio volume`| `<0-100>` | Adjusts digital master volume output. |
| **ESP-NOW Swarm** | `espnow status`| None | Displays 2.4GHz radio status, swarm role, peers, and packets. |
| | `espnow swarm` | `<leader \| follower \| off> [slot] [spacing]` | Configures Leader-Follower swarm coordination role. |
| | `espnow remote`| `<on \| off>` | Enables wireless gamepad remote reception. |
| | `espnow send` | `<dest_mac> <payload_string>` | Sends custom packet to specific peer or broadcast. |
| **BLE 5.0 & Web App**| `ble status` | None | Displays BLE GATT Server state, connected peers, and packet counts. |
| | `ble adv` | `<start \| stop>` | Starts or stops 2.4GHz BLE 5.0 GAP advertising. |
| | `ble disconnect` | None | Forcibly disconnects connected Web Bluetooth or mobile client. |

---

## Detailed Command Specifications

### 1. System & Diagnostic Commands

#### `help`
Lists all registered commands in alphabetical order.
```text
tamimystic> help
Available commands:
  help - List all available commands
  reboot - Restart the system
  wifi - Connect to Wi-Fi AP: wifi <ssid> <password>
  pin - Manage Pin Matrix: pin [show|set <func> <gpio>|reset]
  pnp - Plug and Play hardware discovery: pnp [scan|list]
  robot - Robot motion control: robot [mode|move|strafe|arm|ik|stop|resume|status]
  camera - Camera capture and status: camera [status|snap]
  ai - Edge AI Neural Model: ai [status|model <person|object|lane|gesture>|track <on|off>]
  storage - LittleFS Flash storage: storage [ls|cat <file>|rm <file>|df]
  python - MicroPython interpreter: python [eval "code"|run file.py|stop]
  wasm - WebAssembly runtime: wasm run file.wasm
  ros2 - micro-ROS DDS client: ros2 [status|connect <ip> [port] [domain]|disconnect]
  slam - 2D LiDAR SLAM & A* Navigation: slam [status|map|nav <x> <y>|clear|cancel]
  audio - Audio Edge AI & Voice Control: audio [status|cmd <c>|say "text"|beep [p]|volume <0-100>]
  espnow - ESP-NOW Swarm Mesh Radio: espnow [status|peers|swarm <leader|follower|off>|remote <on|off>|send <mac> <msg>]
  ble - Manage BLE 5.0 GATT server: ble [status|adv <start|stop>|disconnect]
```

---

### 2. Bluetooth Low Energy (BLE 5.0) Commands

#### `ble status`
Displays the active state of the NimBLE GATT Server:
```text
tamimystic> ble status
=== BLE 5.0 GATT Server Status ===
  Device Name:       Tamimystic-Bot
  State:             ADVERTISING
  Advertising:       ACTIVE
  Connected Clients: 0
  Peer Address:      None
  Service UUID:      19B10000-E8F2-537E-4F6C-D104768A1214
  Packets RX:        0 | Packets TX: 0
```

#### `ble adv <start | stop>`
Starts or stops BLE advertising:
```text
tamimystic> ble adv stop
[BLE] Advertising stopped.

tamimystic> ble adv start
[BLE] Advertising started.
```

#### `ble disconnect`
Terminates active connection with a paired Web Bluetooth client or smartphone:
```text
tamimystic> ble disconnect
[BLE] Disconnected active client.
```
