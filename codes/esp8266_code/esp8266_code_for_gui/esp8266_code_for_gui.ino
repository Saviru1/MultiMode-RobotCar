#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

const char* ssid = "Robot_Control_AP";
const char* password = "12345678";

ESP8266WebServer server(80);

// Battery parameters
const float VOLTAGE_MIN = 5.6;
const float VOLTAGE_MAX = 8.4;
const float DIVIDER_RATIO = 0.32; // For 100k+47k divider

// Robot status
int currentMode = 2;
int currentSpeed = 150;
String currentCommand = "stop";
String currentStatus = "Ready";
float batteryPercentage = 100.0;
unsigned long lastBatteryUpdate = 0;

// Function prototypes
void handleRoot();
void handleSetMode();
void handleRemoteControl();
void handleSpeed();
void handleStatus();
void checkBattery();

void setup() {
  Serial.begin(9600);
  pinMode(A0, INPUT);
  
  WiFi.softAP(ssid, password);
  
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setmode", handleSetMode);
  server.on("/remote", handleRemoteControl);
  server.on("/speed", handleSpeed);
  server.on("/status", handleStatus);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  if (millis() - lastBatteryUpdate > 1000) {
    checkBattery();
    lastBatteryUpdate = millis();
  }
  
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (input == "getBattery") {
      Serial.print("battery:");
      Serial.println(batteryPercentage);
    }
  }
}

void checkBattery() {
  int adcValue = analogRead(A0);
  float voltage = adcValue * (1.0 / 1023.0);
  float batteryVoltage = voltage / DIVIDER_RATIO;
  
  batteryPercentage = ((batteryVoltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN)) * 100.0;
  batteryPercentage = constrain(batteryPercentage, 0.0, 100.0);
}

