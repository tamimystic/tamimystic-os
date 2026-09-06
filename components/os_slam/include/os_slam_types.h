#pragma once

#include <vector>
#include <string>
#include <cstdint>

namespace TamimysticOS {

enum class LidarType {
    SIMULATED_360 = 0,
    RPLIDAR_A1_A2,
    LD19_D300
};

inline const char* lidarTypeToString(LidarType type) {
    switch (type) {
        case LidarType::SIMULATED_360: return "Simulated 360 Lidar";
        case LidarType::RPLIDAR_A1_A2: return "RPLiDAR A1/A2";
        case LidarType::LD19_D300:     return "LD19 / D300";
        default:                       return "Unknown Lidar";
    }
}

// 360-degree range measurements in meters
struct LidarScanData {
    float ranges[360]; // index = degree (0 to 359), value in meters (0 = invalid/out of range)
    float min_range = 0.12f;
    float max_range = 8.0f;
    uint32_t timestamp_ms = 0;
    bool is_valid = false;
};

// 2D Point on Continuous Map (cm)
struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
};

// 2D Grid Cell Index (0 to MAP_SIZE-1)
struct GridCoord {
    int x = 0;
    int y = 0;

    bool operator==(const GridCoord& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const GridCoord& other) const {
        return !(*this == other);
    }
};

// 200x200 Grid Map constants
constexpr int SLAM_GRID_WIDTH = 200;
constexpr int SLAM_GRID_HEIGHT = 200;
constexpr float SLAM_CELL_RESOLUTION_CM = 5.0f; // 5 cm per cell -> 10m x 10m area
constexpr int SLAM_GRID_ORIGIN_X = 100; // Robot starts at center
constexpr int SLAM_GRID_ORIGIN_Y = 100;

enum class CellState : int8_t {
    UNKNOWN = 0,
    FREE = -1,
    OCCUPIED = 100
};

struct SlamRobotPose {
    float x_cm = 0.0f;
    float y_cm = 0.0f;
    float yaw_deg = 0.0f;
};

struct NavGoal {
    float target_x_cm = 0.0f;
    float target_y_cm = 0.0f;
    bool active = false;
    bool reached = false;
};

struct SlamStatus {
    LidarType lidar_type = LidarType::SIMULATED_360;
    bool mapping_active = true;
    SlamRobotPose pose;
    NavGoal goal;
    uint32_t explored_cells_count = 0;
    float scan_rate_hz = 10.0f;
    uint32_t total_scans_processed = 0;
    size_t path_waypoints_count = 0;
};

} // namespace TamimysticOS
