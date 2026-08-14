#include "os_storage.h"
#include "os_hal_uart.h"

// ESP-IDF VFS and SPIFFS headers
#include "esp_spiffs.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>

namespace TamimysticOS {

StorageManager& StorageManager::getInstance() {
    static StorageManager instance;
    return instance;
}

void StorageManager::init() {
    hal_uart_print("[STORAGE] Initializing SPIFFS Virtual File System...\n");
    
    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            hal_uart_print("[STORAGE] Failed to mount or format filesystem\n");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            hal_uart_print("[STORAGE] Failed to find SPIFFS partition\n");
        } else {
            hal_uart_print("[STORAGE] Failed to initialize SPIFFS\n");
        }
        return;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        hal_uart_print("[STORAGE] Failed to get SPIFFS partition information\n");
    } else {
        hal_uart_print(("[STORAGE] Partition size: total: " + std::to_string(total) + ", used: " + std::to_string(used) + "\n").c_str());
    }
}

bool StorageManager::readFile(const std::string& path, std::vector<uint8_t>& out_buffer) {
    std::string full_path = "/spiffs/" + path;
    
    FILE* f = fopen(full_path.c_str(), "rb");
    if (f == NULL) {
        hal_uart_print(("[STORAGE] Failed to open file for reading: " + full_path + "\n").c_str());
        return false;
    }
    
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    out_buffer.resize(size);
    size_t read_bytes = fread(out_buffer.data(), 1, size, f);
    fclose(f);
    
    return read_bytes == size;
}

bool StorageManager::writeFile(const std::string& path, const uint8_t* data, size_t size) {
    std::string full_path = "/spiffs/" + path;
    
    FILE* f = fopen(full_path.c_str(), "wb");
    if (f == NULL) {
        hal_uart_print(("[STORAGE] Failed to open file for writing: " + full_path + "\n").c_str());
        return false;
    }
    
    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    
    if (written == size) {
        hal_uart_print(("[STORAGE] Wrote " + std::to_string(size) + " bytes to " + path + "\n").c_str());
        return true;
    }
    return false;
}

} // namespace TamimysticOS
