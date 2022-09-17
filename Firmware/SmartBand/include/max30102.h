#ifndef _MAX30102_H
#define _MAX30102_H

#include <Arduino.h>
#include <Wire.h>
#include <heartRate.h>
#include "MAX30105.h"  //MAX3010x library
#include "pic.h"
#include "oled.h"
#include "led_buzzer_button.h"

#define FINGER_ON 7000                     //紅外線最小量（判斷手指有沒有上）
#define MINIMUM_SPO2 90.0                  //血氧最小量

extern bool Max30102WorkRight;
extern byte rateSpot;
extern long lastBeat;
extern float beatsPerMinute;
extern int beatAvg;
extern double avered;
extern double aveir;
extern double sumirrms;
extern double sumredrms;
extern double SpO2;
extern double ESpO2;
extern double FSpO2;
extern double frate;
extern MAX30105 particleSensor;
void Max30102Init();
void Max30102Measure();
void LedToneAlarm();

#endif
