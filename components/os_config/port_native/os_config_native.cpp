#include "os_config.h"
#include "os_hal_uart.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

#define NATIVE_CONFIG_FILE "I:/tamimystic-os/fs_root/system_config.txt"

namespace TamimysticOS {

static std::map<std::string, std::string> mock_nvs;

static void loadMockNVS() {
    mock_nvs.clear();
    std::ifstream file(NATIVE_CONFIG_FILE);
    if (!file.is_open()) return;
    
    std::string line;
    while (std::getline(file, line)) {
        auto delimiterPos = line.find("=");
        if (delimiterPos == std::string::npos) continue;
        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);
        mock_nvs[key] = value;
    }
}

static void saveMockNVS() {
    std::ofstream file(NATIVE_CONFIG_FILE);
    if (!file.is_open()) return;
    
    for (const auto& pair : mock_nvs) {
        file << pair.first << "=" << pair.second << "\n";
    }
}

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::init() {
    hal_uart_print("[CONFIG] Initializing Native Mock NVS...\n");
    loadMockNVS();
}

bool ConfigManager::setString(const std::string& key, const std::string& value) {
    mock_nvs[key] = value;
    saveMockNVS();
    hal_uart_print(("[CONFIG] Saved string: " + key + "=" + value + "\n").c_str());
    return true;
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) {
    if (mock_nvs.find(key) != mock_nvs.end()) {
        return mock_nvs[key];
    }
    return default_value;
}

bool ConfigManager::setInt(const std::string& key, int value) {
    mock_nvs[key] = std::to_string(value);
    saveMockNVS();
    hal_uart_print(("[CONFIG] Saved int: " + key + "=" + std::to_string(value) + "\n").c_str());
    return true;
}

int ConfigManager::getInt(const std::string& key, int default_value) {
    if (mock_nvs.find(key) != mock_nvs.end()) {
        try {
            return std::stoi(mock_nvs[key]);
        } catch (...) {
            return default_value;
        }
    }
    return default_value;
}

} // namespace TamimysticOS
