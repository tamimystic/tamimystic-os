#pragma once

#include "os_pnp_types.h"
#include <vector>
#include <string>

namespace TamimysticOS {

class PnPManager {
public:
    static PnPManager& getInstance();

    // Initialize PnP subsystem and run initial hardware discovery
    void init();

    // Trigger a scan on the configured I2C bus
    void scanI2CBus();

    // Get list of discovered devices
    std::vector<DiscoveredDevice> getDiscoveredDevices() const;

    // Generate JSON response for Web Dashboard and REST API
    std::string getDevicesJson();

    // Get total count of active devices
    size_t getDeviceCount() const;

private:
    PnPManager() = default;
    ~PnPManager() = default;

    std::vector<DiscoveredDevice> discovered_devices;
};

} // namespace TamimysticOS
