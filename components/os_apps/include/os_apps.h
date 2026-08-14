#pragma once
#include <stdint.h>
#include <string>

namespace TamimysticOS {

class AppManager {
public:
    static AppManager& getInstance();

    // Initialize the Application Manager (and WASM runtime)
    void init();

    // Load and execute a WebAssembly application from a memory buffer
    bool loadWasmApp(const std::string& app_name, const uint8_t* wasm_file_buf, uint32_t size);

private:
    AppManager() = default;
    ~AppManager() = default;
};

} // namespace TamimysticOS
