#include "os_slam.h"
#include "os_robotics.h"
#include "os_hal_uart.h"
#include "os_scheduler.h"
#include <cmath>
#include <queue>
#include <algorithm>
#include <sstream>
#include <cstring>
#include <iomanip>

namespace TamimysticOS {

SlamEngine& SlamEngine::getInstance() {
    static SlamEngine instance;
    return instance;
}

void SlamEngine::init() {
    hal_uart_print("[SLAM] Initializing 2D LiDAR & Occupancy Grid SLAM Engine...\n");
    clearMap();

    // Spawns 10Hz Real-Time SLAM task on Core 1
    OSScheduler::getInstance().createTask("slam_task", 4096, 2, CORE_1, []() {
        while (true) {
            if (SlamEngine::getInstance().getLidarType() == LidarType::SIMULATED_360) {
                SlamEngine::getInstance().generateSimulatedScan();
            }
            SlamEngine::getInstance().processNavigationStep();
            OSScheduler::getInstance().delay(100); // 10 Hz SLAM iteration
        }
    });

    hal_uart_print("[SLAM] 200x200 Grid Map allocated (10m x 10m @ 5cm resolution).\n");
    hal_uart_print("[SLAM] A* Autonomous Path Planning & Waypoint Navigation active.\n");
}

void SlamEngine::clearMap() {
    if (grid_map.size() != SLAM_GRID_WIDTH * SLAM_GRID_HEIGHT) {
        grid_map.assign(SLAM_GRID_WIDTH * SLAM_GRID_HEIGHT, 0);
    } else {
        std::fill(grid_map.begin(), grid_map.end(), 0);
    }
    explored_cells = 0;
    robot_pose = {0.0f, 0.0f, 0.0f};
    nav_goal = {0.0f, 0.0f, false, false};
    current_path.clear();
}

void SlamEngine::setLidarType(LidarType type) {
    lidar_type = type;
}

GridCoord SlamEngine::worldToGrid(float x_cm, float y_cm) const {
    int gx = SLAM_GRID_ORIGIN_X + static_cast<int>(std::round(x_cm / SLAM_CELL_RESOLUTION_CM));
    int gy = SLAM_GRID_ORIGIN_Y + static_cast<int>(std::round(y_cm / SLAM_CELL_RESOLUTION_CM));
    if (gx < 0) gx = 0;
    if (gx >= SLAM_GRID_WIDTH) gx = SLAM_GRID_WIDTH - 1;
    if (gy < 0) gy = 0;
    if (gy >= SLAM_GRID_HEIGHT) gy = SLAM_GRID_HEIGHT - 1;
    return {gx, gy};
}

Point2D SlamEngine::gridToWorld(int gx, int gy) const {
    float wx = (gx - SLAM_GRID_ORIGIN_X) * SLAM_CELL_RESOLUTION_CM;
    float wy = (gy - SLAM_GRID_ORIGIN_Y) * SLAM_CELL_RESOLUTION_CM;
    return {wx, wy};
}

int8_t SlamEngine::getCell(int grid_x, int grid_y) const {
    if (grid_x < 0 || grid_x >= SLAM_GRID_WIDTH || grid_y < 0 || grid_y >= SLAM_GRID_HEIGHT) return 100;
    if (grid_map.empty()) return 0;
    return grid_map[grid_x * SLAM_GRID_HEIGHT + grid_y];
}

void SlamEngine::setCell(int grid_x, int grid_y, int8_t value) {
    if (grid_x < 0 || grid_x >= SLAM_GRID_WIDTH || grid_y < 0 || grid_y >= SLAM_GRID_HEIGHT) return;
    if (grid_map.empty()) return;
    size_t idx = grid_x * SLAM_GRID_HEIGHT + grid_y;
    if (grid_map[idx] == 0 && value != 0) {
        explored_cells++;
    }
    grid_map[idx] = value;
}

void SlamEngine::updatePose(float x_cm, float y_cm, float yaw_deg) {
    robot_pose.x_cm = x_cm;
    robot_pose.y_cm = y_cm;
    robot_pose.yaw_deg = yaw_deg;
}

// Bresenham's Line Algorithm for Ray-casting
void SlamEngine::processBresenhamRay(int x0, int y0, int x1, int y1, bool mark_occupied) {
    int dx = std::abs(x1 - x0);
    int dy = std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    int cur_x = x0;
    int cur_y = y0;

    while (true) {
        if (cur_x == x1 && cur_y == y1) {
            if (mark_occupied) {
                setCell(cur_x, cur_y, 100); // OCCUPIED
            } else {
                setCell(cur_x, cur_y, -1);  // FREE
            }
            break;
        }

        setCell(cur_x, cur_y, -1); // Mark ray path as FREE (-1)

        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            cur_x += sx;
        }
        if (e2 < dx) {
            err += dx;
            cur_y += sy;
        }
    }
}

