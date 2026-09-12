#include "test_framework.h"
#include "os_slam.h"

using namespace TamimysticOS;

void test_occupancy_grid_operations() {
    auto& slam = SlamEngine::getInstance();
    slam.init();
    slam.clearMap();

    // Verify cell states after clearing (should be default UNKNOWN / 0)
    TEST_ASSERT_EQ((int)slam.getCell(40, 40), 0, "Initial grid center should be 0");

    // Test setting cell log-odds values
    slam.setCell(40, 40, 100);
    TEST_ASSERT_EQ((int)slam.getCell(40, 40), 100, "Setting occupied cell");

    slam.setCell(40, 40, -50);
    TEST_ASSERT_EQ((int)slam.getCell(40, 40), -50, "Setting free-space cell");

    // Test clear map restores cells
    slam.clearMap();
    TEST_ASSERT_EQ((int)slam.getCell(40, 40), 0, "Clear map resets cell to 0");
}

void test_astar_pathfinding() {
    auto& slam = SlamEngine::getInstance();
    slam.clearMap();

    GridCoord start{10, 10};
    GridCoord goal{15, 15};

    std::vector<GridCoord> path = slam.planAStar(start, goal);
    TEST_ASSERT(!path.empty(), "A* path planner should find a path between clear start and goal");
    if (!path.empty()) {
        TEST_ASSERT_EQ(path.front().x, start.x, "Path starts at start.x");
        TEST_ASSERT_EQ(path.front().y, start.y, "Path starts at start.y");
        TEST_ASSERT_EQ(path.back().x, goal.x, "Path ends at goal.x");
        TEST_ASSERT_EQ(path.back().y, goal.y, "Path ends at goal.y");
    }
}

void run_slam_nav_test_suite() {
    RUN_TEST_SUITE("2D Occupancy Grid Map Operations", test_occupancy_grid_operations);
    RUN_TEST_SUITE("A* Global Path Planning Algorithm", test_astar_pathfinding);
}
