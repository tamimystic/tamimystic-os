#pragma once
#include <string>
#include <functional>
#include <cstdint>

namespace TamimysticOS {

enum TaskCore {
    CORE_0 = 0, // Wi-Fi, Networking, OS Background Tasks
    CORE_1 = 1, // AI, Camera, User App Logic
    CORE_ANY = -1
};

class OSScheduler {
public:
    static OSScheduler& getInstance();

    // Initialize the scheduler
    void init();

    // Create a new task pinned to a specific core
    bool createTask(const std::string& task_name, 
                    uint32_t stack_size, 
                    uint8_t priority, 
                    TaskCore core, 
                    std::function<void()> task_func);

    // Sleep for milliseconds (Yields CPU to other tasks)
    void delay(uint32_t ms);

private:
    OSScheduler() = default;
    ~OSScheduler() = default;
};

} // namespace TamimysticOS