void SlamEngine::ingestScan(const LidarScanData& scan) {
    latest_scan = scan;
    total_scans++;

    // Calculate robot grid coordinate
    GridCoord robot_grid = worldToGrid(robot_pose.x_cm, robot_pose.y_cm);

    float rad_offset = robot_pose.yaw_deg * 0.0174533f;

    for (int deg = 0; deg < 360; deg += 2) { // Step by 2 deg for efficiency
        float dist_m = scan.ranges[deg];
        if (dist_m < scan.min_range || dist_m > scan.max_range) continue;

        float angle_rad = (deg * 0.0174533f) + rad_offset;
        float obs_x_cm = robot_pose.x_cm + (dist_m * 100.0f * std::cos(angle_rad));
        float obs_y_cm = robot_pose.y_cm + (dist_m * 100.0f * std::sin(angle_rad));

        GridCoord obs_grid = worldToGrid(obs_x_cm, obs_y_cm);
        processBresenhamRay(robot_grid.x, robot_grid.y, obs_grid.x, obs_grid.y, true);
    }
}

void SlamEngine::generateSimulatedScan() {
    LidarScanData scan;
    scan.min_range = 0.15f;
    scan.max_range = 8.0f;
    scan.is_valid = true;

    // Simulate rectangular room 5m x 5m (+- 250cm) with a center obstacle
    float rad_offset = robot_pose.yaw_deg * 0.0174533f;

    for (int deg = 0; deg < 360; deg++) {
        float angle_rad = (deg * 0.0174533f) + rad_offset;
        float cos_a = std::cos(angle_rad);
        float sin_a = std::sin(angle_rad);

        // Raycast against 4 walls at x = +-250cm, y = +-250cm
        float d_wall = 5.0f; // default 5m

        if (std::abs(cos_a) > 0.001f) {
            float target_x = (cos_a > 0) ? 250.0f : -250.0f;
            float d = (target_x - robot_pose.x_cm) / cos_a;
            if (d > 0.0f) {
                float hit_y = robot_pose.y_cm + d * sin_a;
                if (hit_y >= -250.0f && hit_y <= 250.0f && (d / 100.0f) < d_wall) {
                    d_wall = d / 100.0f;
                }
            }
        }

        if (std::abs(sin_a) > 0.001f) {
            float target_y = (sin_a > 0) ? 250.0f : -250.0f;
            float d = (target_y - robot_pose.y_cm) / sin_a;
            if (d > 0.0f) {
                float hit_x = robot_pose.x_cm + d * cos_a;
                if (hit_x >= -250.0f && hit_x <= 250.0f && (d / 100.0f) < d_wall) {
                    d_wall = d / 100.0f;
                }
            }
        }

        // Add a pillar obstacle at (100cm, 80cm) with radius 20cm
        float dx = 100.0f - robot_pose.x_cm;
        float dy = 80.0f - robot_pose.y_cm;
        float dist_pillar_center = std::sqrt(dx * dx + dy * dy);
        float pillar_angle = std::atan2(dy, dx);
        float angle_diff = std::abs(std::remainder(angle_rad - pillar_angle, 6.28318f));
        if (angle_diff < 0.2f && (dist_pillar_center / 100.0f) < d_wall) {
            d_wall = (dist_pillar_center - 20.0f) / 100.0f;
            if (d_wall < 0.15f) d_wall = 0.15f;
        }

        scan.ranges[deg] = d_wall;
    }

    ingestScan(scan);
}

// A* (A-Star) Optimal Grid Path Planning
struct AStarNode {
    GridCoord coord;
    float g_cost = 1e9f;
    float h_cost = 0.0f;
    float f_cost() const { return g_cost + h_cost; }
    GridCoord parent = {-1, -1};
};

struct CompareNode {
    bool operator()(const AStarNode& a, const AStarNode& b) {
        return a.f_cost() > b.f_cost();
    }
};

