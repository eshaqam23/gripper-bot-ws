// Adam Eshaq - 04/03/2026
// Header for helper functions

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

// TEMPORARY bench-test pin map: gripper servos on 9/10 (11 kept free), motor
// direction pins moved to 2/4/7 so they don't fight the servo signals.
// Swap to the real pins once the car is fully wired.

// --------- Left motors ---------- //
const int IN3 = 8;
const int IN4 = 7;
const int ENB = 5;

// --------- Right motors ---------- //
const int IN1 = 4;
const int IN2 = 2;
const int ENA = 6;

// --------- Gripper servos (placeholder limits until tuned) ---------- //
// Note: on an Uno the Servo library disables PWM on pins 9 and 10,
// so ENA/ENB must stay off those pins.
const int CLAW_PIN = 9;
const int JOINT_PIN = 10;

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