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