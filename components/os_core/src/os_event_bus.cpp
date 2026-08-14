#include "os_event_bus.h"
#include "os_hal_uart.h"

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>

#ifdef OS_TARGET_NATIVE
#include <thread>
#else
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#endif

namespace TamimysticOS {

static std::vector<EventCallback> subscribers[static_cast<int>(EventTopic::MAX_TOPICS)];

#ifdef OS_TARGET_NATIVE
static std::queue<SystemEvent> event_queue;
static std::mutex queue_mutex;
static std::condition_variable queue_cv;
static std::thread event_thread;
#else
static QueueHandle_t event_queue = nullptr;
static TaskHandle_t event_task = nullptr;
#endif

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

#ifdef OS_TARGET_NATIVE
static void native_event_task_func() {
    EventBus::getInstance().processEvents();
}
#else
static void esp32_event_task_func(void* arg) {
    EventBus::getInstance().processEvents();
}
#endif

void EventBus::init() {
#ifdef OS_TARGET_NATIVE
    hal_uart_print("[EVENT] Initializing Native Event Bus...\n");
    event_thread = std::thread(native_event_task_func);
    event_thread.detach();
#else
    hal_uart_print("[EVENT] Initializing ESP32 FreeRTOS Event Bus...\n");
    event_queue = xQueueCreate(32, sizeof(SystemEvent));
    xTaskCreate(esp32_event_task_func, "EventBusTask", 3072, nullptr, 5, &event_task);
#endif
}

bool EventBus::publish(EventTopic topic, void* payload, uint32_t size) {
    SystemEvent evt = {topic, size, payload};

#ifdef OS_TARGET_NATIVE
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        event_queue.push(evt);
    }
    queue_cv.notify_one();
    return true;
#else
    if (event_queue != nullptr) {
        return (xQueueSend(event_queue, &evt, 0) == pdTRUE);
    }
    return false;
#endif
}

void EventBus::subscribe(EventTopic topic, EventCallback callback) {
    int topic_idx = static_cast<int>(topic);
    if (topic_idx < static_cast<int>(EventTopic::MAX_TOPICS)) {
        subscribers[topic_idx].push_back(callback);
    }
}

void EventBus::processEvents() {
    SystemEvent evt;
    while (true) {
#ifdef OS_TARGET_NATIVE
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_cv.wait(lock, []{ return !event_queue.empty(); });
            evt = event_queue.front();
            event_queue.pop();
        }
#else
        if (xQueueReceive(event_queue, &evt, portMAX_DELAY) != pdTRUE) {
            continue;
        }
#endif

        // Dispatch to all subscribers
        int topic_idx = static_cast<int>(evt.topic);
        for (auto& cb : subscribers[topic_idx]) {
            cb(evt);
        }
    }
}

} // namespace TamimysticOS
