#pragma once

#include "os_pnp_types.h"
#include <unordered_map>
#include <string>
#include <vector>

namespace TamimysticOS {

struct PinMappingItem {
    PinFunction function;
    std::string func_name;
    std::string label;
    int gpio_pin;
    bool is_safe;
    std::string description;
};

class PinMatrixManager {
public:
    static PinMatrixManager& getInstance();

    // Initialize the Pin Matrix and load configurations from NVS
    void init();

    // Re-assign a pin dynamically and persist to NVS
    bool setPin(const std::string& func_name, int gpio_num);
    bool setPin(PinFunction func, int gpio_num);

    // Get assigned pin for a function
    int getPin(PinFunction func) const;
    int getPin(const std::string& func_name) const;

    // Retrieve all active mappings
    std::vector<PinMappingItem> getAllMappings() const;

    // Check if a GPIO is safe for user configuration
    bool isSafePin(int pin) const;

    // Generate JSON for Web UI
    std::string getPinMatrixJson() const;

    // Reset pin matrix to factory defaults
    void resetToDefaults();

private:
    PinMatrixManager() = default;
    ~PinMatrixManager() = default;

    void loadFromNVS();
    void saveToNVS(const std::string& func_name, int gpio_num);

    std::unordered_map<PinFunction, int> pin_map;
    std::unordered_map<std::string, PinFunction> name_to_func;
};

} // namespace TamimysticOS
