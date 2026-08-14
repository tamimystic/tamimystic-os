#pragma once
#include <string>
#include <mutex>

namespace TamimysticOS {

class AIModule {
public:
    static AIModule& getInstance();

    void init();
    
    // Get the latest detection as a JSON string
    std::string getLatestDetection();

private:
    AIModule() = default;
    ~AIModule() = default;

    std::string latest_object = "";
    int confidence = 0;
    bool alert = false;
    std::mutex mtx;

    void simulationLoop();
};

} // namespace TamimysticOS
