#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cstdint>

namespace TamimysticOS {

struct ScriptExecutionResult {
    bool success = true;
    std::string stdout_output;
    std::string error_message;
    uint32_t execution_time_ms = 0;
};

class PythonRunner {
public:
    static PythonRunner& getInstance();

    void init();

    // Execute raw Python code string
    ScriptExecutionResult eval(const std::string& python_code);

    // Execute a Python script file stored in LittleFS
    ScriptExecutionResult runFile(const std::string& filename);

private:
    PythonRunner() = default;
    ~PythonRunner() = default;

    // Line parser and built-in Tamimystic OS API interpreter
    void executeScriptLine(const std::string& line, std::string& stdout_stream);
};

} // namespace TamimysticOS
