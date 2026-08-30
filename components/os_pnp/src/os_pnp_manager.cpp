#include "os_pnp_manager.h"
#include "os_pnp_registry.h"
#include "os_pin_matrix.h"
#include "os_hal_i2c.h"
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include <sstream>
#include <iomanip>

namespace TamimysticOS {

PnPManager& PnPManager::getInstance() {
    static PnPManager instance;
    return instance;
}

void PnPManager::init() {
    hal_uart_print("[PNP] Initializing Plug & Play Hardware Engine...\n");

    // Initialize Registry & Pin Matrix
    PnPRegistry::getInstance().init();
    PinMatrixManager::getInstance().init();

    // Get I2C Pins from Pin Matrix
    int sda = PinMatrixManager::getInstance().getPin(PinFunction::I2C_SDA);
    int scl = PinMatrixManager::getInstance().getPin(PinFunction::I2C_SCL);

    hal_i2c_master_init(sda, scl, 400000); // 400kHz Fast I2C

    // Run initial bus scan
    scanI2CBus();

    // Listen for Pin Matrix updates to re-init I2C bus if SDA/SCL change
    EventBus::getInstance().subscribe(EventTopic::PIN_CONFIG_CHANGED, [this](const SystemEvent& evt) {
        int new_sda = PinMatrixManager::getInstance().getPin(PinFunction::I2C_SDA);
        int new_scl = PinMatrixManager::getInstance().getPin(PinFunction::I2C_SCL);
        hal_uart_print("[PNP] Re-initializing I2C bus with updated pin configuration...\n");
        hal_i2c_deinit();
        hal_i2c_master_init(new_sda, new_scl, 400000);
        this->scanI2CBus();
    });

    hal_uart_print("[PNP] Ready.\n");
}

void PnPManager::scanI2CBus() {
    hal_uart_print("\n=======================================================\n");
    hal_uart_print("  [PNP] Scanning I2C Bus (Addresses 0x08 - 0x77)...\n");
    hal_uart_print("=======================================================\n");

    discovered_devices.clear();

    for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
        DiscoveredDevice dev;
        if (PnPRegistry::getInstance().probeAndIdentify(addr, dev)) {
            discovered_devices.push_back(dev);

            std::stringstream ss;
            ss << "  [+ FOUND] 0x" << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << (int)addr
               << " | " << dev.name << " [" << deviceCategoryToString(dev.category) << "] "
               << "-> Driver: " << dev.driver_name << "\n";
            hal_uart_print(ss.str().c_str());

            // Fire event
            EventBus::getInstance().publish(EventTopic::DEVICE_CONNECTED, &dev, sizeof(dev));
        }
    }

    if (discovered_devices.empty()) {
        hal_uart_print("  [INFO] No external I2C devices detected on bus.\n");
    } else {
        std::stringstream ss;
        ss << "  [OK] Total " << discovered_devices.size() << " device(s) auto-detected & drivers loaded.\n";
        hal_uart_print(ss.str().c_str());
    }
    hal_uart_print("=======================================================\n\n");
}

std::vector<DiscoveredDevice> PnPManager::getDiscoveredDevices() const {
    return discovered_devices;
}

size_t PnPManager::getDeviceCount() const {
    return discovered_devices.size();
}

std::string PnPManager::getDevicesJson() {
    std::stringstream ss;
    ss << "{\"status\":\"ok\",\"count\":" << discovered_devices.size() << ",\"devices\":[";
    
    for (size_t i = 0; i < discovered_devices.size(); ++i) {
        const auto& dev = discovered_devices[i];
        // Sample fresh live reading
        std::string reading = PnPRegistry::getInstance().sampleDeviceData(dev);

        if (i > 0) ss << ",";
        ss << "{"
           << "\"address\":\"0x" << std::hex << std::uppercase << (int)dev.address << "\","
           << "\"name\":\"" << dev.name << "\","
           << "\"model\":\"" << dev.model << "\","
           << "\"category\":\"" << deviceCategoryToString(dev.category) << "\","
           << "\"icon\":\"" << deviceCategoryToIcon(dev.category) << "\","
           << "\"driver\":\"" << dev.driver_name << "\","
           << "\"active\":" << (dev.active ? "true" : "false") << ","
           << "\"reading\":\"" << reading << "\""
           << "}";
    }
    ss << "]}";
    return ss.str();
}

} // namespace TamimysticOS
