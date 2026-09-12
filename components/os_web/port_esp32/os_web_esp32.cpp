#include "os_web.h"
#include "os_hal_uart.h"
#include "os_robotics.h"
#include "os_ai.h"
#include "os_camera.h"
#include "os_storage.h"
#include "os_apps.h"
#include "os_pnp_manager.h"
#include "os_pin_matrix.h"
#include "os_ros2.h"
#include "os_slam.h"
#include "os_audio.h"
#include "os_espnow.h"
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

// Handlers for micro-ROS & ROS 2
static esp_err_t ros2_status_handler(httpd_req_t *req) {
    std::string json = Ros2Node::getInstance().getStatusJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t ros2_connect_handler(httpd_req_t *req) {
    char query[128];
    char ip_str[32] = "192.168.1.100";
    char port_str[16] = "8888";
    char dom_str[8] = "0";

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "ip", ip_str, sizeof(ip_str));
        httpd_query_key_value(query, "port", port_str, sizeof(port_str));
        httpd_query_key_value(query, "domain", dom_str, sizeof(dom_str));
    }
    uint16_t port = (uint16_t)std::atoi(port_str);
    uint8_t domain = (uint8_t)std::atoi(dom_str);
    Ros2Node::getInstance().connect(ip_str, port, domain);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t ros2_disconnect_handler(httpd_req_t *req) {
    Ros2Node::getInstance().disconnect();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ================= SLAM Handlers =================
static esp_err_t slam_status_handler(httpd_req_t *req) {
    std::string json = SlamEngine::getInstance().getStatusJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t slam_map_handler(httpd_req_t *req) {
    std::string json = SlamEngine::getInstance().getCompressedMapJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t slam_nav_handler(httpd_req_t *req) {
    char query[128];
    char x_str[16] = {0};
    char y_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "x", x_str, sizeof(x_str));
        httpd_query_key_value(query, "y", y_str, sizeof(y_str));
    }
    if (x_str[0] && y_str[0]) {
        float x = (float)std::atof(x_str);
        float y = (float)std::atof(y_str);
        bool ok = SlamEngine::getInstance().setNavigationGoal(x, y);
        httpd_resp_set_type(req, "application/json");
        if (ok) {
            httpd_resp_send(req, "{\"status\":\"ok\",\"navigating\":true}", HTTPD_RESP_USE_STRLEN);
        } else {
            httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Target unreachable\"}", HTTPD_RESP_USE_STRLEN);
        }
        return ESP_OK;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Missing x or y\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t slam_clear_handler(httpd_req_t *req) {
    SlamEngine::getInstance().clearMap();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t slam_cancel_handler(httpd_req_t *req) {
    SlamEngine::getInstance().cancelNavigation();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t slam_lidar_handler(httpd_req_t *req) {
    char query[64];
    char type_str[16] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "type", type_str, sizeof(type_str));
    }
    std::string t(type_str);
    if (t == "rplidar") SlamEngine::getInstance().setLidarType(LidarType::RPLIDAR_A1_A2);
    else if (t == "ld19" || t == "ld06") SlamEngine::getInstance().setLidarType(LidarType::LD19_D300);
    else SlamEngine::getInstance().setLidarType(LidarType::SIMULATED_360);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// ================= Audio Handlers =================
static esp_err_t audio_status_handler(httpd_req_t *req) {
    std::string json = AudioEngine::getInstance().getStatusJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t audio_waveform_handler(httpd_req_t *req) {
    std::string json = AudioEngine::getInstance().getWaveformJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t audio_say_handler(httpd_req_t *req) {
    char query[128];
    char text_str[96] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "text", text_str, sizeof(text_str));
    }
    if (text_str[0]) {
        AudioEngine::getInstance().speak(text_str);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Missing text\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t audio_tone_handler(httpd_req_t *req) {
    char query[64];
    char freq_str[16] = "440";
    char dur_str[16] = "100";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "freq", freq_str, sizeof(freq_str));
        httpd_query_key_value(query, "dur", dur_str, sizeof(dur_str));
    }
    uint16_t freq = (uint16_t)std::atoi(freq_str);
    uint16_t dur = (uint16_t)std::atoi(dur_str);
    AudioEngine::getInstance().playTone(freq, dur);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t audio_beep_handler(httpd_req_t *req) {
    char query[32];
    char pat_str[8] = "1";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "pattern", pat_str, sizeof(pat_str));
    }
    int pat = std::atoi(pat_str);
    AudioEngine::getInstance().playBeepPattern(pat);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t audio_kws_handler(httpd_req_t *req) {
    char query[32];
    char en_str[8] = "1";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "enable", en_str, sizeof(en_str));
    }
    bool en = (en_str[0] == '1' || en_str[0] == 't');
    AudioEngine::getInstance().setKwsEnabled(en);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t audio_cmd_handler(httpd_req_t *req) {
    char query[64];
    char cmd_str[32] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "cmd", cmd_str, sizeof(cmd_str));
    }
    if (cmd_str[0]) {
        AudioEngine::getInstance().triggerKwsTest(cmd_str);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Missing cmd\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t audio_vol_handler(httpd_req_t *req) {
    char query[32];
    char vol_str[8] = "80";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "vol", vol_str, sizeof(vol_str));
    }
    uint8_t vol = (uint8_t)std::atoi(vol_str);
    AudioEngine::getInstance().setVolume(vol);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t espnow_status_handler(httpd_req_t *req) {
    std::string json = EspNowEngine::getInstance().getStatusJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t espnow_peers_handler(httpd_req_t *req) {
    std::string json = EspNowEngine::getInstance().getPeersJson();
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json.c_str(), json.length());
    return ESP_OK;
}

static esp_err_t espnow_swarm_handler(httpd_req_t *req) {
    char query[96];
    char role_str[16] = {0};
    char form_str[16] = {0};
    char slot_str[8] = "0";
    char sp_str[16] = "60.0";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "role", role_str, sizeof(role_str));
        httpd_query_key_value(query, "formation", form_str, sizeof(form_str));
        httpd_query_key_value(query, "slot", slot_str, sizeof(slot_str));
        httpd_query_key_value(query, "spacing", sp_str, sizeof(sp_str));
    }
    uint8_t slot = (uint8_t)std::atoi(slot_str);
    float spacing = (float)std::atof(sp_str);

    if (role_str[0]) {
        SwarmRole role = SwarmRole::STANDALONE;
        if (std::strcmp(role_str, "leader") == 0) role = SwarmRole::LEADER;
        else if (std::strcmp(role_str, "follower") == 0) role = SwarmRole::FOLLOWER;
        EspNowEngine::getInstance().setSwarmRole(role, slot, spacing);
    }
    if (form_str[0]) {
        SwarmFormation f = SwarmFormation::TRIANGLE;
        if (std::strcmp(form_str, "line") == 0) f = SwarmFormation::LINE;
        else if (std::strcmp(form_str, "column") == 0) f = SwarmFormation::COLUMN;
        else if (std::strcmp(form_str, "diamond") == 0) f = SwarmFormation::DIAMOND;
        EspNowEngine::getInstance().setFormation(f, spacing);
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t espnow_remote_handler(httpd_req_t *req) {
    char query[32];
    char en_str[8] = "1";
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "enable", en_str, sizeof(en_str));
    }
    bool en = (en_str[0] == '1' || en_str[0] == 't');
    EspNowEngine::getInstance().setRemoteControlEnabled(en);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t espnow_send_handler(httpd_req_t *req) {
    char query[128];
    char mac_str[24] = {0};
    char msg_str[64] = {0};
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        httpd_query_key_value(query, "mac", mac_str, sizeof(mac_str));
        httpd_query_key_value(query, "msg", msg_str, sizeof(msg_str));
    }
    if (mac_str[0] && msg_str[0]) {
        bool ok = EspNowEngine::getInstance().sendCustomPayload(mac_str, msg_str);
        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, ok ? "{\"status\":\"ok\"}" : "{\"status\":\"error\"}", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, "{\"status\":\"error\",\"message\":\"Missing mac or msg\"}", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void WebServer::start() {
    if (is_running) return;
    
    hal_uart_print("[WEB] Starting ESP32 HTTP Server on Port 80...\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 70;
    
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

        httpd_uri_t uri_ros2_status = { .uri = "/api/ros2/status", .method = HTTP_GET, .handler = ros2_status_handler, .user_ctx = NULL };
        httpd_uri_t uri_ros2_connect = { .uri = "/api/ros2/connect", .method = HTTP_POST, .handler = ros2_connect_handler, .user_ctx = NULL };
        httpd_uri_t uri_ros2_disconnect = { .uri = "/api/ros2/disconnect", .method = HTTP_POST, .handler = ros2_disconnect_handler, .user_ctx = NULL };

        httpd_uri_t uri_slam_status = { .uri = "/api/slam/status", .method = HTTP_GET, .handler = slam_status_handler, .user_ctx = NULL };
        httpd_uri_t uri_slam_map = { .uri = "/api/slam/map", .method = HTTP_GET, .handler = slam_map_handler, .user_ctx = NULL };
        httpd_uri_t uri_slam_nav = { .uri = "/api/slam/nav", .method = HTTP_POST, .handler = slam_nav_handler, .user_ctx = NULL };
        httpd_uri_t uri_slam_clear = { .uri = "/api/slam/clear", .method = HTTP_POST, .handler = slam_clear_handler, .user_ctx = NULL };
        httpd_uri_t uri_slam_cancel = { .uri = "/api/slam/cancel", .method = HTTP_POST, .handler = slam_cancel_handler, .user_ctx = NULL };
        httpd_uri_t uri_slam_lidar = { .uri = "/api/slam/lidar", .method = HTTP_POST, .handler = slam_lidar_handler, .user_ctx = NULL };

        httpd_uri_t uri_audio_status = { .uri = "/api/audio/status", .method = HTTP_GET, .handler = audio_status_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_wave = { .uri = "/api/audio/waveform", .method = HTTP_GET, .handler = audio_waveform_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_say = { .uri = "/api/audio/say", .method = HTTP_POST, .handler = audio_say_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_tone = { .uri = "/api/audio/tone", .method = HTTP_POST, .handler = audio_tone_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_beep = { .uri = "/api/audio/beep", .method = HTTP_POST, .handler = audio_beep_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_kws = { .uri = "/api/audio/kws", .method = HTTP_POST, .handler = audio_kws_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_cmd = { .uri = "/api/audio/cmd", .method = HTTP_POST, .handler = audio_cmd_handler, .user_ctx = NULL };
        httpd_uri_t uri_audio_vol = { .uri = "/api/audio/volume", .method = HTTP_POST, .handler = audio_vol_handler, .user_ctx = NULL };

        httpd_uri_t uri_espnow_status = { .uri = "/api/espnow/status", .method = HTTP_GET, .handler = espnow_status_handler, .user_ctx = NULL };
        httpd_uri_t uri_espnow_peers = { .uri = "/api/espnow/peers", .method = HTTP_GET, .handler = espnow_peers_handler, .user_ctx = NULL };
        httpd_uri_t uri_espnow_swarm = { .uri = "/api/espnow/swarm", .method = HTTP_POST, .handler = espnow_swarm_handler, .user_ctx = NULL };
        httpd_uri_t uri_espnow_remote = { .uri = "/api/espnow/remote", .method = HTTP_POST, .handler = espnow_remote_handler, .user_ctx = NULL };
        httpd_uri_t uri_espnow_send = { .uri = "/api/espnow/send", .method = HTTP_POST, .handler = espnow_send_handler, .user_ctx = NULL };

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
        httpd_register_uri_handler(server, &uri_ros2_status);
        httpd_register_uri_handler(server, &uri_ros2_connect);
        httpd_register_uri_handler(server, &uri_ros2_disconnect);
        httpd_register_uri_handler(server, &uri_slam_status);
        httpd_register_uri_handler(server, &uri_slam_map);
        httpd_register_uri_handler(server, &uri_slam_nav);
        httpd_register_uri_handler(server, &uri_slam_clear);
        httpd_register_uri_handler(server, &uri_slam_cancel);
        httpd_register_uri_handler(server, &uri_slam_lidar);
        httpd_register_uri_handler(server, &uri_audio_status);
        httpd_register_uri_handler(server, &uri_audio_wave);
        httpd_register_uri_handler(server, &uri_audio_say);
        httpd_register_uri_handler(server, &uri_audio_tone);
        httpd_register_uri_handler(server, &uri_audio_beep);
        httpd_register_uri_handler(server, &uri_audio_kws);
        httpd_register_uri_handler(server, &uri_audio_cmd);
        httpd_register_uri_handler(server, &uri_audio_vol);
        httpd_register_uri_handler(server, &uri_espnow_status);
        httpd_register_uri_handler(server, &uri_espnow_peers);
        httpd_register_uri_handler(server, &uri_espnow_swarm);
        httpd_register_uri_handler(server, &uri_espnow_remote);
        httpd_register_uri_handler(server, &uri_espnow_send);

        hal_uart_print("[WEB] Universal Robotics, Edge AI, Python IDE, ROS2, SLAM, Audio & ESP-NOW endpoints active.\n");
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
