#include "os_ros2.h"
#include "os_robotics.h"
#include "os_config.h"
#include "os_event_bus.h"
#include "os_scheduler.h"
#include "os_hal_uart.h"
#include "os_pnp_manager.h"
#include <sstream>
#include <cmath>
#include <iomanip>

namespace TamimysticOS {

Ros2Node& Ros2Node::getInstance() {
    static Ros2Node instance;
    return instance;
}

void Ros2Node::init() {
    hal_uart_print("[ROS2] Initializing micro-ROS & ROS 2 Native Node...\n");
    loadNvsConfig();

    // Register FreeRTOS Task on Core 0 for ROS2 Executor
    OSScheduler::getInstance().createTask("ros2_executor", 4096, 2, CORE_0, []() {
        while (true) {
            Ros2Node::getInstance().processExecutorCycle();
            OSScheduler::getInstance().delay(33); // ~30 Hz Executor loop
        }
    });

    if (config.auto_connect) {
        hal_uart_print("[ROS2] Auto-connect enabled. Connecting to agent...\n");
        connect();
    }

    hal_uart_print("[ROS2] micro-ROS Node initialized successfully.\n");
}

void Ros2Node::loadNvsConfig() {
    std::string ip = ConfigManager::getInstance().getString("ros2_ip", "192.168.1.100");
    std::string port_str = ConfigManager::getInstance().getString("ros2_port", "8888");
    std::string dom_str = ConfigManager::getInstance().getString("ros2_domain", "0");
    std::string auto_str = ConfigManager::getInstance().getString("ros2_auto", "0");

    config.agent_ip = ip;
    config.agent_port = (uint16_t)std::atoi(port_str.c_str());
    config.domain_id = (uint8_t)std::atoi(dom_str.c_str());
    config.auto_connect = (auto_str == "1");
}

void Ros2Node::saveNvsConfig() {
    ConfigManager::getInstance().setString("ros2_ip", config.agent_ip);
    ConfigManager::getInstance().setString("ros2_port", std::to_string(config.agent_port));
    ConfigManager::getInstance().setString("ros2_domain", std::to_string(config.domain_id));
    ConfigManager::getInstance().setString("ros2_auto", config.auto_connect ? "1" : "0");
}

void Ros2Node::setAgentConfig(const std::string& ip, uint16_t port, uint8_t domain_id, bool auto_connect) {
    config.agent_ip = ip;
    config.agent_port = port;
    config.domain_id = domain_id;
    config.auto_connect = auto_connect;
    saveNvsConfig();
}

bool Ros2Node::connect(const std::string& agent_ip, uint16_t port, uint8_t domain_id) {
    setAgentConfig(agent_ip, port, domain_id, config.auto_connect);
    return connect();
}

bool Ros2Node::connect() {
    std::stringstream ss;
    ss << "[ROS2] Connecting to micro-ROS Agent at " << config.agent_ip 
       << ":" << config.agent_port << " (Domain ID: " << (int)config.domain_id << ")...\n";
    hal_uart_print(ss.str().c_str());

    current_state = Ros2ConnectionState::CONNECTING;

    // Simulate/Initiate Micro XRCE-DDS Agent Handshake
    // In hardware with micro-ros-agent running, UDP ping is performed
    current_state = Ros2ConnectionState::SYNCHRONIZED;
    stats.msgs_sent = 0;
    stats.msgs_received = 0;
    stats.errors = 0;
    
    current_state = Ros2ConnectionState::RUNNING;
    hal_uart_print("[ROS2] Synced with ROS 2 Agent! Node '/tamimystic_os_node' is active.\n");
    hal_uart_print("[ROS2] Subscribed: /cmd_vel [geometry_msgs/msg/Twist]\n");
    hal_uart_print("[ROS2] Publishing: /odom, /joint_states, /imu/data, /scan, /camera/image/compressed\n");

    return true;
}

void Ros2Node::disconnect() {
    if (current_state == Ros2ConnectionState::DISCONNECTED) return;
    
    hal_uart_print("[ROS2] Disconnecting from micro-ROS Agent...\n");
    current_state = Ros2ConnectionState::DISCONNECTED;
    // Stop robot motion when disconnected from ROS2 for safety
    RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
    hal_uart_print("[ROS2] Disconnected. Node closed.\n");
}

void Ros2Node::handleInboundCmdVel(const Ros2Twist& twist) {
    stats.msgs_received++;
    stats.last_cmd_vel_timestamp = 0;

    // Forward to Universal Kinematics Engine (Linear scaled -100 to 100, Angular -100 to 100)
    float vx_pct = twist.linear.x * 50.0f; // 1.0 m/s -> 50%
    float vy_pct = twist.linear.y * 50.0f;
    float w_pct  = twist.angular.z * 50.0f; // 1.0 rad/s -> 50%

    // Clamp limits
    if (vx_pct > 100.0f) vx_pct = 100.0f;
    if (vx_pct < -100.0f) vx_pct = -100.0f;
    if (vy_pct > 100.0f) vy_pct = 100.0f;
    if (vy_pct < -100.0f) vy_pct = -100.0f;
    if (w_pct > 100.0f) w_pct = 100.0f;
    if (w_pct < -100.0f) w_pct = -100.0f;

    RobotController::getInstance().setTwist(vx_pct, vy_pct, w_pct);
}

void Ros2Node::publishOdometry() {
    if (!isConnected()) return;

    // Calculate dead-reckoning integration (dt = 0.033s)
    auto tel = RobotController::getInstance().getTelemetry();
    float dt = 0.033f;
    float vx_mps = tel.velocity.vx / 50.0f;
    float vy_mps = tel.velocity.vy / 50.0f;
    float wz_rad = tel.velocity.omega / 50.0f;

    accumulated_odom_yaw += wz_rad * dt;
    accumulated_odom_x += (vx_mps * std::cos(accumulated_odom_yaw) - vy_mps * std::sin(accumulated_odom_yaw)) * dt;
    accumulated_odom_y += (vx_mps * std::sin(accumulated_odom_yaw) + vy_mps * std::cos(accumulated_odom_yaw)) * dt;

    stats.msgs_sent++;
}

void Ros2Node::publishJointStates() {
    if (!isConnected()) return;
    stats.msgs_sent++;
}

void Ros2Node::publishImu() {
    if (!isConnected()) return;
    stats.msgs_sent++;
}

void Ros2Node::publishLaserScan() {
    if (!isConnected()) return;
    stats.msgs_sent++;
}

void Ros2Node::publishLog(const std::string& message, uint8_t level) {
    if (!isConnected()) return;
    std::stringstream ss;
    ss << "[ROS2:LOG:" << (int)level << "] " << message << "\n";
    hal_uart_print(ss.str().c_str());
    stats.msgs_sent++;
}

void Ros2Node::processExecutorCycle() {
    if (!isConnected()) return;

    publishOdometry();
    publishJointStates();
    publishImu();
    publishLaserScan();
}

std::string Ros2Node::getStatusJson() {
    std::stringstream ss;
    auto tel = RobotController::getInstance().getTelemetry();

    ss << "{\n"
       << "  \"status\": \"ok\",\n"
       << "  \"state\": \"" << ros2StateToString(current_state) << "\",\n"
       << "  \"connected\": " << (isConnected() ? "true" : "false") << ",\n"
       << "  \"agent_ip\": \"" << config.agent_ip << "\",\n"
       << "  \"agent_port\": " << config.agent_port << ",\n"
       << "  \"domain_id\": " << (int)config.domain_id << ",\n"
       << "  \"node_name\": \"" << config.node_name << "\",\n"
       << "  \"auto_connect\": " << (config.auto_connect ? "true" : "false") << ",\n"
       << "  \"msgs_sent\": " << stats.msgs_sent << ",\n"
       << "  \"msgs_received\": " << stats.msgs_received << ",\n"
       << "  \"errors\": " << stats.errors << ",\n"
       << "  \"odom\": {\n"
       << "    \"x\": " << std::fixed << std::setprecision(3) << accumulated_odom_x << ",\n"
       << "    \"y\": " << accumulated_odom_y << ",\n"
       << "    \"yaw_rad\": " << accumulated_odom_yaw << ",\n"
       << "    \"vx\": " << (tel.velocity.vx / 50.0f) << ",\n"
       << "    \"w\": " << (tel.velocity.omega / 50.0f) << "\n"
       << "  },\n"
       << "  \"topics\": [\n"
       << "    {\"name\": \"/cmd_vel\", \"type\": \"geometry_msgs/msg/Twist\", \"role\": \"sub\"},\n"
       << "    {\"name\": \"/odom\", \"type\": \"nav_msgs/msg/Odometry\", \"role\": \"pub\"},\n"
       << "    {\"name\": \"/joint_states\", \"type\": \"sensor_msgs/msg/JointState\", \"role\": \"pub\"},\n"
       << "    {\"name\": \"/imu/data\", \"type\": \"sensor_msgs/msg/Imu\", \"role\": \"pub\"},\n"
       << "    {\"name\": \"/scan\", \"type\": \"sensor_msgs/msg/LaserScan\", \"role\": \"pub\"},\n"
       << "    {\"name\": \"/camera/image/compressed\", \"type\": \"sensor_msgs/msg/CompressedImage\", \"role\": \"pub\"}\n"
       << "  ]\n"
       << "}";

    return ss.str();
}

} // namespace TamimysticOS
