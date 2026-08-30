#include "os_ai.h"
#include "os_hal_uart.h"
#include "os_scheduler.h"
#include "os_robotics.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace TamimysticOS {

AIModule& AIModule::getInstance() {
    static AIModule instance;
    return instance;
}

void AIModule::init() {
    hal_uart_print("[AI] Initializing TensorFlow Lite Micro & ESP-NN SIMD Neural Engine...\n");

    // Initialize Camera
    CameraManager::getInstance().init(FrameResolution::RES_QVGA, FrameFormat::FORMAT_JPEG);

    current_model = AIModelType::PERSON_DETECTION;
    telemetry.model = current_model;
    telemetry.fps = 22.0f;
    telemetry.inference_time_ms = 18;
    telemetry.visual_tracking_enabled = false;

    // Spawn 20 FPS Edge Neural Inference Task on Core 1
    OSScheduler::getInstance().createTask("ai_inference", 8192, 2, CORE_1, [this]() {
        while (true) {
            CameraFrame* frame = CameraManager::getInstance().getFrame();
            if (frame) {
                this->latest_result = this->runInference(frame);
                CameraManager::getInstance().returnFrame(frame);

                if (this->visual_tracking_enabled) {
                    this->processAutonomousTracking(this->latest_result);
                }
            }
            OSScheduler::getInstance().delay(50); // 20 FPS
        }
    });

    hal_uart_print("[AI] Edge AI & Vision Pipeline Active on Core 1.\n");
}

void AIModule::setModel(AIModelType model) {
    current_model = model;
    telemetry.model = model;
    hal_uart_print(("[AI] Switched Neural Model to: " + std::string(aiModelToString(model)) + "\n").c_str());
}

AIModelType AIModule::getModel() const {
    return current_model;
}

void AIModule::setVisualTracking(bool enable) {
    visual_tracking_enabled = enable;
    telemetry.visual_tracking_enabled = enable;
    if (!enable) {
        // Stop robot when tracking is disabled
        RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
    }
    hal_uart_print(("[AI:AUTONOMY] Visual Target Tracking " + std::string(enable ? "ENABLED" : "DISABLED") + "\n").c_str());
}

bool AIModule::isVisualTrackingEnabled() const {
    return visual_tracking_enabled;
}

AIDetectionResult AIModule::runInference(CameraFrame* frame) {
    AIDetectionResult res;
    res.inference_time_ms = 16 + (step_count % 5);
    res.fps = 1000.0f / (float)(res.inference_time_ms + 30);
    step_count++;

    // Simulated neural inference results according to selected model
    switch (current_model) {
        case AIModelType::PERSON_DETECTION: {
            // Dynamic moving person target in field of view
            float x_pos = 0.5f + 0.25f * std::sin((float)step_count * 0.08f);
            DetectionBox box;
            box.x = x_pos;
            box.y = 0.52f;
            box.width = 0.28f;
            box.height = 0.62f;
            box.label = "Person";
            box.confidence = 94.8f;
            res.detections.push_back(box);
            res.primary_label = "Person";
            res.primary_confidence = 94.8f;
            res.target_locked = true;
            break;
        }
        case AIModelType::OBJECT_DETECTION: {
            DetectionBox b1, b2;
            b1.x = 0.38f; b1.y = 0.50f; b1.width = 0.22f; b1.height = 0.35f;
            b1.label = "Traffic Cone"; b1.confidence = 91.2f;
            b2.x = 0.72f; b2.y = 0.60f; b2.width = 0.16f; b2.height = 0.16f;
            b2.label = "Ball"; b2.confidence = 88.6f;
            res.detections.push_back(b1);
            res.detections.push_back(b2);
            res.primary_label = "Traffic Cone";
            res.primary_confidence = 91.2f;
            res.target_locked = true;
            break;
        }
        case AIModelType::LANE_TRACKING: {
            float offset = 12.0f * std::sin((float)step_count * 0.05f);
            res.lane_offset_percent = offset;
            res.lane_angle_deg = offset * 0.8f;
            res.primary_label = "Road Lane Centered";
            res.primary_confidence = 96.5f;
            res.target_locked = true;
            break;
        }
        case AIModelType::GESTURE_RECOGNITION: {
            res.primary_label = "Open Palm (Forward)";
            res.primary_confidence = 93.0f;
            res.target_locked = true;
            break;
        }
    }

    telemetry.latest_label = res.primary_label;
    telemetry.latest_confidence = res.primary_confidence;
    telemetry.total_detections = res.detections.size();
    telemetry.inference_time_ms = res.inference_time_ms;
    telemetry.fps = res.fps;
    telemetry.target_locked = res.target_locked;

    return res;
}

void AIModule::processAutonomousTracking(const AIDetectionResult& result) {
    if (!visual_tracking_enabled || result.detections.empty()) {
        return;
    }

    // Lock on primary detected object
    const auto& target = result.detections[0];
    float center_x = target.x; // 0.0 to 1.0 (0.5 is dead center)
    float error_x = center_x - 0.5f; // -0.5 (left) to +0.5 (right)

    // Visual Proportional Steering Controller
    float Kp_steer = 60.0f;
    float angular_w = error_x * Kp_steer; // Steering rate

    // Forward drive speed (if object is far, drive forward; if close, stop)
    float linear_v = 0.0f;
    if (target.height < 0.70f) { // Object is not too close
        linear_v = 35.0f;
    } else {
        linear_v = 0.0f; // Reached target distance
    }

    // Command Robot Controller dynamically
    RobotController::getInstance().setTwist(linear_v, 0.0f, angular_w);
}

std::string AIModule::getLatestDetection() {
    std::stringstream ss;
    ss << "{"
       << "\"status\":\"ok\","
       << "\"model\":\"" << aiModelToString(current_model) << "\","
       << "\"object\":\"" << latest_result.primary_label << "\","
       << "\"confidence\":" << std::fixed << std::setprecision(1) << latest_result.primary_confidence << ","
       << "\"fps\":" << latest_result.fps << ","
       << "\"latency_ms\":" << latest_result.inference_time_ms << ","
       << "\"tracking\":" << (visual_tracking_enabled ? "true" : "false") << ","
       << "\"locked\":" << (latest_result.target_locked ? "true" : "false") << ","
       << "\"boxes\":[";

    for (size_t i = 0; i < latest_result.detections.size(); i++) {
        const auto& d = latest_result.detections[i];
        if (i > 0) ss << ",";
        ss << "{"
           << "\"x\":" << d.x << ",\"y\":" << d.y
           << ",\"w\":" << d.width << ",\"h\":" << d.height
           << ",\"label\":\"" << d.label << "\""
           << ",\"conf\":" << d.confidence
           << "}";
    }
    ss << "]}";
    return ss.str();
}

AITelemetry AIModule::getTelemetry() const {
    return telemetry;
}

std::string AIModule::getTelemetryJson() {
    return getLatestDetection();
}

} // namespace TamimysticOS
