#ifndef _LED_BUZZER_BUTTON_H
#define _LED_BUZZER_BUTTON_H

#include <Arduino.h>
#include "max30102.h"
#include "oled.h"
#include "timer.h"

//定义引脚
#define Tonepin 2
#define Buttonpin1 5  
#define Buttonpin2 18 
#define Buttonpin3 19 
#define Ledpin 23
extern int setHour;
extern int setMinute;
void RemainDrinkEat();
void PinInit();
extern byte rates[10];

#endif