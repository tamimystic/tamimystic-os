#include "os_web.h"
#include "os_hal_uart.h"
#include "esp_http_server.h"

namespace TamimysticOS {

static httpd_handle_t server = NULL;

WebServer& WebServer::getInstance() {
    static WebServer instance;
    return instance;
}

// Handler for the root URL
static esp_err_t dashboard_get_handler(httpd_req_t *req) {
    const char* resp_str = "<html><head><title>Tamimystic OS Dashboard</title></head>"
                           "<body style='font-family:sans-serif; text-align:center; padding:50px;'>"
                           "<h1>Welcome to Tamimystic OS!</h1>"
                           "<p>Your ESP32-S3 Micro-OS is running smoothly.</p>"
                           "</body></html>";
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t dashboard_uri = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = dashboard_get_handler,
    .user_ctx  = NULL
};

void WebServer::start() {
    if (is_running) return;
    
    hal_uart_print("[WEB] Starting ESP32 HTTP Server...\n");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;
    
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &dashboard_uri);
        hal_uart_print("[WEB] Web Dashboard is now available.\n");
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
