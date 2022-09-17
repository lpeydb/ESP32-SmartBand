#ifndef _MPU6050_H
#define _MPU6050_H

#include <Adafruit_MPU6050.h>
#include "oled.h"

extern int step;
extern float thresholdValue;
extern bool stepDown;
extern Adafruit_MPU6050 mpu;
void MpuInit();
void pedometer();

#endif