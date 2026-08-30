#pragma once

#include "os_python_runner.h"
#include "os_wasm_runner.h"
#include <string>

namespace TamimysticOS {

class AppManager {
public:
    static AppManager& getInstance();

    // Initialize App subsystem & check for autorun.py on boot
    void init();

    // Run Python code string
    ScriptExecutionResult evalCode(const std::string& code);

    // Run script file from LittleFS (e.g. "robot_patrol.py" or "app.wasm")
    ScriptExecutionResult runFile(const std::string& filename);

    // Stop current execution
    void stopCurrentApp();

    bool isRunning() const;

private:
    AppManager() = default;
    ~AppManager() = default;

    bool app_running = false;
};

} // namespace TamimysticOS
