# 🌐 HTTP REST API & Web Endpoints Reference

Tamimystic OS provides a complete HTTP REST API on Port 80, enabling integration with ROS2 nodes, mobile applications, web frontends, and IoT dashboards.

---

## 📌 Endpoint Summary

| Category | HTTP Method | URI Path | Description |
|---|---|---|---|
| **Dashboard** | `GET` | `/` | Returns the HTML5 Glassmorphism Web Dashboard. |
| **Robotics** | `POST` | `/api/robot/mode?mode=<diff\|mecanum\|arm>` | Switches active robot mode. |
| | `POST` | `/api/robot/cmd_vel?vx=..&vy=..&w=..` | Commands rover / Mecanum velocity. |
| | `POST` | `/api/robot/arm?j1=..&j2=..&j3=..` | Sets individual robotic arm joint angles. |
| | `POST` | `/api/robot/arm/ik?x=..&y=..&z=..` | Commands Cartesian Inverse Kinematics target. |
| | `POST` | `/api/robot/stop` | Engages Emergency Stop. |
| | `POST` | `/api/robot/resume` | Releases Emergency Stop. |
| | `GET` | `/api/robot/telemetry` | Returns real-time kinematics and sensor JSON. |
| **Vision & AI**| `GET` | `/api/camera/snapshot` | Streams live JPEG camera frame. |
| | `POST` | `/api/ai/model?model=<person\|object\|lane>` | Switches active neural model. |
| | `POST` | `/api/ai/track?enable=<1\|0>` | Enables / disables visual tracking loop. |
| | `GET` | `/api/ai/status` | Returns latest detection bounding boxes JSON. |
| **PnP & Pins** | `GET` | `/api/pnp/devices` | Returns list of discovered I2C sensors. |
| | `POST` | `/api/pnp/scan` | Initiates active I2C bus scan. |
| | `GET` | `/api/pins` | Returns pin matrix assignments JSON. |
| | `POST` | `/api/pins/set?func=..&pin=..` | Reassigns a pin function. |
| **Apps & VFS** | `POST` | `/api/apps/eval` | Evaluates raw Python script in request body. |
| | `POST` | `/api/apps/stop` | Halts running Python script. |
| | `GET` | `/api/files/list` | Returns list of files and disk usage JSON. |
| | `POST` | `/api/files/delete?name=..` | Deletes a file from flash storage. |

---

## 📊 Sample JSON Telemetry Payloads

### `GET /api/robot/telemetry`
```json
{
  "status": "ok",
  "mode": "Mecanum 4WD (Holonomic)",
  "mode_id": 1,
  "twist": {"vx": 50.0, "vy": -20.0, "w": 10.0},
  "wheels": {"fl": 20.0, "fr": 80.0, "rl": 80.0, "rr": 20.0},
  "arm_pose": {"x": 15.0, "y": 10.0, "z": 12.0, "pitch": 0.0, "gripper": 50.0},
  "arm_joints": {"j1": 33.7, "j2": 45.2, "j3": 88.1, "j4": -43.3, "j5": 90.0, "j6": 50.0},
  "obstacle_dist_cm": 24.8,
  "e_stop": false,
  "braking": false,
  "pca9685": true
}
```

### `GET /api/ai/status`
```json
{
  "status": "ok",
  "model": "MobileNet-V2 Person Detector",
  "fps": 22.5,
  "inference_time_ms": 18,
  "target_locked": true,
  "visual_tracking": true,
  "primary_label": "Person",
  "confidence": 94.8,
  "detections": [
    {"x": 0.52, "y": 0.48, "w": 0.28, "h": 0.62, "label": "Person", "confidence": 94.8}
  ]
}
```
