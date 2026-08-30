# 📖 Serial CLI Command Reference (`aeron>`)

The Tamimystic OS interactive shell (`aeron>`) is available over the primary UART console at **115200 baud**.

---

## 📋 Comprehensive Command Table

| Command | Subcommands | Arguments | Description |
|---|---|---|---|
| `help` | — | None | Displays list of all available commands and syntax. |
| `wifi` | — | `<ssid> <password>` | Connects the OS to a local Wi-Fi access point. |
| `pin` | `show` | None | Lists all mapped peripheral functions, assigned GPIOs, and safety status. |
| | `set` | `<func_name> <gpio>` | Reassigns a pin function and persists to NVS. |
| | `reset` | None | Restores factory default pin mapping. |
| `pnp` | `scan` | None | Runs active I2C bus discovery sweep ($0x08 - 0x77$). |
| | `list` | None | Displays all currently identified sensors and hardware modules. |
| `robot` | `mode` | `<diff \| mecanum \| arm \| balance>` | Switches active robotics topology. |
| | `move` | `<linear_spd> <angular_spd>` | Commands 2WD/4WD rover movement ($-100$ to $100\%$). |
| | `strafe` | `<vx> <vy> <omega>` | Commands Mecanum holonomic velocity vector. |
| | `arm` | `<j1> <j2> <j3> <j4> <j5> <j6>` | Sets 6-DOF robotic arm joint angles ($0^\circ - 180^\circ$). |
| | `ik` | `<x> <y> <z> [pitch] [grip]` | Computes and moves arm tip to Cartesian coordinate in cm. |
| | `stop` | None | Engages emergency brake and halts all actuators. |
| | `resume` | None | Releases emergency stop. |
| | `status` | None | Prints detailed robotics kinematics telemetry JSON. |
| `ai` | `status` | None | Displays model FPS, inference latency, and detection confidence. |
| | `model` | `<person \| object \| lane \| gesture>` | Switches active neural network model. |
| | `track` | `<on \| off>` | Toggles autonomous visual target tracking loop. |
| `camera` | `status` | None | Checks DVP driver state and PSRAM buffer allocation. |
| | `snap` | None | Captures a test camera snapshot frame. |
| `python` | `eval` | `"<python_code>"` | Executes raw Python string expression. |
| | `run` | `"<filename.py>"` | Runs a saved Python script from flash storage. |
| | `stop` | None | Terminates active executing script. |
| `wasm` | `run` | `"<filename.wasm>"` | Executes a WebAssembly bytecode module in PSRAM sandbox. |
| `storage` | `ls` | None | Lists all files and sizes in the 6.8MB LittleFS partition. |
| | `df` | None | Displays total, used, and free flash storage in KB. |
| | `rm` | `"<filename>"` | Deletes a file from flash storage. |
| `ota` | `status` | None | Inspects active running partition slot and rollback state. |
