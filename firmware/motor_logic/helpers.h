// Adam Eshaq - 04/03/2026
// Header for helper functions

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

// --------- Left motors ---------- //
const int IN3 = 8;
const int IN4 = 9;
const int ENB = 5;

// --------- Right motors ---------- //
const int IN1 = 10;
const int IN2 = 11;
const int ENA = 6;

// --------- Gripper servos (placeholder pins/limits until wired) ---------- //
// Note: on an Uno the Servo library disables PWM on pins 9 and 10.
// That's fine here because those are only direction pins (IN4, IN1).
const int CLAW_PIN = 3;
const int JOINT_PIN = 12;

const int CLAW_MIN = 0;
const int CLAW_MAX = 180;
const int JOINT_MIN = 0;
const int JOINT_MAX = 180;

// Angles the servos move to on startup
const int CLAW_START = 90;
const int JOINT_START = 90;

// --------- Safety ---------- //
// Stop the drive motors if no D command arrives within this window.
// usb_bridge resends the current drive command at 10 Hz as a heartbeat.
const unsigned long DRIVE_TIMEOUT_MS = 500;

// ----------- Helper functions ---------- //
void initPins();

void leftForward();
void leftBackwards();
void stopLeft();
void rightForward();
void rightBackwards();
void stopRight();
void stall();

#endif