std::vector<GridCoord> SlamEngine::planAStar(const GridCoord& start, const GridCoord& goal) {
    std::vector<GridCoord> path;
    if (start == goal) return path;

    const size_t total_cells = SLAM_GRID_WIDTH * SLAM_GRID_HEIGHT;
    std::vector<float> g_score(total_cells, 1e9f);
    std::vector<GridCoord> parent_map(total_cells, {-1, -1});
    std::vector<uint8_t> closed_set(total_cells, 0);

    auto get_idx = [](int x, int y) { return x * SLAM_GRID_HEIGHT + y; };

    std::priority_queue<AStarNode, std::vector<AStarNode>, CompareNode> open_set;

    AStarNode start_node;
    start_node.coord = start;
    start_node.g_cost = 0.0f;
    start_node.h_cost = std::sqrt(std::pow(goal.x - start.x, 2) + std::pow(goal.y - start.y, 2));
    g_score[get_idx(start.x, start.y)] = 0.0f;
    open_set.push(start_node);

    const int dx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
    const int dy[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    const float step_cost[8] = {1.0f, 1.0f, 1.0f, 1.0f, 1.414f, 1.414f, 1.414f, 1.414f};

    bool found = false;

    while (!open_set.empty()) {
        AStarNode current = open_set.top();
        open_set.pop();

        if (current.coord == goal) {
            found = true;
            break;
        }

        size_t cur_idx = get_idx(current.coord.x, current.coord.y);
        if (closed_set[cur_idx]) continue;
        closed_set[cur_idx] = 1;

        for (int i = 0; i < 8; i++) {
            int nx = current.coord.x + dx[i];
            int ny = current.coord.y + dy[i];

            if (nx < 0 || nx >= SLAM_GRID_WIDTH || ny < 0 || ny >= SLAM_GRID_HEIGHT) continue;
            size_t n_idx = get_idx(nx, ny);
            if (closed_set[n_idx]) continue;

            // Obstacle collision check (cell == 100 is occupied)
            if (getCell(nx, ny) == 100) continue;

            // Safety inflation check (check neighbors within 1 cell)
            bool near_obstacle = false;
            for (int ix = -1; ix <= 1 && !near_obstacle; ix++) {
                for (int iy = -1; iy <= 1; iy++) {
                    int c_x = nx + ix;
                    int c_y = ny + iy;
                    if (c_x >= 0 && c_x < SLAM_GRID_WIDTH && c_y >= 0 && c_y < SLAM_GRID_HEIGHT) {
                        if (getCell(c_x, c_y) == 100) {
                            near_obstacle = true;
                            break;
                        }
                    }
                }
            }
            if (near_obstacle) continue;

            float tentative_g = current.g_cost + step_cost[i];
            if (tentative_g < g_score[n_idx]) {
                g_score[n_idx] = tentative_g;
                parent_map[n_idx] = current.coord;

                AStarNode neighbor;
                neighbor.coord = {nx, ny};
                neighbor.g_cost = tentative_g;
                neighbor.h_cost = std::sqrt(std::pow(goal.x - nx, 2) + std::pow(goal.y - ny, 2));
                open_set.push(neighbor);
            }
        }
    }

    if (found) {
        GridCoord curr = goal;
        while (curr.x != -1) {
            path.push_back(curr);
            if (curr == start) break;
            curr = parent_map[get_idx(curr.x, curr.y)];
        }
        std::reverse(path.begin(), path.end());
    }

    return path;
}

bool SlamEngine::setNavigationGoal(float target_x_cm, float target_y_cm) {
    GridCoord start = worldToGrid(robot_pose.x_cm, robot_pose.y_cm);
    GridCoord goal = worldToGrid(target_x_cm, target_y_cm);

    current_path = planAStar(start, goal);
    if (current_path.empty()) {
        hal_uart_print("[SLAM] No valid collision-free path to target!\n");
        return false;
    }

    nav_goal.target_x_cm = target_x_cm;
    nav_goal.target_y_cm = target_y_cm;
    nav_goal.active = true;
    nav_goal.reached = false;
    is_navigating = true;

    std::stringstream ss;
    ss << "[SLAM] Navigation Goal Accepted: Target (" << target_x_cm << ", " << target_y_cm 
       << " cm) | Waypoints: " << current_path.size() << "\n";
    hal_uart_print(ss.str().c_str());
    return true;
}

void SlamEngine::cancelNavigation() {
    is_navigating = false;
    nav_goal.active = false;
    current_path.clear();
    RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
    hal_uart_print("[SLAM] Navigation Cancelled.\n");
}

void SlamEngine::processNavigationStep() {
    if (!is_navigating || current_path.empty()) return;

    // Check distance to goal
    float dx = nav_goal.target_x_cm - robot_pose.x_cm;
    float dy = nav_goal.target_y_cm - robot_pose.y_cm;
    float dist_to_goal = std::sqrt(dx * dx + dy * dy);

    if (dist_to_goal < 15.0f) { // Within 15cm of goal
        is_navigating = false;
        nav_goal.active = false;
        nav_goal.reached = true;
        current_path.clear();
        RobotController::getInstance().setTwist(0.0f, 0.0f, 0.0f);
        hal_uart_print("[SLAM] Navigation Goal REACHED Successfully!\n");
        return;
    }

    // Look at next waypoint
    GridCoord next_wp = current_path.front();
    Point2D wp_world = gridToWorld(next_wp.x, next_wp.y);

    float wp_dx = wp_world.x - robot_pose.x_cm;
    float wp_dy = wp_world.y - robot_pose.y_cm;
    float wp_dist = std::sqrt(wp_dx * wp_dx + wp_dy * wp_dy);

    if (wp_dist < 10.0f && current_path.size() > 1) {
        current_path.erase(current_path.begin());
        next_wp = current_path.front();
        wp_world = gridToWorld(next_wp.x, next_wp.y);
        wp_dx = wp_world.x - robot_pose.x_cm;
        wp_dy = wp_world.y - robot_pose.y_cm;
    }

    // Proportional Waypoint Steering
    float target_angle_deg = std::atan2(wp_dy, wp_dx) * 57.2958f;
    float angle_err = std::remainder(target_angle_deg - robot_pose.yaw_deg, 360.0f);

    float omega = angle_err * 1.5f; // P-gain
    if (omega > 50.0f) omega = 50.0f;
    if (omega < -50.0f) omega = -50.0f;

    float vx = (std::abs(angle_err) < 30.0f) ? 40.0f : 15.0f;

    RobotController::getInstance().setTwist(vx, 0.0f, omega);
}

SlamStatus SlamEngine::getStatus() const {
    SlamStatus st;
    st.lidar_type = lidar_type;
    st.mapping_active = true;
    st.pose = robot_pose;
    st.goal = nav_goal;
    st.explored_cells_count = explored_cells;
    st.scan_rate_hz = 10.0f;
    st.total_scans_processed = total_scans;
    st.path_waypoints_count = current_path.size();
    return st;
}

std::string SlamEngine::getStatusJson() {
    std::stringstream ss;
    ss << "{\n"
       << "  \"status\": \"ok\",\n"
       << "  \"lidar\": \"" << lidarTypeToString(lidar_type) << "\",\n"
       << "  \"pose\": {\"x\": " << robot_pose.x_cm << ", \"y\": " << robot_pose.y_cm << ", \"yaw\": " << robot_pose.yaw_deg << "},\n"
       << "  \"goal\": {\"x\": " << nav_goal.target_x_cm << ", \"y\": " << nav_goal.target_y_cm << ", \"active\": " << (nav_goal.active ? "true" : "false") << ", \"reached\": " << (nav_goal.reached ? "true" : "false") << "},\n"
       << "  \"explored_cells\": " << explored_cells << ",\n"
       << "  \"total_scans\": " << total_scans << ",\n"
       << "  \"waypoints_left\": " << current_path.size() << "\n"
       << "}";
    return ss.str();
}

std::string SlamEngine::getCompressedMapJson() {
    std::stringstream ss;
    ss << "{\n"
       << "  \"status\": \"ok\",\n"
       << "  \"width\": " << SLAM_GRID_WIDTH << ",\n"
       << "  \"height\": " << SLAM_GRID_HEIGHT << ",\n"
       << "  \"resolution_cm\": " << SLAM_CELL_RESOLUTION_CM << ",\n"
       << "  \"robot\": {\"gx\": " << worldToGrid(robot_pose.x_cm, robot_pose.y_cm).x << ", \"gy\": " << worldToGrid(robot_pose.x_cm, robot_pose.y_cm).y << ", \"yaw\": " << robot_pose.yaw_deg << "},\n"
       << "  \"goal\": {\"gx\": " << worldToGrid(nav_goal.target_x_cm, nav_goal.target_y_cm).x << ", \"gy\": " << worldToGrid(nav_goal.target_x_cm, nav_goal.target_y_cm).y << ", \"active\": " << (nav_goal.active ? "true" : "false") << "},\n"
       << "  \"path\": [";
    for (size_t i = 0; i < current_path.size(); i++) {
        ss << "[" << current_path[i].x << "," << current_path[i].y << "]" << (i + 1 < current_path.size() ? "," : "");
    }
    ss << "],\n"
       << "  \"obstacles\": [";

    // Export list of occupied cells for high-speed canvas drawing
    bool first = true;
    for (int x = 0; x < SLAM_GRID_WIDTH; x++) {
        for (int y = 0; y < SLAM_GRID_HEIGHT; y++) {
            if (getCell(x, y) == 100) {
                if (!first) ss << ",";
                ss << "[" << x << "," << y << "]";
                first = false;
            }
        }
    }
    ss << "]\n}";
    return ss.str();
}

} // namespace TamimysticOS
