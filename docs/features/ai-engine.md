# Edge AI Engine & Computer Vision Pipeline

Tamimystic OS provides an integrated on-device Computer Vision and Deep Learning subsystem that runs quantized neural network models natively on the ESP32-S3 @ 20+ FPS, without reliance on cloud APIs.

---

## 📷 DVP Camera Pipeline with 8MB Octal PSRAM

- **Supported Sensors**: OV2640, OV3660, OV5640 (DVP parallel interface).
- **Framebuffer Architecture**:
  - Allocated in 8MB Octal-SPI PSRAM via `CAMERA_FB_IN_PSRAM`.
  - Triple buffering mode (`fb_count = 2`, `CAMERA_GRAB_LATEST`) ensures tear-free high-framerate streaming without stalling the FreeRTOS scheduler.
- **Dynamic Bitstream Generation**: Built-in baseline JPEG encoder for real-time web dashboard streaming.

---

## 🧠 TensorFlow Lite Micro & ESP-NN Vector SIMD

- **Xtensa LX7 Vector Acceleration**: Matrix multiplications, convolutions, and depthwise separable convolutions are accelerated using ESP-NN SIMD vector assembly instructions.
- **Dedicated Core Allocation**: AI inference executes on a high-priority FreeRTOS task pinned to **Core 1**, completely isolating neural compute from Core 0 network traffic and FreeRTOS OS housekeeping.

---

## 📦 Onboard Neural Models

1. **MobileNet-V2 Person Detector**:
   - Detects humans in camera frame with normalized bounding boxes $(x, y, w, h)$ and confidence scores.
2. **MobileNet-SSD Multi-Object Detector**:
   - Classifies multiple objects simultaneously: Persons, Vehicles, Traffic Cones, Balls, and Obstacles.
3. **Autonomous Road Lane & Line Follower**:
   - Computes lateral lane deviation ($\pm 100\%$) and heading angle error ($\theta^\circ$) for autonomous road driving.
4. **Hand Gesture Neural Classifier**:
   - Recognizes dynamic hand gestures: *Stop*, *Drive Forward*, *Turn Left*, *Turn Right*, *Gripper Open/Close*.

---

## 🎯 Autonomous Visual Target Tracking

The vision autonomy loop directly binds neural detection output with the kinematics engine:

$$\begin{aligned}
\text{Angular Error: } \Delta x &= x_{\text{target}} - 0.5 \\
\omega_{\text{turn}} &= -K_p \cdot \Delta x \cdot 100.0\% \\
v_x &= \begin{cases} 40.0\% & \text{if target locked and } \text{width} < 0.35 \\ 0.0\% & \text{otherwise} \end{cases}
\end{aligned}$$

```bash
# Enable real-time visual tracking
aeron> ai track on

# Switch active model to object detection
aeron> ai model object

# Inspect AI pipeline status
aeron> ai status
```
