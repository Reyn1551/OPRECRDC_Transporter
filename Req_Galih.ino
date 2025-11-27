#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Konfigurasi WiFi
const char* ssid = "Neo2";
const char* password = "indonesi2";

// Konfigurasi Motor
struct MotorConfig {
    const uint8_t A_FORWARD = 25;
    const uint8_t A_BACKWARD = 26;
    const uint8_t B_FORWARD = 33;
    const uint8_t B_BACKWARD = 32;
    const uint8_t ENA = 27;  // PWM motor kiri
    const uint8_t ENB = 14;  // PWM motor kanan
    const uint8_t DEFAULT_SPEED = 130;
    const uint8_t MAX_SPEED = 255;
    const uint8_t MIN_SPEED = 0;
} motor;

// Konfigurasi Gripper
struct GripperConfig {
    const uint8_t GRIPPER_PIN = 12;     // Pin untuk servo atau motor gripper
    const uint8_t LIFT_PIN = 13;        // Pin untuk motor pengangkat gripper
    const uint8_t GRIPPER_OPEN = 180;   // Nilai PWM untuk membuka gripper
    const uint8_t GRIPPER_CLOSE = 0;    // Nilai PWM untuk menutup gripper
    const uint8_t LIFT_UP = 200;        // Nilai PWM untuk mengangkat
    const uint8_t LIFT_DOWN = 50;       // Nilai PWM untuk menurunkan
} gripper;

// Global variables
WebServer server(80);
uint8_t currentSpeed = motor.DEFAULT_SPEED;
bool gripperOpen = true;
bool liftIsUp = false;

// Fungsi helper untuk inisialisasi pin
void initializePins() {
    // Motor pins
    pinMode(motor.A_FORWARD, OUTPUT);
    pinMode(motor.A_BACKWARD, OUTPUT);
    pinMode(motor.B_FORWARD, OUTPUT);
    pinMode(motor.B_BACKWARD, OUTPUT);
    pinMode(motor.ENA, OUTPUT);
    pinMode(motor.ENB, OUTPUT);
    
    // Gripper pins
    pinMode(gripper.GRIPPER_PIN, OUTPUT);
    pinMode(gripper.LIFT_PIN, OUTPUT);
}

// Fungsi kontrol motor yang lebih modular
class MotorController {
private:
    void setMotorDirection(uint8_t forwardPin, uint8_t backwardPin, bool forward) {
        digitalWrite(forwardPin, forward ? HIGH : LOW);
        digitalWrite(backwardPin, forward ? LOW : HIGH);
    }
    
public:
    void setSpeed(uint8_t leftSpeed, uint8_t rightSpeed) {
        analogWrite(motor.ENA, leftSpeed);
        analogWrite(motor.ENB, rightSpeed);
    }
    
    void moveForward(uint8_t speed = currentSpeed) {
        setSpeed(speed, speed);
        setMotorDirection(motor.A_FORWARD, motor.A_BACKWARD, true);
        setMotorDirection(motor.B_FORWARD, motor.B_BACKWARD, true);
    }
    
    void moveBackward(uint8_t speed = currentSpeed) {
        setSpeed(speed, speed);
        setMotorDirection(motor.A_FORWARD, motor.A_BACKWARD, false);
        setMotorDirection(motor.B_FORWARD, motor.B_BACKWARD, false);
    }
    
    void turnLeft(uint8_t speed = currentSpeed) {
        setSpeed(speed, speed);
        setMotorDirection(motor.A_FORWARD, motor.A_BACKWARD, false);
        setMotorDirection(motor.B_FORWARD, motor.B_BACKWARD, true);
    }
    
    void turnRight(uint8_t speed = currentSpeed) {
        setSpeed(speed, speed);
        setMotorDirection(motor.A_FORWARD, motor.A_BACKWARD, true);
        setMotorDirection(motor.B_FORWARD, motor.B_BACKWARD, false);
    }
    
    void stop() {
        digitalWrite(motor.A_FORWARD, LOW);
        digitalWrite(motor.A_BACKWARD, LOW);
        digitalWrite(motor.B_FORWARD, LOW);
        digitalWrite(motor.B_BACKWARD, LOW);
    }
} motorCtrl;

// Fungsi kontrol gripper
class GripperController {
public:
    void openGripper() {
        analogWrite(gripper.GRIPPER_PIN, gripper.GRIPPER_OPEN);
        gripperOpen = true;
    }
    
