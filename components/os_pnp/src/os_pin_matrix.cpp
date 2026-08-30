#include "os_pin_matrix.h"
#include "os_config.h"
#include "os_hal_gpio.h"
#include "os_hal_uart.h"
#include "os_event_bus.h"
#include <sstream>

namespace TamimysticOS {

PinMatrixManager& PinMatrixManager::getInstance() {
    static PinMatrixManager instance;
    return instance;
}

void PinMatrixManager::init() {
    // Populate function lookup map
    name_to_func = {
        {"i2c_sda", PinFunction::I2C_SDA},
        {"i2c_scl", PinFunction::I2C_SCL},
        {"motor_l_in1", PinFunction::MOTOR_L_IN1},
        {"motor_l_in2", PinFunction::MOTOR_L_IN2},
        {"motor_l_pwm", PinFunction::MOTOR_L_PWM},
        {"motor_r_in3", PinFunction::MOTOR_R_IN3},
        {"motor_r_in4", PinFunction::MOTOR_R_IN4},
        {"motor_r_pwm", PinFunction::MOTOR_R_PWM},
        {"servo_pwm", PinFunction::SERVO_PWM},
        {"encoder_l_a", PinFunction::ENCODER_L_A},
        {"encoder_l_b", PinFunction::ENCODER_L_B},
        {"encoder_r_a", PinFunction::ENCODER_R_A},
        {"encoder_r_b", PinFunction::ENCODER_R_B},
        {"ultrasonic_trig", PinFunction::ULTRASONIC_TRIG},
        {"ultrasonic_echo", PinFunction::ULTRASONIC_ECHO},
        {"status_led", PinFunction::STATUS_LED},
        {"buzzer", PinFunction::BUZZER}
    };

    resetToDefaults();
    loadFromNVS();

    hal_uart_print("[PIN_MATRIX] Initialized with persistent NVS backing.\n");
}

void PinMatrixManager::resetToDefaults() {
    // Default safe pin configuration for ESP32-S3
    pin_map[PinFunction::I2C_SDA] = 21;
    pin_map[PinFunction::I2C_SCL] = 22;
    pin_map[PinFunction::MOTOR_L_IN1] = 4;
    pin_map[PinFunction::MOTOR_L_IN2] = 5;
    pin_map[PinFunction::MOTOR_L_PWM] = 6;
    pin_map[PinFunction::MOTOR_R_IN3] = 7;
    pin_map[PinFunction::MOTOR_R_IN4] = 15;
    pin_map[PinFunction::MOTOR_R_PWM] = 16;
    pin_map[PinFunction::SERVO_PWM] = 8;
    pin_map[PinFunction::ENCODER_L_A] = 9;
    pin_map[PinFunction::ENCODER_L_B] = 10;
    pin_map[PinFunction::ENCODER_R_A] = 11;
    pin_map[PinFunction::ENCODER_R_B] = 12;
    pin_map[PinFunction::ULTRASONIC_TRIG] = 13;
    pin_map[PinFunction::ULTRASONIC_ECHO] = 14;
    pin_map[PinFunction::STATUS_LED] = 48;
    pin_map[PinFunction::BUZZER] = 17;
}

void PinMatrixManager::loadFromNVS() {
    auto& cfg = ConfigManager::getInstance();
    for (const auto& pair : name_to_func) {
        std::string nvs_key = "pin_" + pair.first;
        int saved_pin = cfg.getInt(nvs_key, -1);
        if (saved_pin >= 0 && isSafePin(saved_pin)) {
            pin_map[pair.second] = saved_pin;
        }
    }
}

void PinMatrixManager::saveToNVS(const std::string& func_name, int gpio_num) {
    auto& cfg = ConfigManager::getInstance();
    std::string nvs_key = "pin_" + func_name;
    cfg.setInt(nvs_key, gpio_num);
}

bool PinMatrixManager::isSafePin(int pin) const {
    return hal_gpio_is_safe_user_pin(pin);
}

bool PinMatrixManager::setPin(const std::string& func_name, int gpio_num) {
    auto it = name_to_func.find(func_name);
    if (it == name_to_func.end()) {
        hal_uart_print(("[PIN_MATRIX] Unknown function: " + func_name + "\n").c_str());
        return false;
    }
    return setPin(it->second, gpio_num);
}

bool PinMatrixManager::setPin(PinFunction func, int gpio_num) {
    if (gpio_num != -1 && !isSafePin(gpio_num)) {
        hal_uart_print(("[PIN_MATRIX] Error: GPIO" + std::to_string(gpio_num) + " is reserved or unsafe on ESP32-S3!\n").c_str());
        return false;
    }

    pin_map[func] = gpio_num;

    // Find function name to save to NVS
    for (const auto& pair : name_to_func) {
        if (pair.second == func) {
            saveToNVS(pair.first, gpio_num);
            hal_uart_print(("[PIN_MATRIX] Re-mapped " + pair.first + " -> GPIO" + std::to_string(gpio_num) + "\n").c_str());
            break;
        }
    }

    // Publish event
    EventBus::getInstance().publish(EventTopic::PIN_CONFIG_CHANGED);
    return true;
}

int PinMatrixManager::getPin(PinFunction func) const {
    auto it = pin_map.find(func);
    if (it != pin_map.end()) {
        return it->second;
    }
    return -1;
}

int PinMatrixManager::getPin(const std::string& func_name) const {
    auto it = name_to_func.find(func_name);
    if (it != name_to_func.end()) {
        return getPin(it->second);
    }
    return -1;
}

std::vector<PinMappingItem> PinMatrixManager::getAllMappings() const {
    std::vector<PinMappingItem> list;
    for (const auto& pair : name_to_func) {
        int pin = getPin(pair.second);
        list.push_back({
            pair.second,
            pair.first,
            pinFunctionToString(pair.second),
            pin,
            isSafePin(pin),
            "ESP32-S3 Dynamic Peripheral Matrix"
        });
    }
    return list;
}

std::string PinMatrixManager::getPinMatrixJson() const {
    std::stringstream ss;
    ss << "{\"status\":\"ok\",\"pins\":[";
    bool first = true;
    for (const auto& item : getAllMappings()) {
        if (!first) ss << ",";
        first = false;
        ss << "{"
           << "\"func_name\":\"" << item.func_name << "\","
           << "\"label\":\"" << item.label << "\","
           << "\"gpio\":" << item.gpio_pin << ","
           << "\"safe\":" << (item.is_safe ? "true" : "false")
           << "}";
    }
    ss << "]}";
    return ss.str();
}

} // namespace TamimysticOS
