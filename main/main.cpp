#include <iostream>
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include "os_cli.h"
#include "os_config.h"
#include "os_network.h"
#include "os_storage.h"
#include "os_web.h"
#include "os_apps.h"
#include "os_robotics.h"
#include "os_ai.h"

using namespace TamimysticOS;

// The core OS initialization logic, shared between ESP32 and Native
void os_core_start() {
    hal_uart_init();
    hal_uart_print("\n[BOOT] Tamimystic OS Booting...\n");
    
    // Initialize Event Bus
    EventBus::getInstance().init();

    // Initialize Config Manager (NVS)
    ConfigManager::getInstance().init();

    // Initialize Network Manager
    NetworkManager::getInstance().init();

    // Initialize VFS / Storage Manager
    StorageManager::getInstance().init();

    // Initialize WASM App Manager
    AppManager::getInstance().init();

    // Initialize Robotics Motor Driver
    MotorDriver::getInstance().init();

    // Initialize AI Module
    AIModule::getInstance().init();

    // Initialize CLI Shell
    CLI::getInstance().init();

    // Subscribe to network state events
    EventBus::getInstance().subscribe(EventTopic::NETWORK_STATE_CHANGE, [](const SystemEvent& evt) {
        if (NetworkManager::getInstance().getState() == NetworkState::CONNECTED_STA) {
            hal_uart_print("[SYS] Network is now CONNECTED!\n");
            // Start the Web Dashboard automatically once network is up
            WebServer::getInstance().start();
        } else if (NetworkManager::getInstance().getState() == NetworkState::DISCONNECTED) {
            // Stop server if disconnected
            WebServer::getInstance().stop();
        }
    });

    // Subscribe to system boot event
    EventBus::getInstance().subscribe(EventTopic::SYSTEM_BOOT, [](const SystemEvent& evt) {
        hal_uart_print("[SYS] Received SYSTEM_BOOT event. System is up!\n");
    });

    hal_uart_print("[INFO] Starting system services...\n");

    // Publish Boot Event
    EventBus::getInstance().publish(EventTopic::SYSTEM_BOOT);
}

#ifdef OS_TARGET_NATIVE
// Entry point for PC Simulation (Native build)
int main(int argc, char** argv) {
    std::cout << "Starting Native POSIX Simulation..." << std::endl;
    os_core_start();
    
    // Simulate an idle loop
    while(true) {
        // Simple delay mock could be added here
    }
    return 0;
}
#else
// Entry point for ESP32-S3 (ESP-IDF build)
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern "C" void app_main(void) {
    os_core_start();
    
    // FreeRTOS idle loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
#endif
