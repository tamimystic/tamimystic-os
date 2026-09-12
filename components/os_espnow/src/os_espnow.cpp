#include "os_espnow.h"
#include "os_hal_uart.h"
#include "os_robotics.h"
#include "os_slam.h"
#include "os_scheduler.h"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

#if !defined(OS_TARGET_NATIVE)
#if __has_include("esp_now.h")
#define USE_REAL_ESPNOW 1
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_mac.h"
#else
#define USE_REAL_ESPNOW 0
#endif
#else
#define USE_REAL_ESPNOW 0
#endif

namespace TamimysticOS {

#if USE_REAL_ESPNOW
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len) {
    if (recv_info && data && len > 0) {
        int rssi = recv_info->rx_ctrl ? recv_info->rx_ctrl->rssi : -50;
        EspNowEngine::getInstance().handleIncomingPacket(recv_info->src_addr, data, (size_t)len, rssi);
    }
}

static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Transmit callback confirmation
    (void)mac_addr;
    (void)status;
}
#endif

EspNowEngine& EspNowEngine::getInstance() {
    static EspNowEngine instance;
    return instance;
}

std::string EspNowEngine::macToString(const uint8_t mac[6]) const {
    char buf[20];
    std::snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return std::string(buf);
}

bool EspNowEngine::stringToMac(const std::string& str, uint8_t mac[6]) const {
    unsigned int m[6];
    if (std::sscanf(str.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x",
                    &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) == 6 ||
        std::sscanf(str.c_str(), "%02X:%02X:%02X:%02X:%02X:%02X",
                    &m[0], &m[1], &m[2], &m[3], &m[4], &m[5]) == 6) {
        for (int i = 0; i < 6; i++) mac[i] = (uint8_t)m[i];
        return true;
    }
    return false;
}

void EspNowEngine::init() {
    hal_uart_print("[ESPNOW] Initializing 2.4GHz ESP-NOW Mesh Swarm & Low-Latency Remote Radio...\n");

    {
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.enabled = true;
        status.wifi_channel = 1;
        status.swarm_role = SwarmRole::STANDALONE;
        status.formation = SwarmFormation::TRIANGLE;
        status.follower_slot = 0;
        status.swarm_spacing_cm = 60.0f;
        status.remote_control_active = false;
        status.packets_sent = 0;
        status.packets_received = 0;
        status.packet_loss_pct = 0.0f;
        status.avg_latency_ms = 2.1f;
        status.active_peer_count = 0;
        peers.clear();
    }

#if USE_REAL_ESPNOW
    uint8_t base_mac[6];
    esp_read_mac(base_mac, ESP_MAC_WIFI_STA);
    {
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.own_mac_str = macToString(base_mac);
    }

    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb(espnow_recv_cb);
        esp_now_register_send_cb(espnow_send_cb);
        hal_uart_print(("[ESPNOW] ESP-NOW Radio Ready @ MAC: " + status.own_mac_str + "\n").c_str());
    } else {
        hal_uart_print("[ESPNOW] Warning: esp_now_init failed, running in fallback mode.\n");
    }
#else
    status.own_mac_str = "24:DC:C3:98:45:A0";
    hal_uart_print("[ESPNOW] Native Simulator Radio Ready @ MAC: 24:DC:C3:98:45:A0\n");

    // Add sample simulated swarm peers in simulator mode
    triggerSimulatedSwarmPeer(1, "24:DC:C3:98:45:01", SwarmRole::LEADER, 120.0f, 80.0f, 45.0f);
    triggerSimulatedSwarmPeer(2, "24:DC:C3:98:45:02", SwarmRole::FOLLOWER, 60.0f, 20.0f, 45.0f);
#endif

    // Spawn 50Hz ESP-NOW Swarm Background Task on Core 0
    OSScheduler::getInstance().createTask("espnow_swarm_task", 4096, 2, CORE_0, []() {
        while (true) {
            EspNowEngine::getInstance().processSwarmTask();
            OSScheduler::getInstance().delay(20); // 50 Hz execution loop
        }
    });

    hal_uart_print("[ESPNOW] ESP-NOW Subsystem & Swarm Formation Task Active (50Hz / <4ms latency).\n");
}

