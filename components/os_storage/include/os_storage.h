#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace TamimysticOS {

class StorageManager {
public:
    static StorageManager& getInstance();

    void init();

    // Read a file entirely into memory
    bool readFile(const std::string& path, std::vector<uint8_t>& out_buffer);

    // Save buffer to file
    bool writeFile(const std::string& path, const uint8_t* data, size_t size);

private:
    StorageManager() = default;
    ~StorageManager() = default;
};

} // namespace TamimysticOS
