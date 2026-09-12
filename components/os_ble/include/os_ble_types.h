#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <array>

namespace TamimysticOS {

// Standard Tamimystic OS Robotics 128-bit Service & Characteristic UUIDs
// Base: 19B1xxxx-E8F2-537E-4F6C-D104768A1214
inline const char* BLE_ROBOTICS_SERVICE_UUID     = "19B10000-E8F2-537E-4F6C-D104768A1214";
inline const char* BLE_CHAR_JOYSTICK_TWIST_UUID  = "19B10001-E8F2-537E-4F6C-D104768A1214"; // Write Without Response
inline const char* BLE_CHAR_ROBOTIC_ARM_UUID     = "19B10002-E8F2-537E-4F6C-D104768A1214"; // Write
inline const char* BLE_CHAR_TELEMETRY_UUID       = "19B10003-E8F2-537E-4F6C-D104768A1214"; // Notify / Read
inline const char* BLE_CHAR_SENSOR_FEED_UUID     = "19B10004-E8F2-537E-4F6C-D104768A1214"; // Notify / Read
inline const char* BLE_CHAR_SYSTEM_CMD_UUID      = "19B10005-E8F2-537E-4F6C-D104768A1214"; // Write / Read

enum class BleConnectionState : uint8_t {
    DISCONNECTED = 0,
    ADVERTISING,
    CONNECTED,
    ERROR_STATE
};

inline const char* bleConnectionStateToString(BleConnectionState state) {
    switch (state) {
        case BleConnectionState::ADVERTISING: return "ADVERTISING";
        case BleConnectionState::CONNECTED:   return "CONNECTED";
        case BleConnectionState::ERROR_STATE: return "ERROR";
        default:                              return "DISCONNECTED";
    }
}

enum class BleSecurityMode : uint8_t {
    OPEN_NO_AUTH = 0,
    PASSKEY_PAIRING
};

#pragma pack(push, 1)

// Joystick Twist Packet (Received from Web Bluetooth / Smartphone App)
struct BleTwistPacket {
    int8_t linear_x_pct;   // -100 to +100 (%)
    int8_t linear_y_pct;   // -100 to +100 (%) for Mecanum strafe
    int8_t angular_z_pct;  // -100 to +100 (%)
    uint8_t buttons;       // Bit 0: E-Stop, Bit 1: Arm Home, Bit 2: Turbo
    uint32_t sequence;
};

// 6-DOF Robotic Arm Packet
struct BleArmPacket {
    uint16_t joint_angles_deg_x10[6]; // Joint 1..6 (0 to 1800 -> 0.0 to 180.0 deg)
    uint8_t gripper_pct;              // 0 to 100%
    uint8_t flags;
};

// Real-Time System Telemetry Packet (Notified to Smartphone / Browser at 20Hz)
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

// Live Sensor Stream Packet (Notified at 20Hz)
struct BleSensorPacket {
    float roll_deg;
    float pitch_deg;
    float yaw_deg;
    uint16_t distance_mm;
    uint16_t pressure_hpa_x10;
};

#pragma pack(pop)

struct BleConfig {
    std::string device_name = "Tamimystic-Bot";
    uint16_t adv_interval_ms = 100;
    int8_t tx_power_dbm = 9; // +9 dBm for max BLE range
    BleSecurityMode security_mode = BleSecurityMode::OPEN_NO_AUTH;
    bool auto_start_advertising = true;
};

struct BleStatus {
    BleConnectionState state = BleConnectionState::DISCONNECTED;
    std::string device_name = "Tamimystic-Bot";
    bool advertising = false;
    uint16_t connected_clients = 0;
    std::string peer_mac = "";
    int8_t rssi_dbm = -127;
    uint32_t packets_rx = 0;
    uint32_t packets_tx = 0;
    uint32_t last_activity_ms = 0;
};

} // namespace TamimysticOS
