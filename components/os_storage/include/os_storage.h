#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace TamimysticOS {

struct FileEntry {
    std::string name;
    size_t size = 0;
};

struct StorageStats {
    size_t total_bytes = 0;
    size_t used_bytes = 0;
    size_t free_bytes = 0;
};

class StorageManager {
public:
    static StorageManager& getInstance();

    void init();

    // Read a file entirely into memory
    bool readFile(const std::string& path, std::vector<uint8_t>& out_buffer);

    // Save buffer to file
    bool writeFile(const std::string& path, const uint8_t* data, size_t size);

    // Delete a file
    bool deleteFile(const std::string& path);

    // Check if file exists
    bool fileExists(const std::string& path);

    // List all files in storage
    std::vector<FileEntry> listFiles();

    // Get storage capacity stats (6.8MB LittleFS)
    StorageStats getStats();

    // JSON serialization of file list
    std::string getFilesJson();

private:
    StorageManager() = default;
    ~StorageManager() = default;
};

} // namespace TamimysticOS
