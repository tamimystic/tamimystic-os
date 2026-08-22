# Installation Guide

The installation process for Tamimystic OS is designed to be fully reproducible. The continuous integration pipeline automatically compiles the system binaries upon every verified commit.

## Obtaining the Firmware Binaries

1. Navigate to the GitHub Actions page of the repository.
2. Select the latest successful build artifact labeled `tamimystic_os_firmware`.
3. Extract the downloaded archive. The archive contains the following critical binary files:
    *   `bootloader.bin`: The second-stage bootloader responsible for initializing flash and PSRAM.
    *   `partition-table.bin`: The memory map defining the boundaries for NVS, OTA, and Virtual File Systems (SPIFFS).
    *   `tamimystic_os.bin`: The core operating system executable.

## Flashing the Device (Windows)

The standard procedure utilizes the Espressif Flash Download Tool.

1.  Download the official Espressif Flash Download Tool.
2.  Launch the application and select **ESP32-S3** as the Target Chip and **Develop** as the WorkMode.
3.  Load the binaries into the flashing queue and assign their explicit memory offsets:
    *   `bootloader.bin` at offset `0x0`
    *   `partition-table.bin` at offset `0x8000`
    *   `tamimystic_os.bin` at offset `0x10000`
4.  Configure the hardware parameters to match the ESP32-S3 N16R8 specification:
    *   SPI SPEED: **80MHz**
    *   SPI MODE: **QIO**
    *   FLASH SIZE: **16MB**
5.  Select the corresponding COM port, set the baud rate to `460800`, and initiate the flashing process.

## Flashing via Command Line (esptool.py)

For automated environments, the `esptool.py` command-line utility is recommended.

```bash
esptool.py -p COM3 -b 460800 --before default_reset --after hard_reset --chip esp32s3 write_flash --flash_mode qio --flash_size 16MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 tamimystic_os.bin
```

Upon successful flashing, issue a hardware reset to the board to begin the initialization sequence.
