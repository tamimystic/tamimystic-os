# ROS 2 and micro-ROS Integration

Tamimystic OS features native, first-class integration with the **Robot Operating System 2 (ROS 2)** ecosystem through embedded **micro-ROS**. This allows the ESP32-S3 to function as a full-fledged ROS 2 node communicating seamlessly with ROS 2 Jazzy, Iron, and Humble installations.

---

## Architectural Overview & XRCE-DDS Middleware

micro-ROS implements the **eProsima Micro XRCE-DDS** (eXtremely Resource Constrained Environments DDS) client-agent protocol. The ESP32-S3 runs the lightweight micro-ROS Client, which communicates with a host computer running the `micro_ros_agent`:

```mermaid
graph LR
    subgraph ESP32Target["Tamimystic OS (ESP32-S3)"]
        Sensors["IMU / LiDAR / Encoders"] --> Node["micro-ROS Node (/tamimystic_robot)"]
        Motors["1000Hz Motion Controller"] <-- Node
        Arm["6-DOF Arm Kinematics"] <-- Node
    end

    subgraph TransportLayer["Transport Layer (UDP / Wi-Fi or Serial)"]
        Node <-->|eProsima Micro XRCE-DDS UDP:8888| Agent["micro_ros_agent (Docker / Host PC)"]
    end

    subgraph HostROS2["Host Computer (ROS 2 Jazzy / Iron / Humble)"]
        Agent <--> DDS["Standard DDS Network (CycloneDDS / FastDDS)"]
        DDS <--> RViz["RViz2 Visualization"]
        DDS <--> Nav2["Nav2 Navigation Stack"]
        DDS <--> MoveIt["MoveIt 2 Manipulation"]
        DDS <--> Teleop["teleop_twist_keyboard"]
    end
```

---

## Published and Subscribed ROS 2 Topics

### Subscribed Topics (Commands to Robot)

| Topic Name | Message Type | QoS Profile | Description |
|---|---|---|---|
| `/cmd_vel` | `geometry_msgs/msg/Twist` | Default (Reliable) | Mobile base linear and angular velocity commands ($v_x, v_y, \omega_z$). |
| `/arm_cmd` | `std_msgs/msg/Float32MultiArray` | Reliable | Target joint angles ($\theta_1 \dots \theta_6$) in degrees for 6-DOF robotic arm. |
| `/gripper_cmd` | `std_msgs/msg/Int32` | Reliable | Gripper claw open/close percentage ($0 - 100\%$). |
| `/reset_odom` | `std_msgs/msg/Empty` | Reliable | Resets Cartesian odometry coordinates to origin $(0, 0, 0)$. |

### Published Topics (Telemetry & Sensor Feeds)

| Topic Name | Message Type | Rate (Hz) | QoS Profile | Description |
|---|---|---|---|---|
| `/odom` | `nav_msgs/msg/Odometry` | 50 Hz | Best Effort | Real-time Cartesian robot pose $(x, y, \theta)$ and covariance matrices. |
| `/scan` | `sensor_msgs/msg/LaserScan` | 10 Hz | Best Effort | 2D LiDAR 360-degree planar range distance point array. |
| `/imu/data_raw` | `sensor_msgs/msg/Imu` | 100 Hz | Best Effort | 6-DOF linear acceleration and angular velocity readings from MPU-6050. |
| `/joint_states` | `sensor_msgs/msg/JointState` | 20 Hz | Best Effort | Live feedback of 6-DOF robotic arm joint positions and velocities. |
| `/tf` | `tf2_msgs/msg/TFMessage` | 50 Hz | Best Effort | Coordinate frame transforms (`odom` $\to$ `base_link` $\to$ `laser_frame`). |
| `/diagnostics` | `diagnostic_msgs/msg/DiagnosticArray` | 1 Hz | Reliable | Free memory, CPU temperature, Wi-Fi RSSI, and battery voltage. |

---

## Step-by-Step ROS 2 Agent Setup on Host PC

### Step 1: Install and Run micro-ROS Agent via Docker
The easiest and cleanest method to start the micro-ROS Agent on your ROS 2 host PC:

```bash
# Pull and execute micro-ROS Agent container (UDP Transport on port 8888)
docker run -it --rm --net=host microros/micro-ros-agent:jazzy udp4 --port 8888
```

For serial UART connection via USB:
```bash
docker run -it --rm --net=host --privileged -v /dev:/dev microros/micro-ros-agent:jazzy serial --dev /dev/ttyUSB0 -b 115200
```

### Step 2: Configure micro-ROS on Tamimystic OS

#### Option A: Via Serial CLI
```bash
# Configure Agent IP and Port
tamimystic> ros2 config 192.168.1.100 8888

# Connect to the micro-ROS Agent
tamimystic> ros2 start
[ROS2] Connecting to micro-ROS Agent at 192.168.1.100:8888...
[ROS2] Node '/tamimystic_robot' registered successfully!
[ROS2] Publishers initialized: /odom, /scan, /imu/data_raw, /tf
[ROS2] Subscribers initialized: /cmd_vel, /arm_cmd
```

#### Option B: Via HTTP REST API
```bash
curl -X POST "http://192.168.4.1/api/ros2/config?agent_ip=192.168.1.100&port=8888"
curl -X POST "http://192.168.4.1/api/ros2/start"
```

---

## Verifying ROS 2 Communication on Host PC

Once the agent and client handshake completes, open a new terminal on your ROS 2 host machine:

### 1. List Active Nodes
```bash
ros2 node list
# Output:
# /tamimystic_robot
```

### 2. Echo Odometry Feed
```bash
ros2 topic echo /odom
# Output:
# header:
#   stamp: {sec: 1726131400, nanosec: 420000000}
#   frame_id: "odom"
# child_frame_id: "base_link"
# pose:
#   pose:
#     position: {x: 0.452, y: 0.120, z: 0.0}
#     orientation: {x: 0.0, y: 0.0, z: 0.382, w: 0.924}
```

### 3. Drive Robot with Keyboard Teleop
```bash
ros2 run teleop_twist_keyboard teleop_twist_keyboard
```

### 4. Visualize in RViz2
Launch RViz2 to visualize real-time LiDAR point clouds and robot transforms:
```bash
ros2 run rviz2 rviz2
```
1. Set Fixed Frame to `odom`.
2. Add a **LaserScan** display subscribed to `/scan`.
3. Add an **Odometry** display subscribed to `/odom`.
4. Add a **TF** display to view coordinate frames.

---

## ROS 2 Launch File Example (`tamimystic_bringup.launch.py`)

```python
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # micro-ROS Agent Node
        Node(
            package='micro_ros_agent',
            executable='micro_ros_agent',
            name='micro_ros_agent',
            output='screen',
            arguments=['udp4', '--port', '8888']
        ),
        
        # Robot State Publisher & Static Transform (laser_frame to base_link)
        Node(
            package='tf2_ros',
            executable='static_transform_publisher',
            arguments=['0.1', '0.0', '0.08', '0.0', '0.0', '0.0', 'base_link', 'laser_frame']
        )
    ])
```