void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Robot Control Panel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    :root {
      --primary: #4361ee;
      --secondary: #3f37c9;
      --success: #4cc9f0;
      --danger: #f72585;
      --warning: #f8961e;
      --info: #4895ef;
      --light: #f8f9fa;
      --dark: #212529;
    }
    
    body {
      font-family: 'Poppins', sans-serif;
      background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
      margin: 0;
      padding: 0;
      min-height: 100vh;
    }
    
    .container {
      max-width: 1000px;
      margin: 20px auto;
      background: white;
      border-radius: 15px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.1);
      overflow: hidden;
    }
    
    .header {
      background: linear-gradient(to right, var(--primary), var(--secondary));
      color: white;
      padding: 25px;
      text-align: center;
      position: relative;
    }
    
    .header h1 {
      margin: 0;
      font-size: 28px;
      font-weight: 600;
    }
    
    .tabs {
      display: flex;
      background: var(--light);
      border-bottom: 1px solid #dee2e6;
    }
    
    .tab {
      padding: 15px 25px;
      cursor: pointer;
      font-weight: 500;
      color: var(--dark);
      transition: all 0.3s ease;
      border-bottom: 3px solid transparent;
    }
    
    .tab.active {
      color: var(--primary);
      border-bottom: 3px solid var(--primary);
      background: rgba(67, 97, 238, 0.05);
    }
    
    .tab:hover {
      background: rgba(67, 97, 238, 0.1);
    }
    
    .tab-content {
      display: none;
      padding: 25px;
      animation: fadeIn 0.5s ease;
    }
    
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(10px); }
      to { opacity: 1; transform: translateY(0); }
    }
    
    .tab-content.active {
      display: block;
    }
    
    .mode-btn {
      display: block;
      width: 100%;
      padding: 15px;
      margin: 10px 0;
      background: linear-gradient(to right, var(--primary), var(--secondary));
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 16px;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.3s ease;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    
    .mode-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 10px rgba(0,0,0,0.15);
    }
    
    .control-btn {
      padding: 15px;
      margin: 5px;
      background: linear-gradient(to right, var(--success), var(--info));
      color: white;
      border: none;
      border-radius: 8px;
      font-size: 16px;
      font-weight: 500;
      cursor: pointer;
      transition: all 0.3s ease;
      width: 100px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
    }
    
    .control-btn:hover {
      transform: translateY(-2px);
      box-shadow: 0 6px 10px rgba(0,0,0,0.15);
    }
    
    .stop-btn {
      background: linear-gradient(to right, var(--danger), var(--warning));
    }
    
    .control-pad {
      display: grid;
      grid-template-columns: 1fr 1fr 1fr;
      gap: 10px;
      margin: 20px 0;
    }
    
    .speed-control {
      margin: 20px 0;
    }
    
    .speed-control label {
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
      color: var(--dark);
    }
    
    .speed-control input[type="range"] {
      width: 100%;
      height: 8px;
      -webkit-appearance: none;
      background: linear-gradient(to right, var(--primary), var(--secondary));
      border-radius: 4px;
      outline: none;
    }
    
    .speed-control input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 20px;
      height: 20px;
      background: white;
      border-radius: 50%;
      border: 2px solid var(--primary);
      cursor: pointer;
    }
    
    .status-panel {
      background: var(--light);
      padding: 20px;
      border-radius: 10px;
      margin-top: 20px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.05);
    }
    
    .status-item {
      margin: 10px 0;
      display: flex;
      align-items: center;
    }
    
    .status-label {
      font-weight: 600;
      color: var(--dark);
      min-width: 150px;
    }
    
    .status-value {
      font-weight: 500;
      color: var(--primary);
    }
    
    .battery-indicator {
      position: absolute;
      top: 20px;
      right: 20px;
      display: flex;
      align-items: center;
      background: rgba(255,255,255,0.2);
      padding: 5px 10px;
      border-radius: 20px;
      backdrop-filter: blur(5px);
    }
    
    .battery-icon {
      width: 30px;
      height: 15px;
      border: 2px solid white;
      border-radius: 3px;
      margin-right: 8px;
      position: relative;
      overflow: hidden;
    }
    
    .battery-level {
      height: 100%;
      background: white;
      border-radius: 1px;
      transition: width 0.5s ease;
    }
    
    .battery-percent {
      font-size: 14px;
      font-weight: 600;
      color: white;
    }
    
    .battery-tip {
      position: absolute;
      right: -5px;
      top: 4px;
      width: 3px;
      height: 7px;
      background: white;
      border-radius: 0 2px 2px 0;
    }
    
    @media (max-width: 768px) {
      .container {
        margin: 10px;
        border-radius: 10px;
      }
      
      .header {
        padding: 20px 15px;
      }
      
      .header h1 {
        font-size: 22px;
      }
      
      .tab {
        padding: 12px 15px;
        font-size: 14px;
      }
      
      .tab-content {
        padding: 15px;
      }
      
      .control-btn {
        width: 80px;
        padding: 12px;
        font-size: 14px;
      }
      
      .battery-indicator {
        top: 15px;
        right: 15px;
        padding: 4px 8px;
      }
    }
  </style>
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@400;500;600&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.15.3/css/all.min.css">
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Autonomous Robot Controller</h1>
      <div class="battery-indicator">
        <div class="battery-icon">
          <div class="battery-level" id="battery-level" style="width: 100%"></div>
          <div class="battery-tip"></div>
        </div>
        <span class="battery-percent" id="battery-percent">100%</span>
      </div>
    </div>
    
    <div class="tabs">
      <div class="tab active" onclick="switchTab('mode-tab')">Modes</div>
      <div class="tab" onclick="switchTab('remote-tab')">Remote</div>
      <div class="tab" onclick="switchTab('status-tab')">Status</div>
    </div>
    
    <div id="mode-tab" class="tab-content active">
      <h2 style="color: var(--primary); margin-top: 0;">Operation Modes</h2>
      <div class="speed-control">
        <label for="global-speed">Global Speed: <span id="global-speed-value">150</span></label>
        <input type="range" id="global-speed" min="0" max="255" value="150" oninput="updateGlobalSpeed(this.value)">
      </div>
      
      <button class="mode-btn" onclick="setMode(1)">
        <i class="fas fa-robot"></i> Obstacle Avoiding
      </button>
      <button class="mode-btn" onclick="setMode(2)">
        <i class="fas fa-road"></i> Line Follower
      </button>
      <button class="mode-btn" onclick="setMode(3)">
        <i class="fas fa-project-diagram"></i> Line + Obstacle
      </button>
      <button class="mode-btn" onclick="setMode(4)">
        <i class="fas fa-hand-paper"></i> Object Following
      </button>
      <button class="mode-btn" onclick="setMode(5)">
        <i class="fas fa-gamepad"></i> Remote Control
      </button>
      <button class="mode-btn" onclick="setMode(6)">
        <i class="fas fa-battery-full"></i> Battery Monitor
      </button>
    </div>
    
    <div id="remote-tab" class="tab-content">
      <h2 style="color: var(--primary); margin-top: 0;">Remote Control</h2>
      <div class="speed-control">
        <label for="speed">Control Speed: <span id="speed-value">150</span></label>
        <input type="range" id="speed" min="0" max="255" value="150" oninput="updateSpeed(this.value)">
      </div>
      
      <div class="control-pad">
        <div></div>
        <button class="control-btn" onmousedown="sendCommand('forward')" ontouchstart="sendCommand('forward')" 
                onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">
          <i class="fas fa-arrow-up"></i> Forward
        </button>
        <div></div>
        
        <button class="control-btn" onmousedown="sendCommand('left')" ontouchstart="sendCommand('left')" 
                onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">
          <i class="fas fa-arrow-left"></i> Left
        </button>
        <button class="control-btn stop-btn" onclick="sendCommand('stop')">
          <i class="fas fa-stop"></i> Stop
        </button>
        <button class="control-btn" onmousedown="sendCommand('right')" ontouchstart="sendCommand('right')" 
                onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">
          <i class="fas fa-arrow-right"></i> Right
        </button>
        
        <div></div>
        <button class="control-btn" onmousedown="sendCommand('backward')" ontouchstart="sendCommand('backward')" 
                onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">
          <i class="fas fa-arrow-down"></i> Backward
        </button>
        <div></div>
      </div>
    </div>
    
    <div id="status-tab" class="tab-content">
      <h2 style="color: var(--primary); margin-top: 0;">Robot Status</h2>
      <div class="status-panel">
        <div class="status-item">
          <span class="status-label">Current Mode:</span>
          <span class="status-value" id="current-mode">Line Follower</span>
        </div>
        <div class="status-item">
          <span class="status-label">Current Command:</span>
          <span class="status-value" id="current-cmd">None</span>
        </div>
        <div class="status-item">
          <span class="status-label">Current Speed:</span>
          <span class="status-value" id="current-spd">150</span>
        </div>
        <div class="status-item">
          <span class="status-label">Battery Level:</span>
          <span class="status-value" id="current-battery">100%</span>
        </div>
        <div class="status-item">
          <span class="status-label">System Status:</span>
          <span class="status-value" id="status-msg">Ready</span>
        </div>
      </div>
    </div>
  </div>

  <script>
    // Tab switching
    function switchTab(tabId) {
      document.querySelectorAll('.tab-content').forEach(tab => {
        tab.classList.remove('active');
      });
      document.getElementById(tabId).classList.add('active');
      
      document.querySelectorAll('.tab').forEach(tab => {
        tab.classList.remove('active');
      });
      event.currentTarget.classList.add('active');
    }
    
    // Mode selection
    function setMode(mode) {
      fetch('/setmode?mode=' + mode)
        .then(response => response.text())
        .then(text => {
          updateStatus();
          alert('Mode changed successfully!');
          if (mode === 5) {
            switchTab('remote-tab');
          }
        })
        .catch(err => {
          console.error('Error setting mode:', err);
        });
    }
    
    // Remote control commands
    function sendCommand(cmd) {
      fetch('/remote?command=' + cmd)
        .then(response => response.text())
        .then(text => {
          document.getElementById('current-cmd').textContent = cmd;
        })
        .catch(err => {
          console.error('Error sending command:', err);
        });
    }
    
    // Speed adjustment for remote mode
    function updateSpeed(speed) {
      document.getElementById('speed-value').textContent = speed;
      fetch('/speed?value=' + speed)
        .then(response => response.text())
        .catch(err => {
          console.error('Error updating speed:', err);
        });
    }
    
    // Global speed adjustment for all modes
    function updateGlobalSpeed(speed) {
      document.getElementById('global-speed-value').textContent = speed;
      fetch('/speed?value=' + speed)
        .then(response => response.text())
        .catch(err => {
          console.error('Error updating global speed:', err);
        });
    }
    
    // Update battery display
    function updateBatteryDisplay(percent) {
      const batteryLevel = document.getElementById('battery-level');
      const batteryPercent = document.getElementById('battery-percent');
      const batteryStatus = document.getElementById('current-battery');
      
      const roundedPercent = Math.round(percent);
      batteryLevel.style.width = percent + '%';
      batteryPercent.textContent = roundedPercent + '%';
      batteryStatus.textContent = roundedPercent + '%';
      
      // Change color based on level
      if (percent < 20) {
        batteryLevel.style.background = '#ff4757';
      } else if (percent < 40) {
        batteryLevel.style.background = '#ffa502';
      } else {
        batteryLevel.style.background = '#2ed573';
      }
    }
    
    // Update all status from server
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('current-mode').textContent = getModeName(data.mode);
          document.getElementById('current-cmd').textContent = data.command || 'None';
          document.getElementById('current-spd').textContent = data.speed;
          document.getElementById('status-msg').textContent = data.status;
          updateBatteryDisplay(data.battery);
        })
        .catch(err => {
          console.error('Error fetching status:', err);
        });
    }
    
    function getModeName(mode) {
      const modes = {
        1: 'Obstacle Avoiding',
        2: 'Line Follower',
        3: 'Line + Obstacle',
        4: 'Object Following',
        5: 'Remote Control',
        6: 'Battery Monitor'
      };
      return modes[mode] || 'Unknown';
    }
    
    // Keyboard controls
    document.addEventListener('keydown', (e) => {
      switch(e.key) {
        case 'ArrowUp': sendCommand('forward'); break;
        case 'ArrowDown': sendCommand('backward'); break;
        case 'ArrowLeft': sendCommand('left'); break;
        case 'ArrowRight': sendCommand('right'); break;
        case ' ': sendCommand('stop'); break;
      }
    });
    
    document.addEventListener('keyup', (e) => {
      if (['ArrowUp', 'ArrowDown', 'ArrowLeft', 'ArrowRight'].includes(e.key)) {
        sendCommand('stop');
      }
    });
    
    // Update status every 2 seconds
    setInterval(updateStatus, 2000);
    // Initial update
    updateStatus();
  </script>
