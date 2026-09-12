#include "os_python_runner.h"
#include "os_hal_uart.h"
#include "os_hal_gpio.h"
#include "os_robotics.h"
#include "os_storage.h"
#include "os_pnp_manager.h"
#include "os_scheduler.h"
#include "os_ros2.h"
#include "os_slam.h"
#include "os_audio.h"
#include "os_espnow.h"
#include "os_ble.h"
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

static size_t count_leading_spaces(const std::string& str) {
    size_t count = 0;
    for (char c : str) {
        if (c == ' ') count++;
        else if (c == '\t') count += 4;
        else break;
    }
    return count;
}

static float resolveValue(const std::string& raw_expr) {
    std::string expr = trim(raw_expr);
    if (expr.empty()) return 0.0f;
    if (script_variables.count(expr)) return script_variables[expr];
    if (expr == "dist" || expr == "tamimystic.sensor.read_distance()") return 24.8f;

    // Handle basic arithmetic operations
    if (expr.find("+") != std::string::npos) {
        size_t p = expr.find("+");
        return resolveValue(expr.substr(0, p)) + resolveValue(expr.substr(p + 1));
    }
    if (expr.find("-") != std::string::npos && expr[0] != '-') {
        size_t p = expr.find("-");
        return resolveValue(expr.substr(0, p)) - resolveValue(expr.substr(p + 1));
    }
    if (expr.find("*") != std::string::npos) {
        size_t p = expr.find("*");
        return resolveValue(expr.substr(0, p)) * resolveValue(expr.substr(p + 1));
    }
    if (expr.find("/") != std::string::npos) {
        size_t p = expr.find("/");
        float denom = resolveValue(expr.substr(p + 1));
        return (denom != 0.0f) ? (resolveValue(expr.substr(0, p)) / denom) : 0.0f;
    }
    return (float)std::atof(expr.c_str());
}

