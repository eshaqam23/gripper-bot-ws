#include "helpers.h"

// Serial protocol (one command per line, 9600 baud):
//   D,leftPwm,leftDir,rightPwm,rightDir   drive  (pwm 0-255, dir 1 = forward, 0 = backward)
//   G,clawAngle,jointAngle                gripper servo angles in degrees
// Replies "OK" or "ERR" to every line.
//
// On the ESP32 the same protocol also works over WiFi: the board joins the
// network in wifi_secrets.h and listens on TCP port WIFI_PORT. While a WiFi
// client is connected it is used instead of USB serial.

#if defined(ESP32) && __has_include("wifi_secrets.h")
#include <WiFi.h>
#include "wifi_secrets.h"  // WIFI_SSID, WIFI_PASSWORD (copy wifi_secrets.example.h)
#define USE_WIFI
const int WIFI_PORT = 3333;
WiFiServer server(WIFI_PORT);
WiFiClient client;
void connectWifi();
#endif

Stream *conn = &Serial;
char message[32];
unsigned long lastDriveMs = 0;
bool driving = false;

bool handleDrive(const char *args) {
  int leftPwm, leftDir, rightPwm, rightDir;

  if (sscanf(args, "%d,%d,%d,%d", &leftPwm, &leftDir, &rightPwm, &rightDir) != 4) {
    return false;
  }

  setLeftMotor(leftPwm, leftDir);
  setRightMotor(rightPwm, rightDir);
  lastDriveMs = millis();
  driving = (leftPwm > 0 || rightPwm > 0);
  return true;
}

// Shift by CLAW_OFFSET to line up with the gripper, then keep the servo
// inside its safe range.
void writeClaw(int angle) {
  int servoAngle = constrain(angle + CLAW_OFFSET, CLAW_MIN, CLAW_MAX);
  writeClawPulse(angleToPulse(servoAngle));
}

void writeJoint(int angle) {
  writeJointPulse(angleToPulse(constrain(angle, JOINT_MIN, JOINT_MAX)));
}

bool handleGripper(const char *args) {
  int clawAngle, jointAngle;

  if (sscanf(args, "%d,%d", &clawAngle, &jointAngle) != 2) {
    return false;
  }

  writeClaw(clawAngle);
  writeJoint(jointAngle);
  return true;
}

void setup() {
  initPins();
  stall();

  initServos();
  writeClaw(CLAW_START);
  writeJoint(JOINT_START);

#ifdef USE_WIFI
  connectWifi();
#endif

  Serial.println("Ready");
}

#ifdef USE_WIFI
// Doesn't wait: WiFi keeps trying in the background and the board still
// works over USB meanwhile.
void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);  // lower latency
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

// Prints the WiFi status every 3 s until a WiFi client connects, so the
// address shows up whenever the Serial Monitor is opened.
void reportWifi() {
  static unsigned long lastReportMs = 0;
  static bool serverStarted = false;

  if (WiFi.status() == WL_CONNECTED && !serverStarted) {
    server.begin();
    serverStarted = true;
  }

  if (client.connected() || millis() - lastReportMs < 3000) return;
  lastReportMs = millis();

  switch (WiFi.status()) {
    case WL_CONNECTED:
      Serial.print("WiFi connected, use port:=socket://");
      Serial.print(WiFi.localIP());
      Serial.print(":");
      Serial.println(WIFI_PORT);
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println("WiFi: network \"" WIFI_SSID "\" not found (must be 2.4 GHz)");
      break;
    case WL_CONNECT_FAILED:
      Serial.println("WiFi: connection failed, check the password");
      break;
    default:
      Serial.println("WiFi: connecting...");
      break;
  }
}
#endif

void loop() {
#ifdef USE_WIFI
  reportWifi();
  if (server.hasClient()) {
    client = server.accept();  // a new connection replaces the old one
    client.setNoDelay(true);
  }
  conn = client.connected() ? (Stream *)&client : (Stream *)&Serial;
#endif

  if (conn->available()) {
    int bytesRead = conn->readBytesUntil('\n', message, sizeof(message) - 1);
    message[bytesRead] = '\0';

    bool ok = false;
    if (bytesRead >= 2 && message[1] == ',') {
      switch (message[0]) {
        case 'D': ok = handleDrive(message + 2); break;
        case 'G': ok = handleGripper(message + 2); break;
      }
    }

    conn->println(ok ? "OK" : "ERR");
  }

  // Safety stop: lost connection or ROS side stopped sending
  if (driving && millis() - lastDriveMs > DRIVE_TIMEOUT_MS) {
    stall();
    driving = false;
  }
}
