#ifndef _TIMER_H
#define _TIMER_H

#include <Arduino.h>
#include "oled.h"

extern bool counts;
void TimeInit();
void IRAM_ATTR Timer1Event();
void IRAM_ATTR Timer2Event();
void Button1IntEvent();
void Button2IntEvent();
void Button3IntEvent();
void Alarm();
void StopWatch();



#endif