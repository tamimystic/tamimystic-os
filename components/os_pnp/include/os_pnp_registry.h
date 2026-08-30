#pragma once

#include "os_pnp_types.h"
#include <vector>
#include <functional>

namespace TamimysticOS {

struct DeviceSignature {
    uint8_t address;
    uint8_t who_am_i_reg;
    uint8_t expected_id;
    bool check_reg;       // If false, match by address only (e.g. simple OLED or PCF8574)
    std::string name;
    std::string model;
    DeviceCategory category;
    std::string driver_name;
};

class PnPRegistry {
public:
    static PnPRegistry& getInstance();

    // Initialize the signature database
    void init();

    // Probe an I2C address and try to identify the device
    bool probeAndIdentify(uint8_t address, DiscoveredDevice& out_dev);

    // Get all known signatures in database
    const std::vector<DeviceSignature>& getSignatures() const;

    // Read live telemetry sample from identified device
    std::string sampleDeviceData(const DiscoveredDevice& dev);

private:
    PnPRegistry() = default;
    ~PnPRegistry() = default;

    std::vector<DeviceSignature> signatures;
};

} // namespace TamimysticOS
