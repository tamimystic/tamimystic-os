#pragma once

#include "os_ble_types.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>

namespace TamimysticOS {

class BleManager {
public:
    static BleManager& getInstance();

    // Initialize BLE 5.0 Stack & NimBLE GATT Server
    void init(const BleConfig& config = BleConfig());

    // Advertising control
    bool startAdvertising();
    bool stopAdvertising();

    // Disconnect active peer
    void disconnect();

    // Query state
    bool isConnected() const;
    bool isAdvertising() const;
    BleStatus getStatus() const;
    std::string getBleJson() const;

    // Outbound notifications to connected Web Bluetooth / mobile clients
    bool notifyTelemetry(const BleTelemetryPacket& telemetry);
    bool notifySensors(const BleSensorPacket& sensors);

    // Callbacks for inbound commands
    using TwistCallback = std::function<void(float vx, float vy, float omega, uint8_t buttons)>;
    using ArmCallback = std::function<void(const std::vector<float>& joints, uint8_t gripper)>;
    using EstopCallback = std::function<void(bool active)>;

    void setTwistCallback(TwistCallback cb);
    void setArmCallback(ArmCallback cb);
    void setEstopCallback(EstopCallback cb);

    // Process inbound raw characteristic writes
    void processInboundTwist(const uint8_t* data, size_t len);
    void processInboundArm(const uint8_t* data, size_t len);
    void processInboundCmd(const uint8_t* data, size_t len);

    // Connection event handlers
    void onClientConnect(const std::string& peer_addr);
    void onClientDisconnect();

private:
    BleManager() = default;
    ~BleManager() = default;
    BleManager(const BleManager&) = delete;
    BleManager& operator=(const BleManager&) = delete;

    void backgroundNotifyTask();

    BleConfig config_;
    BleStatus status_;
    TwistCallback twist_cb_;
    ArmCallback arm_cb_;
    EstopCallback estop_cb_;
    bool initialized_ = false;
};

} // namespace TamimysticOS
