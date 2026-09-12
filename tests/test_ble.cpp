#include "test_framework.h"
#include "os_ble.h"

using namespace TamimysticOS;

void test_ble_initialization_and_adv() {
    auto& ble = BleManager::getInstance();
    BleConfig cfg;
    cfg.device_name = "Tamimystic-TestBot";
    ble.init(cfg);

    TEST_ASSERT_EQ(ble.getStatus().device_name, "Tamimystic-TestBot", "Device name matches config");

    // Start / Stop Advertising
    bool adv_start = ble.startAdvertising();
    TEST_ASSERT(adv_start, "BLE startAdvertising should succeed");
    TEST_ASSERT(ble.isAdvertising(), "isAdvertising should return true");

    bool adv_stop = ble.stopAdvertising();
    TEST_ASSERT(adv_stop, "BLE stopAdvertising should succeed");
    TEST_ASSERT(!ble.isAdvertising(), "isAdvertising should return false");
}

void test_ble_inbound_packets() {
    auto& ble = BleManager::getInstance();

    bool twist_received = false;
    float rx_vx = 0, rx_vy = 0, rx_w = 0;
    ble.setTwistCallback([&](float vx, float vy, float omega, uint8_t buttons) {
        (void)buttons;
        twist_received = true;
        rx_vx = vx;
        rx_vy = vy;
        rx_w = omega;
    });

    BleTwistPacket pkt{};
    pkt.linear_x_pct = 50;  // 0.5
    pkt.linear_y_pct = -25; // -0.25
    pkt.angular_z_pct = 75; // 0.75
    pkt.buttons = 0;
    pkt.sequence = 1;

    ble.processInboundTwist(reinterpret_cast<const uint8_t*>(&pkt), sizeof(pkt));

    TEST_ASSERT(twist_received, "Twist callback should be triggered");
    TEST_ASSERT_FLOAT_NEAR(rx_vx, 0.5f, 0.001f, "Unpacked Vx");
    TEST_ASSERT_FLOAT_NEAR(rx_vy, -0.25f, 0.001f, "Unpacked Vy");
    TEST_ASSERT_FLOAT_NEAR(rx_w, 0.75f, 0.001f, "Unpacked Omega");
}

void run_ble_test_suite() {
    RUN_TEST_SUITE("BLE 5.0 GATT Stack & Advertising", test_ble_initialization_and_adv);
    RUN_TEST_SUITE("BLE Inbound Packet Serialization & Callback Dispatch", test_ble_inbound_packets);
}
