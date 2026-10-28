// Implementation of helper functions

#include "helpers.h"

// ----------- Initialization ---------- //
void initPins() {
  // IN pins are plain outputs only when an enable pin does the PWM
  if (ENA >= 0) {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
  }
  if (ENB >= 0) {
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENB, OUTPUT);
  }
  stall();

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
// forwardPin is the IN pin driven HIGH to go forward, backPin the other one.
// With an enable pin: IN pins set direction, enable gets the PWM.
// Without one (enPin = -1): the IN pin for the chosen direction gets the PWM
// and the other is held LOW. The IN pins then stay on analogWrite only, since
// on the ESP32 a pin attached to PWM ignores digitalWrite.
void driveSide(int forwardPin, int backPin, int enPin, int pwm, int dir) {
  pwm = constrain(pwm, 0, 255);
  int onPin = (dir == 1) ? forwardPin : backPin;
  int offPin = (dir == 1) ? backPin : forwardPin;

  if (enPin >= 0) {
    digitalWrite(offPin, LOW);
    digitalWrite(onPin, pwm > 0 ? HIGH : LOW);
    analogWrite(enPin, pwm);
  } else {
    analogWrite(offPin, 0);
    analogWrite(onPin, pwm);
  }
}

void setLeftMotor(int pwm, int dir) { driveSide(IN4, IN3, ENB, pwm, dir); }

void setRightMotor(int pwm, int dir) { driveSide(IN2, IN1, ENA, pwm, dir); }

void stall() {
  setLeftMotor(0, 1);
  setRightMotor(0, 1);
}