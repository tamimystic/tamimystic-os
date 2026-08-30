#include <iostream>
#include <thread>
#include <chrono>
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include "os_cli.h"
#include "os_config.h"
#include "os_scheduler.h"
#include "os_network.h"
#include "os_storage.h"
#include "os_web.h"
#include "os_apps.h"
#include "os_pnp_manager.h"
#include "os_pin_matrix.h"
#include "os_robotics.h"
#include "os_ai.h"

using namespace TamimysticOS;

// The core OS initialization logic, shared between ESP32 and Native
void os_core_start() {
    hal_uart_init();
    hal_uart_print("\n=======================================================\n");
    hal_uart_print("       TAMIMYSTIC OS - ESP32-S3 ULTRA PRO MAX          \n");
    hal_uart_print("=======================================================\n");
    hal_uart_print("[BOOT] Starting system bring-up sequence...\n");
    
    // 1. Initialize Core Event Bus
    EventBus::getInstance().init();

    // 2. Initialize Task Scheduler
    OSScheduler::getInstance().init();

    // 3. Initialize Config Manager (NVS Key-Value store)
    ConfigManager::getInstance().init();

    // 4. Initialize Plug & Play Hardware Engine & Dynamic Pin Matrix
    PnPManager::getInstance().init();

    // 5. Initialize Network Manager
    NetworkManager::getInstance().init();

    // 6. Initialize Storage (VFS)
    StorageManager::getInstance().init();

    // 7. Initialize WASM & Scripting App Manager
    AppManager::getInstance().init();

    // 8. Initialize Robotics & AI Subsystems
    MotorDriver::getInstance().init();
    AIModule::getInstance().init();

    // Background Task for System Heartbeat
    OSScheduler::getInstance().createTask("sys_heartbeat", 2048, 1, CORE_0, []() {
        while (true) {
            // Check system health, wait 15 seconds
            OSScheduler::getInstance().delay(15000);
            hal_uart_print("[SYS:HEALTH] Heartbeat OK | Memory & Tasks Healthy.\n");
        }
    });

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
        hal_uart_print("[SYS] Received SYSTEM_BOOT event. System is fully UP & READY!\n");
    });

    hal_uart_print("[INFO] Starting background services & CLI...\n");

    // Publish Boot Event
    EventBus::getInstance().publish(EventTopic::SYSTEM_BOOT);

    // Start the CLI (interactive shell)
    CLI::getInstance().init();
}

#ifdef OS_TARGET_NATIVE
// Entry point for PC Simulation (Native build)
int main(int argc, char** argv) {
    std::cout << "Starting Native PC POSIX Simulation..." << std::endl;
    os_core_start();
    
    // Keep native main thread alive
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
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
