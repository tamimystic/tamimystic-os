# 2D LiDAR SLAM and Occupancy Grid Navigation

Tamimystic OS features an embedded real-time 2D LiDAR SLAM (Simultaneous Localization and Mapping) and autonomous path planning engine. Operating directly on the ESP32-S3 (with 8MB PSRAM acceleration) or in the native PC simulation environment, the SLAM subsystem maintains a 200x200 2D occupancy grid, performs real-time ray-casting, and plans collision-free trajectories using an optimized A* search algorithm.

---

## System Architecture

```mermaid
graph TD
    subgraph Hardware["LiDAR Hardware / Simulation"]
        RPLIDAR["RPLiDAR A1 / A2 (UART 115200)"]
        LD19["LD19 / D300 LiDAR (UART 230400)"]
        SIM["Simulated 360 Laser Scanner"]
    end

    subgraph SLAM_Core["Core 1: SLAM Engine & Navigation"]
        PARSE["LiDAR Packet Parser (360 Range/Angle)"]
        BRESENHAM["Bresenham Fast Ray-Casting"]
        GRID["200x200 PSRAM Occupancy Grid (10m x 10m @ 5cm/cell)"]
        ASTAR["A* Path Planner with Obstacle Inflation"]
        PURSUIT["Pure-Pursuit Waypoint Controller"]
    end

    subgraph Actuation["Robotics Kinematics"]
        KIN["Differential / Mecanum Controller (50Hz)"]
    end

    subgraph Interfaces["User & Network Interfaces"]
        WEB["Web Dashboard (Interactive Canvas & Click-to-Nav)"]
        CLI["Interactive Shell (slam status, nav, scan)"]
        PYTHON["MicroPython (tamimystic.slam.*)"]
        ROS2["micro-ROS (/scan, /map, /nav_goal)"]
    end

    RPLIDAR --> PARSE
    LD19 --> PARSE
    SIM --> PARSE

    PARSE --> BRESENHAM
    BRESENHAM --> GRID
    GRID --> ASTAR
    ASTAR --> PURSUIT
    PURSUIT -->|setTwist(vx, vy, omega)| KIN

    GRID <--> WEB
    GRID <--> CLI
    GRID <--> PYTHON
    GRID <--> ROS2
```

---

## Occupancy Grid Specifications

| Parameter | Value | Description |
|---|---|---|
| **Grid Dimensions** | 200 x 200 cells | 10m x 10m total map coverage |
| **Grid Resolution** | 5.0 cm / cell | Granular navigation precision |
| **Memory Footprint** | 40,000 bytes (40 KB) | Statically mapped into ESP32-S3 8MB Octal PSRAM |
| **Cell State Values** | `0` = Unknown, `-1` = Free/Traversable, `100` = Obstacle | Three-state probability representation |
| **Origin Reference** | Center Cell (100, 100) | Robot initial coordinates (0.0, 0.0 cm) |
| **LiDAR Update Rate** | 10 Hz (Core 1 Task) | Real-time map evolution |

---

## Supported 2D LiDAR Sensors and Pinout

Tamimystic OS provides dedicated UART packet drivers with hardware checksum verification:

### 1. RPLiDAR A1 / A2 (Slamtec)
* **Baud Rate**: 115200 bps (8N1)
* **Default Wiring**:
  * **VCC**: 5V (External Power recommended for motor spin)
  * **GND**: Common Ground
  * **TX (LiDAR)**: Connect to ESP32-S3 **GPIO 18** (RX)
  * **RX (LiDAR)**: Connect to ESP32-S3 **GPIO 17** (TX)
  * **MOTOCTRL**: Connect to 5V or PWM GPIO for speed modulation

