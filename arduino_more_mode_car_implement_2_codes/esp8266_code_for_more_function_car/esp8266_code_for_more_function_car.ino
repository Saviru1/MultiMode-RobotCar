#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

const char* ssid = "Robot_Control_AP";
const char* password = "12345678";

ESP8266WebServer server(80);

// Robot status variables
int currentMode = 2; // Default to line follower mode
int currentSpeed = 150;
String currentCommand = "stop";
String currentStatus = "Ready";

void setup() {
  Serial.begin(9600);
  delay(10);

  // Set up Access Point
  WiFi.softAP(ssid, password);
  
  // Print AP IP address
  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Set up server routes
  server.on("/", handleRoot);
  server.on("/setmode", handleSetMode);
  server.on("/remote", handleRemoteControl);
  server.on("/status", handleStatus);

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>Robot Control Panel</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: 'Arial', sans-serif;
      background: #f0f2f5;
      margin: 0;
      padding: 0;
      color: #333;
    }
    .container {
      max-width: 800px;
      margin: 20px auto;
      background: white;
      border-radius: 10px;
      box-shadow: 0 4px 12px rgba(0,0,0,0.1);
      overflow: hidden;
    }
    .header {
      background: #4285f4;
      color: white;
      padding: 20px;
      text-align: center;
    }
    .content {
      padding: 20px;
    }
    .tabs {
      display: flex;
      border-bottom: 1px solid #ddd;
      margin-bottom: 20px;
    }
    .tab {
      padding: 12px 20px;
      cursor: pointer;
      background: #f1f1f1;
      border: 1px solid #ddd;
      border-bottom: none;
      border-radius: 5px 5px 0 0;
      margin-right: 5px;
    }
    .tab.active {
      background: white;
      border-bottom: 1px solid white;
      margin-bottom: -1px;
      font-weight: bold;
    }
    .tab-content {
      display: none;
    }
    .tab-content.active {
      display: block;
    }
    .mode-btn {
      display: block;
      width: 100%;
      padding: 15px;
      margin: 10px 0;
      background: #4285f4;
      color: white;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      cursor: pointer;
      transition: background 0.3s;
    }
    .mode-btn:hover {
      background: #3367d6;
    }
    .control-btn {
      padding: 15px;
      margin: 5px;
      background: #34a853;
      color: white;
      border: none;
      border-radius: 5px;
      font-size: 16px;
      cursor: pointer;
      transition: background 0.3s;
      width: 100px;
    }
    .control-btn:hover {
      background: #2d9248;
    }
    .stop-btn {
      background: #ea4335;
    }
    .stop-btn:hover {
      background: #d33426;
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
    .status-panel {
      background: #f8f9fa;
      padding: 15px;
      border-radius: 5px;
      margin-top: 20px;
    }
    .status-item {
      margin: 5px 0;
    }
    .status-label {
      font-weight: bold;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <h1>Robot Control Panel</h1>
    </div>
    <div class="content">
      <div class="tabs">
        <div class="tab active" onclick="switchTab('mode-tab')">Modes</div>
        <div class="tab" onclick="switchTab('remote-tab')">Remote Control</div>
        <div class="tab" onclick="switchTab('status-tab')">Status</div>
      </div>
      
      <div id="mode-tab" class="tab-content active">
        <h2>Select Operation Mode</h2>
        <button class="mode-btn" onclick="setMode(1)">Obstacle Avoiding Mode</button>
        <button class="mode-btn" onclick="setMode(2)">Line Follower Mode</button>
        <button class="mode-btn" onclick="setMode(3)">Line + Obstacle Mode</button>
        <button class="mode-btn" onclick="setMode(4)">Object Following Mode</button>
        <button class="mode-btn" onclick="setMode(5)">Remote Control Mode</button>
      </div>
      
      <div id="remote-tab" class="tab-content">
        <h2>Remote Control</h2>
        <div class="speed-control">
          <label for="speed">Speed: <span id="speed-value">150</span></label>
          <input type="range" id="speed" min="0" max="255" value="150" class="slider" oninput="updateSpeed(this.value)">
        </div>
        
        <div class="control-pad">
          <div></div>
          <button class="control-btn" onmousedown="sendCommand('forward')" ontouchstart="sendCommand('forward')" onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">↑ Forward</button>
          <div></div>
          
          <button class="control-btn" onmousedown="sendCommand('left')" ontouchstart="sendCommand('left')" onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">← Left</button>
          <button class="control-btn stop-btn" onclick="sendCommand('stop')">Stop</button>
          <button class="control-btn" onmousedown="sendCommand('right')" ontouchstart="sendCommand('right')" onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">Right →</button>
          
          <div></div>
          <button class="control-btn" onmousedown="sendCommand('backward')" ontouchstart="sendCommand('backward')" onmouseup="sendCommand('stop')" ontouchend="sendCommand('stop')">↓ Backward</button>
          <div></div>
        </div>
      </div>
      
      <div id="status-tab" class="tab-content">
        <h2>Robot Status</h2>
        <div class="status-panel">
          <div class="status-item"><span class="status-label">Current Mode:</span> <span id="current-mode">Line Follower</span></div>
          <div class="status-item"><span class="status-label">Current Command:</span> <span id="current-cmd">None</span></div>
          <div class="status-item"><span class="status-label">Current Speed:</span> <span id="current-spd">150</span></div>
          <div class="status-item"><span class="status-label">Status:</span> <span id="status-msg">Ready</span></div>
        </div>
      </div>
    </div>
  </div>

  <script>
    // Tab switching
    function switchTab(tabId) {
      // Hide all tabs
      document.querySelectorAll('.tab-content').forEach(tab => {
        tab.classList.remove('active');
      });
      // Show selected tab
      document.getElementById(tabId).classList.add('active');
      
      // Update tab styling
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
          alert(text);
          if (mode === 5) {
            switchTab('remote-tab');
          }
        });
    }
    
    // Remote control commands
    function sendCommand(cmd) {
      fetch('/remote?command=' + cmd)
        .then(response => response.text())
        .then(text => {
          document.getElementById('current-cmd').textContent = cmd;
          console.log(text);
        });
    }
    
    // Speed adjustment
    function updateSpeed(speed) {
      document.getElementById('speed-value').textContent = speed;
      document.getElementById('current-spd').textContent = speed;
      fetch('/remote?command=speed:' + speed)
        .then(response => response.text())
        .then(text => console.log(text));
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
    
    // Update status
    function updateStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          document.getElementById('current-mode').textContent = 
            getModeName(data.mode);
          document.getElementById('current-cmd').textContent = data.command;
          document.getElementById('current-spd').textContent = data.speed;
          document.getElementById('status-msg').textContent = data.status;
        });
    }
    
    function getModeName(mode) {
      const modes = {
        1: 'Obstacle Avoiding',
        2: 'Line Follower',
        3: 'Line + Obstacle',
        4: 'Object Following',
        5: 'Remote Control'
      };
      return modes[mode] || 'Unknown';
    }
    
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
    
    // Update status message based on mode
    switch(currentMode) {
      case 1: currentStatus = "Obstacle Avoiding Mode Active"; break;
      case 2: currentStatus = "Line Follower Mode Active"; break;
      case 3: currentStatus = "Line + Obstacle Mode Active"; break;
      case 4: currentStatus = "Object Following Mode Active"; break;
      case 5: currentStatus = "Remote Control Mode Active"; break;
      default: currentStatus = "Unknown Mode"; break;
    }
    
    server.send(200, "text/plain", "Mode set to " + String(currentMode));
  } else {
    server.send(400, "text/plain", "Missing mode parameter");
  }
}

void handleRemoteControl() {
  if (server.hasArg("command")) {
    String command = server.arg("command");
    
    if (command.startsWith("speed:")) {
      currentSpeed = command.substring(6).toInt();
      currentCommand = "speed:" + String(currentSpeed);
    } else {
      currentCommand = command;
      Serial.print("remote:");
      Serial.println(command);
    }
    
    server.send(200, "text/plain", "Command received: " + command);
  } else {
    server.send(400, "text/plain", "Missing command parameter");
  }
}

void handleStatus() {
  String status = "{";
  status += "\"mode\":" + String(currentMode) + ",";
  status += "\"command\":\"" + currentCommand + "\",";
  status += "\"speed\":" + String(currentSpeed) + ",";
  status += "\"status\":\"" + currentStatus + "\"";
  status += "}";
  
  server.send(200, "application/json", status);
}