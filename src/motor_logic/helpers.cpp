// Implementation of helper functions for motor and gripper control.

#include "helpers.h"

// Gripper servo PWM settings (50 Hz = 20 ms period)
static const int SERVO_FREQ = 50;
static const int SERVO_RES_BITS = 14;
static const int SERVO_MIN_US = 500;
static const int SERVO_MAX_US = 2400;

// ----------- Initialization ---------- //
void initPins() {
  // Start serial first so the status prints below are visible.
  Serial.begin(9600);
  delay(1500);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  bool ok = ledcAttach(GRIPPER_SERVO_PIN, SERVO_FREQ, SERVO_RES_BITS);
  Serial.println(ok ? "Servo LEDC attached" : "Servo LEDC attach FAILED");

  closeGripper();
}

// ----------- Gripper ---------- //
// Converts an angle (0-180) to a servo pulse width, then to an LEDC duty value.
void setGripperAngle(int angle) {
  angle = constrain(angle, 0, 180);
  int pulseUs = map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
  uint32_t duty = ((uint32_t)pulseUs * ((1 << SERVO_RES_BITS) - 1)) / 20000;
  ledcWrite(GRIPPER_SERVO_PIN, duty);
}

void openGripper() {
  setGripperAngle(GRIPPER_OPEN_ANGLE);
}

void closeGripper() {
  setGripperAngle(GRIPPER_CLOSED_ANGLE);
}

// ----------- Individual sides ---------- //
void leftForward() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void leftBackwards() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopLeft() {
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void rightForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
}

void rightBackwards() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
}

void stopRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
}

// ----------- Emergency stop ---------- //
// Cuts direction and enable pins on both motors.
void stall() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  digitalWrite(ENA, LOW);
  digitalWrite(ENB, LOW);
}