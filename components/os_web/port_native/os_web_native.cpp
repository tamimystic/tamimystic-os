#include "os_web.h"
#include "os_hal_uart.h"
#include "os_robotics.h"
#include "os_ai.h"
#include "os_camera.h"
#include "os_storage.h"
#include "os_apps.h"
#include "os_pnp_manager.h"
#include "os_pin_matrix.h"
#include <thread>
#include <atomic>
#include "httplib.h"
#include "dashboard_html.h"

namespace TamimysticOS {

static httplib::Server* svr = nullptr;
static std::thread* server_thread = nullptr;
static std::atomic<bool> is_running(false);

WebServer& WebServer::getInstance() {
    static WebServer instance;
    return instance;
}

void WebServer::start() {
    if (is_running) return;
    is_running = true;
    hal_uart_print("[WEB] Starting Native Real Web Server on http://localhost:8080\n");

    server_thread = new std::thread([]() {
        svr = new httplib::Server();
        
        // Serve the Premium Dashboard HTML
        svr->Get("/", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content(dashboard_html, "text/html");
        });

        // Simple API for motor control
        svr->Get("/api/motor", [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("left") && req.has_param("right")) {
                try {
                    int left = std::stoi(req.get_param_value("left"));
                    int right = std::stoi(req.get_param_value("right"));
                    MotorDriver::getInstance().setSpeed(left, right);
                } catch (...) {
                    // Invalid params
                }
            }
            res.set_content("{\"status\":\"ok\"}", "application/json");
        });

        // API for Camera Snapshot
        svr->Get("/api/camera/snapshot", [](const httplib::Request& req, httplib::Response& res) {
            CameraFrame* fb = CameraManager::getInstance().getFrame();
            if (fb && fb->buf && fb->len > 0) {
                res.set_content(reinterpret_cast<const char*>(fb->buf), fb->len, "image/jpeg");
                CameraManager::getInstance().returnFrame(fb);
            } else {
                res.status = 503;
                res.set_content("Camera Unavailable", "text/plain");
            }
        });

        // API for AI status
        svr->Get("/api/ai/status", [](const httplib::Request& req, httplib::Response& res) {
            std::string ai_json = AIModule::getInstance().getLatestDetection();
            res.set_content(ai_json, "application/json");
        });

        // API for AI Model Switch
        auto handle_ai_model = [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("model")) {
                std::string m = req.get_param_value("model");
                if (m == "person") AIModule::getInstance().setModel(AIModelType::PERSON_DETECTION);
                else if (m == "object") AIModule::getInstance().setModel(AIModelType::OBJECT_DETECTION);
                else if (m == "lane") AIModule::getInstance().setModel(AIModelType::LANE_TRACKING);
                else if (m == "gesture") AIModule::getInstance().setModel(AIModelType::GESTURE_RECOGNITION);
            }
            res.set_content("{\"status\":\"ok\"}", "application/json");
        };
        svr->Post("/api/ai/model", handle_ai_model);
        svr->Get("/api/ai/model", handle_ai_model);

