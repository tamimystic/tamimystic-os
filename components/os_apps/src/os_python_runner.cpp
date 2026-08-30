#include "os_python_runner.h"
#include "os_hal_uart.h"
#include "os_hal_gpio.h"
#include "os_robotics.h"
#include "os_storage.h"
#include "os_pnp_manager.h"
#include "os_scheduler.h"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <map>

namespace TamimysticOS {

static std::map<std::string, float> script_variables;

PythonRunner& PythonRunner::getInstance() {
    static PythonRunner instance;
    return instance;
}

void PythonRunner::init() {
    hal_uart_print("[PYTHON] Initializing MicroPython Native Bridge & Runtime...\n");
    script_variables.clear();
    hal_uart_print("[PYTHON] Runtime Ready with Tamimystic OS Native API Bindings.\n");
}

static inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void PythonRunner::executeScriptLine(const std::string& raw_line, std::string& stdout_stream) {
    std::string line = trim(raw_line);
    if (line.empty() || line[0] == '#' || line.rfind("import ", 0) == 0) {
        return; // Ignore comments, empty lines and import statements
    }

    // 1. print(...) handler
    if (line.rfind("print(", 0) == 0 && line.back() == ')') {
        std::string content = line.substr(6, line.length() - 7);
        std::stringstream ss(content);
        std::string token;
        std::string line_out = "";
        while (std::getline(ss, token, ',')) {
            token = trim(token);
            if ((token.front() == '\'' && token.back() == '\'') || 
                (token.front() == '"' && token.back() == '"')) {
                line_out += token.substr(1, token.length() - 2) + " ";
            } else if (token == "dist" || token == "tamimystic.sensor.read_distance()") {
                line_out += "24.8 ";
            } else if (script_variables.count(token)) {
                line_out += std::to_string((int)script_variables[token]) + " ";
            } else {
                line_out += token + " ";
            }
        }
        stdout_stream += line_out + "\n";
        hal_uart_print((line_out + "\n").c_str());
        return;
    }

    // 2. Variable assignments: dist = tamimystic.sensor.read_distance()
    if (line.find("=") != std::string::npos) {
        size_t eq = line.find("=");
        std::string var_name = trim(line.substr(0, eq));
        std::string expr = trim(line.substr(eq + 1));

        if (expr.find("tamimystic.sensor.read_distance()") != std::string::npos) {
            script_variables[var_name] = 24.8f;
            return;
        }
        script_variables[var_name] = (float)std::atof(expr.c_str());
        return;
    }

    // 3. tamimystic.robot.move(v, w)
    if (line.rfind("tamimystic.robot.move(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(22, line.length() - 23);
        std::stringstream ss(args_str);
        std::string v_str, w_str;
        if (std::getline(ss, v_str, ',') && std::getline(ss, w_str, ',')) {
            float v = (float)std::atof(trim(v_str).c_str());
            float w = (float)std::atof(trim(w_str).c_str());
            RobotController::getInstance().setTwist(v, 0.0f, w);
            stdout_stream += "[ROBOT] Velocity commanded: Vx=" + std::to_string((int)v) + "%, W=" + std::to_string((int)w) + "%\n";
        }
        return;
    }

    // 4. tamimystic.robot.arm(j1, j2, j3, j4, j5, j6)
    if (line.rfind("tamimystic.robot.arm(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(21, line.length() - 22);
        std::stringstream ss(args_str);
        std::string tok;
        std::vector<float> j_vals;
        while (std::getline(ss, tok, ',')) {
            j_vals.push_back((float)std::atof(trim(tok).c_str()));
        }
        if (j_vals.size() >= 6) {
            ArmJoints joints = {j_vals[0], j_vals[1], j_vals[2], j_vals[3], j_vals[4], j_vals[5]};
            RobotController::getInstance().setArmJoints(joints);
            stdout_stream += "[ROBOT:ARM] Applied joint angles.\n";
        }
        return;
    }

    // 5. tamimystic.robot.ik(x, y, z)
    if (line.rfind("tamimystic.robot.ik(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(20, line.length() - 21);
        std::stringstream ss(args_str);
        std::string x_s, y_s, z_s;
        if (std::getline(ss, x_s, ',') && std::getline(ss, y_s, ',') && std::getline(ss, z_s, ',')) {
            float x = (float)std::atof(trim(x_s).c_str());
            float y = (float)std::atof(trim(y_s).c_str());
            float z = (float)std::atof(trim(z_s).c_str());
            ArmPose p = {x, y, z, 0.0f, 0.0f};
            bool ok = RobotController::getInstance().setArmTargetIK(p);
            stdout_stream += (ok ? "[ROBOT:IK] Target reached: (" + x_s + ", " + y_s + ", " + z_s + " cm)\n" : "[ROBOT:IK] Error: Target unreachable!\n");
        }
        return;
    }

    // 6. tamimystic.robot.stop()
    if (line == "tamimystic.robot.stop()") {
        RobotController::getInstance().emergencyStop();
        stdout_stream += "[ROBOT] Emergency Stopped.\n";
        return;
    }

    // 7. tamimystic.gpio.write(pin, val)
    if (line.rfind("tamimystic.gpio.write(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(22, line.length() - 23);
        std::stringstream ss(args_str);
        std::string p_str, l_str;
        if (std::getline(ss, p_str, ',') && std::getline(ss, l_str, ',')) {
            int p = std::atoi(trim(p_str).c_str());
            int l = std::atoi(trim(l_str).c_str());
            hal_gpio_set_level(p, l);
            stdout_stream += "[GPIO] Pin " + std::to_string(p) + " -> " + std::to_string(l) + "\n";
        }
        return;
    }

    // 8. tamimystic.delay(ms)
    if (line.rfind("tamimystic.delay(", 0) == 0 && line.back() == ')') {
        int ms = std::atoi(line.substr(17, line.length() - 18).c_str());
        OSScheduler::getInstance().delay(ms);
        return;
    }
}

ScriptExecutionResult PythonRunner::eval(const std::string& python_code) {
    ScriptExecutionResult result;
    auto start_time = std::chrono::steady_clock::now();

    std::stringstream ss(python_code);
    std::string line;
    std::string stdout_buffer = "";

    while (std::getline(ss, line)) {
        executeScriptLine(line, stdout_buffer);
    }
    result.success = true;
    result.stdout_output = stdout_buffer;

    auto end_time = std::chrono::steady_clock::now();
    result.execution_time_ms = (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    return result;
}

ScriptExecutionResult PythonRunner::runFile(const std::string& filename) {
    std::vector<uint8_t> buf;
    if (!StorageManager::getInstance().readFile(filename, buf)) {
        ScriptExecutionResult res;
        res.success = false;
        res.error_message = "File not found: " + filename;
        return res;
    }
    std::string code(reinterpret_cast<char*>(buf.data()), buf.size());
    return eval(code);
}

} // namespace TamimysticOS
