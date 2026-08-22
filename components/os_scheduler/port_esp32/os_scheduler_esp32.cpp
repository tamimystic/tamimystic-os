#include "os_scheduler.h"
#include "os_hal_uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace TamimysticOS {

// Wrapper structure to pass std::function to FreeRTOS C-style task
struct TaskWrapper {
    std::function<void()> func;
};

static void FreeRTOSTaskEntry(void* pvParameters) {
    TaskWrapper* wrapper = static_cast<TaskWrapper*>(pvParameters);
    if (wrapper && wrapper->func) {
        wrapper->func();
    }
    delete wrapper;
    vTaskDelete(NULL); // Auto-delete task when function finishes
}

OSScheduler& OSScheduler::getInstance() {
    static OSScheduler instance;
    return instance;
}

void OSScheduler::init() {
    hal_uart_print("[SCHEDULER] Initializing FreeRTOS Scheduler (ESP32)...\n");
}

bool OSScheduler::createTask(const std::string& task_name, 
                             uint32_t stack_size, 
                             uint8_t priority, 
                             TaskCore core, 
                             std::function<void()> task_func) {
    
    TaskWrapper* wrapper = new TaskWrapper{task_func};
    BaseType_t xCoreID = (core == CORE_ANY) ? tskNO_AFFINITY : (BaseType_t)core;

    BaseType_t ret = xTaskCreatePinnedToCore(
        FreeRTOSTaskEntry,
        task_name.c_str(),
        stack_size,
        wrapper,
        priority,
        NULL,
        xCoreID
    );

    if (ret != pdPASS) {
        hal_uart_print(("[SCHEDULER] Failed to spawn Task: " + task_name + "\n").c_str());
        delete wrapper;
        return false;
    }

    return true;
}

void OSScheduler::delay(uint32_t ms) {
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

} // namespace TamimysticOS
