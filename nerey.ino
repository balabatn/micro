#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi Credentials
const char* ssid = "hehe";
const char* password = "12345678";

WebServer server(80);

// Motor pins
const int IN1 = 14;
const int IN2 = 27;
const int IN3 = 26;
const int IN4 = 25;
const int ENA = 33;
const int ENB = 32;

int motorSpeed = 200;  // Default speed
bool powerOn = true;   // Power state

// Battery monitoring pin
const int batteryPin = 34;  // Change to the pin you're using for battery monitoring

void setupMotors() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
}

// Movement functions
void moveForward() {
  if (!powerOn) return;
  digitalWrite(IN1, HIGH);  // Left motor forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  // Right motor forward
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  if (!powerOn) return;
  digitalWrite(IN1, LOW);   // Left motor backward
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);   // Right motor backward
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnLeft() {
  if (!powerOn) return;
  digitalWrite(IN1, LOW);   // Left motor stop
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);  // Right motor forward
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);      // No power to left
  analogWrite(ENB, motorSpeed);
}

void turnRight() {
  if (!powerOn) return;
  digitalWrite(IN1, HIGH);  // Left motor forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);   // Right motor stop
  digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, 0);      // No power to right
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// Function to get battery voltage
float readBatteryVoltage() {
  int rawValue = analogRead(batteryPin);  // Read raw value from the battery pin
  float voltage = (rawValue / 4095.0) * 3.3;  // Convert to voltage (3.3V reference)
  return voltage * (3.3 / 1.0);  // Adjust this value based on your voltage divider ratio
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html lang="en">
    <head>
      <meta charset="UTF-8">
      <meta name="viewport" content="width=device-width, initial-scale=1.0">
      <title>Sumo Bot Controller 🌈</title>
      <style>
        @import url('https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&display=swap');

        body {
          margin: 0;
          font-family: 'Poppins', sans-serif;
          color: white;
          min-height: 100vh;
          display: flex;
          flex-direction: column;
          align-items: center;
          justify-content: center;
          background: linear-gradient(-45deg, #ff6ec4, #7873f5, #4ade80, #38bdf8);
          background-size: 600% 600%;
          animation: rainbow 18s ease infinite;
        }

        @keyframes rainbow {
          0% {background-position: 0% 50%;}
          50% {background-position: 100% 50%;}
          100% {background-position: 0% 50%;}
        }

        h1 {
          font-size: 3rem;
          font-weight: 600;
          margin: 10px 0 5px;
          text-align: center;
          text-shadow: 2px 2px 10px rgba(0,0,0,0.4);
        }

        #status {
          margin-bottom: 15px;
          font-size: 1rem;
          color: #eee;
        }

        #battery {
          margin-top: 10px;
          font-size: 1.2rem;
          color: #eee;
        }

        .switch {
          position: relative;
          display: inline-block;
          width: 70px;
          height: 38px;
          margin-bottom: 20px;
        }
        .switch input { opacity: 0; width: 0; height: 0; }
        .slider {
          position: absolute;
          cursor: pointer;
          top: 0; left: 0; right: 0; bottom: 0;
          background-color: #ccc;
          transition: 0.4s;
          border-radius: 34px;
        }
        .slider:before {
          position: absolute;
          content: "";
          height: 30px;
          width: 30px;
          left: 4px;
          bottom: 4px;
          background-color: white;
          transition: 0.4s;
          border-radius: 50%;
        }
        input:checked + .slider { background-color: #4ade80; }
        input:checked + .slider:before { transform: translateX(32px); }

        .controller {
          display: grid;
          grid-template-columns: repeat(3, 90px);
          grid-template-rows: repeat(3, 90px);
          gap: 15px;
          justify-items: center;
          align-items: center;
        }

        .btn {
          width: 90px;
          height: 90px;
          display: flex;
          align-items: center;
          justify-content: center;
          font-size: 2rem;
          border-radius: 20px;
          background: rgba(255, 255, 255, 0.1);
          border: 2px solid white;
          cursor: pointer;
          transition: 0.3s;
        }

        .btn:hover {
          transform: scale(1.05);
          background: rgba(255, 255, 255, 0.2);
        }

        .speed-control {
          margin-top: 30px;
          text-align: center;
        }

        input[type="range"] { width: 250px; }
        footer {
          margin-top: 30px;
          color: #eee;
          font-size: 0.9rem;
        }

        @media (max-width: 500px) {
          .controller {
            grid-template-columns: repeat(3, 70px);
            grid-template-rows: repeat(3, 70px);
            gap: 10px;
          }
          .btn {
            width: 70px;
            height: 70px;
            font-size: 1.5rem;
          }
        }
      </style>
    </head>
    <body>
      <h1>Sumo Bot Controller 🌈</h1>
      <div id="status">Status: Connecting...</div>
      <div id="battery">Battery Voltage: -- V</div>

      <label class="switch">
        <input type="checkbox" id="powerSwitch" checked onchange="togglePower()">
        <span class="slider"></span>
      </label>

      <div class="controller">
        <div></div>
        <div class="btn" onclick="fetch('/forward')">⬆️</div>
        <div></div>
        <div class="btn" onclick="fetch('/left')">⬅️</div>
        <div class="btn" onclick="fetch('/stop')">⏹️</div>
        <div class="btn" onclick="fetch('/right')">➡️</div>
        <div></div>
        <div class="btn" onclick="fetch('/backward')">⬇️</div>
        <div></div>
      </div>

      <div class="speed-control">
        <label for="speed">Speed: <span id="speedValue">200</span></label><br>
        <input type="range" id="speed" min="100" max="255" value="200" 
               oninput="document.getElementById('speedValue').innerText=this.value"
               onchange="fetch('/speed?value=' + this.value)">
      </div>

      <script>
        function togglePower() {
          fetch('/togglePower');
        }

        function updateStatus() {
          fetch('/')
            .then(() => {
              document.getElementById("status").innerText = "Status: ✅ Connected";
            })
            .catch(() => {
              document.getElementById("status").innerText = "Status: ❌ Not Connected";
            });
        }

        function updateBattery() {
          fetch('/battery')
            .then(response => response.json())
            .then(data => {
              document.getElementById("battery").innerText = "Battery Voltage: " + data.voltage.toFixed(2) + " V";
            });
        }

        setInterval(updateStatus, 3000);
        setInterval(updateBattery, 5000);
        window.onload = updateStatus;
      </script>
    </body>
    </html>
  )rawliteral");
}

