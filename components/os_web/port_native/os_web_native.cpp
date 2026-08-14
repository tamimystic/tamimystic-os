#include "os_web.h"
#include "os_hal_uart.h"
#include "os_robotics.h"
#include "os_ai.h"
#include "os_storage.h"
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

        // API for AI status
        svr->Get("/api/ai/status", [](const httplib::Request& req, httplib::Response& res) {
            std::string ai_json = AIModule::getInstance().getLatestDetection();
            res.set_content(ai_json, "application/json");
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
