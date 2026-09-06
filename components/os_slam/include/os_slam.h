#pragma once

#include "os_slam_types.h"
#include <vector>
#include <string>

namespace TamimysticOS {

class SlamEngine {
public:
    static SlamEngine& getInstance();

    // Initialize SLAM engine, allocate PSRAM grid, spawn Core 1 mapping task
    void init();

    // LiDAR Sensor interfacing
    void setLidarType(LidarType type);
    LidarType getLidarType() const { return lidar_type; }
    void ingestScan(const LidarScanData& scan);
    const LidarScanData& getLatestScan() const { return latest_scan; }

    // SLAM Grid Map Operations
    void clearMap();
    int8_t getCell(int grid_x, int grid_y) const;
    void setCell(int grid_x, int grid_y, int8_t value);
    void updatePose(float x_cm, float y_cm, float yaw_deg);

    // Bresenham Ray-Casting Algorithm
    void processBresenhamRay(int x0, int y0, int x1, int y1, bool mark_occupied);

    // A* (A-Star) Path Planning
    std::vector<GridCoord> planAStar(const GridCoord& start, const GridCoord& goal);
    bool setNavigationGoal(float target_x_cm, float target_y_cm);
    void cancelNavigation();

    // Real-time navigation step (waypoints pursuit)
    void processNavigationStep();

    // Telemetry & JSON for Web Dashboard / CLI
    SlamStatus getStatus() const;
    std::string getStatusJson();
    std::string getCompressedMapJson();

private:
    SlamEngine() = default;
    ~SlamEngine() = default;

    void generateSimulatedScan();
    GridCoord worldToGrid(float x_cm, float y_cm) const;
    Point2D gridToWorld(int gx, int gy) const;

    LidarType lidar_type = LidarType::SIMULATED_360;
    int8_t grid_map[SLAM_GRID_WIDTH][SLAM_GRID_HEIGHT]; // Allocated in PSRAM

    LidarScanData latest_scan;
    SlamRobotPose robot_pose;
    NavGoal nav_goal;
    std::vector<GridCoord> current_path;

    uint32_t total_scans = 0;
    uint32_t explored_cells = 0;
    bool is_navigating = false;
};

} // namespace TamimysticOS
