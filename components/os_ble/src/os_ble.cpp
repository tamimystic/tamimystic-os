#include "os_ble.h"
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include "os_scheduler.h"
#include "os_robotics.h"
#include "os_pnp_manager.h"
#include <sstream>
#include <iomanip>
#include <cstring>
#include <cmath>
#include <chrono>

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
#if __has_include("esp_nimble_hci.h")
#include "esp_nimble_hci.h"
#endif
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#endif

namespace TamimysticOS {

static uint32_t get_current_millis() {
    static auto start_time = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

BleManager& BleManager::getInstance() {
    static BleManager instance;
    return instance;
}

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)

// Hardware NimBLE GATT Server Callbacks
static uint16_t g_telemetry_handle = 0;
static uint16_t g_sensors_handle = 0;
static uint16_t g_conn_handle = BLE_HS_CONN_HANDLE_NONE;

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg);

static int gatt_svr_chr_access_robotics(uint16_t conn_handle, uint16_t attr_handle,
                                       struct ble_gatt_access_ctxt *ctxt, void *arg) {
    auto& mgr = BleManager::getInstance();
    
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
        std::vector<uint8_t> buf(om_len);
        os_mbuf_copydata(ctxt->om, 0, om_len, buf.data());

        if (om_len == sizeof(BleTwistPacket)) {
            mgr.processInboundTwist(buf.data(), om_len);
        } else if (om_len == sizeof(BleArmPacket)) {
            mgr.processInboundArm(buf.data(), om_len);
        } else {
            mgr.processInboundCmd(buf.data(), om_len);
        }
        return 0;
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        std::string status_json = mgr.getBleJson();
        os_mbuf_append(ctxt->om, status_json.data(), status_json.size());
        return 0;
    }
    return 0;
}

static const ble_uuid128_t gatt_svr_svc_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x00, 0x00, 0xb1, 0x19);
static const ble_uuid128_t gatt_svr_chr_twist_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x01, 0x00, 0xb1, 0x19);
static const ble_uuid128_t gatt_svr_chr_arm_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x02, 0x00, 0xb1, 0x19);
static const ble_uuid128_t gatt_svr_chr_telemetry_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x03, 0x00, 0xb1, 0x19);
static const ble_uuid128_t gatt_svr_chr_sensors_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x04, 0x00, 0xb1, 0x19);
static const ble_uuid128_t gatt_svr_chr_estop_uuid =
    BLE_UUID128_INIT(0x14, 0x12, 0x8a, 0x76, 0x04, 0xd1, 0x6c, 0x4f, 0x7e, 0x53, 0xf2, 0xe8, 0x05, 0x00, 0xb1, 0x19);

static const struct ble_gatt_svc_def gatt_svr_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &gatt_svr_svc_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                // Twist Characteristic (0xFF01)
                .uuid = &gatt_svr_chr_twist_uuid.u,
                .access_cb = gatt_svr_chr_access_robotics,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
            },
            {
                // Arm Characteristic (0xFF02)
                .uuid = &gatt_svr_chr_arm_uuid.u,
                .access_cb = gatt_svr_chr_access_robotics,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {
                // Telemetry Characteristic (0xFF03)
                .uuid = &gatt_svr_chr_telemetry_uuid.u,
                .access_cb = gatt_svr_chr_access_robotics,
                .val_handle = &g_telemetry_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            {
                // Sensor Feed Characteristic (0xFF04)
                .uuid = &gatt_svr_chr_sensors_uuid.u,
                .access_cb = gatt_svr_chr_access_robotics,
                .val_handle = &g_sensors_handle,
                .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
            },
            {
                // System Command / E-STOP Characteristic (0xFF05)
                .uuid = &gatt_svr_chr_estop_uuid.u,
                .access_cb = gatt_svr_chr_access_robotics,
                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ,
            },
            { 0 }
        },
    },
    { 0 }
};

