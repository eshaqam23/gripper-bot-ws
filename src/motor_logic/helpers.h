// Adam Eshaq - 04/03/2026
// Header for helper functions

#ifndef HELPERS_H
#define HELPERS_H

#include <Arduino.h>

// --------- Gripper servo ---------- //
const int GRIPPER_SERVO_PIN = 15;
const int GRIPPER_OPEN_ANGLE = 120;
const int GRIPPER_CLOSED_ANGLE = 30;

// --------- Left motor ---------- //
const int IN3 = 18;
const int IN4 = 9;
const int ENB = 25;

// --------- Right motor ---------- //
const int IN1 = 20;
const int IN2 = 19;
const int ENA = 26;

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