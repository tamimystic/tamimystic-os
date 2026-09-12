#include "test_framework.h"
#include "os_event_bus.h"

using namespace TamimysticOS;

#include <thread>
#include <chrono>

void test_event_bus_publish_subscribe() {
    auto& bus = EventBus::getInstance();
    bus.init();

    bool event_received = false;
    bus.subscribe(EventTopic::PIN_CONFIG_CHANGED, [&](const SystemEvent& evt) {
        if (evt.topic == EventTopic::PIN_CONFIG_CHANGED) {
            event_received = true;
        }
    });

    bus.publish(EventTopic::PIN_CONFIG_CHANGED);
    
    // Wait for asynchronous event dispatcher thread
    for (int i = 0; i < 20 && !event_received; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    TEST_ASSERT(event_received, "Subscribed event should be received after publish");
}

void run_event_bus_test_suite() {
    RUN_TEST_SUITE("Event Bus Publish / Subscribe Messaging Engine", test_event_bus_publish_subscribe);
}
