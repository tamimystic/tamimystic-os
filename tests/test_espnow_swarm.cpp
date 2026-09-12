#include "test_framework.h"
#include "os_espnow.h"

using namespace TamimysticOS;

void test_swarm_role_and_formation() {
    auto& espnow = EspNowEngine::getInstance();
    espnow.init();

    // Configure role as Leader
    espnow.setSwarmRole(SwarmRole::LEADER, 0, 75.0f);
    TEST_ASSERT_EQ((int)espnow.getSwarmRole(), (int)SwarmRole::LEADER, "Swarm role Leader");

    // Configure formation as Triangle
    espnow.setFormation(SwarmFormation::TRIANGLE, 80.0f);
    auto st = espnow.getStatus();
    TEST_ASSERT_EQ((int)st.formation, (int)SwarmFormation::TRIANGLE, "Swarm formation Triangle");
    TEST_ASSERT_FLOAT_NEAR(st.swarm_spacing_cm, 80.0f, 0.001f, "Swarm spacing 80cm");

    // Configure role as Follower Slot 2
    espnow.setSwarmRole(SwarmRole::FOLLOWER, 2, 60.0f);
    TEST_ASSERT_EQ((int)espnow.getSwarmRole(), (int)SwarmRole::FOLLOWER, "Swarm role Follower");
    auto st2 = espnow.getStatus();
    TEST_ASSERT_EQ((int)st2.follower_slot, 2, "Follower slot 2");
}

void test_remote_control_toggle() {
    auto& espnow = EspNowEngine::getInstance();

    espnow.setRemoteControlEnabled(true);
    TEST_ASSERT(espnow.isRemoteControlEnabled(), "Remote control should be enabled");

    espnow.setRemoteControlEnabled(false);
    TEST_ASSERT(!espnow.isRemoteControlEnabled(), "Remote control should be disabled");
}

void run_espnow_swarm_test_suite() {
    RUN_TEST_SUITE("ESP-NOW Swarm Role & Formation Configuration", test_swarm_role_and_formation);
    RUN_TEST_SUITE("ESP-NOW Handheld Remote Gamepad Mode", test_remote_control_toggle);
}
