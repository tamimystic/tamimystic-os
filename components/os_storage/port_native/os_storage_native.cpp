#include "os_storage.h"
#include "os_hal_uart.h"
#include <fstream>
#include <iostream>
#include <filesystem>

// For native simulation, we mock the VFS by storing files in a local folder
#define NATIVE_VFS_ROOT "I:/tamimystic-os/fs_root"

namespace TamimysticOS {

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

void StorageManager::init() {
    hal_uart_print("[STORAGE] Initializing Native Mock VFS...\n");
    // Ensure root exists
    std::filesystem::create_directories(NATIVE_VFS_ROOT);
}

bool StorageManager::readFile(const std::string& path, std::vector<uint8_t>& out_buffer) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    
    std::ifstream file(full_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        hal_uart_print(("[STORAGE] Failed to open file: " + full_path + "\n").c_str());
        return false;
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    out_buffer.resize(size);
    if (file.read(reinterpret_cast<char*>(out_buffer.data()), size)) {
        return true;
    }
    return false;
}

bool StorageManager::writeFile(const std::string& path, const uint8_t* data, size_t size) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    
    std::ofstream file(full_path, std::ios::binary);
    if (!file.is_open()) {
        hal_uart_print(("[STORAGE] Failed to write file: " + full_path + "\n").c_str());
        return false;
    }
    
    file.write(reinterpret_cast<const char*>(data), size);
    hal_uart_print(("[STORAGE] Wrote " + std::to_string(size) + " bytes to " + path + "\n").c_str());
    return true;
}

} // namespace TamimysticOS
