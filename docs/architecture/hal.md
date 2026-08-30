# Universal Hardware Abstraction Layer & Plug and Play Matrix

The Tamimystic OS Hardware Abstraction Layer (HAL) abstracts ESP32-S3 peripherals (GPIO, I2C, SPI, PWM, UART) into a software-configurable matrix with automatic sensor discovery.

---

## 🔍 Plug & Play (PnP) Auto-Discovery Registry

Tamimystic OS maintains an onboard signature database of 15+ industry-standard I2C sensors and actuators:

| I2C Address | Device Name | Category | Primary Function |
|---|---|---|---|
| `0x68` / `0x69` | **MPU-6050** | IMU / Motion | 6-Axis Accelerometer & Gyroscope |
| `0x76` / `0x77` | **BME280 / BMP280** | Environmental | Temperature, Humidity & Barometric Pressure |
| `0x3C` / `0x3D` | **SSD1306** | Display | $128 \times 64$ Monochrome OLED Display |
| `0x29` | **VL53L0X** | Distance / ToF | Time-of-Flight Laser Distance Ranging ($2\text{ cm} - 200\text{ cm}$) |
| `0x40` | **PCA9685** | Actuator Expander | 16-Channel 12-bit I2C PWM / Servo Expander |
| `0x48` | **ADS1115** | ADC / Sensor | 16-bit 4-Channel Precision Analog-to-Digital Converter |

On boot or when requested via the Web UI / CLI (`pnp scan`), the OS scans the I2C bus and matches device who-am-i registers to auto-configure appropriate drivers.

---

## 🔀 Dynamic Software Pin Matrix with NVS Persistence

Users can re-route any peripheral to any safe ESP32-S3 GPIO pin at runtime without recompiling firmware:

```bash
# View current pin assignments
aeron> pin show

# Reassign I2C SDA to GPIO 21 and SCL to GPIO 22
aeron> pin set i2c_sda 21
aeron> pin set i2c_scl 22

# Reset pin matrix to factory defaults
aeron> pin reset
```

### 🔒 Octal PSRAM & Flash Protection Filter
To prevent bricking or crashing the ESP32-S3 N16R8, the pin matrix enforces strict protection rules:
- **GPIO 33, 34, 35, 36, 37**: Permanently locked (dedicated to high-speed 8MB Octal PSRAM / Flash bus).
- **GPIO 19, 20**: Reserved for USB-JTAG / Native USB OTG.
- **GPIO 45, 46**: Strapping pins guarded.
