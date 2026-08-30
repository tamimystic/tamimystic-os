#include "os_apps.h"
#include "os_hal_uart.h"
#include "os_storage.h"
#include "os_robotics.h"

namespace TamimysticOS {

AppManager& AppManager::getInstance() {
    static AppManager instance;
    return instance;
}

void AppManager::init() {
    hal_uart_print("[APPS] Initializing Dynamic Application & Scripting Engine...\n");
    PythonRunner::getInstance().init();
    WasmRunner::getInstance().init();

    // Check if an autorun script exists in storage
    if (StorageManager::getInstance().fileExists("autorun.py")) {
        hal_uart_print("[APPS] Found autorun.py in flash! Executing...\n");
        runFile("autorun.py");
    }

    hal_uart_print("[APPS] Dynamic Apps Subsystem Ready.\n");
}

ScriptExecutionResult AppManager::evalCode(const std::string& code) {
    app_running = true;
    ScriptExecutionResult res = PythonRunner::getInstance().eval(code);
    app_running = false;
    return res;
}

ScriptExecutionResult AppManager::runFile(const std::string& filename) {
    app_running = true;
    ScriptExecutionResult res;

    // Check file extension
    if (filename.length() > 5 && filename.substr(filename.length() - 5) == ".wasm") {
        std::string wasm_log;
        bool ok = WasmRunner::getInstance().runWasmFile(filename, wasm_log);
        res.success = ok;
        res.stdout_output = wasm_log;
    } else {
        res = PythonRunner::getInstance().runFile(filename);
    }

    app_running = false;
    return res;
}

void AppManager::stopCurrentApp() {
    app_running = false;
    RobotController::getInstance().emergencyStop();
    hal_uart_print("[APPS] Active app/script execution stopped.\n");
}

bool AppManager::isRunning() const {
    return app_running;
}

} // namespace TamimysticOS
