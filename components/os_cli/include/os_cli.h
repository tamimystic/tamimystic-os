#pragma once
#include <string>
#include <vector>
#include <functional>

namespace TamimysticOS {

using CliCommandHandler = std::function<void(const std::vector<std::string>& args)>;

class CLI {
public:
    static CLI& getInstance();

    // Initialize the CLI task/thread
    void init();

    // Register a new command
    void registerCommand(const std::string& name, const std::string& help, CliCommandHandler handler);

    // Block and process CLI input (called by the CLI task/thread)
    void processLoop();

private:
    CLI() = default;
    ~CLI() = default;

    struct CommandEntry {
        std::string name;
        std::string help;
        CliCommandHandler handler;
    };
    std::vector<CommandEntry> commands;
};

} // namespace TamimysticOS
