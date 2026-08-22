# First Boot and Configuration

Upon receiving a hard reset after flashing, Tamimystic OS initiates its primary boot sequence. This document details the expected behavior and configuration steps required to bring the system online.

## Initialization Sequence

When the device powers on, the following sequence occurs internally:
1.  **Hardware Verification**: The OS validates the presence and integrity of the 8MB PSRAM and 16MB Flash memory.
2.  **Virtual File System (VFS) Mounting**: The SPIFFS partition is mounted. If this is the first boot, the OS automatically formats the partition.
3.  **Non-Volatile Storage (NVS) Loading**: The OS reads the `os_config` registry to determine network states and peripheral bindings.
4.  **Network Initialization**: If no valid Wi-Fi credentials exist in the NVS, the OS defaults to Access Point (AP) mode.

## Accessing the Configuration Dashboard

1.  Open the Wi-Fi settings on your host machine (PC or Smartphone).
2.  Scan for a network SSID named **TamimysticOS_Setup** (or similar fallback SSID).
3.  Connect to the network.
4.  Open a modern web browser and navigate to the default gateway IP address (typically `http://192.168.4.1`).

## Network and Peripheral Configuration

The Web Dashboard serves as the primary interface for system configuration.

*   **Station Mode Setup**: Enter your local router's SSID and Password to transition the device from AP mode to Station mode. Upon saving, the OS will attempt to connect to the provided network and assign itself a local IP via DHCP.
*   **Hardware Routing**: Navigate to the peripherals tab. Here, you can map physical ESP32-S3 pins to logical OS functions. For example, you can map Pin 18 and 19 to `MotorDriver_Left`. The OS writes this mapping to NVS, ensuring it persists across reboots.
*   **Application Uploads**: Use the Over-The-Air (OTA) file manager to upload your Python scripts (`main.py`) directly to the device's Virtual File System.
