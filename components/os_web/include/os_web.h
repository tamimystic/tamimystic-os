#pragma once

namespace TamimysticOS {

class WebServer {
public:
    static WebServer& getInstance();

    // Start the HTTP Web Server (dashboard)
    void start();

    // Stop the HTTP Web Server
    void stop();

private:
    WebServer() = default;
    ~WebServer() = default;

    bool is_running = false;
};

} // namespace TamimysticOS