static void ble_host_task(void *param) {
    nimble_port_run();
    nimble_port_freertos_deinit();
}

static void ble_on_sync(void) {
    BleManager::getInstance().startAdvertising();
}

static int ble_gap_event_cb(struct ble_gap_event *event, void *arg) {
    auto& mgr = BleManager::getInstance();
    switch (event->type) {
        case BLE_GAP_EVENT_CONNECT:
            if (event->connect.status == 0) {
                g_conn_handle = event->connect.conn_handle;
                char addr_str[32];
                snprintf(addr_str, sizeof(addr_str), "%02X:%02X:%02X:%02X:%02X:%02X",
                         event->connect.conn_desc.peer_id_addr.val[5],
                         event->connect.conn_desc.peer_id_addr.val[4],
                         event->connect.conn_desc.peer_id_addr.val[3],
                         event->connect.conn_desc.peer_id_addr.val[2],
                         event->connect.conn_desc.peer_id_addr.val[1],
                         event->connect.conn_desc.peer_id_addr.val[0]);
                mgr.onClientConnect(addr_str);
            } else {
                mgr.startAdvertising();
            }
            break;

        case BLE_GAP_EVENT_DISCONNECT:
            g_conn_handle = BLE_HS_CONN_HANDLE_NONE;
            mgr.onClientDisconnect();
            mgr.startAdvertising();
            break;

        case BLE_GAP_EVENT_ADV_COMPLETE:
            mgr.startAdvertising();
            break;
    }
    return 0;
}
#endif

void BleManager::init(const BleConfig& config) {
    if (initialized_) return;
    config_ = config;
    status_.device_name = config.device_name;
    status_.state = BleConnectionState::DISCONNECTED;
    status_.advertising = false;

    // Default callbacks wired to RobotController
    setTwistCallback([](float vx, float vy, float omega, uint8_t buttons) {
        auto& rc = RobotController::getInstance();
        if (buttons & 0x01) {
            rc.emergencyStop();
            return;
        }
        rc.resume();
        
        // Convert normalized -1.0..+1.0 into m/s & rad/s
        float linear_x = vx * 0.8f;   // max 0.8 m/s
        float linear_y = vy * 0.8f;
        float angular_z = omega * 2.0f; // max 2.0 rad/s
        
        rc.setTwist(linear_x, linear_y, angular_z);
    });

    setArmCallback([](const std::vector<float>& joints, uint8_t gripper) {
        auto& rc = RobotController::getInstance();
        ArmJoints aj;
        if (joints.size() >= 5) {
            aj.base_yaw = joints[0];
            aj.shoulder_pitch = joints[1];
            aj.elbow_pitch = joints[2];
            aj.wrist_pitch = joints[3];
            aj.wrist_roll = joints[4];
        }
        aj.gripper = (float)gripper;
        rc.setArmJoints(aj);
    });

    setEstopCallback([](bool active) {
        if (active) {
            RobotController::getInstance().emergencyStop();
        } else {
            RobotController::getInstance().resume();
        }
    });

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    nimble_port_init();
    ble_svc_gap_device_name_set(config_.device_name.c_str());
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svr_svcs);
    ble_gatts_add_svcs(gatt_svr_svcs);

    ble_hs_cfg.sync_cb = ble_on_sync;
    nimble_port_freertos_init(ble_host_task);
#else
    hal_uart_print("[BLE] Native Simulator BLE 5.0 Stack Initialized.\n");
    if (config_.auto_start_advertising) {
        startAdvertising();
    }
#endif

    // Start background telemetry notification task (20 Hz)
    OSScheduler::getInstance().createTask("ble_notify", 4096, 2, CORE_0, [this]() {
        this->backgroundNotifyTask();
    });

    initialized_ = true;
    std::string msg = "[BLE] Bluetooth Low Energy (BLE 5.0) Engine Ready. Device: " + config_.device_name + "\n";
    hal_uart_print(msg.c_str());
}

