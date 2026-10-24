// Adam Eshaq - 04/03/2026
// Header for helper functions

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

// ======================== PIN SETTINGS ======================== //
// The only place pins are set. The block for the board picked in the
// Arduino IDE (Tools > Board) is used automatically.
// Right motors: IN1, IN2, ENA. Left motors: IN3, IN4, ENB.
// Set JOINT_PIN to -1 if there is no joint servo; its angle is then ignored.

#if defined(ESP32)
// ESP32-C6 (mule). IN1-IN4 from electrical; ENA/ENB and the claw are
// placeholders until electrical wires them (ENA/ENB copied from the Uno;
// claw on 10 because 9 is IN4 here).
// Avoid GPIO 4, 5, 8, 9, 15 (boot strapping), 12/13 (USB) and 16/17 (UART0).
// IN4 is on GPIO 9 (the BOOT pin): if the board won't boot or flash with the
// motor driver plugged in, move IN4 to a free pin.
const int IN1 = 20;  // brown
const int IN2 = 19;  // white
const int IN3 = 18;  // green
const int IN4 = 9;   // blue
const int ENA = 6;   // placeholder
const int ENB = 5;   // placeholder (boot pin, fine once running)
const int CLAW_PIN = 10;   // placeholder
const int JOINT_PIN = -1;  // TBD: not sure there is a joint servo yet

#else
// Arduino Uno bench-test map: gripper servos on 9/10 (11 kept free), motor
// direction pins on 2/4/7 so they don't fight the servo signals.
// The Servo library disables PWM on pins 9 and 10, so ENA/ENB must stay off them.
const int IN1 = 4;
const int IN2 = 2;
const int ENA = 6;
const int IN3 = 8;
const int IN4 = 7;
const int ENB = 5;
const int CLAW_PIN = 9;
const int JOINT_PIN = 10;
#endif

// --------- Gripper servos (placeholder limits until tuned) ---------- //
// Pulse widths for 0 and 180 degrees (the Arduino Servo library's defaults),
// set here so every board uses the same angle scale.
const int SERVO_MIN_US = 544;
const int SERVO_MAX_US = 2400;

// Added to every claw angle before it reaches the servo. If the claw slips on
// the servo shaft, change only this so the same commands line up again.
// Example: if G,60,90 now puts the claw where G,90,90 used to, use -30.
const int CLAW_OFFSET = 0;

// Safe range of the claw SERVO itself (after CLAW_OFFSET is applied), so a
// big offset or bad command can't drive it into its stops. Below 0 uses the
// servo's extra travel (about 10 us of pulse per degree); -25 made it buzz.
const int CLAW_MIN = -20;
const int CLAW_MAX = 180;
const int JOINT_MIN = 0;
const int JOINT_MAX = 180;

// Angles the servos move to on startup
const int CLAW_START = 0;
const int JOINT_START = 90;

// --------- Safety ---------- //
// Stop the drive motors if no D command arrives within this window.
// esp_bridge resends the current drive command at 10 Hz as a heartbeat.
const unsigned long DRIVE_TIMEOUT_MS = 500;

// ----------- Helper functions ---------- //
void initPins();

int angleToPulse(int angle);
void initServos();
void writeClawPulse(int us);
void writeJointPulse(int us);

void leftForward();
void leftBackwards();
void stopLeft();
void rightForward();
void rightBackwards();
void stopRight();
void stall();

#endif