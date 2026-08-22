#pragma once
#include <string>
#include <cstdint>

namespace TamimysticOS {

class AIModule {
public:
    static AIModule& getInstance();
    void init();
    
    // Run inference on a camera frame
    std::string runInference(const uint8_t* image_data, size_t length);

    // Get latest detection result (JSON)
    std::string getLatestDetection();

private:
    AIModule() = default;
    ~AIModule() = default;
    std::string latest_result = "{\"object\":\"none\",\"confidence\":0}";
};

} // namespace TamimysticOS
