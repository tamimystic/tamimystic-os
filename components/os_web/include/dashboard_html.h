#pragma once

const char* dashboard_html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Tamimystic OS | Ultra Pro Max Dashboard</title>
    <style>
        :root {
            --bg: #0f172a;
            --surface: #1e293b;
            --primary: #3b82f6;
            --primary-glow: rgba(59, 130, 246, 0.5);
            --success: #10b981;
            --danger: #ef4444;
            --text: #f8fafc;
            --text-muted: #94a3b8;
        }
        
        * { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; }
        
        body { background-color: var(--bg); color: var(--text); min-height: 100vh; display: flex; flex-direction: column; }
        
        header {
            background-color: var(--surface); padding: 20px 40px;
            display: flex; justify-content: space-between; align-items: center;
            border-bottom: 1px solid rgba(255,255,255,0.1);
            box-shadow: 0 4px 20px rgba(0,0,0,0.3);
        }
        
        .brand { font-size: 24px; font-weight: 700; display: flex; align-items: center; gap: 10px; }
        .brand span { color: var(--primary); }
        .status-badge { background: var(--success); padding: 5px 12px; border-radius: 20px; font-size: 14px; font-weight: bold; }
        
        .container { flex: 1; padding: 40px; display: grid; grid-template-columns: 1fr 1fr; gap: 30px; max-width: 1400px; margin: 0 auto; width: 100%; }
        
        .card {
            background-color: var(--surface); border-radius: 16px; padding: 30px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2); border: 1px solid rgba(255,255,255,0.05);
            display: flex; flex-direction: column; gap: 20px;
        }
        
        .card h2 { font-size: 20px; font-weight: 600; border-bottom: 2px solid var(--primary); padding-bottom: 10px; display: inline-block; margin-bottom: 10px; }
        
        /* Motor Control */
        .joystick-container { display: flex; flex-direction: column; gap: 20px; }
        .slider-group { display: flex; flex-direction: column; gap: 10px; }
        .slider-group label { color: var(--text-muted); display: flex; justify-content: space-between; }
        input[type=range] { width: 100%; height: 8px; border-radius: 5px; appearance: none; background: #334155; outline: none; }
        input[type=range]::-webkit-slider-thumb { appearance: none; width: 24px; height: 24px; border-radius: 50%; background: var(--primary); cursor: pointer; box-shadow: 0 0 10px var(--primary-glow); }
        
        .btn-stop {
            background: var(--danger); color: white; border: none; padding: 15px; border-radius: 12px;
            font-size: 18px; font-weight: bold; cursor: pointer; transition: 0.3s; margin-top: 10px;
        }
        .btn-stop:hover { transform: scale(1.02); box-shadow: 0 0 15px rgba(239, 68, 68, 0.5); }
        
        /* AI Feed */
        .ai-feed {
            background: #000; border-radius: 12px; height: 300px; position: relative; overflow: hidden;
            display: flex; justify-content: center; align-items: center; border: 2px solid #334155;
        }
        .ai-overlay { position: absolute; top: 10px; left: 10px; color: var(--success); font-family: monospace; font-size: 14px; background: rgba(0,0,0,0.7); padding: 5px 10px; border-radius: 5px; }
        .ai-placeholder { color: var(--text-muted); font-size: 18px; text-align: center; }
        
        /* Logs */
        .log-box { background: #020617; border-radius: 8px; padding: 15px; height: 200px; overflow-y: auto; font-family: monospace; font-size: 14px; color: #a5b4fc; }
        .log-box p { margin-bottom: 5px; border-bottom: 1px solid rgba(255,255,255,0.05); padding-bottom: 5px; }
        .log-time { color: var(--text-muted); font-size: 12px; margin-right: 10px; }
        
        @media(max-width: 900px) { .container { grid-template-columns: 1fr; } }
    </style>
</head>
<body>

<header>
    <div class="brand">Tamimystic <span>OS</span></div>
    <div class="status-badge">● SYSTEM ONLINE</div>
</header>

<div class="container">
    <!-- Motor Control Card -->
    <div class="card">
        <h2>Robotics Control</h2>
        <div class="joystick-container">
            <div class="slider-group">
                <label>Left Motor Speed <span id="left-val">0%</span></label>
                <input type="range" id="left-motor" min="-100" max="100" value="0">
            </div>
            <div class="slider-group">
                <label>Right Motor Speed <span id="right-val">0%</span></label>
                <input type="range" id="right-motor" min="-100" max="100" value="0">
            </div>
            <button class="btn-stop" id="btn-stop">EMERGENCY STOP</button>
        </div>
    </div>

    <!-- AI Module Card -->
    <div class="card">
        <h2>AI Object Detection</h2>
        <div class="ai-feed">
            <div class="ai-overlay">Model: TFLite Micro | FPS: 24</div>
            <div class="ai-placeholder">
                <p>Live Feed Simulated</p>
                <h3 id="ai-detect-result" style="color:var(--primary); margin-top:10px;">Waiting for objects...</h3>
            </div>
        </div>
    </div>

    <!-- File System OTA Card -->
    <div class="card">
        <h2>OS File Manager (OTA)</h2>
        <div style="display:flex; flex-direction:column; gap:15px; margin-top:10px;">
            <p style="color:var(--text-muted); font-size:14px;">Upload AI Models (.tflite) or Apps (.wasm) directly to the Virtual File System.</p>
            <input type="file" id="ota-file" style="padding: 10px; background: #334155; border-radius: 8px; color: white; outline: none; border: 1px solid rgba(255,255,255,0.1);">
            <button class="btn-stop" id="btn-upload" style="background: var(--primary); margin-top: 0;">UPLOAD TO VFS</button>
            <p id="upload-status" style="font-size: 14px; font-weight: bold; margin-top: 5px;"></p>
        </div>
    </div>

    <!-- System Logs Card (Spans full width) -->
    <div class="card" style="grid-column: 1 / -1;">
        <h2>System Event Logs</h2>
        <div class="log-box" id="sys-logs">
            <p><span class="log-time">00:00:01</span> [BOOT] Tamimystic OS initialized.</p>
            <p><span class="log-time">00:00:02</span> [NET] Wi-Fi Connected. Web Server Started.</p>
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

    // Motor Control Logic
    const leftMotor = document.getElementById('left-motor');
    const rightMotor = document.getElementById('right-motor');
    const leftVal = document.getElementById('left-val');
    const rightVal = document.getElementById('right-val');

    function updateMotors() {
        const l = leftMotor.value;
        const r = rightMotor.value;
        leftVal.innerText = l + '%';
        rightVal.innerText = r + '%';
        
        fetch(`/api/motor?left=${l}&right=${r}`)
            .then(res => res.json())
            .catch(err => console.error(err));
    }

    leftMotor.addEventListener('input', updateMotors);
    rightMotor.addEventListener('input', updateMotors);
    
    document.getElementById('btn-stop').addEventListener('click', () => {
        leftMotor.value = 0; rightMotor.value = 0;
        updateMotors();
        log("[ROBOT] Emergency Stop Triggered!");
    });

    // File Upload Logic
    document.getElementById('btn-upload').addEventListener('click', () => {
        const fileInput = document.getElementById('ota-file');
        const statusText = document.getElementById('upload-status');
        
        if (!fileInput.files.length) {
            statusText.innerText = "Please select a file first.";
            statusText.style.color = "var(--danger)";
            return;
        }

        const file = fileInput.files[0];
        const formData = new FormData();
        formData.append("file", file);

        statusText.innerText = "Uploading " + file.name + "...";
        statusText.style.color = "var(--primary)";
        log(`[OTA] Starting upload for ${file.name}...`);

        fetch('/api/upload', {
            method: 'POST',
            body: formData
        })
        .then(res => res.json())
        .then(data => {
            if(data.status === 'ok') {
                statusText.innerText = "Upload Successful!";
                statusText.style.color = "var(--success)";
                log(`[OTA] Upload complete: ${file.name} saved to VFS.`);
            } else {
                statusText.innerText = "Upload Failed.";
                statusText.style.color = "var(--danger)";
                log(`[OTA] Upload error: ${data.message}`);
            }
        })
        .catch(err => {
            statusText.innerText = "Upload Error.";
            statusText.style.color = "var(--danger)";
            log(`[OTA] Network error during upload.`);
        });
    });

    // Mock AI Feed polling
    setInterval(() => {
        fetch('/api/ai/status')
            .then(res => res.json())
            .then(data => {
                if(data.object) {
                    document.getElementById('ai-detect-result').innerText = `Detected: ${data.object} (${data.confidence}%)`;
                    if(data.alert) log(`[AI] Alert: ${data.object} detected in path!`);
                }
            })
            .catch(() => {});
    }, 2000);
</script>

</body>
</html>
)=====";
