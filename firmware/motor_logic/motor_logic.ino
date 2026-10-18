#include "helpers.h"

// Serial protocol (one command per line, 9600 baud):
//   D,leftPwm,leftDir,rightPwm,rightDir   drive  (pwm 0-255, dir 1 = forward, 0 = backward)
// Replies "OK" or "ERR" to every line.

char message[32];
unsigned long lastDriveMs = 0;
bool driving = false;

void setLeftMotor(int pwm, int dir) {
  pwm = constrain(pwm, 0, 255);

  if (pwm == 0) {
    stopLeft();
    analogWrite(ENB, 0);
    return;
  }

  if (dir == 1) leftForward();
  else leftBackwards();

  analogWrite(ENB, pwm);
}

void setRightMotor(int pwm, int dir) {
  pwm = constrain(pwm, 0, 255);

  if (pwm == 0) {
    stopRight();
    analogWrite(ENA, 0);
    return;
  }

  if (dir == 1) rightForward();
  else rightBackwards();

  analogWrite(ENA, pwm);
}

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

void setup() {
  initPins();
  stall();
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
