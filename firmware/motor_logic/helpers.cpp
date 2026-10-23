// Implementation of helper functions

#include "helpers.h"

// ----------- Initialization ---------- //
void initPins() {
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  Serial.begin(9600);
  delay(100);
}

// ----------- Gripper servos ---------- //
// Same scale as Servo::write() (0 deg = 544 us, 180 deg = 2400 us), but it also
// works below 0 / above 180 so the claw can use the servo's extra travel.
int angleToPulse(int angle) {
  return map(angle, 0, 180, SERVO_MIN_US, SERVO_MAX_US);
}

#if defined(ESP32)
// ESP32: drive the servos straight from the LEDC PWM hardware (ESP32 Arduino
// core 3.x), so no servo library is needed and pulses below 544 us still work.
const int SERVO_HZ = 50;
const int SERVO_BITS = 14;

void writeServoPulse(int pin, int us) {
  const uint32_t periodUs = 1000000 / SERVO_HZ;
  ledcWrite(pin, (uint32_t)us * ((1 << SERVO_BITS) - 1) / periodUs);
}

void initServos() {
  ledcAttach(CLAW_PIN, SERVO_HZ, SERVO_BITS);
  if (JOINT_PIN >= 0) ledcAttach(JOINT_PIN, SERVO_HZ, SERVO_BITS);
}

void writeClawPulse(int us) { writeServoPulse(CLAW_PIN, us); }

void writeJointPulse(int us) {
  if (JOINT_PIN >= 0) writeServoPulse(JOINT_PIN, us);
}

#else
#include <Servo.h>

Servo claw;
Servo joint;

void initServos() {
  // Widen the claw's allowed pulse range so angles below 0 aren't cut off
  claw.attach(CLAW_PIN, angleToPulse(CLAW_MIN), angleToPulse(CLAW_MAX));
  if (JOINT_PIN >= 0) joint.attach(JOINT_PIN, SERVO_MIN_US, SERVO_MAX_US);
}

void writeClawPulse(int us) { claw.writeMicroseconds(us); }

void writeJointPulse(int us) {
  if (JOINT_PIN >= 0) joint.writeMicroseconds(us);
}
#endif

// ----------- Individual Side ---------- //
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

void stall()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}