        // API for AI Visual Tracking Toggle
        auto handle_ai_track = [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("enable")) {
                std::string val = req.get_param_value("enable");
                bool enable = (val == "1" || val == "true" || val == "on");
                AIModule::getInstance().setVisualTracking(enable);
            }
            res.set_content("{\"status\":\"ok\"}", "application/json");
        };
        svr->Post("/api/ai/track", handle_ai_track);
        svr->Get("/api/ai/track", handle_ai_track);

        // API for Plug & Play Devices
        svr->Get("/api/pnp/devices", [](const httplib::Request& req, httplib::Response& res) {
            std::string json = PnPManager::getInstance().getDevicesJson();
            res.set_content(json, "application/json");
        });

        svr->Post("/api/pnp/scan", [](const httplib::Request& req, httplib::Response& res) {
            PnPManager::getInstance().scanI2CBus();
            std::string json = PnPManager::getInstance().getDevicesJson();
            res.set_content(json, "application/json");
        });

        // API for Dynamic Pin Matrix
        svr->Get("/api/pins", [](const httplib::Request& req, httplib::Response& res) {
            std::string json = PinMatrixManager::getInstance().getPinMatrixJson();
            res.set_content(json, "application/json");
        });

        auto handle_pin_set = [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("func") && req.has_param("pin")) {
                std::string func = req.get_param_value("func");
                int pin = std::stoi(req.get_param_value("pin"));
                bool ok = PinMatrixManager::getInstance().setPin(func, pin);
                if (ok) {
                    res.set_content("{\"status\":\"ok\"}", "application/json");
                } else {
                    res.set_content("{\"status\":\"error\",\"message\":\"Unsafe GPIO pin\"}", "application/json");
                }
            } else {
                res.set_content("{\"status\":\"error\",\"message\":\"Missing parameters\"}", "application/json");
            }
        };
        svr->Post("/api/pins/set", handle_pin_set);
        svr->Get("/api/pins/set", handle_pin_set);

        // ================= Universal Robotics API =================
        // 1. Robot Mode Switcher
        auto handle_robot_mode = [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("mode")) {
                std::string m = req.get_param_value("mode");
                if (m == "diff") RobotController::getInstance().setMode(RobotMode::DIFFERENTIAL_ROVER);
                else if (m == "mecanum") RobotController::getInstance().setMode(RobotMode::MECANUM_4WD);
                else if (m == "arm") RobotController::getInstance().setMode(RobotMode::ROBOTIC_ARM);
                else if (m == "balance") RobotController::getInstance().setMode(RobotMode::BALANCE_BOT);
            }
            res.set_content("{\"status\":\"ok\"}", "application/json");
        };
        svr->Post("/api/robot/mode", handle_robot_mode);
        svr->Get("/api/robot/mode", handle_robot_mode);

        // 2. Velocity Command (cmd_vel)
        auto handle_robot_cmd_vel = [](const httplib::Request& req, httplib::Response& res) {
            float vx = req.has_param("vx") ? std::stof(req.get_param_value("vx")) : 0.0f;
            float vy = req.has_param("vy") ? std::stof(req.get_param_value("vy")) : 0.0f;
            float w  = req.has_param("w")  ? std::stof(req.get_param_value("w"))  : 0.0f;
            RobotController::getInstance().setTwist(vx, vy, w);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        };
        svr->Post("/api/robot/cmd_vel", handle_robot_cmd_vel);
        svr->Get("/api/robot/cmd_vel", handle_robot_cmd_vel);

        // 3. Robotic Arm Joint Control
        auto handle_robot_arm = [](const httplib::Request& req, httplib::Response& res) {
            ArmJoints j = RobotController::getInstance().getTelemetry().joints;
            if (req.has_param("j1")) j.base_yaw = std::stof(req.get_param_value("j1"));
            if (req.has_param("j2")) j.shoulder_pitch = std::stof(req.get_param_value("j2"));
            if (req.has_param("j3")) j.elbow_pitch = std::stof(req.get_param_value("j3"));
            if (req.has_param("j4")) j.wrist_pitch = std::stof(req.get_param_value("j4"));
            if (req.has_param("j5")) j.wrist_roll = std::stof(req.get_param_value("j5"));
            if (req.has_param("j6")) j.gripper = std::stof(req.get_param_value("j6"));
            RobotController::getInstance().setArmJoints(j);
            res.set_content("{\"status\":\"ok\"}", "application/json");
        };
        svr->Post("/api/robot/arm", handle_robot_arm);
        svr->Get("/api/robot/arm", handle_robot_arm);

        // 4. Robotic Arm Cartesian IK Solver
        auto handle_robot_ik = [](const httplib::Request& req, httplib::Response& res) {
            ArmPose pose = RobotController::getInstance().getTelemetry().pose;
            if (req.has_param("x")) pose.x = std::stof(req.get_param_value("x"));
            if (req.has_param("y")) pose.y = std::stof(req.get_param_value("y"));
            if (req.has_param("z")) pose.z = std::stof(req.get_param_value("z"));
            if (req.has_param("pitch")) pose.pitch = std::stof(req.get_param_value("pitch"));
            if (req.has_param("gripper")) pose.gripper = std::stof(req.get_param_value("gripper"));

            bool ok = RobotController::getInstance().setArmTargetIK(pose);
            if (ok) {
                res.set_content("{\"status\":\"ok\"}", "application/json");
            } else {
                res.set_content("{\"status\":\"error\",\"message\":\"Target position is out of reach\"}", "application/json");
            }
        };
        svr->Post("/api/robot/arm/ik", handle_robot_ik);
        svr->Get("/api/robot/arm/ik", handle_robot_ik);

        // 5. Emergency Stop / Resume
        svr->Post("/api/robot/stop", [](const httplib::Request& req, httplib::Response& res) {
            RobotController::getInstance().emergencyStop();
            res.set_content("{\"status\":\"ok\",\"e_stop\":true}", "application/json");
        });
        svr->Post("/api/robot/resume", [](const httplib::Request& req, httplib::Response& res) {
            RobotController::getInstance().resume();
            res.set_content("{\"status\":\"ok\",\"e_stop\":false}", "application/json");
        });

        // 6. Live Telemetry
        svr->Get("/api/robot/telemetry", [](const httplib::Request& req, httplib::Response& res) {
            std::string json = RobotController::getInstance().getTelemetryJson();
            res.set_content(json, "application/json");
        });

        // ================= Dynamic Scripting & Apps API =================
        // 1. Python Code Eval
        svr->Post("/api/apps/eval", [](const httplib::Request& req, httplib::Response& res) {
            std::string code = req.body;
            auto result = AppManager::getInstance().evalCode(code);
            std::stringstream ss;
            ss << "{"
               << "\"status\":\"" << (result.success ? "ok" : "error") << "\","
               << "\"stdout\":\"";
            // Escape special chars in stdout
            for (char c : result.stdout_output) {
                if (c == '"') ss << "\\\"";
                else if (c == '\\') ss << "\\\\";
                else if (c == '\n') ss << "\\n";
                else if (c == '\r') continue;
                else ss << c;
            }
            ss << "\","
               << "\"execution_time_ms\":" << result.execution_time_ms << ","
               << "\"error\":\"" << result.error_message << "\""
               << "}";
            res.set_content(ss.str(), "application/json");
        });

        // 2. Run Script File
        svr->Post("/api/apps/run", [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("file")) {
                std::string fname = req.get_param_value("file");
                auto result = AppManager::getInstance().runFile(fname);
                res.set_content("{\"status\":\"ok\"}", "application/json");
            } else {
                res.set_content("{\"status\":\"error\",\"message\":\"Missing file parameter\"}", "application/json");
            }
        });

        // 3. Stop Script
        svr->Post("/api/apps/stop", [](const httplib::Request& req, httplib::Response& res) {
            AppManager::getInstance().stopCurrentApp();
            res.set_content("{\"status\":\"ok\"}", "application/json");
        });

        // 4. File Listing
        svr->Get("/api/files/list", [](const httplib::Request& req, httplib::Response& res) {
            std::string json = StorageManager::getInstance().getFilesJson();
            res.set_content(json, "application/json");
        });

        // 5. File Delete
        auto handle_file_delete = [](const httplib::Request& req, httplib::Response& res) {
            if (req.has_param("file")) {
                std::string fname = req.get_param_value("file");
                bool ok = StorageManager::getInstance().deleteFile(fname);
                res.set_content(ok ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", "application/json");
            } else {
                res.set_content("{\"status\":\"error\"}", "application/json");
            }
        };
        svr->Post("/api/files/delete", handle_file_delete);
        svr->Get("/api/files/delete", handle_file_delete);

        // 6. Storage Stats
        svr->Get("/api/storage/stats", [](const httplib::Request& req, httplib::Response& res) {
            auto stats = StorageManager::getInstance().getStats();
            std::stringstream ss;
            ss << "{\"status\":\"ok\",\"total\":" << stats.total_bytes << ",\"used\":" << stats.used_bytes << ",\"free\":" << stats.free_bytes << "}";
            res.set_content(ss.str(), "application/json");
        });

        // API for File Upload (OTA / Models / Apps)
        svr->Post("/api/upload", [](const httplib::Request& req, httplib::Response& res) {
            if (req.form.has_file("file")) {
                const auto& file = req.form.get_file("file");
                std::string filename = file.filename;
                
                // Write to VFS
                bool success = StorageManager::getInstance().writeFile(filename, 
                                  reinterpret_cast<const uint8_t*>(file.content.c_str()), 
                                  file.content.length());
                
                if (success) {
                    res.set_content("{\"status\":\"ok\",\"message\":\"File uploaded successfully\"}", "application/json");
                } else {
                    res.status = 500;
                    res.set_content("{\"status\":\"error\",\"message\":\"Failed to save file\"}", "application/json");
                }
            } else {
                res.status = 400;
                res.set_content("{\"status\":\"error\",\"message\":\"No file provided\"}", "application/json");
            }
        });

        svr->listen("0.0.0.0", 8080);
    });
}

void WebServer::stop() {
    if (!is_running) return;
    hal_uart_print("[WEB] Stopping Native Web Server...\n");
    if (svr) {
        svr->stop();
    }
    if (server_thread && server_thread->joinable()) {
        server_thread->join();
        delete server_thread;
        server_thread = nullptr;
    }
    if (svr) {
        delete svr;
        svr = nullptr;
    }
    is_running = false;
}

} // namespace TamimysticOS
