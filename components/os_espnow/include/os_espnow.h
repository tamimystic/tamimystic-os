#pragma once

#include "os_espnow_types.h"
#include <string>
#include <vector>
#include <mutex>
#include <map>

namespace TamimysticOS {

class EspNowEngine {
public:
    static EspNowEngine& getInstance();

    // Initialize ESP-NOW radio & Core 0/1 background communication task
    void init();

    // Peer Management
    bool addPeer(const uint8_t mac[6], uint8_t channel = 1);
    bool removePeer(const uint8_t mac[6]);
    std::vector<EspNowPeer> getPeers() const;
    std::string getPeersJson() const;

    // Swarm Coordination
    void setSwarmRole(SwarmRole role, uint8_t follower_slot = 0, float spacing_cm = 60.0f);
    void setFormation(SwarmFormation formation, float spacing_cm = 60.0f);
    SwarmRole getSwarmRole() const { return status.swarm_role; }
    void broadcastSwarmHeartbeat();
    void broadcastFormationCommand(SwarmFormation formation, float spacing_cm);

    // Wireless Handheld Remote Controller
    void setRemoteControlEnabled(bool enabled);
    bool isRemoteControlEnabled() const { return status.remote_control_active; }
    void handleRemoteJoystickPacket(const RemoteJoystickPacket& pkt);

    // Packet transmission & reception
    bool sendCustomPayload(const std::string& target_mac, const std::string& message);
    void handleIncomingPacket(const uint8_t* mac, const uint8_t* data, size_t len, int rssi = -50);

    // Telemetry & JSON for Web Dashboard / CLI
    EspNowStatus getStatus() const;
    std::string getStatusJson() const;

    // Simulated / Test Injection methods (for simulation & unit tests)
    void triggerSimulatedRemoteInput(float vx_pct, float vy_pct, float omega_pct, bool estop, bool gripper);
    void triggerSimulatedSwarmPeer(uint8_t id, const std::string& mac, SwarmRole role, float x, float y, float yaw);

    // Background Execution Task (Runs at 50Hz)
    void processSwarmTask();

private:
    EspNowEngine() = default;
    ~EspNowEngine() = default;

    void updateFollowerTracking();
    std::string macToString(const uint8_t mac[6]) const;
    bool stringToMac(const std::string& str, uint8_t mac[6]) const;

    EspNowStatus status;
    std::vector<EspNowPeer> peers;
    mutable std::mutex espnow_mutex;

    uint32_t task_ticks = 0;
    uint32_t last_remote_packet_time_ms = 0;

    // Leader pose tracking for follower kinematic compensation
    float leader_pose_x = 0.0f;
    float leader_pose_y = 0.0f;
    float leader_pose_yaw = 0.0f;
    uint32_t last_leader_heartbeat_ms = 0;
};

} // namespace TamimysticOS
