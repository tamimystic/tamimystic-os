#pragma once
#include <string>

namespace TamimysticOS {

enum class NetworkState {
    DISCONNECTED,
    CONNECTING,
    CONNECTED_STA,
    AP_STARTED
};

class NetworkManager {
public:
    static NetworkManager& getInstance();

    // Initialize the network subsystem
    void init();

    // Connect to a Wi-Fi Access Point
    void connectWiFi(const std::string& ssid, const std::string& password);

    // Get current network state
    NetworkState getState() const;

private:
    NetworkManager() = default;
    ~NetworkManager() = default;

    NetworkState current_state = NetworkState::DISCONNECTED;
    
    // Allow ports to update the state internally
    friend void internal_set_network_state(NetworkState state);
};

} // namespace TamimysticOS