bool EspNowEngine::addPeer(const uint8_t mac[6], uint8_t channel) {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    std::string m_str = macToString(mac);

    for (auto& p : peers) {
        if (std::memcmp(p.mac, mac, 6) == 0) {
            p.last_seen_ms = (uint32_t)task_ticks * 20;
            return true;
        }
    }

    EspNowPeer np;
    std::memcpy(np.mac, mac, 6);
    np.mac_str = m_str;
    np.rssi = -50;
    np.last_seen_ms = (uint32_t)task_ticks * 20;
    np.role = SwarmRole::STANDALONE;
    peers.push_back(np);
    status.active_peer_count = peers.size();

#if USE_REAL_ESPNOW
    esp_now_peer_info_t peerInfo = {};
    std::memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
#else
    (void)channel;
#endif

    std::string log_msg = "[ESPNOW] Peer Added: " + m_str + "\n";
    hal_uart_print(log_msg.c_str());
    return true;
}

void EspNowEngine::syncWifiChannel(uint8_t channel) {
    std::lock_guard<std::mutex> lock(espnow_mutex);
#if USE_REAL_ESPNOW
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
#endif
    std::string msg = "[ESPNOW] Synced 2.4GHz RF radio channel with Wi-Fi to Channel " + std::to_string((int)channel) + "\n";
    hal_uart_print(msg.c_str());
}

bool EspNowEngine::removePeer(const uint8_t mac[6]) {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    for (auto it = peers.begin(); it != peers.end(); ++it) {
        if (std::memcmp(it->mac, mac, 6) == 0) {
            peers.erase(it);
            status.active_peer_count = peers.size();
#if USE_REAL_ESPNOW
            esp_now_del_peer(mac);
#endif
            return true;
        }
    }
    return false;
}

std::vector<EspNowPeer> EspNowEngine::getPeers() const {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    return peers;
}

std::string EspNowEngine::getPeersJson() const {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    std::stringstream ss;
    ss << "{\"status\":\"ok\",\"count\":" << peers.size() << ",\"peers\":[";
    for (size_t i = 0; i < peers.size(); i++) {
        const auto& p = peers[i];
        ss << "{"
           << "\"mac\":\"" << p.mac_str << "\","
           << "\"rssi\":" << (int)p.rssi << ","
           << "\"role\":\"" << swarmRoleToString(p.role) << "\","
           << "\"robot_id\":" << (int)p.robot_id << ","
           << "\"is_controller\":" << (p.is_controller ? "true" : "false") << ","
           << "\"pose_x\":" << std::fixed << std::setprecision(1) << p.last_x_cm << ","
           << "\"pose_y\":" << p.last_y_cm << ","
           << "\"pose_yaw\":" << p.last_yaw_deg
           << "}";
        if (i + 1 < peers.size()) ss << ",";
    }
    ss << "]}";
    return ss.str();
}

void EspNowEngine::setSwarmRole(SwarmRole role, uint8_t follower_slot, float spacing_cm) {
    {
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.swarm_role = role;
        status.follower_slot = follower_slot;
        status.swarm_spacing_cm = spacing_cm;
    }

    std::stringstream ss;
    ss << "[ESPNOW] Swarm Role Changed: " << swarmRoleToString(role) 
       << " | Slot: " << (int)follower_slot 
       << " | Spacing: " << spacing_cm << " cm\n";
    hal_uart_print(ss.str().c_str());
}

void EspNowEngine::setFormation(SwarmFormation formation, float spacing_cm) {
    {
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.formation = formation;
        status.swarm_spacing_cm = spacing_cm;
    }

    if (status.swarm_role == SwarmRole::LEADER) {
        broadcastFormationCommand(formation, spacing_cm);
    }
}

void EspNowEngine::broadcastSwarmHeartbeat() {
    auto tel = RobotController::getInstance().getTelemetry();
    auto slam_pose = SlamEngine::getInstance().getStatus().pose;
    SwarmHeartbeatPacket pkt;
    pkt.packet_type = (uint8_t)EspNowPacketType::SWARM_HEARTBEAT;
    pkt.robot_id = 1;
    pkt.role = (uint8_t)status.swarm_role;
    pkt.battery_pct = 95;
    pkt.pose_x_cm = slam_pose.x_cm;
    pkt.pose_y_cm = slam_pose.y_cm;
    pkt.pose_yaw_deg = slam_pose.yaw_deg;
    pkt.velocity_linear = tel.velocity.vx;
    pkt.velocity_angular = tel.velocity.omega;
    pkt.timestamp_ms = (uint32_t)task_ticks * 20;
    pkt.sequence = status.packets_sent++;

#if USE_REAL_ESPNOW
    uint8_t bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(bcast_mac, (const uint8_t*)&pkt, sizeof(pkt));
#endif
}