    void closeGripper() {
        analogWrite(gripper.GRIPPER_PIN, gripper.GRIPPER_CLOSE);
        gripperOpen = false;
    }
    
    void liftUp() {
        analogWrite(gripper.LIFT_PIN, gripper.LIFT_UP);
        liftIsUp = true;
    }
    
    void liftDown() {
        analogWrite(gripper.LIFT_PIN, gripper.LIFT_DOWN);
        liftIsUp = false;
    }
    
    void stopLift() {
        analogWrite(gripper.LIFT_PIN, 0);
    }
    
    bool getGripperState() { return gripperOpen; }
    bool getLiftState() { return liftIsUp; }
} gripperCtrl;

// HTML yang dioptimasi dengan slider PWM dan kontrol gripper
String getOptimizedHTML() {
    String html = "<!DOCTYPE html>";
    html += "<html lang='id'>";
    html += "<head>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<title>Kontrol RC</title>";
    html += "<style>";
    html += "body {";
    html += "font-family: Arial, sans-serif;";
    html += "background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);";
    html += "margin: 0;";
    html += "padding: 20px;";
    html += "min-height: 100vh;";
    html += "display: flex;";
    html += "justify-content: center;";
    html += "align-items: center;";
    html += "}";
    html += ".container {";
    html += "background: rgba(255, 255, 255, 0.95);";
    html += "border-radius: 20px;";
    html += "box-shadow: 0 10px 30px rgba(0, 0, 0, 0.2);";
    html += "padding: 30px;";
    html += "width: 100%;";
    html += "max-width: 480px;";
    html += "text-align: center;";
    html += "}";
    html += "h1 {";
    html += "color: #2563eb;";
    html += "margin-bottom: 30px;";
    html += "font-size: 32px;";
    html += "font-weight: bold;";
    html += "}";
    html += ".controls {";
    html += "display: grid;";
    html += "grid-template-columns: repeat(3, 1fr);";
    html += "gap: 15px;";
    html += "margin-bottom: 30px;";
    html += "}";
    html += ".btn {";
    html += "background: linear-gradient(145deg, #2563eb, #1d4ed8);";
    html += "color: white;";
    html += "border: none;";
    html += "border-radius: 15px;";
    html += "width: 80px;";
    html += "height: 80px;";
    html += "font-size: 24px;";
    html += "cursor: pointer;";
    html += "transition: all 0.3s;";
    html += "box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);";
    html += "margin: 5px;";
    html += "}";
    html += ".btn:hover {";
    html += "transform: translateY(-2px);";
    html += "box-shadow: 0 6px 20px rgba(0, 0, 0, 0.3);";
    html += "}";
    html += ".btn:active {";
    html += "transform: scale(0.95);";
    html += "}";
    html += ".btn-up { grid-column: 2; background: linear-gradient(145deg, #10b981, #059669); }";
    html += ".btn-left { grid-column: 1; grid-row: 2; }";
    html += ".btn-right { grid-column: 3; grid-row: 2; }";
    html += ".btn-down { grid-column: 2; grid-row: 3; background: linear-gradient(145deg, #10b981, #059669); }";
    html += ".btn-center { grid-column: 2; grid-row: 2; background: linear-gradient(145deg, #f59e0b, #d97706); }";
    html += ".pwm-control {";
    html += "margin-bottom: 25px;";
    html += "padding: 20px;";
    html += "background: rgba(37, 99, 235, 0.1);";
    html += "border-radius: 15px;";
    html += "}";
    html += ".pwm-control h3 {";
    html += "color: #2563eb;";
    html += "margin-bottom: 15px;";
    html += "font-size: 18px;";
    html += "}";
    html += ".slider-container {";
    html += "display: flex;";
    html += "align-items: center;";
    html += "gap: 15px;";
    html += "}";
    html += ".slider {";
    html += "flex: 1;";
    html += "height: 8px;";
    html += "border-radius: 5px;";
    html += "background: #ddd;";
    html += "outline: none;";
    html += "-webkit-appearance: none;";
    html += "}";
    html += ".slider::-webkit-slider-thumb {";
    html += "-webkit-appearance: none;";
    html += "appearance: none;";
    html += "width: 25px;";
    html += "height: 25px;";
    html += "border-radius: 50%;";
    html += "background: #2563eb;";
    html += "cursor: pointer;";
    html += "}";
    html += ".speed-value {";
    html += "min-width: 50px;";
    html += "text-align: center;";
    html += "font-weight: bold;";
    html += "color: #2563eb;";
    html += "font-size: 18px;";
    html += "background: rgba(37, 99, 235, 0.1);";
    html += "padding: 8px 12px;";
    html += "border-radius: 8px;";
    html += "}";
    html += ".gripper-control {";
    html += "margin-bottom: 25px;";
    html += "padding: 20px;";
    html += "background: rgba(245, 158, 11, 0.1);";
    html += "border-radius: 15px;";
    html += "}";
    html += ".gripper-control h3 {";
    html += "color: #f59e0b;";
    html += "margin-bottom: 15px;";
    html += "font-size: 18px;";
    html += "}";
    html += ".gripper-buttons {";
    html += "display: grid;";
    html += "grid-template-columns: 1fr 1fr;";
    html += "gap: 15px;";
    html += "margin-bottom: 15px;";
    html += "}";
    html += ".btn-gripper {";
    html += "background: linear-gradient(145deg, #f59e0b, #d97706);";
    html += "color: white;";
    html += "border: none;";
    html += "padding: 15px 20px;";
    html += "border-radius: 10px;";
    html += "font-size: 14px;";
    html += "font-weight: bold;";
    html += "cursor: pointer;";
    html += "transition: all 0.3s;";
    html += "box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);";
    html += "text-transform: uppercase;";
    html += "}";
    html += ".btn-gripper:active { transform: scale(0.95); }";
    html += ".btn-gripper.open { background: linear-gradient(145deg, #10b981, #059669); }";
    html += ".btn-gripper.close { background: linear-gradient(145deg, #ef4444, #dc2626); }";
    html += ".lift-buttons {";
    html += "display: grid;";
    html += "grid-template-columns: 1fr 1fr;";
    html += "gap: 15px;";
    html += "}";
    html += ".btn-lift {";
    html += "background: linear-gradient(145deg, #2563eb, #1d4ed8);";
    html += "color: white;";
    html += "border: none;";
    html += "padding: 15px 20px;";
    html += "border-radius: 10px;";
    html += "font-size: 14px;";
    html += "font-weight: bold;";
    html += "cursor: pointer;";
    html += "transition: all 0.3s;";
    html += "box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);";
    html += "text-transform: uppercase;";
    html += "}";
    html += ".btn-lift:active { transform: scale(0.95); }";
    html += ".action-buttons {";
    html += "display: flex;";
    html += "gap: 10px;";
    html += "justify-content: center;";
    html += "margin-bottom: 20px;";
    html += "}";
    html += ".btn-action {";
    html += "background: linear-gradient(145deg, #ef4444, #dc2626);";
    html += "color: white;";
    html += "border: none;";
    html += "padding: 15px 25px;";
    html += "border-radius: 15px;";
    html += "font-size: 14px;";
    html += "font-weight: bold;";
    html += "cursor: pointer;";
    html += "transition: all 0.3s;";
    html += "box-shadow: 0 4px 15px rgba(0, 0, 0, 0.2);";
    html += "text-transform: uppercase;";
    html += "}";
    html += ".btn-action:active { transform: scale(0.95); }";
    html += ".status {";
    html += "margin-top: 20px;";
    html += "padding: 15px;";
    html += "background: rgba(37, 99, 235, 0.1);";
    html += "border-radius: 10px;";
    html += "text-align: center;";
    html += "font-family: monospace;";
    html += "font-size: 14px;";
    html += "}";
    html += ".status-item {";
    html += "display: inline-block;";
    html += "margin: 0 8px;";
    html += "padding: 4px 8px;";
    html += "background: rgba(255, 255, 255, 0.8);";
    html += "border-radius: 6px;";
    html += "font-weight: bold;";
    html += "}";
    html += ".status-item strong { color: #2563eb; }";
    html += "@media (max-width: 640px) {";
    html += ".container { padding: 20px; }";
    html += "h1 { font-size: 28px; }";
    html += ".btn { width: 70px; height: 70px; font-size: 20px; }";
    html += ".btn-gripper, .btn-lift { padding: 12px 15px; font-size: 12px; }";
    html += ".btn-action { padding: 12px 20px; font-size: 12px; }";
    html += ".status { font-size: 12px; }";
    html += ".status-item { display: block; margin: 4px 0; }";
    html += "}";
    html += "@media (max-width: 480px) {";
    html += ".container { padding: 15px; }";
    html += "h1 { font-size: 24px; }";
    html += ".btn { width: 60px; height: 60px; font-size: 18px; }";
    html += ".slider-container { flex-direction: column; gap: 10px; }";
    html += ".slider { width: 100%; }";
    html += "}";
    html += "</style>";
    html += "</head>";
    html += "<body>";
    html += "<div class='container'>";
    html += "<h1>Kontrol RC</h1>";
    
    html += "<div class='controls'>";
    html += "<button class='btn btn-up' onmousedown='sendCommand(\"forward\")' onmouseup='sendCommand(\"stop\")' ontouchstart='sendCommand(\"forward\")' ontouchend='sendCommand(\"stop\")'>↑</button>";
    html += "<button class='btn btn-left' onmousedown='sendCommand(\"left\")' onmouseup='sendCommand(\"stop\")' ontouchstart='sendCommand(\"left\")' ontouchend='sendCommand(\"stop\")'>←</button>";
    html += "<button class='btn btn-center' onclick='sendCommand(\"stop\")'>■</button>";
    html += "<button class='btn btn-right' onmousedown='sendCommand(\"right\")' onmouseup='sendCommand(\"stop\")' ontouchstart='sendCommand(\"right\")' ontouchend='sendCommand(\"stop\")'>→</button>";
    html += "<button class='btn btn-down' onmousedown='sendCommand(\"backward\")' onmouseup='sendCommand(\"stop\")' ontouchstart='sendCommand(\"backward\")' ontouchend='sendCommand(\"stop\")'>↓</button>";
    html += "</div>";
    
    html += "<div class='pwm-control'>";
    html += "<h3>Kecepatan Motor</h3>";
    html += "<div class='slider-container'>";
    html += "<span>0</span>";
    html += "<input type='range' min='0' max='255' value='130' class='slider' id='speedSlider' onchange='updateSpeed(this.value)'>";
    html += "<span>255</span>";
    html += "<span class='speed-value' id='speedValue'>130</span>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='gripper-control'>";
    html += "<h3>Kontrol Gripper</h3>";
    html += "<div class='gripper-buttons'>";
    html += "<button class='btn-gripper open' onclick='sendCommand(\"openGripper\")'>Buka</button>";
    html += "<button class='btn-gripper close' onclick='sendCommand(\"closeGripper\")'>Tutup</button>";
    html += "</div>";
    html += "<div class='lift-buttons'>";
    html += "<button class='btn-lift' onclick='sendCommand(\"liftUp\")'>Naikkan</button>";
    html += "<button class='btn-lift' onclick='sendCommand(\"liftDown\")'>Turunkan</button>";
    html += "</div>";
    html += "</div>";
    
    html += "<div class='action-buttons'>";
    html += "<button class='btn-action' onclick='sendCommand(\"stop\")'>Stop</button>";
    html += "</div>";
    
    html += "<div class='status' id='status'>";
    html += "<span class='status-item'>Status: <strong id='statusText'>Ready</strong></span>";
    html += "<span class='status-item'>Speed: <strong id='currentSpeed'>130</strong></span>";
    html += "<span class='status-item'>Gripper: <strong id='gripperState'>Open</strong></span>";
    html += "<span class='status-item'>Lift: <strong id='liftState'>Down</strong></span>";
    html += "</div>";
    html += "</div>";
    
    html += "<script>";
    html += "function sendCommand(command) {";
    html += "fetch('/' + command)";
    html += ".then(response => response.json())";
    html += ".then(data => {";
    html += "console.log(data);";
    html += "if (command === 'stop') {";
    html += "document.getElementById('statusText').textContent = 'Stopped';";
    html += "} else if (command === 'openGripper') {";
    html += "document.getElementById('gripperState').textContent = 'Open';";
    html += "} else if (command === 'closeGripper') {";
    html += "document.getElementById('gripperState').textContent = 'Closed';";
    html += "} else if (command === 'liftUp') {";
    html += "document.getElementById('liftState').textContent = 'Up';";
    html += "} else if (command === 'liftDown') {";
    html += "document.getElementById('liftState').textContent = 'Down';";
    html += "} else {";
    html += "document.getElementById('statusText').textContent = command.charAt(0).toUpperCase() + command.slice(1);";
    html += "}";
    html += "})";
    html += ".catch(err => console.error('Error:', err));";
    html += "}";
    html += "function updateSpeed(speed) {";
    html += "document.getElementById('speedValue').textContent = speed;";
    html += "document.getElementById('currentSpeed').textContent = speed;";
    html += "fetch('/setSpeed?speed=' + speed)";
    html += ".then(response => response.json())";
    html += ".then(data => console.log('Speed updated:', data))";
    html += ".catch(err => console.error('Speed update error:', err));";
    html += "}";
    html += "// Prevent zoom on double tap for mobile";
    html += "let lastTouchEnd = 0;";
    html += "document.addEventListener('touchend', function(e) {";
    html += "const now = Date.now();";
    html += "if (now - lastTouchEnd <= 300) {";
    html += "e.preventDefault();";
    html += "}";
    html += "lastTouchEnd = now;";
    html += "}, false);";
    html += "</script>";
    html += "</body>";
    html += "</html>";
    
    return html;
}

