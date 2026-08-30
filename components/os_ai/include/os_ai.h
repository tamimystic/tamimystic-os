#pragma once

#include "os_ai_types.h"
#include "os_camera.h"
#include <string>
#include <vector>

namespace TamimysticOS {

class AIModule {
public:
    static AIModule& getInstance();

    // Initialize TensorFlow Lite Micro / Edge Neural Engine
    void init();

    // Model selection
    void setModel(AIModelType model);
    AIModelType getModel() const;

    // Visual Autonomy: Automatic Robot Follow / Target Track Mode
    void setVisualTracking(bool enable);
    bool isVisualTrackingEnabled() const;

    // Process a camera frame through neural inference
    AIDetectionResult runInference(CameraFrame* frame);

    // Get latest detection JSON for Web UI
    std::string getLatestDetection();

    // Get telemetry struct and JSON
    AITelemetry getTelemetry() const;
    std::string getTelemetryJson();

private:
    AIModule() = default;
    ~AIModule() = default;

    void processAutonomousTracking(const AIDetectionResult& result);

    AIModelType current_model = AIModelType::PERSON_DETECTION;
    bool visual_tracking_enabled = false;
    AIDetectionResult latest_result;
    AITelemetry telemetry;
    uint32_t step_count = 0;
};

} // namespace TamimysticOS
