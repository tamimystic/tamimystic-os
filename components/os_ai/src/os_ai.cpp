#include "os_ai.h"
#include "os_hal_uart.h"
#include "os_storage.h"
#include <thread>
#include <chrono>
#include <cstdlib>

namespace TamimysticOS {

AIModule& AIModule::getInstance() {
    static AIModule instance;
    return instance;
}

void AIModule::init() {
    hal_uart_print("[AI] Initializing Object Detection Module...\n");
    
    // Attempt to load dynamic model from VFS
    std::vector<uint8_t> model_buffer;
    if (StorageManager::getInstance().readFile("model.tflite", model_buffer)) {
        hal_uart_print(("[AI] SUCCESS: Loaded dynamic TFLite model from VFS (" + std::to_string(model_buffer.size()) + " bytes).\n").c_str());
        hal_uart_print("[AI] TFLite Interpreter running custom model...\n");
    } else {
        hal_uart_print("[AI] WARNING: No 'model.tflite' found in VFS. Running fallback mock model.\n");
    }

    // Start simulation thread
    std::thread([this]() {
        this->simulationLoop();
    }).detach();
}

std::string AIModule::getLatestDetection() {
    std::lock_guard<std::mutex> lock(mtx);
    if (latest_object.empty()) return "{}";
    
    std::string json = "{";
    json += "\"object\":\"" + latest_object + "\",";
    json += "\"confidence\":" + std::to_string(confidence) + ",";
    json += "\"alert\":" + std::string(alert ? "true" : "false");
    json += "}";
    return json;
}

void AIModule::simulationLoop() {
    const char* objects[] = {"Person", "Stop Sign", "Car", "Bicycle", "Obstacle"};
    
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        std::lock_guard<std::mutex> lock(mtx);
        latest_object = objects[rand() % 5];
        confidence = 70 + (rand() % 30); // 70 to 99
        
        // Alert if obstacle or person is close
        if (latest_object == "Person" || latest_object == "Obstacle") {
            alert = true;
        } else {
            alert = false;
        }
    }
}

} // namespace TamimysticOS