</body>
</html>
)=====";

  server.send(200, "text/html", html);
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    currentMode = server.arg("mode").toInt();
    Serial.print("mode:");
    Serial.println(currentMode);
    
    switch(currentMode) {
      case 1: currentStatus = "Obstacle Avoiding"; break;
      case 2: currentStatus = "Line Follower"; break;
      case 3: currentStatus = "Line + Obstacle"; break;
      case 4: currentStatus = "Object Following"; break;
      case 5: currentStatus = "Remote Control"; break;
      case 6: currentStatus = "Battery Monitoring"; break;
      default: currentStatus = "Unknown Mode"; break;
    }
    
    server.send(200, "text/plain", "Mode set to " + String(currentMode));
  } else {
    server.send(400, "text/plain", "Missing mode parameter");
  }
}

void handleRemoteControl() {
  if (server.hasArg("command")) {
    currentCommand = server.arg("command");
    Serial.print("remote:");
    Serial.println(currentCommand);
    server.send(200, "text/plain", "Command received: " + currentCommand);
  } else {
    server.send(400, "text/plain", "Missing command parameter");
  }
}

void handleSpeed() {
  if (server.hasArg("value")) {
    currentSpeed = server.arg("value").toInt();
    Serial.print("speed:");
    Serial.println(currentSpeed);
    server.send(200, "text/plain", "Speed set to " + String(currentSpeed));
  } else {
    server.send(400, "text/plain", "Missing speed value");
  }
}

void handleStatus() {
  String status = "{";
  status += "\"mode\":" + String(currentMode) + ",";
  status += "\"command\":\"" + currentCommand + "\",";
  status += "\"speed\":" + String(currentSpeed) + ",";
  status += "\"status\":\"" + currentStatus + "\",";
  status += "\"battery\":" + String(batteryPercentage);
  status += "}";
  
  server.send(200, "application/json", status);
}