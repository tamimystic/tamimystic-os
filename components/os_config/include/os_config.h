#pragma once
#include <string>

namespace TamimysticOS {

class ConfigManager {
public:
    static ConfigManager& getInstance();

    void init();

    // Key-Value store operations
    bool setString(const std::string& key, const std::string& value);
    std::string getString(const std::string& key, const std::string& default_value = "");
    
    bool setInt(const std::string& key, int value);
    int getInt(const std::string& key, int default_value = 0);

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
};

} // namespace TamimysticOS
