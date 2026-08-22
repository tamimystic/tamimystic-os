#include "os_scheduler.h"
#include "os_hal_uart.h"
#include <thread>
#include <chrono>
#include <vector>
#include <memory>

namespace TamimysticOS {

// To keep threads alive in native simulation
static std::vector<std::thread> native_threads;

OSScheduler& OSScheduler::getInstance() {
    static OSScheduler instance;
    return instance;
}

void OSScheduler::init() {
    hal_uart_print("[SCHEDULER] Initializing Native Thread Scheduler...\n");
}

bool OSScheduler::createTask(const std::string& task_name, 
                             uint32_t stack_size, 
                             uint8_t priority, 
                             TaskCore core, 
                             std::function<void()> task_func) {
    
    std::string core_str = (core == CORE_0) ? "Core 0" : (core == CORE_1) ? "Core 1" : "Any Core";
    hal_uart_print(("[SCHEDULER] Spawning Task: " + task_name + " on " + core_str + "\n").c_str());

    // In standard C++, we cannot strictly pin threads to a core easily without OS-specific APIs.
    // Since this is a simulation, we just spawn a standard std::thread.
    native_threads.emplace_back([task_func]() {
        task_func();
    });

    // Detach it so it runs independently in the background
    native_threads.back().detach();
    return true;
}

void OSScheduler::delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace TamimysticOS
