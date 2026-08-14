#include "os_network.h"
#include "os_event_bus.h"
#include "os_cli.h"
#include "os_hal_uart.h"

// ESP-IDF specific includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include <string.h>

namespace TamimysticOS {

void internal_set_network_state(NetworkState state) {
    NetworkManager::getInstance().current_state = state;
    EventBus::getInstance().publish(EventTopic::NETWORK_STATE_CHANGE);
}

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkState NetworkManager::getState() const {
    return current_state;
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        hal_uart_print("[NET] Wi-Fi disconnected.\n");
        internal_set_network_state(NetworkState::DISCONNECTED);
        esp_wifi_connect(); // Auto-reconnect
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        hal_uart_print("[NET] Wi-Fi Connected. Got IP.\n");
        internal_set_network_state(NetworkState::CONNECTED_STA);
    }
}

void NetworkManager::init() {
    hal_uart_print("[NET] Initializing ESP32 Network Manager...\n");

    // Initialize NVS (Required for Wi-Fi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // Register CLI Command
    CLI::getInstance().registerCommand("wifi", "Connect to Wi-Fi: wifi <ssid> <pass>", [](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            hal_uart_print("Usage: wifi <ssid> <password>\n");
            return;
        }
        NetworkManager::getInstance().connectWiFi(args[1], args[2]);
    });
}

void NetworkManager::connectWiFi(const std::string& ssid, const std::string& password) {
    internal_set_network_state(NetworkState::CONNECTING);
    wifi_config_t wifi_config = {};
    
    // Safe string copy
    strncpy((char*)wifi_config.sta.ssid, ssid.c_str(), sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, password.c_str(), sizeof(wifi_config.sta.password) - 1);

    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_connect();
    
    hal_uart_print("[NET] Connecting to Wi-Fi...\n");
}

} // namespace TamimysticOS
