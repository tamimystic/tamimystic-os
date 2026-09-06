#pragma once

#include "os_ros2_types.h"
#include <string>
#include <vector>
#include <functional>

namespace TamimysticOS {

class Ros2Node {
public:
    static Ros2Node& getInstance();

    // Initialize ROS 2 subsystem, load NVS config, create background task
    void init();

    // Connection lifecycle
    bool connect(const std::string& agent_ip, uint16_t port, uint8_t domain_id = 0);
    bool connect();
    void disconnect();

    // State queries
    Ros2ConnectionState getState() const { return current_state; }
    bool isConnected() const { return current_state == Ros2ConnectionState::RUNNING || current_state == Ros2ConnectionState::SYNCHRONIZED; }
    const Ros2Config& getConfig() const { return config; }
    const Ros2Stats& getStats() const { return stats; }
    std::string getStatusJson();

    // Configuration updates (saved to NVS)
    void setAgentConfig(const std::string& ip, uint16_t port, uint8_t domain_id, bool auto_connect = false);

    // Callbacks & Execution
    void processExecutorCycle();
    void handleInboundCmdVel(const Ros2Twist& twist);

    // Outbound Publishing helpers
    void publishOdometry();
    void publishJointStates();
    void publishImu();
    void publishLaserScan();
    void publishLog(const std::string& message, uint8_t level = 20); // 20 = INFO

private:
    Ros2Node() = default;
    ~Ros2Node() = default;

    void loadNvsConfig();
    void saveNvsConfig();

    Ros2Config config;
    Ros2ConnectionState current_state = Ros2ConnectionState::DISCONNECTED;
    Ros2Stats stats;

    uint32_t last_sync_ping_ms = 0;
    uint32_t last_publish_ms = 0;
    float accumulated_odom_x = 0.0f;
    float accumulated_odom_y = 0.0f;
    float accumulated_odom_yaw = 0.0f;
    uint32_t last_odom_calc_ms = 0;
};

} // namespace TamimysticOS