void handleBattery() {
  float voltage = readBatteryVoltage();
  server.send(200, "application/json", "{\"voltage\": " + String(voltage) + "}");
}

void setup() {
  Serial.begin(115200);
  setupMotors();

  WiFi.softAP(ssid, password);
  delay(100);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Web server routes
  server.on("/", handleRoot);
  server.on("/forward", []() { moveForward(); server.send(200, "text/plain", "Moving Forward"); });
  server.on("/backward", []() { moveBackward(); server.send(200, "text/plain", "Moving Backward"); });
  server.on("/left", []() { turnLeft(); server.send(200, "text/plain", "Turning Left"); });
  server.on("/right", []() { turnRight(); server.send(200, "text/plain", "Turning Right"); });
  server.on("/stop", []() { stopMotors(); server.send(200, "text/plain", "Stopping"); });
  server.on("/speed", []() {
    if (server.hasArg("value")) {
      motorSpeed = server.arg("value").toInt();
      Serial.print("Speed set to: ");
      Serial.println(motorSpeed);
      server.send(200, "text/plain", "Speed updated");
    } else {
      server.send(400, "text/plain", "No speed value received");
    }
  });
  server.on("/togglePower", []() {
    powerOn = !powerOn;
    if (!powerOn) stopMotors();
    Serial.print("Power: ");
    Serial.println(powerOn ? "ON" : "OFF");
    server.send(200, "text/plain", powerOn ? "Power ON" : "Power OFF");
  });32
  server.on("/battery", handleBattery);

  server.begin();
  Serial.println("Web server started!");
}

void loop() {
  server.handleClient();
}
