# 2D LiDAR and SLAM Navigation

Tamimystic OS features an onboard **2D LiDAR SLAM (Simultaneous Localization and Mapping)** and **Autonomous Path Planning Engine** (`os_slam`). By executing lightweight log-odds Bayesian mapping and $A^*$ heuristic pathfinding in the 8 MB Octal PSRAM of the ESP32-S3, the robot achieves fully autonomous indoor navigation without requiring an external companion computer.

---

## Hardware Interfacing: 2D UART LiDAR Protocol

Tamimystic OS supports standard UART-based 360-degree 2D LiDAR sensors (RPLiDAR A1/A2, LD06, D200, YDLIDAR X2/X4):

- **Physical Interface**: Hardware UART2 (`RX`: GPIO 17, `TX`: GPIO 18) at `115200` or `230400 baud`.
- **Scan Rate**: 5 Hz to 12 Hz (360 samples per revolution).
- **Measurement Envelope**: $0.15\text{ m}$ to $12.0\text{ m}$ with $\pm 1.5\%$ ranging accuracy.

```mermaid
graph LR
    LiDAR["2D UART LiDAR Scanner"] -->|UART2 DMA Ingress| Parser["Packet Frame Parser & CRC Validator"]
    Parser -->|Raw Polar Points (theta_i, r_i)| CoordTransform["Polar to Cartesian Transform (x_i, y_i)"]
    CoordTransform --> Raycaster["Bresenham Discrete Raycasting Engine"]
    Raycaster --> GridMap["80x80 Occupancy Grid (Log-Odds PSRAM Buffer)"]
    GridMap --> AStar["A* Global Path Planning Algorithm"]
    AStar --> MotionCtrl["1000Hz Motion Controller Execution"]
```

---

## Polar to Cartesian Coordinate Transformation

Each incoming LiDAR beam measurement consists of an angle $\theta_i \in [0, 2\pi)$ and radial distance $r_i$:

### Robot Frame Transformation:
$$x_{\text{robot}} = r_i \cdot \cos(\theta_i)$$

$$y_{\text{robot}} = r_i \cdot \sin(\theta_i)$$

### Global Map Frame Transformation:
Given the robot's current odometry pose $(x_{\text{odom}}, y_{\text{odom}}, \theta_{\text{odom}})$:

$$x_{\text{map}} = x_{\text{odom}} + x_{\text{robot}} \cdot \cos(\theta_{\text{odom}}) - y_{\text{robot}} \cdot \sin(\theta_{\text{odom}})$$

$$y_{\text{map}} = y_{\text{odom}} + x_{\text{robot}} \cdot \sin(\theta_{\text{odom}}) + y_{\text{robot}} \cdot \cos(\theta_{\text{odom}})$$

---

## 2D Occupancy Grid Mapping & Bayesian Log-Odds Updates

The map is represented as an $80 \times 80$ discrete grid with a cell resolution of $5\text{ cm}$ per pixel (representing a $4.0\text{ m} \times 4.0\text{ m}$ local navigation arena).

Each grid cell $m_{i,j}$ stores a log-odds occupancy probability value $l(m_{i,j})$:

$$l_t(m_{i,j}) = l_{t-1}(m_{i,j}) + \text{sensor\_model}(m_{i,j}, z_t) - l_0$$

Where:
- $l_0 = 0.0$ represents the prior unmapped state ($P(\text{occupied}) = 0.5$).
- $\text{sensor\_model} = +0.85$ (Log-Odds increment for obstacle hit cell).
- $\text{sensor\_model} = -0.40$ (Log-Odds decrement for free-space raycasted cells).

To convert log-odds back to visual occupancy probability $[0.0, 1.0]$ for the Web Dashboard:
$$P(m_{i,j}) = 1 - \frac{1}{1 + \exp(l(m_{i,j}))}$$

### Bresenham Discrete Raycasting:
For every valid LiDAR distance reading, the kernel executes Bresenham's integer line algorithm to trace all grid cells from the robot center $(x_r, y_r)$ to the target obstacle cell $(x_o, y_o)$, decrementing free-space cells and incrementing the terminal obstacle cell.

---

## $A^*$ (A-Star) Global Path Planning

When a target waypoint $(x_{\text{goal}}, y_{\text{goal}})$ is designated, the $A^*$ planner computes the shortest collision-free path across the 8-connected grid graph:

$$f(n) = g(n) + h(n)$$

Where:
- $g(n)$ is the exact path cost from the start node to node $n$.
  - Straight step cost $= 1.0$ ($5\text{ cm}$).
  - Diagonal step cost $= \sqrt{2} \approx 1.414$ ($7.07\text{ cm}$).
- $h(n)$ is the admissible Euclidean distance heuristic:
  $$h(n) = \sqrt{(x_n - x_{\text{goal}})^2 + (y_n - y_{\text{goal}})^2}$$

### Obstacle Inflation:
To prevent the robot's physical chassis (radius $R_{\text{robot}} = 15\text{ cm}$) from clipping corners, all detected occupied cells undergo a 3-cell morphological dilation (obstacle inflation radius).

---

## Dynamic PSRAM Memory Allocation

To prevent internal SRAM heap overflow, the entire SLAM state is dynamically allocated in the 8 MB Octal PSRAM:
- Grid Occupancy Buffer: $80 \times 80 \times 4\text{ bytes} = 25.6\text{ KB}$
- $A^*$ Open/Closed Score Buffers: $80 \times 80 \times 8\text{ bytes} = 51.2\text{ KB}$
- Polar Scan Raw Buffer: $360 \times 4\text{ bytes} = 1.44\text{ KB}$

---

## MicroPython SLAM API

```python
import tamimystic
import time

# Start LiDAR scanning and SLAM engine
tamimystic.slam.start()

# Query current robot SLAM pose
pose = tamimystic.slam.get_pose()
print(f"SLAM Estimated Pose: X={pose['x']:.2f}m, Y={pose['y']:.2f}m, Theta={pose['theta']:.1f}deg")

# Plan and navigate to target waypoint (X=1.5m, Y=2.0m)
path = tamimystic.slam.plan_path(goal_x=1.5, goal_y=2.0)
print(f"Generated Path contains {len(path)} waypoints")

# Start autonomous waypoint following
tamimystic.slam.navigate_to(goal_x=1.5, goal_y=2.0)

# Export Occupancy Grid as Base64 image
grid_data = tamimystic.slam.get_grid_png()
```

---

## Serial CLI and REST API Reference

### Serial CLI:
```bash
# Start LiDAR scan and mapping daemon
tamimystic> slam start

# Display ASCII visualization of 80x80 occupancy grid
tamimystic> slam map

# Plan and execute path to target waypoint (x, y in meters)
tamimystic> slam goto 1.5 2.0

# Clear map grid
tamimystic> slam clear
```

### HTTP REST API:
```bash
# Fetch live 80x80 Occupancy Grid Map
GET /api/slam/map

# Start / Stop SLAM
POST /api/slam/start
POST /api/slam/stop

# Dispatch Navigation Waypoint
POST /api/slam/goto?x=1.5&y=2.0
```