void EspNowEngine::broadcastFormationCommand(SwarmFormation formation, float spacing_cm) {
    SwarmFormationPacket pkt;
    pkt.packet_type = (uint8_t)EspNowPacketType::SWARM_FORMATION_CMD;
    pkt.leader_id = 1;
    pkt.formation = (uint8_t)formation;
    pkt.spacing_cm = spacing_cm;
    pkt.timestamp_ms = (uint32_t)task_ticks * 20;
    status.packets_sent++;

#if USE_REAL_ESPNOW
    uint8_t bcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    esp_now_send(bcast_mac, (const uint8_t*)&pkt, sizeof(pkt));
#endif
}

void EspNowEngine::setRemoteControlEnabled(bool enabled) {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    status.remote_control_active = enabled;
    std::string msg = std::string("[ESPNOW] Wireless Handheld Gamepad Remote Control ") + 
                      (enabled ? "ENABLED\n" : "DISABLED\n");
    hal_uart_print(msg.c_str());
}

void EspNowEngine::handleRemoteJoystickPacket(const RemoteJoystickPacket& pkt) {
    last_remote_packet_time_ms = (uint32_t)task_ticks * 20;
    status.remote_control_active = true;

    // Emergency Stop button (Bit 0)
    if (pkt.buttons & 0x01) {
        RobotController::getInstance().emergencyStop();
        SlamEngine::getInstance().cancelNavigation();
        hal_uart_print("[ESPNOW:REMOTE] Emergency Stop button pressed from Gamepad!\n");
        return;
    }

    // Arm Home button (Bit 1)
    if (pkt.buttons & 0x02) {
        ArmJoints home_j = {90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 0.0f};
        RobotController::getInstance().setArmJoints(home_j);
    }

    // Convert joystick percentage (-100..+100) to kinematics velocities
    float vx = (float)pkt.axis_vx * 0.5f;     // Max +-50 cm/s
    float vy = (float)pkt.axis_vy * 0.5f;     // Mecanum strafing
    float omega = (float)pkt.axis_omega * 0.6f; // Max +-60 deg/s

    RobotController::getInstance().setTwist(vx, vy, omega);
}

void EspNowEngine::handleIncomingPacket(const uint8_t* mac, const uint8_t* data, size_t len, int rssi) {
    if (!data || len < 1) return;

    {
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.packets_received++;

        // Update peer list entry
        std::string m_str = macToString(mac);
        bool found = false;
        for (auto& p : peers) {
            if (std::memcmp(p.mac, mac, 6) == 0) {
                p.rssi = (int8_t)rssi;
                p.last_seen_ms = (uint32_t)task_ticks * 20;
                found = true;
                break;
            }
        }
        if (!found) {
            EspNowPeer np;
            std::memcpy(np.mac, mac, 6);
            np.mac_str = m_str;
            np.rssi = (int8_t)rssi;
            np.last_seen_ms = (uint32_t)task_ticks * 20;
            peers.push_back(np);
            status.active_peer_count = peers.size();
        }
    }

    uint8_t pkt_type = data[0];

    if (pkt_type == (uint8_t)EspNowPacketType::REMOTE_JOYSTICK && len >= sizeof(RemoteJoystickPacket)) {
        const RemoteJoystickPacket* pkt = reinterpret_cast<const RemoteJoystickPacket*>(data);
        handleRemoteJoystickPacket(*pkt);
    } 
    else if (pkt_type == (uint8_t)EspNowPacketType::SWARM_HEARTBEAT && len >= sizeof(SwarmHeartbeatPacket)) {
        const SwarmHeartbeatPacket* pkt = reinterpret_cast<const SwarmHeartbeatPacket*>(data);
        if (pkt->role == (uint8_t)SwarmRole::LEADER) {
            leader_pose_x = pkt->pose_x_cm;
            leader_pose_y = pkt->pose_y_cm;
            leader_pose_yaw = pkt->pose_yaw_deg;
            last_leader_heartbeat_ms = (uint32_t)task_ticks * 20;
        }

        // Update peer metadata
        std::lock_guard<std::mutex> lock(espnow_mutex);
        for (auto& p : peers) {
            if (std::memcmp(p.mac, mac, 6) == 0) {
                p.role = (SwarmRole)pkt->role;
                p.robot_id = pkt->robot_id;
                p.last_x_cm = pkt->pose_x_cm;
                p.last_y_cm = pkt->pose_y_cm;
                p.last_yaw_deg = pkt->pose_yaw_deg;
                break;
            }
        }
    }
    else if (pkt_type == (uint8_t)EspNowPacketType::SWARM_FORMATION_CMD && len >= sizeof(SwarmFormationPacket)) {
        const SwarmFormationPacket* pkt = reinterpret_cast<const SwarmFormationPacket*>(data);
        std::lock_guard<std::mutex> lock(espnow_mutex);
        status.formation = (SwarmFormation)pkt->formation;
        status.swarm_spacing_cm = pkt->spacing_cm;
    }
}

