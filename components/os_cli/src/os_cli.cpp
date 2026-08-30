#include "os_cli.h"
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include "os_pnp_manager.h"
#include "os_pin_matrix.h"
#include "os_robotics.h"
#include "os_ai.h"
#include "os_camera.h"
#include "os_storage.h"
#include "os_apps.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#ifdef OS_TARGET_NATIVE
#include <thread>
#else
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#endif

namespace TamimysticOS {

CLI& CLI::getInstance() {
    static CLI instance;
    return instance;
}

#ifdef OS_TARGET_NATIVE
static std::thread cli_thread;
static void native_cli_task_func() {
    CLI::getInstance().processLoop();
}
#else
static TaskHandle_t cli_task = nullptr;
static void esp32_cli_task_func(void* arg) {
    CLI::getInstance().processLoop();
}
#endif

void CLI::init() {
    // Register built-in commands
    registerCommand("help", "List all available commands", [this](const std::vector<std::string>& args) {
        hal_uart_print("Available commands:\n");
        for (const auto& cmd : commands) {
            hal_uart_print(("  " + cmd.name + " - " + cmd.help + "\n").c_str());
        }
    });

    registerCommand("reboot", "Restart the system", [](const std::vector<std::string>& args) {
        hal_uart_print("Rebooting system...\n");
        // We'd typically call esp_restart() here.
    });

    // Plug & Play Hardware commands
    registerCommand("pnp", "Hardware PnP management (pnp scan, pnp list)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: pnp <scan | list>\n");
            return;
        }
        if (args[1] == "scan") {
            PnPManager::getInstance().scanI2CBus();
        } else if (args[1] == "list") {
            auto devices = PnPManager::getInstance().getDiscoveredDevices();
            hal_uart_print("\n--- Discovered Hardware Peripherals ---\n");
            if (devices.empty()) {
                hal_uart_print("  No devices currently registered.\n");
            } else {
                for (const auto& d : devices) {
                    std::stringstream ss;
                    ss << "  " << deviceCategoryToIcon(d.category) << " [0x" 
                       << std::hex << std::uppercase << (int)d.address << "] "
                       << d.name << " (" << d.model << ")\n"
                       << "     Category: " << deviceCategoryToString(d.category) << " | Driver: " << d.driver_name << "\n"
                       << "     Reading: " << d.live_reading << "\n";
                    hal_uart_print(ss.str().c_str());
                }
            }
            hal_uart_print("---------------------------------------\n\n");
        } else {
            hal_uart_print("Unknown pnp subcommand. Use 'pnp scan' or 'pnp list'.\n");
        }
    });

    // Dynamic Pin Matrix commands
    registerCommand("pin", "GPIO pin matrix management (pin list, pin set <func> <gpio>, pin reset)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: pin <list | set <func> <gpio> | reset>\n");
            return;
        }
        if (args[1] == "list") {
            auto mappings = PinMatrixManager::getInstance().getAllMappings();
            hal_uart_print("\n=== Dynamic GPIO Pin Matrix (ESP32-S3) ===\n");
            hal_uart_print("  Function Name     | Assigned GPIO | Safety Status\n");
            hal_uart_print("  ------------------+---------------+---------------\n");
            for (const auto& item : mappings) {
                std::stringstream ss;
                ss << "  " << std::left << std::setw(17) << item.func_name
                   << " | " << std::setw(13) << (item.gpio_pin >= 0 ? "GPIO " + std::to_string(item.gpio_pin) : "UNASSIGNED")
                   << " | " << (item.is_safe ? "[SAFE]" : "[WARNING/RESERVED]") << "\n";
                hal_uart_print(ss.str().c_str());
            }
            hal_uart_print("===========================================\n\n");
        } else if (args[1] == "set") {
            if (args.size() < 4) {
                hal_uart_print("Usage: pin set <func_name> <gpio_num>\n");
                hal_uart_print("Example: pin set i2c_sda 21\n");
                return;
            }
            std::string func = args[2];
            int gpio = std::atoi(args[3].c_str());
            if (PinMatrixManager::getInstance().setPin(func, gpio)) {
                hal_uart_print(("Pin updated: " + func + " -> GPIO " + std::to_string(gpio) + " (Saved to NVS)\n").c_str());
            } else {
                hal_uart_print(("Failed to set pin. Ensure GPIO " + std::to_string(gpio) + " is a safe GPIO on ESP32-S3.\n").c_str());
            }
        } else if (args[1] == "reset") {
            PinMatrixManager::getInstance().resetToDefaults();
            hal_uart_print("Pin matrix reset to factory defaults.\n");
        }
    });

    // Universal Robot Brain commands
    registerCommand("robot", "Universal Robot control (robot mode, move, strafe, arm, ik, stop, status)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: robot <mode | move | strafe | arm | ik | stop | resume | status>\n");
            return;
        }
        std::string sub = args[1];
        if (sub == "mode") {
            if (args.size() < 3) {
                hal_uart_print("Usage: robot mode <diff | mecanum | arm | balance>\n");
                return;
            }
            std::string m = args[2];
            if (m == "diff") RobotController::getInstance().setMode(RobotMode::DIFFERENTIAL_ROVER);
            else if (m == "mecanum") RobotController::getInstance().setMode(RobotMode::MECANUM_4WD);
            else if (m == "arm") RobotController::getInstance().setMode(RobotMode::ROBOTIC_ARM);
            else if (m == "balance") RobotController::getInstance().setMode(RobotMode::BALANCE_BOT);
            else hal_uart_print("Unknown mode. Choose diff, mecanum, arm, or balance.\n");
        } else if (sub == "move") {
            if (args.size() < 4) {
                hal_uart_print("Usage: robot move <linear_speed -100..100> <angular_speed -100..100>\n");
                return;
            }
            float v = (float)std::atof(args[2].c_str());
            float w = (float)std::atof(args[3].c_str());
            RobotController::getInstance().setTwist(v, 0.0f, w);
            hal_uart_print("Rover movement velocity applied.\n");
        } else if (sub == "strafe") {
            if (args.size() < 5) {
                hal_uart_print("Usage: robot strafe <vx> <vy> <omega>\n");
                return;
            }
            float vx = (float)std::atof(args[2].c_str());
            float vy = (float)std::atof(args[3].c_str());
            float w = (float)std::atof(args[4].c_str());
            RobotController::getInstance().setTwist(vx, vy, w);
            hal_uart_print("Holonomic velocity applied.\n");
        } else if (sub == "arm") {
            if (args.size() < 8) {
                hal_uart_print("Usage: robot arm <j1_base> <j2_shoulder> <j3_elbow> <j4_wrist_p> <j5_wrist_r> <j6_gripper>\n");
                return;
            }
            ArmJoints j;
            j.base_yaw = (float)std::atof(args[2].c_str());
            j.shoulder_pitch = (float)std::atof(args[3].c_str());
            j.elbow_pitch = (float)std::atof(args[4].c_str());
            j.wrist_pitch = (float)std::atof(args[5].c_str());
            j.wrist_roll = (float)std::atof(args[6].c_str());
            j.gripper = (float)std::atof(args[7].c_str());
            RobotController::getInstance().setArmJoints(j);
            hal_uart_print("Robotic arm joints commanded.\n");
        } else if (sub == "ik") {
            if (args.size() < 5) {
                hal_uart_print("Usage: robot ik <X_cm> <Y_cm> <Z_cm> [pitch_deg] [gripper_pct]\n");
                return;
            }
            ArmPose pose;
            pose.x = (float)std::atof(args[2].c_str());
            pose.y = (float)std::atof(args[3].c_str());
            pose.z = (float)std::atof(args[4].c_str());
            if (args.size() >= 6) pose.pitch = (float)std::atof(args[5].c_str());
            if (args.size() >= 7) pose.gripper = (float)std::atof(args[6].c_str());
            if (RobotController::getInstance().setArmTargetIK(pose)) {
                auto tel = RobotController::getInstance().getTelemetry();
                std::stringstream ss;
                ss << "IK Solved & Applied! Target=(" << pose.x << ", " << pose.y << ", " << pose.z 
                   << "cm) -> Joints: Base=" << tel.joints.base_yaw << "°, Shoulder=" << tel.joints.shoulder_pitch 
                   << "°, Elbow=" << tel.joints.elbow_pitch << "°, Wrist=" << tel.joints.wrist_pitch << "°\n";
                hal_uart_print(ss.str().c_str());
            } else {
                hal_uart_print("IK Error: Target coordinate is outside arm reach workspace!\n");
            }
        } else if (sub == "stop") {
            RobotController::getInstance().emergencyStop();
        } else if (sub == "resume") {
            RobotController::getInstance().resume();
        } else if (sub == "status") {
            auto tel = RobotController::getInstance().getTelemetry();
            hal_uart_print("\n=== Universal Robot Brain Telemetry ===\n");
            hal_uart_print(("  Mode:       " + std::string(robotModeToString(tel.mode)) + "\n").c_str());
            hal_uart_print(("  Velocity:   Vx=" + std::to_string((int)tel.velocity.vx) + "%, Vy=" + std::to_string((int)tel.velocity.vy) + "%, W=" + std::to_string((int)tel.velocity.omega) + "%\n").c_str());
            hal_uart_print(("  Wheels:     FL=" + std::to_string((int)tel.wheels.front_left) + "%, FR=" + std::to_string((int)tel.wheels.front_right) + "%, RL=" + std::to_string((int)tel.wheels.rear_left) + "%, RR=" + std::to_string((int)tel.wheels.rear_right) + "%\n").c_str());
            hal_uart_print(("  Arm Pose:   X=" + std::to_string((int)tel.pose.x) + "cm, Y=" + std::to_string((int)tel.pose.y) + "cm, Z=" + std::to_string((int)tel.pose.z) + "cm, Pitch=" + std::to_string((int)tel.pose.pitch) + "°\n").c_str());
            hal_uart_print(("  Arm Joints: J1=" + std::to_string((int)tel.joints.base_yaw) + "°, J2=" + std::to_string((int)tel.joints.shoulder_pitch) + "°, J3=" + std::to_string((int)tel.joints.elbow_pitch) + "°, J4=" + std::to_string((int)tel.joints.wrist_pitch) + "°\n").c_str());
            hal_uart_print(("  Safety:     E-STOP=" + std::string(tel.emergency_stop ? "ENGAGED" : "OFF") + ", Obstacle Braking=" + std::string(tel.obstacle_braking ? "ACTIVE" : "OFF") + "\n").c_str());
            hal_uart_print("=======================================\n\n");
        } else {
            hal_uart_print("Unknown robot subcommand. Type 'robot' for options.\n");
        }
    });

    // Edge AI & Neural Vision Commands
    registerCommand("ai", "Edge AI vision control (ai status, ai model <name>, ai track <on|off>)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: ai <status | model <person|object|lane|gesture> | track <on|off>>\n");
            return;
        }
        std::string sub = args[1];
        if (sub == "status") {
            auto tel = AIModule::getInstance().getTelemetry();
            hal_uart_print("\n=== Edge AI & Vision Pipeline Status ===\n");
            hal_uart_print(("  Model:         " + std::string(aiModelToString(tel.model)) + "\n").c_str());
            hal_uart_print(("  Inference FPS: " + std::to_string(tel.fps).substr(0, 4) + " FPS\n").c_str());
            hal_uart_print(("  Latency:       " + std::to_string(tel.inference_time_ms) + " ms (SIMD Accelerated)\n").c_str());
            hal_uart_print(("  Target Lock:   " + std::string(tel.target_locked ? "LOCKED" : "SEARCHING") + "\n").c_str());
            hal_uart_print(("  Latest Object: " + tel.latest_label + " (" + std::to_string((int)tel.latest_confidence) + "% conf)\n").c_str());
            hal_uart_print(("  Auto Follow:   " + std::string(tel.visual_tracking_enabled ? "ENABLED" : "DISABLED") + "\n").c_str());
            hal_uart_print("========================================\n\n");
        } else if (sub == "model") {
            if (args.size() < 3) {
                hal_uart_print("Usage: ai model <person | object | lane | gesture>\n");
                return;
            }
            std::string m = args[2];
            if (m == "person") AIModule::getInstance().setModel(AIModelType::PERSON_DETECTION);
            else if (m == "object") AIModule::getInstance().setModel(AIModelType::OBJECT_DETECTION);
            else if (m == "lane") AIModule::getInstance().setModel(AIModelType::LANE_TRACKING);
            else if (m == "gesture") AIModule::getInstance().setModel(AIModelType::GESTURE_RECOGNITION);
            else hal_uart_print("Unknown model name. Use person, object, lane, or gesture.\n");
        } else if (sub == "track") {
            if (args.size() < 3) {
                hal_uart_print("Usage: ai track <on | off>\n");
                return;
            }
            bool enable = (args[2] == "on" || args[2] == "1" || args[2] == "true");
            AIModule::getInstance().setVisualTracking(enable);
        } else {
            hal_uart_print("Unknown ai subcommand.\n");
        }
    });

    // Camera Hardware Commands
    registerCommand("camera", "Camera hardware management (camera snap, camera status)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: camera <snap | status>\n");
            return;
        }
        if (args[1] == "snap") {
            CameraFrame* fb = CameraManager::getInstance().getFrame();
            if (fb) {
                std::stringstream ss;
                ss << "[CAMERA] Snapshot captured! Resolution: " << fb->width << "x" << fb->height 
                   << " | Size: " << fb->len << " bytes | Format: JPEG\n";
                hal_uart_print(ss.str().c_str());
                CameraManager::getInstance().returnFrame(fb);
            } else {
                hal_uart_print("[CAMERA] Error: Failed to acquire camera frame!\n");
            }
        } else if (args[1] == "status") {
            bool ok = CameraManager::getInstance().isInitialized();
            hal_uart_print(("[CAMERA] Status: " + std::string(ok ? "ONLINE (PSRAM Framebuffer Ready)" : "OFFLINE") + "\n").c_str());
        }
    });

    // Dynamic Python Scripting Commands
    registerCommand("python", "Python script engine (python eval <code>, python run <file>, python stop)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: python <eval <code> | run <file.py> | stop>\n");
            return;
        }
        std::string sub = args[1];
        if (sub == "eval") {
            if (args.size() < 3) {
                hal_uart_print("Usage: python eval \"<python_code>\"\n");
                return;
            }
            std::string code = "";
            for (size_t i = 2; i < args.size(); i++) {
                code += args[i] + (i + 1 < args.size() ? " " : "");
            }
            auto res = AppManager::getInstance().evalCode(code);
            if (!res.stdout_output.empty()) hal_uart_print(res.stdout_output.c_str());
            if (!res.success) hal_uart_print(("Python Error: " + res.error_message + "\n").c_str());
            hal_uart_print(("[PYTHON] Execution finished in " + std::to_string(res.execution_time_ms) + " ms\n").c_str());
        } else if (sub == "run") {
            if (args.size() < 3) {
                hal_uart_print("Usage: python run <filename.py>\n");
                return;
            }
            std::string fname = args[2];
            auto res = AppManager::getInstance().runFile(fname);
            if (!res.stdout_output.empty()) hal_uart_print(res.stdout_output.c_str());
            if (!res.success) hal_uart_print(("Python Error: " + res.error_message + "\n").c_str());
        } else if (sub == "stop") {
            AppManager::getInstance().stopCurrentApp();
        } else {
            hal_uart_print("Unknown python subcommand.\n");
        }
    });

    // WebAssembly Sandbox Commands
    registerCommand("wasm", "WASM micro-runtime (wasm run <file.wasm>)", [](const std::vector<std::string>& args) {
        if (args.size() < 3 || args[1] != "run") {
            hal_uart_print("Usage: wasm run <filename.wasm>\n");
            return;
        }
        std::string wasm_log;
        WasmRunner::getInstance().runWasmFile(args[2], wasm_log);
        hal_uart_print(wasm_log.c_str());
    });

    // Storage Management Commands
    registerCommand("storage", "LittleFS 6.8MB Flash VFS (storage ls, storage df, storage rm <file>)", [](const std::vector<std::string>& args) {
        if (args.size() < 2) {
            hal_uart_print("Usage: storage <ls | df | rm <file>>\n");
            return;
        }
        std::string sub = args[1];
        if (sub == "ls") {
            auto files = StorageManager::getInstance().listFiles();
            hal_uart_print("\n=== LittleFS Flash Filesystem (6.8MB Partition) ===\n");
            hal_uart_print("  Filename                       | Size (Bytes)\n");
            hal_uart_print("  -------------------------------+--------------\n");
            if (files.empty()) {
                hal_uart_print("  (Storage is empty)\n");
            } else {
                for (const auto& f : files) {
                    std::stringstream ss;
                    ss << "  " << std::left << std::setw(30) << f.name << " | " << f.size << " B\n";
                    hal_uart_print(ss.str().c_str());
                }
            }
            hal_uart_print("===================================================\n\n");
        } else if (sub == "df") {
            auto stats = StorageManager::getInstance().getStats();
            std::stringstream ss;
            ss << "\n=== Flash Storage Capacity (ESP32-S3-N16R8) ===\n"
               << "  Total Capacity: " << (stats.total_bytes / 1024) << " KB (6.8 MB)\n"
               << "  Used Space:     " << (stats.used_bytes / 1024) << " KB\n"
               << "  Free Space:     " << (stats.free_bytes / 1024) << " KB\n"
               << "================================================\n\n";
            hal_uart_print(ss.str().c_str());
        } else if (sub == "rm") {
            if (args.size() < 3) {
                hal_uart_print("Usage: storage rm <filename>\n");
                return;
            }
            if (StorageManager::getInstance().deleteFile(args[2])) {
                hal_uart_print(("File deleted: " + args[2] + "\n").c_str());
            } else {
                hal_uart_print(("Failed to delete file: " + args[2] + "\n").c_str());
            }
        }
    });

    // Dual-Bank OTA Status Commands
    registerCommand("ota", "Dual-bank OTA firmware management (ota status)", [](const std::vector<std::string>& args) {
        hal_uart_print("\n=== Dual-Bank OTA Firmware Status ===\n");
        hal_uart_print("  Active Partition:     app0 (OTA_0 @ 0x20000 - 4.5 MB)\n");
        hal_uart_print("  Rollback Partition:   app1 (OTA_1 @ 0x4A0000 - 4.5 MB)\n");
        hal_uart_print("  Firmware Version:     v2.4.0-ULTRA (Dual-Core LX7)\n");
        hal_uart_print("  OTA Boot State:       VALIDATED (Rollback Armed)\n");
        hal_uart_print("=====================================\n\n");
    });

