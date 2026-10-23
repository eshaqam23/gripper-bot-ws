#include <Servo.h>

#include "helpers.h"

// Serial protocol (one command per line, 9600 baud):
//   D,leftPwm,leftDir,rightPwm,rightDir   drive  (pwm 0-255, dir 1 = forward, 0 = backward)
//   G,clawAngle,jointAngle                gripper servo angles in degrees
// Replies "OK" or "ERR" to every line.

char message[32];
Servo claw;
Servo joint;
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

// Same scale as Servo::write() (0 deg = 544 us, 180 deg = 2400 us), but it also
// works below 0 / above 180 so the claw can use the servo's extra travel.
int angleToPulse(int angle) {
  return map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
}

// Shift by CLAW_OFFSET to line up with the gripper, then keep the servo
// inside its safe range.
void writeClaw(int angle) {
  int servoAngle = constrain(angle + CLAW_OFFSET, CLAW_MIN, CLAW_MAX);
  claw.writeMicroseconds(angleToPulse(servoAngle));
}

bool handleGripper(const char *args) {
  int clawAngle, jointAngle;

  if (sscanf(args, "%d,%d", &clawAngle, &jointAngle) != 2) {
    return false;
  }

  writeClaw(clawAngle);
  joint.write(constrain(jointAngle, JOINT_MIN, JOINT_MAX));
  return true;
}

void setup() {
  initPins();
  stall();

  // Widen the claw's allowed pulse range so angles below 0 aren't cut off
  claw.attach(CLAW_PIN, angleToPulse(CLAW_MIN), angleToPulse(CLAW_MAX));
  joint.attach(JOINT_PIN);
  writeClaw(CLAW_START);
  joint.write(JOINT_START);

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