bool BleManager::startAdvertising() {
    status_.advertising = true;
    status_.state = BleConnectionState::ADVERTISING;

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    memset(&adv_params, 0, sizeof(adv_params));
    memset(&fields, 0, sizeof(fields));

    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    fields.name = (uint8_t *)config_.device_name.c_str();
    fields.name_len = config_.device_name.length();
    fields.name_is_complete = 1;

    ble_gap_adv_set_fields(&fields);

    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    adv_params.itvl_min = (config_.adv_interval_ms * 1000) / 625;
    adv_params.itvl_max = adv_params.itvl_min;

    int rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, ble_gap_event_cb, NULL);
    return (rc == 0);
#else
    std::string msg = "[BLE:SIM] Advertising started on 2.4GHz BLE 5.0 radio as '" + config_.device_name + "'...\n";
    hal_uart_print(msg.c_str());
    return true;
#endif
}

bool BleManager::stopAdvertising() {
    status_.advertising = false;
    if (status_.state == BleConnectionState::ADVERTISING) {
        status_.state = BleConnectionState::DISCONNECTED;
    }
#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    ble_gap_adv_stop();
#else
    hal_uart_print("[BLE:SIM] Advertising stopped.\n");
#endif
    return true;
}

void BleManager::disconnect() {
#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    if (g_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ble_gap_terminate(g_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }
#else
    onClientDisconnect();
#endif
}

bool BleManager::isConnected() const {
    return status_.state == BleConnectionState::CONNECTED;
}

bool BleManager::isAdvertising() const {
    return status_.advertising;
}

BleStatus BleManager::getStatus() const {
    return status_;
}

void BleManager::setTwistCallback(TwistCallback cb) {
    twist_cb_ = cb;
}

void BleManager::setArmCallback(ArmCallback cb) {
    arm_cb_ = cb;
}

void BleManager::setEstopCallback(EstopCallback cb) {
    estop_cb_ = cb;
}

void BleManager::processInboundTwist(const uint8_t* data, size_t len) {
    if (len < sizeof(BleTwistPacket)) return;
    status_.packets_rx++;
    status_.last_activity_ms = get_current_millis();

    const auto* pkt = reinterpret_cast<const BleTwistPacket*>(data);
    float vx = (float)pkt->linear_x_pct / 100.0f;
    float vy = (float)pkt->linear_y_pct / 100.0f;
    float omega = (float)pkt->angular_z_pct / 100.0f;

    if (twist_cb_) {
        twist_cb_(vx, vy, omega, pkt->buttons);
    }
}

void BleManager::processInboundArm(const uint8_t* data, size_t len) {
    if (len < sizeof(BleArmPacket)) return;
    status_.packets_rx++;
    status_.last_activity_ms = get_current_millis();

    const auto* pkt = reinterpret_cast<const BleArmPacket*>(data);
    std::vector<float> joints(6);
    for (int i = 0; i < 6; ++i) {
        joints[i] = (float)pkt->joint_angles_deg_x10[i] / 10.0f;
    }

    if (arm_cb_) {
        arm_cb_(joints, pkt->gripper_pct);
    }
}

void BleManager::processInboundCmd(const uint8_t* data, size_t len) {
    if (len == 0) return;
    status_.packets_rx++;
    status_.last_activity_ms = get_current_millis();

    if (data[0] == 0xFF) { // Emergency Stop
        if (estop_cb_) estop_cb_(true);
    } else if (data[0] == 0x00) { // Clear E-Stop
        if (estop_cb_) estop_cb_(false);
    }
}

void BleManager::onClientConnect(const std::string& peer_addr) {
    status_.state = BleConnectionState::CONNECTED;
    status_.connected_clients = 1;
    status_.peer_mac = peer_addr;
    status_.advertising = false;
    std::string msg = "[BLE] Web Bluetooth / App Client CONNECTED! Peer: " + peer_addr + "\n";
    hal_uart_print(msg.c_str());
}