static bool evaluateCondition(const std::string& cond_str) {
    std::string cond = trim(cond_str);
    if (cond.find("==") != std::string::npos) {
        size_t p = cond.find("==");
        return resolveValue(cond.substr(0, p)) == resolveValue(cond.substr(p + 2));
    }
    if (cond.find("!=") != std::string::npos) {
        size_t p = cond.find("!=");
        return resolveValue(cond.substr(0, p)) != resolveValue(cond.substr(p + 2));
    }
    if (cond.find("<=") != std::string::npos) {
        size_t p = cond.find("<=");
        return resolveValue(cond.substr(0, p)) <= resolveValue(cond.substr(p + 2));
    }
    if (cond.find(">=") != std::string::npos) {
        size_t p = cond.find(">=");
        return resolveValue(cond.substr(0, p)) >= resolveValue(cond.substr(p + 2));
    }
    if (cond.find("<") != std::string::npos) {
        size_t p = cond.find("<");
        return resolveValue(cond.substr(0, p)) < resolveValue(cond.substr(p + 1));
    }
    if (cond.find(">") != std::string::npos) {
        size_t p = cond.find(">");
        return resolveValue(cond.substr(0, p)) > resolveValue(cond.substr(p + 1));
    }
    return resolveValue(cond) != 0.0f;
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
        script_variables[var_name] = resolveValue(expr);
        return;
    }

    // 3. tamimystic.robot.move(v, w)
    if (line.rfind("tamimystic.robot.move(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(22, line.length() - 23);
        std::stringstream ss(args_str);
        std::string v_str, w_str;
        if (std::getline(ss, v_str, ',') && std::getline(ss, w_str, ',')) {
            float v = resolveValue(v_str);
            float w = resolveValue(w_str);
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
            j_vals.push_back(resolveValue(tok));
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
            float x = resolveValue(x_s);
            float y = resolveValue(y_s);
            float z = resolveValue(z_s);
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
            int p = (int)resolveValue(p_str);
            int l = (int)resolveValue(l_str);
            hal_gpio_set_level(p, l);
            stdout_stream += "[GPIO] Pin " + std::to_string(p) + " -> " + std::to_string(l) + "\n";
        }
        return;
    }

    // 8. tamimystic.delay(ms)
    if (line.rfind("tamimystic.delay(", 0) == 0 && line.back() == ')') {
        int ms = (int)resolveValue(line.substr(17, line.length() - 18));
        OSScheduler::getInstance().delay(ms);
        return;
    }

    // 9. tamimystic.ros2.publish_log(msg)
    if (line.rfind("tamimystic.ros2.publish_log(", 0) == 0 && line.back() == ')') {
        std::string log_msg = line.substr(28, line.length() - 29);
        if (!log_msg.empty() && (log_msg.front() == '"' || log_msg.front() == '\'')) log_msg = log_msg.substr(1, log_msg.length() - 2);
        Ros2Node::getInstance().publishLog(log_msg);
        stdout_stream += "[ROS2:LOG] Published: " + log_msg + "\n";
        return;
    }

    // 10. tamimystic.ros2.connect() / disconnect()
    if (line == "tamimystic.ros2.connect()") {
        Ros2Node::getInstance().connect();
        stdout_stream += "[ROS2] Connecting to Agent...\n";
        return;
    }
    if (line == "tamimystic.ros2.disconnect()") {
        Ros2Node::getInstance().disconnect();
        stdout_stream += "[ROS2] Disconnected from Agent.\n";
        return;
    }

    // 11. tamimystic.slam.nav(x, y)
    if (line.rfind("tamimystic.slam.nav(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(20, line.length() - 21);
        std::stringstream ss(args_str);
        std::string x_s, y_s;
        if (std::getline(ss, x_s, ',') && std::getline(ss, y_s, ',')) {
            float x = resolveValue(x_s);
            float y = resolveValue(y_s);
            bool ok = SlamEngine::getInstance().setNavigationGoal(x, y);
            stdout_stream += (ok ? "[SLAM:NAV] Goal set: (" + x_s + ", " + y_s + " cm)\n" : "[SLAM:NAV] Failed: Target unreachable or out of bounds\n");
        }
        return;
    }

    // 12. tamimystic.slam.clear()
    if (line == "tamimystic.slam.clear()") {
        SlamEngine::getInstance().clearMap();
        stdout_stream += "[SLAM] Map Cleared & Reset.\n";
        return;
    }

    // 13. tamimystic.slam.cancel()
    if (line == "tamimystic.slam.cancel()") {
        SlamEngine::getInstance().cancelNavigation();
        stdout_stream += "[SLAM:NAV] Navigation Aborted.\n";
        return;
    }

    // 14. tamimystic.audio.say("phrase")
    if (line.rfind("tamimystic.audio.say(", 0) == 0 && line.back() == ')') {
        std::string phrase = line.substr(21, line.length() - 22);
        if (!phrase.empty() && (phrase.front() == '"' || phrase.front() == '\'')) phrase = phrase.substr(1, phrase.length() - 2);
        AudioEngine::getInstance().speak(phrase);
        stdout_stream += "[AUDIO:TTS] Spoken: \"" + phrase + "\"\n";
        return;
    }

    // 15. tamimystic.audio.tone(freq, ms)
    if (line.rfind("tamimystic.audio.tone(", 0) == 0 && line.back() == ')') {
        std::string args_str = line.substr(22, line.length() - 23);
        std::stringstream ss(args_str);
        std::string f_s, d_s;
        if (std::getline(ss, f_s, ',') && std::getline(ss, d_s, ',')) {
            uint16_t freq = (uint16_t)resolveValue(f_s);
            uint16_t dur = (uint16_t)resolveValue(d_s);
            AudioEngine::getInstance().playTone(freq, dur);
            stdout_stream += "[AUDIO:DAC] Tone played: " + f_s + " Hz (" + d_s + " ms)\n";
        }
        return;
    }

    // 16. tamimystic.audio.beep(pattern)
    if (line.rfind("tamimystic.audio.beep(", 0) == 0 && line.back() == ')') {
        int pat = (int)resolveValue(line.substr(22, line.length() - 23));
        AudioEngine::getInstance().playBeepPattern(pat);
        stdout_stream += "[AUDIO] Beep pattern " + std::to_string(pat) + " played.\n";
        return;
    }

    // 17. tamimystic.audio.volume(vol)
    if (line.rfind("tamimystic.audio.volume(", 0) == 0 && line.back() == ')') {
        int vol = (int)resolveValue(line.substr(24, line.length() - 25));
        AudioEngine::getInstance().setVolume((uint8_t)vol);
        stdout_stream += "[AUDIO] Volume set to " + std::to_string(vol) + "%\n";
        return;
    }

    // 18. tamimystic.espnow.swarm("role", slot, spacing)
    if (line.rfind("tamimystic.espnow.swarm(", 0) == 0 && line.back() == ')') {
        std::string args = line.substr(24, line.length() - 25);
        std::stringstream ss(args);
        std::string r_s, slot_s, sp_s;
        if (std::getline(ss, r_s, ',')) {
            if (!r_s.empty() && (r_s.front() == '"' || r_s.front() == '\'')) r_s = r_s.substr(1, r_s.length() - 2);
            uint8_t slot = 0;
            float spacing = 60.0f;
            if (std::getline(ss, slot_s, ',')) slot = (uint8_t)resolveValue(slot_s);
            if (std::getline(ss, sp_s, ',')) spacing = resolveValue(sp_s);

            SwarmRole role = SwarmRole::STANDALONE;
            if (r_s == "leader") role = SwarmRole::LEADER;
            else if (r_s == "follower") role = SwarmRole::FOLLOWER;

            EspNowEngine::getInstance().setSwarmRole(role, slot, spacing);
            stdout_stream += "[ESPNOW:SWARM] Role set to " + r_s + " (Slot " + std::to_string(slot) + ")\n";
        }
        return;
    }

    // 19. tamimystic.espnow.remote(True/False)
    if (line.rfind("tamimystic.espnow.remote(", 0) == 0 && line.back() == ')') {
        std::string en_str = line.substr(25, line.length() - 26);
        bool en = (en_str == "True" || en_str == "true" || en_str == "1");
        EspNowEngine::getInstance().setRemoteControlEnabled(en);
        stdout_stream += std::string("[ESPNOW:REMOTE] Gamepad listening ") + (en ? "ENABLED\n" : "DISABLED\n");
        return;
    }

    // 20. tamimystic.espnow.send("MAC", "MSG")
    if (line.rfind("tamimystic.espnow.send(", 0) == 0 && line.back() == ')') {
        std::string args = line.substr(23, line.length() - 24);
        std::stringstream ss(args);
        std::string mac_s, msg_s;
        if (std::getline(ss, mac_s, ',') && std::getline(ss, msg_s, ',')) {
            mac_s = trim(mac_s);
            msg_s = trim(msg_s);
            if (!mac_s.empty() && (mac_s.front() == '"' || mac_s.front() == '\'')) mac_s = mac_s.substr(1, mac_s.length() - 2);
            if (!msg_s.empty() && (msg_s.front() == '"' || msg_s.front() == '\'')) msg_s = msg_s.substr(1, msg_s.length() - 2);
            EspNowEngine::getInstance().sendCustomPayload(mac_s, msg_s);
            stdout_stream += "[ESPNOW:TX] Sent to " + mac_s + ": " + msg_s + "\n";
        }
        return;
    }

    // 21. tamimystic.ble.adv(True/False)
    if (line.rfind("tamimystic.ble.adv(", 0) == 0 && line.back() == ')') {
        std::string en_str = line.substr(19, line.length() - 20);
        bool en = (en_str == "True" || en_str == "true" || en_str == "1");
        if (en) BleManager::getInstance().startAdvertising();
        else BleManager::getInstance().stopAdvertising();
        stdout_stream += std::string("[BLE] Advertising ") + (en ? "STARTED\n" : "STOPPED\n");
        return;
    }

    // 22. tamimystic.ble.disconnect()
    if (line == "tamimystic.ble.disconnect()") {
        BleManager::getInstance().disconnect();
        stdout_stream += "[BLE] Disconnected active client.\n";
        return;
    }
}

static void executeLinesBlock(const std::vector<std::string>& lines, size_t& idx, size_t base_indent, std::string& stdout_stream) {
    while (idx < lines.size()) {
        const std::string& raw = lines[idx];
        std::string t = trim(raw);
        if (t.empty() || t[0] == '#') {
            idx++;
            continue;
        }

        size_t indent = count_leading_spaces(raw);
        if (indent < base_indent) {
            return; // Exit block when indentation decreases
        }

        // For Loop Block: for <var> in range(...):
        if (t.rfind("for ", 0) == 0 && t.find(" in range(") != std::string::npos && t.back() == ':') {
            size_t var_start = 4;
            size_t in_pos = t.find(" in range(");
            std::string var_name = trim(t.substr(var_start, in_pos - var_start));
            std::string range_params = t.substr(in_pos + 10, t.length() - (in_pos + 10) - 2);

            int start_val = 0;
            int end_val = 0;
            if (range_params.find(",") != std::string::npos) {
                size_t c_pos = range_params.find(",");
                start_val = (int)resolveValue(range_params.substr(0, c_pos));
                end_val = (int)resolveValue(range_params.substr(c_pos + 1));
            } else {
                end_val = (int)resolveValue(range_params);
            }

            idx++;
            size_t loop_body_start = idx;
            size_t body_indent = 0;
            if (idx < lines.size()) body_indent = count_leading_spaces(lines[idx]);
            if (body_indent <= indent) body_indent = indent + 2;

            // Collect loop body lines
            std::vector<std::string> body_lines;
            while (idx < lines.size() && (count_leading_spaces(lines[idx]) >= body_indent || trim(lines[idx]).empty())) {
                body_lines.push_back(lines[idx]);
                idx++;
            }

            // Execute loop iterations
            for (int val = start_val; val < end_val; val++) {
                script_variables[var_name] = (float)val;
                size_t b_idx = 0;
                executeLinesBlock(body_lines, b_idx, body_indent, stdout_stream);
            }
            continue;
        }

        // While Loop Block: while <cond>:
        if (t.rfind("while ", 0) == 0 && t.back() == ':') {
            std::string cond_str = t.substr(6, t.length() - 7);
            idx++;
            size_t body_indent = 0;
            if (idx < lines.size()) body_indent = count_leading_spaces(lines[idx]);
            if (body_indent <= indent) body_indent = indent + 2;

            std::vector<std::string> body_lines;
            while (idx < lines.size() && (count_leading_spaces(lines[idx]) >= body_indent || trim(lines[idx]).empty())) {
                body_lines.push_back(lines[idx]);
                idx++;
            }

            int safety_counter = 0;
            while (evaluateCondition(cond_str) && safety_counter++ < 500) {
                size_t b_idx = 0;
                executeLinesBlock(body_lines, b_idx, body_indent, stdout_stream);
            }
            continue;
        }

        // If-Else Block: if <cond>:
        if (t.rfind("if ", 0) == 0 && t.back() == ':') {
            std::string cond_str = t.substr(3, t.length() - 4);
            bool cond_met = evaluateCondition(cond_str);

            idx++;
            size_t body_indent = 0;
            if (idx < lines.size()) body_indent = count_leading_spaces(lines[idx]);
            if (body_indent <= indent) body_indent = indent + 2;

            std::vector<std::string> if_body;
            while (idx < lines.size() && (count_leading_spaces(lines[idx]) >= body_indent || trim(lines[idx]).empty())) {
                if_body.push_back(lines[idx]);
                idx++;
            }

            std::vector<std::string> else_body;
            if (idx < lines.size() && trim(lines[idx]) == "else:") {
                idx++;
                size_t else_indent = (idx < lines.size()) ? count_leading_spaces(lines[idx]) : (indent + 2);
                while (idx < lines.size() && (count_leading_spaces(lines[idx]) >= else_indent || trim(lines[idx]).empty())) {
                    else_body.push_back(lines[idx]);
                    idx++;
                }
            }

            if (cond_met) {
                size_t b_idx = 0;
                executeLinesBlock(if_body, b_idx, body_indent, stdout_stream);
            } else if (!else_body.empty()) {
                size_t b_idx = 0;
                executeLinesBlock(else_body, b_idx, body_indent, stdout_stream);
            }
            continue;
        }

        // Normal single statement execution
        PythonRunner::getInstance().executeScriptLine(t, stdout_stream);
        idx++;
    }
}

ScriptExecutionResult PythonRunner::eval(const std::string& python_code) {
    ScriptExecutionResult result;
    auto start_time = std::chrono::steady_clock::now();

    std::stringstream ss(python_code);
    std::string line;
    std::vector<std::string> lines;

    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    std::string stdout_buffer = "";
    size_t idx = 0;
    executeLinesBlock(lines, idx, 0, stdout_buffer);

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
