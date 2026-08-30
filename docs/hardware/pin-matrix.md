# 🔀 Dynamic Software Pin Matrix

The **Dynamic Software Pin Matrix** is one of the cornerstone innovations of Tamimystic OS. It eliminates the traditional requirement of hardcoded GPIO pin definitions in embedded C++.

---

## 💡 How It Works

Peripherals (such as I2C SDA/SCL, Motor PWM, Encoders, Servos, Ultrasonic Sensors) are represented as abstract **Pin Functions**.

At runtime:
1. The OS queries the active pin configuration from Non-Volatile Storage (NVS).
2. The user or web dashboard can modify any pin assignment instantly.
3. When a pin assignment changes, the OS triggers a `PIN_CONFIG_CHANGED` event on the internal Event Bus, re-initializing the affected hardware peripheral drivers automatically.

```mermaid
graph LR
    UI["Web Dashboard / CLI"] -->|Set Pin (e.g. i2c_sda -> 21)| MGR["Pin Matrix Manager"]
    MGR -->|Persist Key-Value| NVS["NVS Storage (Flash)"]
    MGR -->|Publish PIN_CONFIG_CHANGED| BUS["Event Bus"]
    BUS -->|Re-initialize Driver| HAL["HAL Drivers (I2C, PWM, GPIO)"]
```

---

## 🔒 Hardware Safety & Octal PSRAM Protection Filter

To protect your ESP32-S3-N16R8 hardware against fatal crashes or permanent silicon damage, the Pin Matrix enforces strict safety filters:

| Pin Range | Status | Reason |
|---|---|---|
| **GPIO 33, 34, 35, 36, 37** | ⛔ **LOCKED** | Dedicated high-speed Octal-SPI PSRAM & Quad Flash bus. Rewiring causes immediate crash. |
| **GPIO 19, 20** | ⚠️ **RESERVED** | Native USB D+/D- and JTAG debugging lines. |
| **GPIO 45, 46** | ⚠️ **STRAPPING** | Boot voltage / VDD_SPI strapping pins. |
| **GPIO 1 - 18, 21, 38 - 44, 47, 48** | ✅ **SAFE** | Fully available for general-purpose user peripheral routing. |

If a user or script attempts to assign a function to a locked pin (e.g., `pin set motor_pwm 35`), the OS rejects the command with an explicit warning message.

---

## 📋 Default Pin Configuration Table

| Function Name | String Identifier | Default GPIO | Safe to Remap? | Function Description |
|---|---|---|---|---|
| `I2C_SDA` | `i2c_sda` | **GPIO 21** | ✅ Yes | I2C Master Data line |
| `I2C_SCL` | `i2c_scl` | **GPIO 22** | ✅ Yes | I2C Master Clock line |
| `MOTOR_L_IN1` | `motor_l_in1` | **GPIO 4** | ✅ Yes | Left Motor H-Bridge Direction 1 |
| `MOTOR_L_IN2` | `motor_l_in2` | **GPIO 5** | ✅ Yes | Left Motor H-Bridge Direction 2 |
| `MOTOR_L_PWM` | `motor_l_pwm` | **GPIO 6** | ✅ Yes | Left Motor Speed PWM (10kHz) |
| `MOTOR_R_IN3` | `motor_r_in3` | **GPIO 7** | ✅ Yes | Right Motor H-Bridge Direction 1 |
| `MOTOR_R_IN4` | `motor_r_in4` | **GPIO 15** | ✅ Yes | Right Motor H-Bridge Direction 2 |
| `MOTOR_R_PWM` | `motor_r_pwm` | **GPIO 16** | ✅ Yes | Right Motor Speed PWM (10kHz) |
| `SERVO_PWM` | `servo_pwm` | **GPIO 8** | ✅ Yes | Onboard PWM RC Servo fallback |
| `ENCODER_L_A` | `encoder_l_a` | **GPIO 9** | ✅ Yes | Left Wheel Quadrature Encoder Phase A |
| `ENCODER_L_B` | `encoder_l_b` | **GPIO 10** | ✅ Yes | Left Wheel Quadrature Encoder Phase B |
| `ENCODER_R_A` | `encoder_r_a` | **GPIO 11** | ✅ Yes | Right Wheel Quadrature Encoder Phase A |
| `ENCODER_R_B` | `encoder_r_b` | **GPIO 12** | ✅ Yes | Right Wheel Quadrature Encoder Phase B |
| `ULTRASONIC_TRIG` | `ultrasonic_trig` | **GPIO 13** | ✅ Yes | Ultrasonic Distance Sensor Trigger |
| `ULTRASONIC_ECHO` | `ultrasonic_echo` | **GPIO 14** | ✅ Yes | Ultrasonic Distance Sensor Echo |
| `STATUS_LED` | `status_led` | **GPIO 48** | ✅ Yes | Built-in RGB / Status LED |
| `BUZZER` | `buzzer` | **GPIO 38** | ✅ Yes | Audio Alert PWM Buzzer |

---

## 💻 CLI & HTTP Commands

### Using Serial CLI:
```bash
# View all current pin assignments
aeron> pin show

# Change I2C pins to GPIO 1 and GPIO 2
aeron> pin set i2c_sda 1
aeron> pin set i2c_scl 2

# Reset all pins to factory defaults
aeron> pin reset
```

### Using HTTP REST API:
```bash
# Get pin matrix JSON
curl http://<device-ip>/api/pins

# Set pin
curl -X POST "http://<device-ip>/api/pins/set?func=motor_l_pwm&pin=8"
```
