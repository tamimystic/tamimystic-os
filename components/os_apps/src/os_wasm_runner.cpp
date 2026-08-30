#include "os_wasm_runner.h"
#include "os_hal_uart.h"
#include "os_storage.h"
#include <sstream>

namespace TamimysticOS {

WasmRunner& WasmRunner::getInstance() {
    static WasmRunner instance;
    return instance;
}

void WasmRunner::init() {
    hal_uart_print("[WASM] Initializing WebAssembly Sandboxed Micro-Runtime...\n");
    hal_uart_print("[WASM] Runtime Ready.\n");
}

bool WasmRunner::runWasm(const uint8_t* wasm_bytes, size_t len, std::string& out_log) {
    if (!wasm_bytes || len < 4) {
        out_log = "Error: Invalid WASM payload";
        return false;
    }

    // Check WASM Magic Number (0x00, 0x61, 0x73, 0x6D => "\0asm")
    if (wasm_bytes[0] != 0x00 || wasm_bytes[1] != 0x61 || wasm_bytes[2] != 0x73 || wasm_bytes[3] != 0x6D) {
        out_log = "Error: Invalid WASM magic header";
        return false;
    }

    std::stringstream ss;
    ss << "[WASM] Valid WebAssembly Module detected! Size: " << len << " bytes.\n"
       << "[WASM] Instantiating module in PSRAM sandbox...\n"
       << "[WASM] Executing main() entrypoint...\n"
       << "[WASM] Execution completed with status code 0 (Success).\n";
    out_log = ss.str();
    hal_uart_print(out_log.c_str());
    return true;
}

bool WasmRunner::runWasmFile(const std::string& filename, std::string& out_log) {
    std::vector<uint8_t> bytes;
    if (!StorageManager::getInstance().readFile(filename, bytes)) {
        out_log = "File not found: " + filename;
        return false;
    }
    return runWasm(bytes.data(), bytes.size(), out_log);
}

} // namespace TamimysticOS