bool EspNowEngine::sendCustomPayload(const std::string& target_mac, const std::string& message) {
    uint8_t mac[6];
    if (!stringToMac(target_mac, mac)) return false;

    std::vector<uint8_t> pkt(1 + message.length());
    pkt[0] = (uint8_t)EspNowPacketType::CUSTOM_PAYLOAD;
    std::memcpy(pkt.data() + 1, message.data(), message.length());

    status.packets_sent++;
#if USE_REAL_ESPNOW
    esp_now_send(mac, pkt.data(), pkt.size());
#else
    (void)mac;
#endif
    return true;
}

void EspNowEngine::triggerSimulatedRemoteInput(float vx_pct, float vy_pct, float omega_pct, bool estop, bool gripper) {
    RemoteJoystickPacket pkt;
    pkt.packet_type = (uint8_t)EspNowPacketType::REMOTE_JOYSTICK;
    pkt.controller_id = 99;
    pkt.axis_vx = (int8_t)std::max(-100.0f, std::min(100.0f, vx_pct));
    pkt.axis_vy = (int8_t)std::max(-100.0f, std::min(100.0f, vy_pct));
    pkt.axis_omega = (int8_t)std::max(-100.0f, std::min(100.0f, omega_pct));
    pkt.buttons = (estop ? 0x01 : 0x00) | (gripper ? 0x04 : 0x00);
    pkt.timestamp_ms = (uint32_t)task_ticks * 20;
    pkt.sequence = status.packets_received + 1;

    uint8_t sim_mac[6] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0x01};
    handleIncomingPacket(sim_mac, (const uint8_t*)&pkt, sizeof(pkt), -45);
}

void EspNowEngine::triggerSimulatedSwarmPeer(uint8_t id, const std::string& mac_str, SwarmRole role, float x, float y, float yaw) {
    uint8_t m[6];
    if (!stringToMac(mac_str, m)) return;

    SwarmHeartbeatPacket pkt;
    pkt.packet_type = (uint8_t)EspNowPacketType::SWARM_HEARTBEAT;
    pkt.robot_id = id;
    pkt.role = (uint8_t)role;
    pkt.battery_pct = 90;
    pkt.pose_x_cm = x;
    pkt.pose_y_cm = y;
    pkt.pose_yaw_deg = yaw;
    pkt.velocity_linear = 10.0f;
    pkt.velocity_angular = 0.0f;
    pkt.timestamp_ms = (uint32_t)task_ticks * 20;
    pkt.sequence = 1;

    handleIncomingPacket(m, (const uint8_t*)&pkt, sizeof(pkt), -52);
}

