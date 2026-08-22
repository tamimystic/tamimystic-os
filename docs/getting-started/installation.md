# Installation Guide

Installing Tamimystic OS is incredibly straightforward. Since we utilize a CI/CD pipeline, every successful commit automatically generates the compiled `.bin` firmware files.

## 📥 Download Firmware
1. Go to the [GitHub Actions page](https://github.com/tamimystic/tamimystic-os/actions).
2. Click on the latest successful `Build ESP32-S3 Firmware` run.
3. Scroll down to **Artifacts** and download the `tamimystic_os_firmware.zip` file.
4. Unzip the file. You will need:
    * `bootloader.bin`
    * `partition-table.bin`
    * `tamimystic_os.bin`

## ⚡ Flashing the ESP32-S3

### Using Espressif Flash Download Tool (Windows)
1. Download the [Espressif Flash Download Tool](https://www.espressif.com/en/support/download/other-tools).
2. Run the tool and select **ESP32-S3** and **Develop** mode.
3. Check the first three boxes and select your `.bin` files. Set their addresses exactly as follows:
    * `bootloader.bin` @ `0x0`
    * `partition-table.bin` @ `0x8000`
    * `tamimystic_os.bin` @ `0x10000`
4. Set the following configurations:
    * SPI SPEED: **80MHz**
    * SPI MODE: **QIO**
    * FLASH SIZE: **16MB** (or 128Mbit)
5. Select your COM port, set the BAUD rate to `460800` or `921600`, and click **START**.

### Using esptool.py (CLI)
If you prefer the command line, ensure you have Python installed and run:
```bash
pip install esptool
esptool.py -p COM3 -b 460800 --before default_reset --after hard_reset --chip esp32s3  write_flash --flash_mode qio --flash_size 16MB --flash_freq 80m 0x0 bootloader.bin 0x8000 partition-table.bin 0x10000 tamimystic_os.bin
```

---
Once flashing is complete (the progress bar reaches 100%), hit the `RST` (Reset) button on your board. You are now ready for the [First Boot](first-boot.md)!
