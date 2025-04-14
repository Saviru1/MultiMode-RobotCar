#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "Robot_Control_AP";
const char* password = "12345678";

ESP8266WebServer server(80);
int selectedMode = 0;

// Forward declarations
void handleRoot();
void handleSetMode();

void setup() {
  Serial.begin(9600);

  WiFi.softAP(ssid, password);
  Serial.println("Access Point Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/setmode", handleSetMode);

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();
}

// HTML UI with attractive styling
void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Robot Mode Selector</title>
      <style>
        body {
          font-family: 'Segoe UI', sans-serif;
          background: #eef2f3;
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
        }
        .container {
          background: white;
          padding: 40px 30px;
          border-radius: 15px;
          box-shadow: 0 10px 25px rgba(0,0,0,0.1);
          text-align: center;
          width: 320px;
        }
        h2 {
          margin-bottom: 20px;
          color: #333;
        }
        button {
          width: 100%;
          padding: 15px;
          margin: 10px 0;
          font-size: 16px;
          border: none;
          border-radius: 8px;
          cursor: pointer;
          background-color: #007BFF;
          color: white;
          transition: all 0.3s ease;
        }
        button:hover {
          background-color: #0056b3;
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h2>Select Robot Mode</h2>
        <button onclick="selectMode(1)"> Obstacle Avoiding</button>
        <button onclick="selectMode(2)"> Line Follower</button>
        <button onclick="selectMode(3)"> Line + Obstacle</button>
        <button onclick="selectMode(4)"> Object Following</button>
      </div>

      <script>
        function selectMode(mode) {
          fetch("/setmode?mode=" + mode)
            .then(response => response.text())
            .then(text => alert(text));
        }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void handleSetMode() {
  if (server.hasArg("mode")) {
    selectedMode = server.arg("mode").toInt();
    Serial.println(selectedMode);
    server.send(200, "text/plain", "Mode " + String(selectedMode) + " selected!");
  } else {
    server.send(400, "text/plain", "Missing mode");
  }
}