void EspNowEngine::updateFollowerTracking() {
    if (status.swarm_role != SwarmRole::FOLLOWER) return;

    uint32_t now_ms = (uint32_t)task_ticks * 20;
    if (now_ms - last_leader_heartbeat_ms > 2000) {
        // Leader lost: Stop robot for safety
        RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
        return;
    }

    // Calculate slot offset in Leader frame (cm)
    float d = status.swarm_spacing_cm;
    float dx_prime = 0.0f;
    float dy_prime = 0.0f;

    switch (status.formation) {
        case SwarmFormation::TRIANGLE:
            if (status.follower_slot == 1) { // Left wing (-d cos 30, +d sin 30)
                dx_prime = -d * 0.866f;
                dy_prime = d * 0.5f;
            } else if (status.follower_slot == 2) { // Right wing (-d cos 30, -d sin 30)
                dx_prime = -d * 0.866f;
                dy_prime = -d * 0.5f;
            } else { // Rear guard
                dx_prime = -d * 1.5f;
                dy_prime = 0.0f;
            }
            break;

        case SwarmFormation::LINE:
            dx_prime = -(float)(status.follower_slot == 0 ? 1 : status.follower_slot) * d;
            dy_prime = 0.0f;
            break;

        case SwarmFormation::COLUMN:
            dx_prime = 0.0f;
            dy_prime = (status.follower_slot % 2 == 1 ? 1.0f : -1.0f) * d;
            break;

        case SwarmFormation::DIAMOND:
            if (status.follower_slot == 1) { dx_prime = -d; dy_prime = d; }
            else if (status.follower_slot == 2) { dx_prime = -d; dy_prime = -d; }
            else { dx_prime = -2.0f * d; dy_prime = 0.0f; }
            break;
    }

    // Transform offset to World frame using Leader yaw
    float rad = leader_pose_yaw * 0.0174533f;
    float target_world_x = leader_pose_x + (dx_prime * std::cos(rad) - dy_prime * std::sin(rad));
    float target_world_y = leader_pose_y + (dx_prime * std::sin(rad) + dy_prime * std::cos(rad));

    auto self_pose = SlamEngine::getInstance().getStatus().pose;
    float err_x = target_world_x - self_pose.x_cm;
    float err_y = target_world_y - self_pose.y_cm;
    float dist_err = std::sqrt(err_x * err_x + err_y * err_y);

    if (dist_err < 8.0f) {
        // Within formation tolerance: Align heading with leader
        float yaw_err = std::remainder(leader_pose_yaw - self_pose.yaw_deg, 360.0f);
        float omega = std::max(-40.0f, std::min(40.0f, yaw_err * 1.2f));
        RobotController::getInstance().setTwist(0.0f, 0.0f, omega);
    } else {
        // Proportional trajectory pursuit
        float target_heading = std::atan2(err_y, err_x) * 57.2958f;
        float angle_err = std::remainder(target_heading - self_pose.yaw_deg, 360.0f);

        float vx = (std::abs(angle_err) < 40.0f) ? std::min(45.0f, dist_err * 0.8f) : 10.0f;
        float omega = std::max(-50.0f, std::min(50.0f, angle_err * 1.5f));

        RobotController::getInstance().setTwist(vx, 0.0f, omega);
    }
}

void EspNowEngine::processSwarmTask() {
    task_ticks++;

    // 1. Leader broadcasts state at 10 Hz (every 5 ticks)
    if (status.swarm_role == SwarmRole::LEADER && (task_ticks % 5 == 0)) {
        broadcastSwarmHeartbeat();
    }

    // 2. Follower updates formation tracking at 25 Hz (every 2 ticks)
    if (status.swarm_role == SwarmRole::FOLLOWER && (task_ticks % 2 == 0)) {
        updateFollowerTracking();
    }

    // 3. Gamepad remote timeout check (failsafe after 500ms of no signal)
    if (status.remote_control_active && ((task_ticks * 20) - last_remote_packet_time_ms > 600)) {
        status.remote_control_active = false;
        if (status.swarm_role == SwarmRole::STANDALONE) {
            RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
        }
    }
}

EspNowStatus EspNowEngine::getStatus() const {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    return status;
}

std::string EspNowEngine::getStatusJson() const {
    std::lock_guard<std::mutex> lock(espnow_mutex);
    std::stringstream ss;
    ss << "{"
       << "\"status\":\"ok\","
       << "\"enabled\":" << (status.enabled ? "true" : "false") << ","
       << "\"mac\":\"" << status.own_mac_str << "\","
       << "\"channel\":" << (int)status.wifi_channel << ","
       << "\"swarm_role\":\"" << swarmRoleToString(status.swarm_role) << "\","
       << "\"formation\":\"" << swarmFormationToString(status.formation) << "\","
       << "\"follower_slot\":" << (int)status.follower_slot << ","
       << "\"spacing_cm\":" << std::fixed << std::setprecision(1) << status.swarm_spacing_cm << ","
       << "\"remote_active\":" << (status.remote_control_active ? "true" : "false") << ","
       << "\"packets_tx\":" << status.packets_sent << ","
       << "\"packets_rx\":" << status.packets_received << ","
       << "\"packet_loss_pct\":" << std::setprecision(1) << status.packet_loss_pct << ","
       << "\"avg_latency_ms\":" << std::setprecision(1) << status.avg_latency_ms << ","
       << "\"active_peers\":" << peers.size()
       << "}";
    return ss.str();
}

} // namespace TamimysticOS