### 2. LD06 / LD19 / D300 (LDROBOT)
* **Baud Rate**: 230400 bps (8N1)
* **Default Wiring**:
  * **VCC**: 5V (5V motor + 3.3V logic)
  * **GND**: Common Ground
  * **TX (LiDAR)**: Connect to ESP32-S3 **GPIO 18** (RX)
  * **PWM / EN**: Connect to 3.3V for constant 10Hz rotation

---

## Mapping and Path Planning Algorithms

### 1. Bresenham Fast Ray-Casting
Every laser scan point (distance d, angle theta) is converted into cartesian coordinates relative to the robot's current pose (x_r, y_r, theta_r):

- x_w = x_r + d * cos(theta_r + theta)
- y_w = y_r + d * sin(theta_r + theta)

A discrete Bresenham line algorithm traces the line segment from the robot origin cell to (x_w, y_w):
* All intermediate grid cells along the beam are marked as **Free** (`-1`).
* The termination endpoint cell is marked as **Occupied** (`100`).

### 2. A* (A-Star) Path Search with Inflation
When an autonomous target is commanded:
1. **Safety Inflation**: Obstacle cells are dynamically inflated by a safety radius (3 cells / 15 cm) to keep the robot from scraping walls.
2. **Heuristic Evaluation**: The priority queue evaluates nodes via f(n) = g(n) + h(n), where g(n) is the exact travel cost and h(n) is Euclidean distance to the goal.
3. **Waypoint Navigation**: The generated path is fed into a pure-pursuit trajectory controller that adjusts linear and angular velocities to smoothly follow waypoints.

---

## Interactive Web Dashboard Navigation

The integrated Web Dashboard includes a live 2D SLAM Canvas with click-to-navigate functionality:

1. Open `http://<device-ip>/` in your browser.
2. Locate the **2D LiDAR SLAM and A* Navigation** card.
3. The canvas renders:
   * **Dark Blue Cells**: Explored, traversable terrain.
   * **Red Cells**: Detected walls, obstacles, and furniture.
   * **Cyan Triangle**: Live robot position and heading orientation.
   * **Cyan Trajectory Line**: A* planned path waypoints.
   * **Gold Marker**: Current destination goal.
4. **Click-to-Navigate**: Click anywhere on the map to set a new goal. The robot calculates a collision-free path and navigates autonomously.

---

## Python API Reference

Scripts executed via the in-browser IDE or uploaded as `.py` files can access SLAM routines via `tamimystic.slam`:

```python
import tamimystic

# Clear or reset the existing grid map
tamimystic.slam.clear()

# Set an autonomous navigation goal (x, y in cm)
tamimystic.slam.nav(150.0, 80.0)

# Abort active navigation
tamimystic.slam.cancel()
```

---

## Serial CLI Commands

Manage the SLAM subsystem directly from the interactive shell:

```bash
# Check SLAM status, explored cells, robot pose, and active goals
aeron> slam status

# Set an autonomous navigation target (x, y in cm)
aeron> slam nav 120.0 50.0

# Print a 360-degree point-cloud sample
aeron> slam scan

# Clear and reset the occupancy grid map
aeron> slam clear

# Switch LiDAR sensor driver (sim, rplidar, ld19)
aeron> slam lidar rplidar
```

---

## REST API Endpoints

| Endpoint | Method | Parameters | Response Description |
|---|---|---|---|
| `/api/slam/status` | `GET` | None | JSON object with pose, scan counts, and navigation state. |
| `/api/slam/map` | `GET` | None | Compressed occupancy grid coordinates, robot pose, and active waypoints. |
| `/api/slam/nav` | `POST` / `GET` | `x=<float>&y=<float>` | Sets an autonomous goal coordinate in centimeters. |
| `/api/slam/clear` | `POST` / `GET` | None | Clears the occupancy map and resets exploration counters. |
| `/api/slam/cancel` | `POST` / `GET` | None | Aborts the active A* navigation trajectory. |
| `/api/slam/lidar` | `POST` / `GET` | `type=<sim|rplidar|ld19>` | Selects the active LiDAR driver. |
