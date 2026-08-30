#include "os_storage.h"
#include "os_hal_uart.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

#define NATIVE_VFS_ROOT "I:/tamimystic-os/fs_root"
#define TOTAL_FLASH_STORAGE_BYTES (7 * 1024 * 1024) // 7.0 MB (6.8MB LittleFS)

namespace TamimysticOS {

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

void StorageManager::init() {
    hal_uart_print("[STORAGE] Initializing Native Mock LittleFS VFS (6.8MB Partition)...\n");
    std::filesystem::create_directories(NATIVE_VFS_ROOT);

    // Create sample autorun and demo scripts if they don't exist
    std::string sample_script = 
        "# Tamimystic OS Autonomous Robot Script\n"
        "import tamimystic\n"
        "\n"
        "print('Starting autonomous patrol routine...')\n"
        "dist = tamimystic.sensor.read_distance()\n"
        "print('Initial obstacle distance:', dist, 'cm')\n"
        "\n"
        "if dist > 20.0:\n"
        "    print('Path clear, driving forward...')\n"
        "    tamimystic.robot.move(40, 0)\n"
        "else:\n"
        "    print('Obstacle detected! Turning...')\n"
        "    tamimystic.robot.move(0, 30)\n"
        "\n"
        "print('Script finished successfully.')\n";

    if (!fileExists("robot_patrol.py")) {
        writeFile("robot_patrol.py", reinterpret_cast<const uint8_t*>(sample_script.c_str()), sample_script.length());
    }

    std::string sample_arm_script =
        "# Tamimystic OS Robotic Arm IK Pick & Place\n"
        "import tamimystic\n"
        "\n"
        "print('Homing robotic arm...')\n"
        "tamimystic.robot.arm(90, 90, 90, 90, 90, 0)\n"
        "print('Moving to Cartesian target (14.0, 4.0, 10.0 cm)...')\n"
        "ok = tamimystic.robot.ik(14.0, 4.0, 10.0)\n"
        "print('IK Solved & Reached:', ok)\n";

    if (!fileExists("arm_demo.py")) {
        writeFile("arm_demo.py", reinterpret_cast<const uint8_t*>(sample_arm_script.c_str()), sample_arm_script.length());
    }

    hal_uart_print("[STORAGE] LittleFS Mount Ready (6.8MB Flash VFS).\n");
}

bool StorageManager::readFile(const std::string& path, std::vector<uint8_t>& out_buffer) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    std::ifstream file(full_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) return false;
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    out_buffer.resize(size);
    return (bool)file.read(reinterpret_cast<char*>(out_buffer.data()), size);
}

bool StorageManager::writeFile(const std::string& path, const uint8_t* data, size_t size) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    std::ofstream file(full_path, std::ios::binary);
    if (!file.is_open()) return false;
    file.write(reinterpret_cast<const char*>(data), size);
    return true;
}

bool StorageManager::deleteFile(const std::string& path) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    try {
        return std::filesystem::remove(full_path);
    } catch (...) {
        return false;
    }
}

bool StorageManager::fileExists(const std::string& path) {
    std::string full_path = std::string(NATIVE_VFS_ROOT) + "/" + path;
    return std::filesystem::exists(full_path);
}

std::vector<FileEntry> StorageManager::listFiles() {
    std::vector<FileEntry> entries;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(NATIVE_VFS_ROOT)) {
            if (entry.is_regular_file()) {
                FileEntry fe;
                fe.name = entry.path().filename().string();
                fe.size = (size_t)entry.file_size();
                entries.push_back(fe);
            }
        }
    } catch (...) {}
    return entries;
}

StorageStats StorageManager::getStats() {
    StorageStats stats;
    stats.total_bytes = TOTAL_FLASH_STORAGE_BYTES;
    stats.used_bytes = 0;
    for (const auto& f : listFiles()) {
        stats.used_bytes += f.size;
    }
    stats.free_bytes = (stats.total_bytes > stats.used_bytes) ? (stats.total_bytes - stats.used_bytes) : 0;
    return stats;
}

std::string StorageManager::getFilesJson() {
    auto files = listFiles();
    auto stats = getStats();

    std::stringstream ss;
    ss << "{"
       << "\"status\":\"ok\","
       << "\"total_bytes\":" << stats.total_bytes << ","
       << "\"used_bytes\":" << stats.used_bytes << ","
       << "\"free_bytes\":" << stats.free_bytes << ","
       << "\"files\":[";

    for (size_t i = 0; i < files.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"name\":\"" << files[i].name << "\",\"size\":" << files[i].size << "}";
    }
    ss << "]}";
    return ss.str();
}

} // namespace TamimysticOS
