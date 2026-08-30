#include "os_network.h"
#include "os_event_bus.h"
#include "os_cli.h"
#include "os_hal_uart.h"
#include "os_config.h"
#include <thread>
#include <chrono>

namespace TamimysticOS {

void internal_set_network_state(NetworkState state) {
    NetworkManager::getInstance().current_state = state;
    // Notify system about state change
    EventBus::getInstance().publish(EventTopic::NETWORK_STATE_CHANGE);
}

NetworkManager& NetworkManager::getInstance() {
    static NetworkManager instance;
    return instance;
}

NetworkState NetworkManager::getState() const {
    return current_state;
}

void NetworkManager::init() {
    hal_uart_print("[NET] Initializing Native Network Simulation...\n");
    
    // Register CLI Command
    CLI::getInstance().registerCommand("wifi", "Connect to Wi-Fi: wifi <ssid> <pass>", [](const std::vector<std::string>& args) {
        if (args.size() < 3) {
            hal_uart_print("Usage: wifi <ssid> <password>\n");
            return;
        }
        NetworkManager::getInstance().connectWiFi(args[1], args[2]);
    });

    // Auto-connect simulation network so Web Server is immediately accessible on http://localhost:8080
    connectWiFi("Tamimystic-SimNet", "s3-n16r8");
}

void NetworkManager::connectWiFi(const std::string& ssid, const std::string& password) {
    hal_uart_print(("[NET] Attempting to connect to Wi-Fi: " + ssid + "...\n").c_str());
    internal_set_network_state(NetworkState::CONNECTING);

    // Simulate async connection delay
    std::thread([ssid]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        hal_uart_print(("[NET] Successfully connected to mock Wi-Fi: " + ssid + "\n").c_str());
        internal_set_network_state(NetworkState::CONNECTED_STA);
    }).detach();
}

} // namespace TamimysticOS
