#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace TamimysticOS {

enum class Ros2ConnectionState {
    DISCONNECTED = 0,
    CONNECTING,
    SYNCHRONIZED,
    RUNNING,
    ERROR_STATE
};

inline const char* ros2StateToString(Ros2ConnectionState state) {
    switch (state) {
        case Ros2ConnectionState::DISCONNECTED: return "DISCONNECTED";
        case Ros2ConnectionState::CONNECTING:   return "CONNECTING";
        case Ros2ConnectionState::SYNCHRONIZED: return "SYNCHRONIZED";
        case Ros2ConnectionState::RUNNING:      return "RUNNING";
        case Ros2ConnectionState::ERROR_STATE:  return "ERROR";
        default:                                return "UNKNOWN";
    }
}

enum class Ros2TransportType {
    UDP_WIFI = 0,
    SERIAL_UART = 1
};

struct Ros2Config {
    std::string agent_ip = "192.168.1.100";
    uint16_t agent_port = 8888;
    uint8_t domain_id = 0;
    std::string node_name = "tamimystic_os_node";
    std::string namespace_name = "";
    Ros2TransportType transport = Ros2TransportType::UDP_WIFI;
    bool auto_connect = false;
    uint32_t publish_rate_hz = 30;
};

// Standard ROS2 message representations
struct Ros2Vector3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Ros2Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;
};

struct Ros2Twist {
    Ros2Vector3 linear;
    Ros2Vector3 angular;
};

struct Ros2Pose {
    Ros2Vector3 position;
    Ros2Quaternion orientation;
};

struct Ros2Odometry {
    std::string frame_id = "odom";
    std::string child_frame_id = "base_link";
    Ros2Pose pose;
    Ros2Twist twist;
    uint32_t timestamp_ms = 0;
};

struct Ros2JointState {
    std::vector<std::string> name = {"j1_base", "j2_shoulder", "j3_elbow", "j4_wrist_pitch", "j5_wrist_roll", "j6_gripper"};
    std::vector<float> position = {1.57f, 0.78f, 1.57f, 1.57f, 1.57f, 0.0f};
    std::vector<float> velocity = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    uint32_t timestamp_ms = 0;
};

struct Ros2Imu {
    std::string frame_id = "imu_link";
    Ros2Quaternion orientation;
    Ros2Vector3 angular_velocity;
    Ros2Vector3 linear_acceleration;
    uint32_t timestamp_ms = 0;
};

struct Ros2LaserScan {
    std::string frame_id = "laser_frame";
    float angle_min = -3.14159f;
    float angle_max = 3.14159f;
    float angle_increment = 0.01745f;
    float time_increment = 0.0001f;
    float scan_time = 0.1f;
    float range_min = 0.1f;
    float range_max = 12.0f;
    std::vector<float> ranges;
    uint32_t timestamp_ms = 0;
};

struct Ros2Stats {
    uint32_t msgs_received = 0;
    uint32_t msgs_sent = 0;
    uint32_t errors = 0;
    uint32_t last_cmd_vel_timestamp = 0;
    float current_loop_hz = 0.0f;
};

} // namespace TamimysticOS
