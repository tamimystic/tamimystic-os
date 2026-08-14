#include "os_cli.h"
#include "os_hal_uart.h"
#include "os_event_bus.h"

#include <iostream>
#include <sstream>

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