#ifdef OS_TARGET_NATIVE
    hal_uart_print("[CLI] Starting Native CLI Thread...\n");
    cli_thread = std::thread(native_cli_task_func);
    cli_thread.detach();
#else
    hal_uart_print("[CLI] Starting ESP32 CLI Task...\n");
    xTaskCreate(esp32_cli_task_func, "CliTask", 4096, nullptr, 1, &cli_task);
#endif
}

void CLI::registerCommand(const std::string& name, const std::string& help, CliCommandHandler handler) {
    commands.push_back({name, help, handler});
}

void CLI::processLoop() {
    char input_buffer[256];
    
    // Slight delay to ensure boot logs finish before prompting
#ifdef OS_TARGET_NATIVE
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
#else
    vTaskDelay(pdMS_TO_TICKS(500));
#endif

    while (true) {
        hal_uart_print("aeron> ");
        hal_uart_read_line(input_buffer, sizeof(input_buffer));
        
        std::string line(input_buffer);
        if (line.empty()) continue;

        // Simple space-based tokenizer
        std::vector<std::string> args;
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            args.push_back(token);
        }

        if (args.empty()) continue;

        std::string cmd_name = args[0];
        bool found = false;
        
        for (const auto& cmd : commands) {
            if (cmd.name == cmd_name) {
                cmd.handler(args);
                found = true;
                break;
            }
        }

        if (!found) {
            hal_uart_print(("Unknown command: " + cmd_name + ". Type 'help' for a list.\n").c_str());
        }
    }
}

} // namespace TamimysticOS
