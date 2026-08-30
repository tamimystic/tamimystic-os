#include "os_storage.h"
#include "os_hal_uart.h"
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sstream>

namespace TamimysticOS {

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

void StorageManager::init() {
    hal_uart_print("[STORAGE] Initializing 6.8MB LittleFS/SPIFFS Partition for ESP32-S3...\n");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/storage",
      .partition_label = "storage",
      .max_files = 16,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        // Fallback to default partition label if named partition not yet flashed
        conf.partition_label = NULL;
        ret = esp_vfs_spiffs_register(&conf);
    }

    if (ret != ESP_OK) {
        hal_uart_print("[STORAGE] Failed to mount storage partition!\n");
        return;
    }
    
    size_t total = 0, used = 0;
    esp_spiffs_info(conf.partition_label, &total, &used);
    hal_uart_print(("[STORAGE] Flash VFS Mounted: Total: " + std::to_string(total / 1024) + " KB, Used: " + std::to_string(used / 1024) + " KB\n").c_str());
}

bool StorageManager::readFile(const std::string& path, std::vector<uint8_t>& out_buffer) {
    std::string full_path = "/storage/" + path;
    FILE* f = fopen(full_path.c_str(), "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    out_buffer.resize(size);
    size_t read_bytes = fread(out_buffer.data(), 1, size, f);
    fclose(f);
    return read_bytes == size;
}

bool StorageManager::writeFile(const std::string& path, const uint8_t* data, size_t size) {
    std::string full_path = "/storage/" + path;
    FILE* f = fopen(full_path.c_str(), "wb");
    if (!f) return false;
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    return written == size;
}

bool StorageManager::deleteFile(const std::string& path) {
    std::string full_path = "/storage/" + path;
    return (remove(full_path.c_str()) == 0);
}

bool StorageManager::fileExists(const std::string& path) {
    std::string full_path = "/storage/" + path;
    struct stat st;
    return (stat(full_path.c_str(), &st) == 0);
}

std::vector<FileEntry> StorageManager::listFiles() {
    std::vector<FileEntry> entries;
    DIR* dir = opendir("/storage");
    if (!dir) return entries;

    struct dirent* de;
    while ((de = readdir(dir)) != NULL) {
        if (de->d_type == DT_REG) {
            std::string fname = de->d_name;
            std::string full_path = "/storage/" + fname;
            struct stat st;
            size_t sz = 0;
            if (stat(full_path.c_str(), &st) == 0) {
                sz = st.st_size;
            }
            entries.push_back({fname, sz});
        }
    }
    closedir(dir);
    return entries;
}

StorageStats StorageManager::getStats() {
    StorageStats stats;
    size_t total = 0, used = 0;
    esp_spiffs_info("storage", &total, &used);
    stats.total_bytes = total;
    stats.used_bytes = used;
    stats.free_bytes = (total > used) ? (total - used) : 0;
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
