#pragma once

#include <stdint.h>
#include <functional>

namespace TamimysticOS {

enum class EventTopic {
    SYSTEM_BOOT,
    NETWORK_STATE_CHANGE,
    DEVICE_CONNECTED,
    DEVICE_DISCONNECTED,
    PIN_CONFIG_CHANGED,
    ROBOTICS_FAULT,
    CUSTOM_APP_EVENT,
    MAX_TOPICS
};

struct SystemEvent {
    EventTopic topic;
    uint32_t payload_size;
    void* payload; // Note: Memory management of payload is currently up to the publisher
};

using EventCallback = std::function<void(const SystemEvent&)>;

class EventBus {
public:
    static EventBus& getInstance();

    // Initialize the event bus structures
    void init();

    // Publish an event to the bus
    bool publish(EventTopic topic, void* payload = nullptr, uint32_t size = 0);

    // Subscribe to a topic
    void subscribe(EventTopic topic, EventCallback callback);

    // Process events (called by a dedicated task/thread)
    void processEvents();

private:
    EventBus() = default;
    ~EventBus() = default;
};

} // namespace TamimysticOS
