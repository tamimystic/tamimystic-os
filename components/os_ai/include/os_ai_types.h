#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace TamimysticOS {

enum class AIModelType {
    PERSON_DETECTION = 0,   // MobileNetV2 Person Detector
    OBJECT_DETECTION,       // MobileNet SSD (Person, Car, Cone, Ball, Obstacle)
    LANE_TRACKING,          // Autonomous Car Visual Road Lane & Line Follower
    GESTURE_RECOGNITION     // Hand Gesture Classifier (Stop, Forward, Left, Right)
};

inline const char* aiModelToString(AIModelType type) {
    switch (type) {
        case AIModelType::PERSON_DETECTION:   return "MobileNet-V2 Person Detector";
        case AIModelType::OBJECT_DETECTION:   return "MobileNet-SSD Object Detector";
        case AIModelType::LANE_TRACKING:      return "Autonomous Lane & Line Follower";
        case AIModelType::GESTURE_RECOGNITION:return "Hand Gesture Neural Classifier";
        default:                              return "Custom TFLite Model";
    }
}

struct DetectionBox {
    float x = 0.0f;          // Normalized X center (0.0 to 1.0)
    float y = 0.0f;          // Normalized Y center (0.0 to 1.0)
    float width = 0.0f;      // Normalized width (0.0 to 1.0)
    float height = 0.0f;     // Normalized height (0.0 to 1.0)
    std::string label;       // Class label (e.g. "Person", "Traffic Cone", "Ball")
    float confidence = 0.0f; // Confidence (0 to 100%)
};

struct AIDetectionResult {
    std::vector<DetectionBox> detections;
    std::string primary_label = "None";
    float primary_confidence = 0.0f;
    float lane_offset_percent = 0.0f; // -100% (left) to +100% (right)
    float lane_angle_deg = 0.0f;      // Lane heading deviation angle
    uint32_t inference_time_ms = 18;
    float fps = 22.5f;
    bool target_locked = false;
};

struct AITelemetry {
    AIModelType model = AIModelType::PERSON_DETECTION;
    float fps = 22.5f;
    uint32_t inference_time_ms = 18;
    bool visual_tracking_enabled = false;
    bool target_locked = false;
    size_t total_detections = 0;
    std::string latest_label = "None";
    float latest_confidence = 0.0f;
};

} // namespace TamimysticOS
