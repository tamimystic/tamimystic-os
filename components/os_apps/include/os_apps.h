#pragma once
#include <stdint.h>
#include <string>

namespace TamimysticOS {

class AppManager {
public:
    static AppManager& getInstance();

    // Initialize the Application Manager (and WASM runtime)
    void init();

    // Execute a python or wasm script from the Virtual File System
    bool executeScript(const std::string& filename);

    // Load and execute a WebAssembly application from a memory buffer
    bool loadWasmApp(const std::string& app_name, const uint8_t* wasm_file_buf, uint32_t size);

private:
    AppManager() = default;
    ~AppManager() = default;
};

} // namespace TamimysticOS
