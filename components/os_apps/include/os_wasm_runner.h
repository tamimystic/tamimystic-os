#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace TamimysticOS {

class WasmRunner {
public:
    static WasmRunner& getInstance();

    void init();

    // Execute compiled WebAssembly bytecode
    bool runWasm(const uint8_t* wasm_bytes, size_t len, std::string& out_log);

    // Execute a .wasm binary file from flash storage
    bool runWasmFile(const std::string& filename, std::string& out_log);

private:
    WasmRunner() = default;
    ~WasmRunner() = default;
};

} // namespace TamimysticOS
