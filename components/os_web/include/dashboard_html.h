#pragma once

const char* dashboard_html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no">
    <title>Tamimystic OS | ESP32-S3 Robotics & Cloud IDE</title>
    <style>
        :root {
            --bg: #090d16;
            --surface: #131b2e;
            --surface-card: #182238;
            --primary: #38bdf8;
            --primary-glow: rgba(56, 189, 248, 0.4);
            --accent: #818cf8;
            --success: #34d399;
            --warning: #fbbf24;
            --danger: #f87171;
            --text: #f1f5f9;
            --text-muted: #94a3b8;
            --border: rgba(255, 255, 255, 0.08);
        }
        
        * { margin: 0; padding: 0; box-sizing: border-box; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
        body { background-color: var(--bg); color: var(--text); min-height: 100vh; display: flex; flex-direction: column; }
        
        header {
            background-color: var(--surface); padding: 16px 32px;
            display: flex; justify-content: space-between; align-items: center;
            border-bottom: 1px solid var(--border); box-shadow: 0 4px 20px rgba(0,0,0,0.4);
        }
        
        .brand { font-size: 20px; font-weight: 800; display: flex; align-items: center; gap: 10px; }
        .brand span { color: var(--primary); }
        .brand-chip { font-size: 11px; background: rgba(56, 189, 248, 0.15); color: var(--primary); padding: 3px 8px; border-radius: 6px; font-weight: bold; border: 1px solid rgba(56, 189, 248, 0.3); }
        .status-badge { background: var(--success); color: #022c22; padding: 5px 12px; border-radius: 20px; font-size: 12px; font-weight: 700; }
        
        .container { flex: 1; padding: 24px; display: grid; grid-template-columns: repeat(auto-fit, minmax(440px, 1fr)); gap: 20px; max-width: 1600px; margin: 0 auto; width: 100%; }
        
        .card {
            background-color: var(--surface-card); border-radius: 16px; padding: 22px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.25); border: 1px solid var(--border);
            display: flex; flex-direction: column; gap: 16px;
        }
        
        .card-header { display: flex; justify-content: space-between; align-items: center; border-bottom: 1px solid var(--border); padding-bottom: 10px; }
        .card-header h2 { font-size: 17px; font-weight: 700; color: var(--primary); display: flex; align-items: center; gap: 8px; }
        
        .btn-action {
            background: var(--primary); color: #082f49; border: none; padding: 6px 12px; border-radius: 8px;
            font-size: 12px; font-weight: 700; cursor: pointer; transition: 0.2s; display: inline-flex; align-items: center; gap: 4px;
        }
        .btn-action:hover { opacity: 0.9; box-shadow: 0 0 10px var(--primary-glow); }
        
        /* Robot Mode Tabs */
        .mode-tabs { display: flex; background: rgba(15, 23, 42, 0.7); border-radius: 10px; padding: 4px; gap: 6px; }
        .mode-tab {
            flex: 1; padding: 8px 10px; font-size: 12px; font-weight: 700; text-align: center; border-radius: 8px;
            cursor: pointer; color: var(--text-muted); transition: 0.2s; border: none; background: transparent;
        }
        .mode-tab.active { background: var(--primary); color: #082f49; box-shadow: 0 0 10px var(--primary-glow); }

        /* Teleop Layout */
        .teleop-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 16px; align-items: center; }
        .joystick-panel { display: flex; flex-direction: column; align-items: center; justify-content: center; background: rgba(15, 23, 42, 0.6); border-radius: 12px; padding: 14px; border: 1px solid var(--border); }
        #joystick-canvas { touch-action: none; background: radial-gradient(circle, #1e293b 0%, #0f172a 70%); border-radius: 50%; border: 2px solid var(--border); }
        
        .hud-panel { display: flex; flex-direction: column; gap: 10px; font-size: 12px; }
        .hud-row { display: flex; justify-content: space-between; background: rgba(15, 23, 42, 0.5); padding: 8px 12px; border-radius: 8px; border: 1px solid var(--border); }
        .hud-val { font-family: monospace; font-weight: 700; color: var(--primary); }
        
        /* Arm Controls */
        .arm-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; font-size: 12px; }
        .joint-group { display: flex; flex-direction: column; gap: 4px; }
        .joint-group label { color: var(--text-muted); display: flex; justify-content: space-between; }
        input[type=range] { width: 100%; height: 6px; border-radius: 4px; appearance: none; background: #1e293b; outline: none; }
        input[type=range]::-webkit-slider-thumb { appearance: none; width: 18px; height: 18px; border-radius: 50%; background: var(--primary); cursor: pointer; }
        
        .ik-box { background: rgba(15, 23, 42, 0.6); border-radius: 10px; padding: 12px; border: 1px solid var(--border); display: flex; flex-direction: column; gap: 8px; margin-top: 8px; }
        .ik-inputs { display: flex; gap: 8px; }
        .ik-field { flex: 1; display: flex; flex-direction: column; gap: 2px; font-size: 11px; color: var(--text-muted); }
        .ik-input { width: 100%; background: #0f172a; border: 1px solid var(--border); color: #fff; padding: 5px; border-radius: 6px; font-weight: bold; }
        
        .btn-stop {
            background: var(--danger); color: white; border: none; padding: 12px; border-radius: 10px;
            font-size: 14px; font-weight: bold; cursor: pointer; transition: 0.2s; width: 100%;
        }
        .btn-stop:hover { box-shadow: 0 0 15px rgba(248, 113, 113, 0.4); }

        /* Camera & AI Box */
        .camera-wrapper {
            position: relative; width: 100%; height: 200px; background: #000; border-radius: 10px;
            overflow: hidden; display: flex; justify-content: center; align-items: center; border: 1px solid var(--border);
        }
        #camera-frame { width: 100%; height: 100%; object-fit: cover; }
        #ai-overlay { position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none; }
        .ai-chip { position: absolute; top: 8px; left: 8px; background: rgba(15, 23, 42, 0.8); color: var(--success); font-family: monospace; font-size: 10px; padding: 4px 8px; border-radius: 6px; border: 1px solid rgba(52, 211, 153, 0.3); }
        .ai-fps-chip { position: absolute; top: 8px; right: 8px; background: rgba(15, 23, 42, 0.8); color: var(--primary); font-family: monospace; font-size: 10px; padding: 4px 8px; border-radius: 6px; border: 1px solid rgba(56, 189, 248, 0.3); }
        .ai-ctrl-bar { display: flex; gap: 10px; align-items: center; }
        .ai-select { flex: 1; background: #0f172a; border: 1px solid var(--border); color: #fff; padding: 7px; border-radius: 8px; font-size: 12px; font-weight: bold; }
        .btn-track {
            background: #334155; color: #cbd5e1; border: 1px solid var(--border); padding: 7px 14px; border-radius: 8px;
            font-size: 12px; font-weight: bold; cursor: pointer; transition: 0.2s;
        }
        .btn-track.active { background: var(--accent); color: #fff; box-shadow: 0 0 10px rgba(129, 140, 248, 0.5); }

        /* Python IDE Box */
        .code-editor {
            width: 100%; height: 160px; background: #080c14; border: 1px solid var(--border); border-radius: 8px;
            color: #38bdf8; font-family: 'Courier New', Courier, monospace; font-size: 12px; padding: 10px;
            resize: vertical; outline: none; line-height: 1.4;
        }
        .ide-controls { display: flex; justify-content: space-between; align-items: center; }
        .ide-output {
            background: #080c14; border-radius: 8px; padding: 8px 12px; height: 90px; overflow-y: auto;
            font-family: monospace; font-size: 11px; color: var(--success); border: 1px solid var(--border);
        }

        /* File and Storage Manager */
        .storage-bar-bg { width: 100%; height: 8px; background: #1e293b; border-radius: 4px; overflow: hidden; margin-top: 4px; }
        .storage-bar-fill { height: 100%; background: var(--primary); width: 5%; transition: 0.3s; }
        .file-list-box { max-height: 140px; overflow-y: auto; display: flex; flex-direction: column; gap: 6px; }
        .file-row { background: rgba(15, 23, 42, 0.6); padding: 6px 10px; border-radius: 6px; display: flex; justify-content: space-between; align-items: center; font-size: 12px; border: 1px solid var(--border); }

        /* PnP and Pin tables */
        .device-grid { display: flex; flex-direction: column; gap: 8px; max-height: 200px; overflow-y: auto; }
        .device-item { background: rgba(15, 23, 42, 0.6); border: 1px solid var(--border); border-radius: 8px; padding: 8px 12px; display: flex; justify-content: space-between; align-items: center; }
        .dev-info { display: flex; align-items: center; gap: 10px; }
        .dev-title { font-weight: 700; font-size: 12px; }
        .dev-meta { font-size: 10px; color: var(--text-muted); display: flex; gap: 6px; }
        .dev-badge { background: #334155; padding: 2px 5px; border-radius: 4px; font-family: monospace; font-size: 9px; }
        .dev-reading { font-size: 11px; font-family: monospace; color: var(--success); font-weight: 600; text-align: right; }

        .pin-table-container { max-height: 200px; overflow-y: auto; }
        .pin-table { width: 100%; border-collapse: collapse; font-size: 12px; }
        .pin-table th { text-align: left; padding: 6px 8px; color: var(--text-muted); border-bottom: 1px solid var(--border); font-size: 10px; }
        .pin-table td { padding: 6px 8px; border-bottom: 1px solid rgba(255,255,255,0.03); }
        .pin-input { width: 60px; background: #0f172a; border: 1px solid var(--border); color: #fff; padding: 4px 6px; border-radius: 6px; font-size: 11px; font-weight: bold; }
        .badge-safe { background: rgba(52, 211, 153, 0.15); color: var(--success); padding: 2px 5px; border-radius: 4px; font-size: 9px; font-weight: 700; }
        .badge-warn { background: rgba(248, 113, 113, 0.15); color: var(--danger); padding: 2px 5px; border-radius: 4px; font-size: 9px; font-weight: 700; }

        /* Logs */
        .log-box { background: #080c14; border-radius: 8px; padding: 10px; height: 130px; overflow-y: auto; font-family: monospace; font-size: 11px; color: #a5b4fc; border: 1px solid var(--border); }
        .log-box p { margin-bottom: 3px; }
        .log-time { color: var(--text-muted); font-size: 10px; margin-right: 6px; }
        
        @media(max-width: 900px) { .container { grid-template-columns: 1fr; } .teleop-grid { grid-template-columns: 1fr; } }
    </style>
</head>
<body>

<header>
    <div class="brand">
        Tamimystic <span>OS</span>
        <div class="brand-chip">ESP32-S3-N16R8 (16MB Flash)</div>
    </div>
    <div class="status-badge" id="hud-status-badge">● SYSTEM READY</div>
</header>

<div class="container">
    <!-- Card 1: Universal Robotics & Teleoperation -->
    <div class="card">
        <div class="card-header">
            <h2>🦾 Universal Robotics Teleop</h2>
            <div id="obstacle-radar" style="font-size:12px; font-weight:bold; color:var(--success);">Radar: Clear</div>
        </div>

        <div class="mode-tabs">
            <button class="mode-tab active" data-mode="0">🏎️ Rover (2WD/4WD)</button>
            <button class="mode-tab" data-mode="1">🔄 Mecanum 4WD</button>
            <button class="mode-tab" data-mode="2">🦾 Robotic Arm</button>
        </div>

        <!-- Panel A: Rover / Mecanum 360 Joystick -->
        <div id="view-rover">
            <div class="teleop-grid">
                <div class="joystick-panel">
                    <canvas id="joystick-canvas" width="150" height="150"></canvas>
                    <div style="font-size:11px; color:var(--text-muted); margin-top:4px;">Drag 360° Touch Joystick</div>
                </div>
                <div class="hud-panel">
                    <div class="hud-row"><span>Linear (Vx):</span><span class="hud-val" id="hud-vx">0%</span></div>
                    <div class="hud-row"><span>Strafe (Vy):</span><span class="hud-val" id="hud-vy">0%</span></div>
                    <div class="hud-row"><span>Angular (Ω):</span><span class="hud-val" id="hud-w">0%</span></div>
                    <div class="hud-row"><span>Obstacle ToF:</span><span class="hud-val" id="hud-dist" style="color:var(--success);">24.8 cm</span></div>
                </div>
            </div>
        </div>

        <!-- Panel B: Robotic Arm IK & Joint Sliders -->
        <div id="view-arm" style="display:none; flex-direction:column; gap:10px;">
            <div class="arm-grid">
                <div class="joint-group"><label>Base Yaw <span id="val-j1">90°</span></label><input type="range" id="arm-j1" min="0" max="180" value="90"></div>
                <div class="joint-group"><label>Shoulder <span id="val-j2">90°</span></label><input type="range" id="arm-j2" min="0" max="180" value="90"></div>
                <div class="joint-group"><label>Elbow <span id="val-j3">90°</span></label><input type="range" id="arm-j3" min="0" max="180" value="90"></div>
                <div class="joint-group"><label>Wrist Pitch <span id="val-j4">90°</span></label><input type="range" id="arm-j4" min="0" max="180" value="90"></div>
                <div class="joint-group"><label>Wrist Roll <span id="val-j5">90°</span></label><input type="range" id="arm-j5" min="0" max="180" value="90"></div>
                <div class="joint-group"><label>Gripper <span id="val-j6">0%</span></label><input type="range" id="arm-j6" min="0" max="100" value="0"></div>
            </div>
            <!-- Inverse Kinematics Box -->
            <div class="ik-box">
                <div style="font-weight:700; font-size:12px; color:var(--primary);">🎯 Inverse Kinematics (IK) Cartesian Solver</div>
                <div class="ik-inputs">
                    <div class="ik-field"><span>Target X (cm)</span><input type="number" id="ik-x" class="ik-input" value="15.0" step="0.5"></div>
                    <div class="ik-field"><span>Target Y (cm)</span><input type="number" id="ik-y" class="ik-input" value="0.0" step="0.5"></div>
                    <div class="ik-field"><span>Target Z (cm)</span><input type="number" id="ik-z" class="ik-input" value="10.0" step="0.5"></div>
                    <button class="btn-action" id="btn-solve-ik" style="height:30px; align-self:flex-end;">Solve IK</button>
                </div>
                <div id="ik-status" style="font-size:10px; color:var(--text-muted);">Current Pose: (15.0, 0.0, 10.0 cm)</div>
            </div>
        </div>

        <button class="btn-stop" id="btn-emergency-stop">🛑 EMERGENCY BRAKE</button>
    </div>

    <!-- Card 2: In-Browser Python IDE & App Runner -->
    <div class="card">
        <div class="card-header">
            <h2>🐍 Web Python IDE & Script Runner</h2>
            <div style="font-size:11px; font-family:monospace; color:var(--success);">MicroPython Engine</div>
        </div>
        <textarea class="code-editor" id="py-code" spellcheck="false"># Tamimystic OS Embedded Python
import tamimystic

print("Running autonomous routine...")
dist = tamimystic.sensor.read_distance()
print("Front distance:", dist, "cm")

if dist > 20.0:
    print("Moving forward!")
    tamimystic.robot.move(40, 0)
else:
    print("Obstacle detected! Turning...")
    tamimystic.robot.move(0, 35)

print("Finished.")</textarea>
        
        <div class="ide-controls">
            <button class="btn-action" id="btn-run-code" style="background:var(--success); color:#022c22;">▶️ Run Script</button>
            <button class="btn-action" id="btn-stop-code" style="background:var(--danger); color:#fff;">⏹️ Stop</button>
            <button class="btn-action" id="btn-load-arm-demo" style="background:#334155; color:#fff;">🦾 Load Arm Demo</button>
        </div>
        <div class="ide-output" id="py-output">Stdout console output ready.</div>
    </div>

    <!-- Card 3: Edge AI & Live Camera Vision Pipeline -->
    <div class="card">
        <div class="card-header">
            <h2>🧠 Edge AI & Live Camera</h2>
            <div style="font-size:11px; font-family:monospace; color:var(--primary);">TFLite Micro (ESP-NN)</div>
        </div>
        <div class="camera-wrapper">
            <img id="camera-frame" src="/api/camera/snapshot" alt="Live Camera Video">
            <canvas id="ai-overlay" width="320" height="240"></canvas>
            <div class="ai-chip" id="ai-target-badge">🎯 Locked: Searching...</div>
            <div class="ai-fps-chip" id="ai-fps-badge">22.5 FPS | 18ms</div>
        </div>
        <div class="ai-ctrl-bar">
            <select class="ai-select" id="ai-model-select">
                <option value="person">MobileNet-V2 Person Detector</option>
                <option value="object">MobileNet-SSD Object Detector</option>
                <option value="lane">Autonomous Lane & Line Follower</option>
                <option value="gesture">Hand Gesture Neural Classifier</option>
            </select>
            <button class="btn-track" id="btn-toggle-track">🎯 Auto-Follow: OFF</button>
        </div>
    </div>

    <!-- Card 4: Flash File Manager (6.8MB LittleFS) & Dual-Bank OTA -->
    <div class="card">
        <div class="card-header">
            <h2>📁 6.8MB Flash Filesystem & Dual OTA</h2>
            <button class="btn-action" id="btn-refresh-files">🔄 Refresh</button>
        </div>
        <div>
            <div style="display:flex; justify-content:space-between; font-size:11px; color:var(--text-muted);">
                <span>LittleFS VFS Usage:</span>
                <span id="storage-usage-text">Used: 2.4 KB / 6.8 MB</span>
            </div>
            <div class="storage-bar-bg"><div class="storage-bar-fill" id="storage-bar"></div></div>
        </div>
        <div class="file-list-box" id="file-list-container">
            <div style="text-align:center; padding:10px; font-size:11px; color:var(--text-muted);">Loading files...</div>
        </div>
        <!-- Dual OTA Bar -->
        <div style="background:rgba(15, 23, 42, 0.6); padding:10px; border-radius:8px; border:1px solid var(--border); font-size:11px; display:flex; justify-content:space-between; align-items:center;">
            <div>
                <strong style="color:var(--primary);">Active Boot Partition:</strong> app0 (OTA_0 4.5MB)<br>
                <span style="color:var(--text-muted);">Rollback Protected (app1 ready)</span>
            </div>
            <button class="btn-action" style="background:#334155; color:#cbd5e1;" onclick="alert('OTA Endpoint Ready. Upload .bin to flash.')">⬆️ Upload .BIN</button>
        </div>
    </div>

    <!-- Card 5: Plug & Play Hardware Devices -->
    <div class="card">
        <div class="card-header">
            <h2>🔌 Plug & Play Hardware</h2>
            <button class="btn-action" id="btn-scan-pnp">⚡ Scan Buses</button>
        </div>
        <div class="device-grid" id="pnp-device-list">
            <div style="text-align:center; padding:15px; color:var(--text-muted); font-size:12px;">Discovering I2C sensors...</div>
        </div>
    </div>

    <!-- Card 6: Dynamic Pin Matrix Manager -->
    <div class="card">
        <div class="card-header">
            <h2>🎛️ Dynamic Pin Matrix</h2>
            <button class="btn-action" id="btn-save-pins" style="background:var(--success); color:#022c22;">💾 Save Pins</button>
        </div>
        <div class="pin-table-container">
            <table class="pin-table">
                <thead>
                    <tr><th>Peripheral</th><th>GPIO Pin</th><th>Safety</th></tr>
                </thead>
                <tbody id="pin-table-body">
                    <tr><td colspan="3" style="text-align:center; color:var(--text-muted);">Loading pins...</td></tr>
                </tbody>
            </table>
        </div>
    </div>

    <!-- Card 7: Live Event Logs (Full Width) -->
    <div class="card" style="grid-column: 1 / -1;">
        <div class="card-header">
            <h2>📜 Real-Time System Event Logs</h2>
            <button class="btn-action" id="btn-clear-logs" style="background:#334155; color:#cbd5e1;">Clear</button>
        </div>
        <div class="log-box" id="sys-logs">
            <p><span class="log-time">00:00:01</span> [BOOT] Tamimystic OS initialized on ESP32-S3-N16R8 (16MB Flash, 8MB PSRAM).</p>
            <p><span class="log-time">00:00:02</span> [ROBOTICS] Core 1 real-time 50Hz kinematics loop running.</p>
            <p><span class="log-time">00:00:03</span> [AI] TensorFlow Lite Micro with ESP-NN SIMD active (20 FPS).</p>
            <p><span class="log-time">00:00:04</span> [APPS] MicroPython and WASM runtime engine active.</p>
        </div>
    </div>
</div>

<script>
    function log(msg) {
        const box = document.getElementById('sys-logs');
        const time = new Date().toLocaleTimeString();
        box.innerHTML += `<p><span class="log-time">${time}</span> ${msg}</p>`;
        box.scrollTop = box.scrollHeight;
    }

    document.getElementById('btn-clear-logs').addEventListener('click', () => {
        document.getElementById('sys-logs').innerHTML = '';
    });

    // 1. Robot Mode Switcher
    let currentRobotMode = 0;
    const modeTabs = document.querySelectorAll('.mode-tab');
    const viewRover = document.getElementById('view-rover');
    const viewArm = document.getElementById('view-arm');

    modeTabs.forEach(tab => {
        tab.addEventListener('click', () => {
            modeTabs.forEach(t => t.classList.remove('active'));
            tab.classList.add('active');
            currentRobotMode = parseInt(tab.getAttribute('data-mode'));
            if (currentRobotMode === 2) {
                viewRover.style.display = 'none';
                viewArm.style.display = 'flex';
            } else {
                viewRover.style.display = 'block';
                viewArm.style.display = 'none';
            }
            const modeNames = ["diff", "mecanum", "arm"];
            fetch(`/api/robot/mode?mode=${modeNames[currentRobotMode]}`, { method: 'POST' });
            log(`[ROBOT] Switched to mode: ${tab.innerText}`);
        });
    });

    // 2. 360 Virtual Touch Joystick
    const canvas = document.getElementById('joystick-canvas');
    const ctx = canvas.getContext('2d');
    const centerX = canvas.width / 2;
    const centerY = canvas.height / 2;
    const radius = 55;
    let touchX = centerX;
    let touchY = centerY;
    let isDragging = false;

    function drawJoystick() {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        ctx.beginPath(); ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.strokeStyle = 'rgba(56, 189, 248, 0.3)'; ctx.lineWidth = 3; ctx.stroke();

        ctx.beginPath();
        ctx.moveTo(centerX - 12, centerY); ctx.lineTo(centerX + 12, centerY);
        ctx.moveTo(centerX, centerY - 12); ctx.lineTo(centerX, centerY + 12);
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)'; ctx.stroke();

        ctx.beginPath(); ctx.arc(touchX, touchY, 20, 0, Math.PI * 2);
        ctx.fillStyle = '#38bdf8'; ctx.shadowColor = 'rgba(56, 189, 248, 0.6)';
        ctx.shadowBlur = 10; ctx.fill(); ctx.shadowBlur = 0;
    }
    drawJoystick();

    function handleJoystickMove(x, y) {
        const dx = x - centerX;
        const dy = y - centerY;
        const dist = Math.sqrt(dx * dx + dy * dy);

        if (dist <= radius) {
            touchX = x; touchY = y;
        } else {
            touchX = centerX + (dx / dist) * radius;
            touchY = centerY + (dy / dist) * radius;
        }
        drawJoystick();

        let vx = -((touchY - centerY) / radius) * 100;
        let vy = ((touchX - centerX) / radius) * 100;
        let w = (currentRobotMode === 1) ? 0 : vy;

        document.getElementById('hud-vx').innerText = Math.round(vx) + '%';
        document.getElementById('hud-vy').innerText = Math.round(vy) + '%';
        document.getElementById('hud-w').innerText = Math.round(w) + '%';

        fetch(`/api/robot/cmd_vel?vx=${vx.toFixed(1)}&vy=${vy.toFixed(1)}&w=${w.toFixed(1)}`, { method: 'POST' });
    }

    function resetJoystick() {
        touchX = centerX; touchY = centerY;
        drawJoystick();
        document.getElementById('hud-vx').innerText = '0%';
        document.getElementById('hud-vy').innerText = '0%';
        document.getElementById('hud-w').innerText = '0%';
        fetch(`/api/robot/cmd_vel?vx=0&vy=0&w=0`, { method: 'POST' });
    }

    canvas.addEventListener('mousedown', (e) => { isDragging = true; const r = canvas.getBoundingClientRect(); handleJoystickMove(e.clientX - r.left, e.clientY - r.top); });
    window.addEventListener('mousemove', (e) => { if (isDragging) { const r = canvas.getBoundingClientRect(); handleJoystickMove(e.clientX - r.left, e.clientY - r.top); } });
    window.addEventListener('mouseup', () => { if (isDragging) { isDragging = false; resetJoystick(); } });

    canvas.addEventListener('touchstart', (e) => { isDragging = true; const r = canvas.getBoundingClientRect(); handleJoystickMove(e.touches[0].clientX - r.left, e.touches[0].clientY - r.top); e.preventDefault(); });
    canvas.addEventListener('touchmove', (e) => { if (isDragging) { const r = canvas.getBoundingClientRect(); handleJoystickMove(e.touches[0].clientX - r.left, e.touches[0].clientY - r.top); } e.preventDefault(); });
    canvas.addEventListener('touchend', () => { isDragging = false; resetJoystick(); });

    // 3. Robotic Arm Joint Sliders
    const armSliders = [
        document.getElementById('arm-j1'), document.getElementById('arm-j2'),
        document.getElementById('arm-j3'), document.getElementById('arm-j4'),
        document.getElementById('arm-j5'), document.getElementById('arm-j6')
    ];

    function sendArmJoints() {
        const j1 = armSliders[0].value; const j2 = armSliders[1].value;
        const j3 = armSliders[2].value; const j4 = armSliders[3].value;
        const j5 = armSliders[4].value; const j6 = armSliders[5].value;

        document.getElementById('val-j1').innerText = j1 + '°';
        document.getElementById('val-j2').innerText = j2 + '°';
        document.getElementById('val-j3').innerText = j3 + '°';
        document.getElementById('val-j4').innerText = j4 + '°';
        document.getElementById('val-j5').innerText = j5 + '°';
        document.getElementById('val-j6').innerText = j6 + '%';

        fetch(`/api/robot/arm?j1=${j1}&j2=${j2}&j3=${j3}&j4=${j4}&j5=${j5}&j6=${j6}`, { method: 'POST' });
    }

    armSliders.forEach(s => s.addEventListener('input', sendArmJoints));

    // 4. Robotic Arm IK Solver Button
    document.getElementById('btn-solve-ik').addEventListener('click', () => {
        const x = document.getElementById('ik-x').value;
        const y = document.getElementById('ik-y').value;
        const z = document.getElementById('ik-z').value;
        const status = document.getElementById('ik-status');

        status.innerText = `Solving IK for (${x}, ${y}, ${z} cm)...`;
        fetch(`/api/robot/arm/ik?x=${x}&y=${y}&z=${z}`, { method: 'POST' })
            .then(res => res.json())
            .then(data => {
                if (data.status === 'ok') {
                    status.innerText = `IK Solved! Target (${x}, ${y}, ${z} cm) reached.`;
                    status.style.color = "var(--success)";
                    log(`[ROBOT:IK] Target coordinate (${x}, ${y}, ${z} cm) applied.`);
                } else {
                    status.innerText = "IK Error: Target point unreachable!";
                    status.style.color = "var(--danger)";
                }
            });
    });

    // 5. Emergency Stop
    let isEStopped = false;
    const btnStop = document.getElementById('btn-emergency-stop');
    btnStop.addEventListener('click', () => {
        if (!isEStopped) {
            isEStopped = true;
            btnStop.innerText = "🟢 RESUME MOTION";
            btnStop.style.background = "var(--success)";
            btnStop.style.color = "#022c22";
            fetch('/api/robot/stop', { method: 'POST' });
            log("[ROBOT:SAFETY] Emergency Stop Triggered!");
        } else {
            isEStopped = false;
            btnStop.innerText = "🛑 EMERGENCY BRAKE";
            btnStop.style.background = "var(--danger)";
            btnStop.style.color = "white";
            fetch('/api/robot/resume', { method: 'POST' });
            log("[ROBOT:SAFETY] Emergency Stop Released.");
        }
    });

    // 6. Python IDE Execution
    document.getElementById('btn-run-code').addEventListener('click', () => {
        const code = document.getElementById('py-code').value;
        const out = document.getElementById('py-output');
        out.innerText = "Executing Python code...";
        out.style.color = "var(--primary)";

        fetch('/api/apps/eval', {
            method: 'POST',
            body: code
        })
        .then(res => res.json())
        .then(data => {
            if (data.status === 'ok') {
                out.innerText = data.stdout || "Script completed with no output.";
                out.style.color = "var(--success)";
                log(`[PYTHON] Execution completed in ${data.execution_time_ms} ms`);
            } else {
                out.innerText = "Error: " + data.error;
                out.style.color = "var(--danger)";
            }
        });
    });

    document.getElementById('btn-stop-code').addEventListener('click', () => {
        fetch('/api/apps/stop', { method: 'POST' });
        document.getElementById('py-output').innerText = "Script terminated by user.";
        log("[PYTHON] Script execution terminated.");
    });

    document.getElementById('btn-load-arm-demo').addEventListener('click', () => {
        document.getElementById('py-code').value = 
`# Robotic Arm Cartesian Pick & Place
import tamimystic

print("Homing arm...")
tamimystic.robot.arm(90, 90, 90, 90, 90, 0)
tamimystic.delay(500)

print("Moving to pick location (14.0, 4.0, 8.0 cm)...")
tamimystic.robot.ik(14.0, 4.0, 8.0)
tamimystic.delay(500)

print("Closing gripper...")
tamimystic.robot.arm(90, 110, 136, 22, 90, 100)
print("Pick complete!")`;
    });

    // 7. Flash Filesystem Manager
    function fetchFiles() {
        fetch('/api/files/list')
            .then(res => res.json())
            .then(data => {
                const box = document.getElementById('file-list-container');
                if (!data.files || data.files.length === 0) {
                    box.innerHTML = '<div style="text-align:center; padding:10px; font-size:11px; color:var(--text-muted);">No files in storage.</div>';
                    return;
                }
                let html = '';
                data.files.forEach(f => {
                    html += `
                    <div class="file-row">
                        <span><strong>📄 ${f.name}</strong> <small style="color:var(--text-muted);">(${f.size} B)</small></span>
                        <button class="btn-action" style="background:var(--danger); color:#fff; padding:2px 6px; font-size:10px;" onclick="deleteFile('${f.name}')">Delete</button>
                    </div>`;
                });
                box.innerHTML = html;

                if (data.total_bytes) {
                    const pct = ((data.used_bytes / data.total_bytes) * 100).toFixed(1);
                    document.getElementById('storage-usage-text').innerText = `Used: ${(data.used_bytes / 1024).toFixed(1)} KB / ${(data.total_bytes / (1024*1024)).toFixed(1)} MB (${pct}%)`;
                    document.getElementById('storage-bar').style.width = Math.max(pct, 2) + '%';
                }
            });
    }

    function deleteFile(name) {
        if (confirm(`Delete file ${name}?`)) {
            fetch(`/api/files/delete?file=${name}`, { method: 'POST' }).then(() => fetchFiles());
        }
    }

    document.getElementById('btn-refresh-files').addEventListener('click', fetchFiles);

    // 8. Edge AI Vision Overlay & Camera Stream
    const aiCanvas = document.getElementById('ai-overlay');
    const aiCtx = aiCanvas.getContext('2d');
    const aiModelSelect = document.getElementById('ai-model-select');
    const btnTrack = document.getElementById('btn-toggle-track');
    let isTracking = false;

    aiModelSelect.addEventListener('change', () => {
        const m = aiModelSelect.value;
        fetch(`/api/ai/model?model=${m}`, { method: 'POST' });
        log(`[AI] Active model changed to: ${aiModelSelect.options[aiModelSelect.selectedIndex].text}`);
    });

    btnTrack.addEventListener('click', () => {
        isTracking = !isTracking;
        btnTrack.classList.toggle('active', isTracking);
        btnTrack.innerText = isTracking ? "🎯 Auto-Follow: ON" : "🎯 Auto-Follow: OFF";
        fetch(`/api/ai/track?enable=${isTracking ? '1' : '0'}`, { method: 'POST' });
        log(`[AI:AUTONOMY] Visual target tracking ${isTracking ? 'ACTIVATED' : 'DEACTIVATED'}`);
    });

    function updateAIOverlay() {
        fetch('/api/ai/status')
            .then(res => res.json())
            .then(data => {
                if (data.status === 'ok') {
                    document.getElementById('ai-fps-badge').innerText = `${data.fps.toFixed(1)} FPS | ${data.latency_ms}ms`;
                    document.getElementById('ai-target-badge').innerText = `🎯 Locked: ${data.object} (${data.confidence.toFixed(1)}%)`;

                    aiCtx.clearRect(0, 0, aiCanvas.width, aiCanvas.height);
                    if (data.boxes) {
                        data.boxes.forEach(box => {
                            const bx = (box.x - box.w / 2) * aiCanvas.width;
                            const by = (box.y - box.h / 2) * aiCanvas.height;
                            const bw = box.w * aiCanvas.width;
                            const bh = box.h * aiCanvas.height;

                            aiCtx.strokeStyle = '#38bdf8'; aiCtx.lineWidth = 2;
                            aiCtx.strokeRect(bx, by, bw, bh);

                            aiCtx.fillStyle = 'rgba(56, 189, 248, 0.8)';
                            aiCtx.fillRect(bx, by - 16, bw, 16);

                            aiCtx.fillStyle = '#082f49'; aiCtx.font = 'bold 10px sans-serif';
                            aiCtx.fillText(`${box.label} ${box.conf.toFixed(0)}%`, bx + 4, by - 4);
                        });
                    }
                }
            })
            .catch(() => {});
    }

    setInterval(() => {
        document.getElementById('camera-frame').src = '/api/camera/snapshot?t=' + Date.now();
        updateAIOverlay();
    }, 150);

    // 9. Fetch PnP Hardware Devices
    function fetchPnPDevices() {
        fetch('/api/pnp/devices')
            .then(res => res.json())
            .then(data => {
                const container = document.getElementById('pnp-device-list');
                if (!data.devices || data.devices.length === 0) {
                    container.innerHTML = '<div style="text-align:center; padding:15px; color:var(--text-muted); font-size:12px;">No external I2C devices connected.</div>';
                    return;
                }
                let html = '';
                data.devices.forEach(dev => {
                    html += `
                    <div class="device-item">
                        <div class="dev-info">
                            <span style="font-size:18px;">${dev.icon || '🔌'}</span>
                            <div>
                                <div class="dev-title">${dev.name}</div>
                                <div class="dev-meta">
                                    <span class="dev-badge">${dev.address}</span>
                                    <span>${dev.category}</span>
                                </div>
                            </div>
                        </div>
                        <div class="dev-reading">${dev.reading}</div>
                    </div>`;
                });
                container.innerHTML = html;
            });
    }

    document.getElementById('btn-scan-pnp').addEventListener('click', () => {
        log("[PNP] Running manual I2C bus scan...");
        fetch('/api/pnp/scan', { method: 'POST' }).then(() => fetchPnPDevices());
    });

    // 10. Fetch Pin Matrix
    function fetchPinMatrix() {
        fetch('/api/pins')
            .then(res => res.json())
            .then(data => {
                const tbody = document.getElementById('pin-table-body');
                if (!data.pins) return;
                let html = '';
                data.pins.forEach(p => {
                    html += `
                    <tr>
                        <td><strong>${p.label}</strong></td>
                        <td><input type="number" min="-1" max="48" class="pin-input" data-func="${p.func_name}" value="${p.gpio}"></td>
                        <td><span class="${p.safe ? 'badge-safe' : 'badge-warn'}">${p.safe ? 'SAFE' : 'RESERVED'}</span></td>
                    </tr>`;
                });
                tbody.innerHTML = html;
            });
    }

    document.getElementById('btn-save-pins').addEventListener('click', () => {
        const inputs = document.querySelectorAll('.pin-input');
        inputs.forEach(input => {
            const func = input.getAttribute('data-func');
            const pin = input.value;
            fetch(`/api/pins/set?func=${func}&pin=${pin}`, { method: 'POST' });
        });
        log("[PIN_MATRIX] Pin configuration updated.");
    });

    // 11. Fetch Telemetry
    function fetchTelemetry() {
        fetch('/api/robot/telemetry')
            .then(res => res.json())
            .then(data => {
                if (data.status === 'ok') {
                    if (data.obstacle_dist_cm < 900) {
                        document.getElementById('hud-dist').innerText = data.obstacle_dist_cm.toFixed(1) + ' cm';
                        const radar = document.getElementById('obstacle-radar');
                        if (data.braking) {
                            radar.innerText = "🛑 Obstacle Braking!";
                            radar.style.color = "var(--danger)";
                        } else {
                            radar.innerText = "Radar: Clear";
                            radar.style.color = "var(--success)";
                        }
                    }
                }
            })
            .catch(() => {});
    }

    fetchPnPDevices();
    fetchPinMatrix();
    fetchFiles();
    setInterval(fetchPnPDevices, 4000);
    setInterval(fetchTelemetry, 1000);
</script>

</body>
</html>
)=====";
