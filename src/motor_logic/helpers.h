// Adam Eshaq - 04/03/2026
// Header for motor and gripper helper functions.

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

// --------- Gripper servo ---------- //
const int GRIPPER_SERVO_PIN = 15;
const int GRIPPER_OPEN_ANGLE = 120;
const int GRIPPER_CLOSED_ANGLE = 30;

// --------- Left motor ---------- //
const int IN3 = 20;
const int IN4 = 19;
const int ENB = 22;   // PWM speed

// --------- Right motor ---------- //
const int IN1 = 9;
const int IN2 = 18;
const int ENA = 23;   // PWM speed

// ----------- Helper functions ---------- //
void initPins();

void leftForward();
void leftBackwards();
void stopLeft();
void rightForward();
void rightBackwards();
void stopRight();
void stall();

void setGripperAngle(int angle);
void openGripper();
void closeGripper();

#endif