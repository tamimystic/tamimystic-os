#include "os_web.h"
#include "os_hal_uart.h"
#include "os_robotics.h"
#include "os_ai.h"
#include "os_camera.h"
#include "os_storage.h"
#include "os_apps.h"
#include "os_pnp_manager.h"
#include "os_pin_matrix.h"
#include "dashboard_html.h"
#include "esp_http_server.h"
#include <string>

namespace TamimysticOS {

static httpd_handle_t server = NULL;

WebServer& WebServer::getInstance() {
    static WebServer instance;
    return instance;
}

// Handler for the root dashboard
static esp_err_t dashboard_get_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, dashboard_html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for PnP devices
static esp_err_t pnp_devices_handler(httpd_req_t *req) {
    std::string json = PnPManager::getInstance().getDevicesJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for PnP manual scan
static esp_err_t pnp_scan_handler(httpd_req_t *req) {
    PnPManager::getInstance().scanI2CBus();
    std::string json = PnPManager::getInstance().getDevicesJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for Pin Matrix GET
static esp_err_t pin_matrix_get_handler(httpd_req_t *req) {
    std::string json = PinMatrixManager::getInstance().getPinMatrixJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for Pin Matrix SET
static esp_err_t pin_matrix_set_handler(httpd_req_t *req) {
    char query[128];
    char param_func[32] = {0};
    char param_pin[16] = {0};

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "func", param_func, sizeof(param_func));
        httpd_query_key_value(query, "pin", param_pin, sizeof(param_pin));
        
        if (param_func[0] && param_pin[0]) {
            int pin_num = std::atoi(param_pin);
            bool ok = PinMatrixManager::getInstance().setPin(param_func, pin_num);
            if (ok) {
                httpd_resp_set_type(req, "application/json");
                httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
                return ESP_OK;
            }
        }
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Failed to set pin\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot Mode Switch
static esp_err_t robot_mode_handler(httpd_req_t *req) {
    char query[64];
    char mode_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "mode", mode_str, sizeof(mode_str));
        std::string m(mode_str);
        if (m == "diff") RobotController::getInstance().setMode(RobotMode::DIFFERENTIAL_ROVER);
        else if (m == "mecanum") RobotController::getInstance().setMode(RobotMode::MECANUM_4WD);
        else if (m == "arm") RobotController::getInstance().setMode(RobotMode::ROBOTIC_ARM);
        else if (m == "balance") RobotController::getInstance().setMode(RobotMode::BALANCE_BOT);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot cmd_vel
static esp_err_t robot_cmd_vel_handler(httpd_req_t *req) {
    char query[128];
    char vx_str[16] = {0}, vy_str[16] = {0}, w_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "vx", vx_str, sizeof(vx_str));
        httpd_query_key_value(query, "vy", vy_str, sizeof(vy_str));
        httpd_query_key_value(query, "w", w_str, sizeof(w_str));
        float vx = vx_str[0] ? (float)std::atof(vx_str) : 0.0f;
        float vy = vy_str[0] ? (float)std::atof(vy_str) : 0.0f;
        float w  = w_str[0]  ? (float)std::atof(w_str)  : 0.0f;
        RobotController::getInstance().setTwist(vx, vy, w);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot Arm Joints
static esp_err_t robot_arm_handler(httpd_req_t *req) {
    char query[256];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ArmJoints j = RobotController::getInstance().getTelemetry().joints;
        char val[16] = {0};
        if (httpd_query_key_value(query, "j1", val, sizeof(val)) == ESP_OK) j.base_yaw = (float)std::atof(val);
        if (httpd_query_key_value(query, "j2", val, sizeof(val)) == ESP_OK) j.shoulder_pitch = (float)std::atof(val);
        if (httpd_query_key_value(query, "j3", val, sizeof(val)) == ESP_OK) j.elbow_pitch = (float)std::atof(val);
        if (httpd_query_key_value(query, "j4", val, sizeof(val)) == ESP_OK) j.wrist_pitch = (float)std::atof(val);
        if (httpd_query_key_value(query, "j5", val, sizeof(val)) == ESP_OK) j.wrist_roll = (float)std::atof(val);
        if (httpd_query_key_value(query, "j6", val, sizeof(val)) == ESP_OK) j.gripper = (float)std::atof(val);
        RobotController::getInstance().setArmJoints(j);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot Arm IK
static esp_err_t robot_arm_ik_handler(httpd_req_t *req) {
    char query[128];
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        ArmPose pose = RobotController::getInstance().getTelemetry().pose;
        char val[16] = {0};
        if (httpd_query_key_value(query, "x", val, sizeof(val)) == ESP_OK) pose.x = (float)std::atof(val);
        if (httpd_query_key_value(query, "y", val, sizeof(val)) == ESP_OK) pose.y = (float)std::atof(val);
        if (httpd_query_key_value(query, "z", val, sizeof(val)) == ESP_OK) pose.z = (float)std::atof(val);
        if (httpd_query_key_value(query, "pitch", val, sizeof(val)) == ESP_OK) pose.pitch = (float)std::atof(val);
        if (httpd_query_key_value(query, "gripper", val, sizeof(val)) == ESP_OK) pose.gripper = (float)std::atof(val);

        if (RobotController::getInstance().setArmTargetIK(pose)) {
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Target unreachable\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot Telemetry
static esp_err_t robot_telemetry_handler(httpd_req_t *req) {
    std::string json = RobotController::getInstance().getTelemetryJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for Robot Stop
static esp_err_t robot_stop_handler(httpd_req_t *req) {
    RobotController::getInstance().emergencyStop();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\",\"e_stop\":true}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Robot Resume
static esp_err_t robot_resume_handler(httpd_req_t *req) {
    RobotController::getInstance().resume();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\",\"e_stop\":false}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for Camera Snapshot
static esp_err_t camera_snapshot_handler(httpd_req_t *req) {
    CameraFrame* fb = CameraManager::getInstance().getFrame();
    if (fb && fb->buf && fb->len > 0) {
        httpd_resp_set_type(req, "image/jpeg");
        httpd_resp_send(req, reinterpret_cast<const char*>(fb->buf), fb->len);
        CameraManager::getInstance().returnFrame(fb);
        return ESP_OK;
    }
    httpd_resp_set_type(req, "text/plain");
    httpd_resp_send(req, "Camera Unavailable", HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
}

// Handler for AI Model Switch
static esp_err_t ai_model_handler(httpd_req_t *req) {
    char query[64];
    char model_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "model", model_str, sizeof(model_str));
        std::string m(model_str);
        if (m == "person") AIModule::getInstance().setModel(AIModelType::PERSON_DETECTION);
        else if (m == "object") AIModule::getInstance().setModel(AIModelType::OBJECT_DETECTION);
        else if (m == "lane") AIModule::getInstance().setModel(AIModelType::LANE_TRACKING);
        else if (m == "gesture") AIModule::getInstance().setModel(AIModelType::GESTURE_RECOGNITION);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for AI Visual Tracking
static esp_err_t ai_track_handler(httpd_req_t *req) {
    char query[64];
    char enable_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "enable", enable_str, sizeof(enable_str));
        std::string val(enable_str);
        bool enable = (val == "1" || val == "true" || val == "on");
        AIModule::getInstance().setVisualTracking(enable);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for AI status
static esp_err_t ai_status_handler(httpd_req_t *req) {
    std::string json = AIModule::getInstance().getLatestDetection();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for Python Eval
static esp_err_t apps_eval_handler(httpd_req_t *req) {
    char buf[1024] = {0};
    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret > 0) {
        buf[ret] = '\0';
        std::string code(buf);
        auto result = AppManager::getInstance().evalCode(code);
        std::string json = "{\"status\":\"" + std::string(result.success ? "ok" : "error") + 
                           "\",\"stdout\":\"" + result.stdout_output + 
                           "\",\"execution_time_ms\":" + std::to_string(result.execution_time_ms) + "}";
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json.c_str(), json.length());
        return ESP_OK;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Empty body\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
}

// Handler for App Stop
static esp_err_t apps_stop_handler(httpd_req_t *req) {
    AppManager::getInstance().stopCurrentApp();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Handler for File List
static esp_err_t files_list_handler(httpd_req_t *req) {
    std::string json = StorageManager::getInstance().getFilesJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

// Handler for File Delete
static esp_err_t files_delete_handler(httpd_req_t *req) {
    char query[64];
    char file_str[32] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "file", file_str, sizeof(file_str));
        if (file_str[0]) {
            bool ok = StorageManager::getInstance().deleteFile(file_str);
            httpd_resp_set_type(req, "application/json");
            httpd_resp_send(req, ok ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", HTTPD_RESP_USE_STRLEN);
            return ESP_OK;
        }
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_FAIL;
}

void WebServer::start() {
    if (is_running) return;
    
    hal_uart_print("[WEB] Starting ESP32 HTTP Server on Port 80...\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 40;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_dash = { .uri = "/", .method = HTTP_GET, .handler = dashboard_get_handler, .user_ctx = NULL };
        httpd_uri_t uri_pnp_dev = { .uri = "/api/pnp/devices", .method = HTTP_GET, .handler = pnp_devices_handler, .user_ctx = NULL };
        httpd_uri_t uri_pnp_scan = { .uri = "/api/pnp/scan", .method = HTTP_POST, .handler = pnp_scan_handler, .user_ctx = NULL };
        httpd_uri_t uri_pins_get = { .uri = "/api/pins", .method = HTTP_GET, .handler = pin_matrix_get_handler, .user_ctx = NULL };
        httpd_uri_t uri_pins_set = { .uri = "/api/pins/set", .method = HTTP_POST, .handler = pin_matrix_set_handler, .user_ctx = NULL };
        
        httpd_uri_t uri_robot_mode = { .uri = "/api/robot/mode", .method = HTTP_POST, .handler = robot_mode_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_cmd_vel = { .uri = "/api/robot/cmd_vel", .method = HTTP_POST, .handler = robot_cmd_vel_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_arm = { .uri = "/api/robot/arm", .method = HTTP_POST, .handler = robot_arm_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_ik = { .uri = "/api/robot/arm/ik", .method = HTTP_POST, .handler = robot_arm_ik_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_stop = { .uri = "/api/robot/stop", .method = HTTP_POST, .handler = robot_stop_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_resume = { .uri = "/api/robot/resume", .method = HTTP_POST, .handler = robot_resume_handler, .user_ctx = NULL };
        httpd_uri_t uri_robot_telemetry = { .uri = "/api/robot/telemetry", .method = HTTP_GET, .handler = robot_telemetry_handler, .user_ctx = NULL };
        
        httpd_uri_t uri_camera_snap = { .uri = "/api/camera/snapshot", .method = HTTP_GET, .handler = camera_snapshot_handler, .user_ctx = NULL };
        httpd_uri_t uri_ai_model = { .uri = "/api/ai/model", .method = HTTP_POST, .handler = ai_model_handler, .user_ctx = NULL };
        httpd_uri_t uri_ai_track = { .uri = "/api/ai/track", .method = HTTP_POST, .handler = ai_track_handler, .user_ctx = NULL };
        httpd_uri_t uri_ai = { .uri = "/api/ai/status", .method = HTTP_GET, .handler = ai_status_handler, .user_ctx = NULL };

        httpd_uri_t uri_apps_eval = { .uri = "/api/apps/eval", .method = HTTP_POST, .handler = apps_eval_handler, .user_ctx = NULL };
        httpd_uri_t uri_apps_stop = { .uri = "/api/apps/stop", .method = HTTP_POST, .handler = apps_stop_handler, .user_ctx = NULL };
        httpd_uri_t uri_files_list = { .uri = "/api/files/list", .method = HTTP_GET, .handler = files_list_handler, .user_ctx = NULL };
        httpd_uri_t uri_files_delete = { .uri = "/api/files/delete", .method = HTTP_POST, .handler = files_delete_handler, .user_ctx = NULL };

        httpd_register_uri_handler(server, &uri_dash);
        httpd_register_uri_handler(server, &uri_pnp_dev);
        httpd_register_uri_handler(server, &uri_pnp_scan);
        httpd_register_uri_handler(server, &uri_pins_get);
        httpd_register_uri_handler(server, &uri_pins_set);
        httpd_register_uri_handler(server, &uri_robot_mode);
        httpd_register_uri_handler(server, &uri_robot_cmd_vel);
        httpd_register_uri_handler(server, &uri_robot_arm);
        httpd_register_uri_handler(server, &uri_robot_ik);
        httpd_register_uri_handler(server, &uri_robot_stop);
        httpd_register_uri_handler(server, &uri_robot_resume);
        httpd_register_uri_handler(server, &uri_robot_telemetry);
        httpd_register_uri_handler(server, &uri_camera_snap);
        httpd_register_uri_handler(server, &uri_ai_model);
        httpd_register_uri_handler(server, &uri_ai_track);
        httpd_register_uri_handler(server, &uri_ai);
        httpd_register_uri_handler(server, &uri_apps_eval);
        httpd_register_uri_handler(server, &uri_apps_stop);
        httpd_register_uri_handler(server, &uri_files_list);
        httpd_register_uri_handler(server, &uri_files_delete);

        hal_uart_print("[WEB] Universal Robotics, Edge AI, Python IDE & File System endpoints active.\n");
        is_running = true;
    } else {
        hal_uart_print("[WEB] Failed to start HTTP Server!\n");
    }
}

void WebServer::stop() {
    if (!is_running) return;
    
    if (server) {
        httpd_stop(server);
        server = NULL;
        hal_uart_print("[WEB] ESP32 HTTP Server stopped.\n");
    }
    is_running = false;
}

} // namespace TamimysticOS
