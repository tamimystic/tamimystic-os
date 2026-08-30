# Dynamic Scripting Engine, Flash VFS & Web IDE

Tamimystic OS embeds a dynamic scripting engine with high-level Python and WASM runtime support, enabling rapid prototyping without compiling firmware.

---

## 🐍 Native Python API Reference

User scripts access OS hardware and robotics primitives via the `tamimystic` module:

```python
import tamimystic

# 1. Drive Robot Platform
tamimystic.robot.move(linear_speed=60, angular_speed=0)

# 2. Control 6-DOF Robotic Arm via Inverse Kinematics
tamimystic.robot.ik(x=15.0, y=0.0, z=12.0)

# 3. Read Sensors
distance = tamimystic.sensor.read_distance()
print("Front Obstacle Distance:", distance)

# 4. Hardware GPIO Control
tamimystic.gpio.write(pin=48, value=1)

# 5. Non-Blocking Delay
tamimystic.delay(1000)
```

---

## 📁 6.8MB LittleFS Flash Virtual File System (VFS)

The 16MB partition table allocates **6.8MB** exclusively for user applications and file storage:

```csv
# Name,     Type, SubType, Offset,   Size,     Flags
nvs,        data, nvs,     0x9000,   0x7000,
otadata,    data, ota,     0x10000,  0x2000,
phy_init,   data, phy,     0x12000,  0x1000,
app0,       app,  ota_0,   0x20000,  4500K,
app1,       app,  ota_1,   ,         4500K,
storage,    data, spiffs,  ,         6800K,
```

- If an `autorun.py` file is present in the `/storage` root, Tamimystic OS executes it automatically on startup.

---

## 🌐 In-Browser Web Python IDE & File Manager

Access the OS dashboard by connecting to the ESP32 Wi-Fi network and navigating to `http://<device-ip>/`:

- **Interactive Code Editor**: Write and test Python scripts directly from your web browser with syntax highlighting.
- **Run / Stop Control**: Instantly execute or stop scripts on the hardware.
- **Flash File Manager**: Upload, download, inspect, and delete files on the 6.8MB partition.
- **Dual-Bank OTA Widget**: Upload `.bin` firmware updates directly through the browser with automated verification and rollback protection.