void BleManager::onClientDisconnect() {
    status_.state = BleConnectionState::DISCONNECTED;
    status_.connected_clients = 0;
    status_.peer_mac = "";
    hal_uart_print("[BLE] Web Bluetooth Client DISCONNECTED.\n");
}

bool BleManager::notifyTelemetry(const BleTelemetryPacket& telemetry) {
    if (!isConnected()) return false;
    status_.packets_tx++;

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    if (g_telemetry_handle != 0 && g_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        struct os_mbuf *om = ble_hs_mbuf_from_flat(&telemetry, sizeof(telemetry));
        ble_gattc_notify_custom(g_conn_handle, g_telemetry_handle, om);
    }
#else
    (void)telemetry;
#endif
    return true;
}

bool BleManager::notifySensors(const BleSensorPacket& sensors) {
    if (!isConnected()) return false;
    status_.packets_tx++;

#if !defined(OS_TARGET_NATIVE) && defined(CONFIG_BT_NIMBLE_ENABLED)
    if (g_sensors_handle != 0 && g_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        struct os_mbuf *om = ble_hs_mbuf_from_flat(&sensors, sizeof(sensors));
        ble_gattc_notify_custom(g_conn_handle, g_sensors_handle, om);
    }
#else
    (void)sensors;
#endif
    return true;
}

void BleManager::backgroundNotifyTask() {
    while (true) {
        OSScheduler::getInstance().delay(50); // 20 Hz notification cycle
        if (isConnected()) {
            auto& rc = RobotController::getInstance();
            auto tel = rc.getTelemetry();
            
            BleTelemetryPacket pkt{};
            pkt.battery_pct = 95;
            pkt.cpu_temp_c = 42;
            pkt.voltage_mv = 11850;
            pkt.uptime_sec = get_current_millis() / 1000;
            pkt.free_psram_kb = 7840;
            pkt.motion_mode = static_cast<uint8_t>(tel.mode);
            pkt.safety_flags = tel.emergency_stop ? 0x01 : 0x00;
            pkt.odom_x_m = 0.0f;
            pkt.odom_y_m = 0.0f;
            pkt.odom_theta_rad = 0.0f;
            notifyTelemetry(pkt);

            BleSensorPacket sens{};
            sens.roll_deg = 0.2f;
            sens.pitch_deg = -0.5f;
            sens.yaw_deg = 0.0f;
            sens.distance_mm = static_cast<uint16_t>(tel.obstacle_distance_cm * 10.0f);
            sens.pressure_hpa_x10 = 10132;
            notifySensors(sens);
        }
    }
}

std::string BleManager::getBleJson() const {
    std::ostringstream ss;
    ss << "{"
       << "\"status\":\"" << bleConnectionStateToString(status_.state) << "\","
       << "\"device_name\":\"" << status_.device_name << "\","
       << "\"advertising\":" << (status_.advertising ? "true" : "false") << ","
       << "\"connected_clients\":" << status_.connected_clients << ","
       << "\"peer_mac\":\"" << status_.peer_mac << "\","
       << "\"rssi_dbm\":" << (int)status_.rssi_dbm << ","
       << "\"packets_rx\":" << status_.packets_rx << ","
       << "\"packets_tx\":" << status_.packets_tx << ","
       << "\"service_uuid\":\"" << BLE_ROBOTICS_SERVICE_UUID << "\","
       << "\"char_twist_uuid\":\"" << BLE_CHAR_JOYSTICK_TWIST_UUID << "\","
       << "\"char_arm_uuid\":\"" << BLE_CHAR_ROBOTIC_ARM_UUID << "\","
       << "\"char_telemetry_uuid\":\"" << BLE_CHAR_TELEMETRY_UUID << "\","
       << "\"char_sensor_uuid\":\"" << BLE_CHAR_SENSOR_FEED_UUID << "\""
       << "}";
    return ss.str();
}

} // namespace TamimysticOS
