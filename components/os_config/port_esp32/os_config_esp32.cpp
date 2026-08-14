#include "os_config.h"
#include "os_hal_uart.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

namespace TamimysticOS {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::init() {
    hal_uart_print("[CONFIG] Initializing NVS (Non-Volatile Storage)...\n");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    hal_uart_print("[CONFIG] NVS initialized successfully.\n");
}

bool ConfigManager::setString(const std::string& key, const std::string& value) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    err = nvs_set_str(my_handle, key.c_str(), value.c_str());
    nvs_commit(my_handle);
    nvs_close(my_handle);
    return err == ESP_OK;
}

std::string ConfigManager::getString(const std::string& key, const std::string& default_value) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return default_value;

    size_t required_size;
    err = nvs_get_str(my_handle, key.c_str(), NULL, &required_size);
    if (err != ESP_OK) {
        nvs_close(my_handle);
        return default_value;
    }

    char* server_name = (char*)malloc(required_size);
    nvs_get_str(my_handle, key.c_str(), server_name, &required_size);
    std::string result(server_name);
    free(server_name);
    nvs_close(my_handle);
    
    return result;
}

bool ConfigManager::setInt(const std::string& key, int value) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return false;

    err = nvs_set_i32(my_handle, key.c_str(), value);
    nvs_commit(my_handle);
    nvs_close(my_handle);
    return err == ESP_OK;
}

int ConfigManager::getInt(const std::string& key, int default_value) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) return default_value;

    int32_t out_value = 0;
    err = nvs_get_i32(my_handle, key.c_str(), &out_value);
    nvs_close(my_handle);
    
    if (err == ESP_OK) return out_value;
    return default_value;
}

} // namespace TamimysticOS