// API endpoints
void setupAPIEndpoints() {
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", getOptimizedHTML());
    });
    
    server.on("/forward", HTTP_GET, []() {
        motorCtrl.moveForward();
        server.send(200, "application/json", "{\"status\":\"forward\"}");
    });
    
    server.on("/backward", HTTP_GET, []() {
        motorCtrl.moveBackward();
        server.send(200, "application/json", "{\"status\":\"backward\"}");
    });
    
    server.on("/left", HTTP_GET, []() {
        motorCtrl.turnLeft();
        server.send(200, "application/json", "{\"status\":\"left\"}");
    });
    
    server.on("/right", HTTP_GET, []() {
        motorCtrl.turnRight();
        server.send(200, "application/json", "{\"status\":\"right\"}");
    });
    
    server.on("/stop", HTTP_GET, []() {
        motorCtrl.stop();
        server.send(200, "application/json", "{\"status\":\"stopped\"}");
    });
    
    server.on("/setSpeed", HTTP_GET, []() {
        if (server.hasArg("speed")) {
            int speed = server.arg("speed").toInt();
            if (speed >= motor.MIN_SPEED && speed <= motor.MAX_SPEED) {
                currentSpeed = speed;
                server.send(200, "application/json", "{\"status\":\"speed_set\",\"speed\":" + String(currentSpeed) + "}");
            } else {
                server.send(400, "application/json", "{\"error\":\"invalid_speed\"}");
            }
        } else {
            server.send(400, "application/json", "{\"error\":\"missing_speed\"}");
        }
    });
    
    server.on("/openGripper", HTTP_GET, []() {
        gripperCtrl.openGripper();
        server.send(200, "application/json", "{\"status\":\"gripper_opened\"}");
    });
    
    server.on("/closeGripper", HTTP_GET, []() {
        gripperCtrl.closeGripper();
        server.send(200, "application/json", "{\"status\":\"gripper_closed\"}");
    });
    
    server.on("/liftUp", HTTP_GET, []() {
        gripperCtrl.liftUp();
        server.send(200, "application/json", "{\"status\":\"lift_up\"}");
    });
    
    server.on("/liftDown", HTTP_GET, []() {
        gripperCtrl.liftDown();
        server.send(200, "application/json", "{\"status\":\"lift_down\"}");
    });
    
    server.on("/getStatus", HTTP_GET, []() {
        String json = "{\"speed\":" + String(currentSpeed) + 
                     ",\"gripper_open\":" + String(gripperCtrl.getGripperState() ? "true" : "false") + 
                     ",\"lift_up\":" + String(gripperCtrl.getLiftState() ? "true" : "false") + "}";
        server.send(200, "application/json", json);
    });
}

void setup() {
    Serial.begin(115200);
    
    // Connect WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    
    // Initialize hardware
    initializePins();
    motorCtrl.stop();
    gripperCtrl.openGripper(); // Start with gripper open
    gripperCtrl.liftDown();    // Start with lift down
    
    // Setup web server
    setupAPIEndpoints();
    server.begin();
    Serial.println("Server started");
}

void loop() {
    server.handleClient();
}