#include "helpers.h"

// Serial protocol (one command per line, 9600 baud):
//   D,leftPwm,leftDir,rightPwm,rightDir   drive  (pwm 0-255, dir 1 = forward, 0 = backward)
//   G,clawAngle,jointAngle                gripper servo angles in degrees
// Replies "OK" or "ERR" to every line.

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

  Serial.println("Ready");
}

void loop() {
  if (Serial.available()) {
    int bytesRead = Serial.readBytesUntil('\n', message, sizeof(message) - 1);
    message[bytesRead] = '\0';

    bool ok = false;
    if (bytesRead >= 2 && message[1] == ',') {
      switch (message[0]) {
        case 'D': ok = handleDrive(message + 2); break;
        case 'G': ok = handleGripper(message + 2); break;
      }
    }

    Serial.println(ok ? "OK" : "ERR");
  }

  // Safety stop: lost connection or ROS side stopped sending
  if (driving && millis() - lastDriveMs > DRIVE_TIMEOUT_MS) {
    stall();
    driving = false;
  }
}
