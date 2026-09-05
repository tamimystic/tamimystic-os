# Flashing and Installation Guide

This guide covers flashing Tamimystic OS onto your ESP32-S3-N16R8 board using either the command line (`esptool.py`), GUI tools (ESP Flash Download Tool), or Web Flasher.

---

## Download Firmware Binaries

You can download the latest automated firmware binaries directly from the **[GitHub Releases Page](https://github.com/tamimystic/tamimystic-os/releases)** or from the latest **[GitHub Actions Build Artifacts](https://github.com/tamimystic/tamimystic-os/actions/workflows/build.yml)**.

The firmware package contains:
1. `bootloader.bin` (Second stage bootloader @ `0x0000`)
2. `partition-table.bin` (16MB Dual OTA and LittleFS partition table @ `0x8000`)
3. `tamimystic_os.bin` (Main Operating System application image @ `0x20000`)

---

## Method 1: Flashing via `esptool.py` (Recommended)

`esptool.py` is the official, high-speed Python command-line utility from Espressif.

### Step 1: Install `esptool`
If you have Python installed, run:
```bash
pip install esptool
```

### Step 2: Put ESP32-S3 in Bootloader Mode
1. Connect your ESP32-S3 board to your PC via the **USB-to-UART** or **Native USB** port.
2. Hold down the **BOOT** button (GPIO 0).
3. Press and release the **RESET / EN** button.
4. Release the **BOOT** button.

### Step 3: Erase Flash (Recommended for First Install)
```bash
# Replace COMx with your port (e.g., COM3 on Windows or /dev/ttyUSB0 on Linux)
esptool.py --port COM3 erase_flash
```

### Step 4: Flash the Complete Binary Set
```bash
esptool.py --chip esp32s3 --port COM3 --baud 921600 \
  --before default_reset --after hard_reset write_flash -z \
  --flash_mode dio --flash_freq 80m --flash_size 16MB \
  0x0000 bootloader.bin \
  0x8000 partition-table.bin \
  0x20000 tamimystic_os.bin
```

---

## Partition Table Architecture (16MB Layout)

Tamimystic OS utilizes a customized partition layout that fully leverages the 16MB Quad-SPI flash:

| Partition Name | Type | SubType | Offset | Size | Purpose |
|---|---|---|---|---|---|
| `nvs` | Data | NVS | `0x9000` | **28 KB** | Non-Volatile Storage (Pin Matrix, Wi-Fi credentials, Robot modes) |
| `otadata` | Data | OTA | `0x10000` | **8 KB** | Active OTA boot-slot selector and rollback state |
| `phy_init` | Data | PHY | `0x12000` | **4 KB** | Wi-Fi RF physical calibration parameters |
| `app0` | App | OTA_0 | `0x20000` | **4.5 MB** | Active firmware slot (Primary OS binary) |
| `app1` | App | OTA_1 | Automatic | **4.5 MB** | Backup firmware slot (Secondary OTA update target) |
| `storage` | Data | LittleFS | Automatic | **6.8 MB** | User Virtual File System (MicroPython scripts, AI models, logs) |

---

## Method 2: Testing on PC via Native Simulator

If you do not have physical hardware on hand, you can run the full Tamimystic OS simulator natively on Windows or Linux!

### On Windows:
```bash
# From the root repository directory
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
./tamimystic_os_sim.exe
```

### On Linux / macOS:
```bash
mkdir build && cd build
cmake ..
make -j4
./tamimystic_os_sim
```

You will be greeted with the interactive `aeron>` serial terminal and simulated Web Dashboard!
