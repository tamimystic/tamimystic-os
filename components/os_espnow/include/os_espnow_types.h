#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <array>

namespace TamimysticOS {

enum class EspNowPacketType : uint8_t {
    UNKNOWN = 0,
    REMOTE_JOYSTICK = 1,       // Handheld gamepad remote controller
    SWARM_HEARTBEAT = 2,       // Swarm node status & pose broadcast
    SWARM_FORMATION_CMD = 3,   // Leader formation change
    SWARM_COORDINATION = 4,    // Mutual coordinate broadcast for collision avoidance
    CUSTOM_PAYLOAD = 5,        // Arbitrary user / Python payload
    PING_REQUEST = 6,
    PING_RESPONSE = 7
};

inline const char* espNowPacketTypeToString(EspNowPacketType type) {
    switch (type) {
        case EspNowPacketType::REMOTE_JOYSTICK:     return "Remote Joystick";
        case EspNowPacketType::SWARM_HEARTBEAT:     return "Swarm Heartbeat";
        case EspNowPacketType::SWARM_FORMATION_CMD: return "Formation Command";
        case EspNowPacketType::SWARM_COORDINATION:  return "Swarm Coordination";
        case EspNowPacketType::CUSTOM_PAYLOAD:      return "Custom Payload";
        case EspNowPacketType::PING_REQUEST:        return "Ping Request";
        case EspNowPacketType::PING_RESPONSE:       return "Ping Response";
        default:                                   return "Unknown";
    }
}

enum class SwarmRole : uint8_t {
    STANDALONE = 0,
    LEADER,
    FOLLOWER
};

inline const char* swarmRoleToString(SwarmRole role) {
    switch (role) {
        case SwarmRole::LEADER:   return "LEADER";
        case SwarmRole::FOLLOWER: return "FOLLOWER";
        default:                  return "STANDALONE";
    }
}

enum class SwarmFormation : uint8_t {
    LINE = 0,       // Followers trail in a line behind leader
    COLUMN,         // Side-by-side column
    TRIANGLE,       // V-Shape / Triangle formation
    DIAMOND         // Diamond guard formation
};

inline const char* swarmFormationToString(SwarmFormation form) {
    switch (form) {
        case SwarmFormation::LINE:     return "LINE";
        case SwarmFormation::COLUMN:   return "COLUMN";
        case SwarmFormation::TRIANGLE: return "TRIANGLE";
        case SwarmFormation::DIAMOND:  return "DIAMOND";
        default:                       return "UNKNOWN";
    }
}

#pragma pack(push, 1)

// Remote Wireless Gamepad Packet (< 4ms latency)
struct RemoteJoystickPacket {
    uint8_t packet_type;          // EspNowPacketType::REMOTE_JOYSTICK
    uint8_t controller_id;
    int8_t axis_vx;               // Linear velocity X (-100 to +100%)
    int8_t axis_vy;               // Linear velocity Y (for Mecanum strafing)
    int8_t axis_omega;            // Angular yaw rotation (-100 to +100%)
    uint8_t buttons;              // Bit 0: E-Stop, Bit 1: Arm Home, Bit 2: Gripper Toggle, Bit 3: Speed Boost
    uint16_t arm_joint_target[6]; // Target arm joint angles in 0.1 deg (0 to 1800)
    uint32_t timestamp_ms;
    uint32_t sequence;
};

// Swarm Heartbeat Packet
struct SwarmHeartbeatPacket {
    uint8_t packet_type;          // EspNowPacketType::SWARM_HEARTBEAT
    uint8_t robot_id;
    uint8_t role;                 // SwarmRole
    uint8_t battery_pct;
    float pose_x_cm;
    float pose_y_cm;
    float pose_yaw_deg;
    float velocity_linear;
    float velocity_angular;
    uint32_t timestamp_ms;
    uint32_t sequence;
};

// Formation Command Packet
struct SwarmFormationPacket {
    uint8_t packet_type;          // EspNowPacketType::SWARM_FORMATION_CMD
    uint8_t leader_id;
    uint8_t formation;            // SwarmFormation
    float spacing_cm;             // Inter-robot distance
    uint32_t timestamp_ms;
};

#pragma pack(pop)

struct EspNowPeer {
    uint8_t mac[6] = {0};
    std::string mac_str;
    int8_t rssi = -55;
    uint32_t last_seen_ms = 0;
    SwarmRole role = SwarmRole::STANDALONE;
    uint8_t robot_id = 0;
    bool is_controller = false;
    float last_x_cm = 0.0f;
    float last_y_cm = 0.0f;
    float last_yaw_deg = 0.0f;
};

struct EspNowStatus {
    bool enabled = true;
    std::string own_mac_str = "24:DC:C3:98:45:A0";
    uint8_t wifi_channel = 1;
    SwarmRole swarm_role = SwarmRole::STANDALONE;
    SwarmFormation formation = SwarmFormation::TRIANGLE;
    uint8_t follower_slot = 0;    // 0 = Leader, 1 = Left wing, 2 = Right wing, 3 = Rear guard
    float swarm_spacing_cm = 60.0f;
    bool remote_control_active = false;
    uint32_t packets_sent = 0;
    uint32_t packets_received = 0;
    float packet_loss_pct = 0.0f;
    float avg_latency_ms = 2.4f;
    size_t active_peer_count = 0;
};

} // namespace TamimysticOS
