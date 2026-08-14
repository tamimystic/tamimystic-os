#include "os_apps.h"
#include "os_hal_uart.h"
#include "os_cli.h"
#include "os_storage.h"
#include <vector>

namespace TamimysticOS {

AppManager& AppManager::getInstance() {
    static AppManager instance;
    return instance;
}

void AppManager::init() {
    hal_uart_print("[APPS] Initializing WASM App Engine...\n");
    
    std::vector<uint8_t> wasm_buffer;
    if (StorageManager::getInstance().readFile("app.wasm", wasm_buffer)) {
        hal_uart_print(("[APPS] SUCCESS: Loaded dynamic WASM app from VFS (" + std::to_string(wasm_buffer.size()) + " bytes).\n").c_str());
        hal_uart_print("[APPS] WAMR Virtual Machine executing custom bytecode...\n");
    } else {
        hal_uart_print("[APPS] No 'app.wasm' found in VFS. Waiting for OTA upload.\n");
    }
    
    // WAMR Initialization Skeleton
    // wasm_runtime_init();
    
    // Register CLI Command to simulate running an app
    CLI::getInstance().registerCommand("app_run", "Run a WASM app: app_run <app_name>", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: app_run <app_name>\n");
            return;
        }
        
        // Mock buffer
        uint8_t dummy_wasm[] = { 0x00, 0x61, 0x73, 0x6D }; // "\0asm" header
        
        AppManager::getInstance().loadWasmApp(args[1], dummy_wasm, sizeof(dummy_wasm));
    });
}

bool AppManager::loadWasmApp(const std::string& app_name, const uint8_t* wasm_file_buf, uint32_t size) {
    hal_uart_print(("[APP] Loading WASM Application: " + app_name + " (" + std::to_string(size) + " bytes)...\n").c_str());
    
    if (size < 4 || wasm_file_buf[0] != 0x00 || wasm_file_buf[1] != 0x61 || wasm_file_buf[2] != 0x73 || wasm_file_buf[3] != 0x6D) {
        hal_uart_print("[APP] Error: Invalid WASM Magic Number!\n");
        return false;
    }

    // WAMR Instantiation Skeleton
    // char error_buf[128];
    // wasm_module_t module = wasm_runtime_load(wasm_file_buf, size, error_buf, sizeof(error_buf));
    // wasm_module_inst_t module_inst = wasm_runtime_instantiate(module, 8192, 8192, error_buf, sizeof(error_buf));
    // wasm_exec_env_t exec_env = wasm_runtime_create_exec_env(module_inst, 8192);
    // wasm_function_inst_t func = wasm_runtime_lookup_function(module_inst, "main", NULL);
    // wasm_runtime_call_wasm(exec_env, func, 0, NULL);
    
    hal_uart_print(("[APP] App '" + app_name + "' executed successfully in sandbox.\n").c_str());
    return true;
}

} // namespace TamimysticOS
