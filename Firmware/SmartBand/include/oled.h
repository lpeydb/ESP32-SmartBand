#ifndef _OLED_H
#define _OLED_H

#include <Adafruit_GFX.h> 
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "pic.h"
#include "network.h"
#include "timer.h"
#include "mpu6050.h"
#include "max30102.h"

#define SCREEN_WIDTH 128												  // OLED寬度
#define SCREEN_HEIGHT 64												  // OLED高度
#define OLED_RESET -1													  // Reset pin

extern bool confirm;
extern bool menu;
extern int eventSwitch;
extern bool button2Press;
extern Adafruit_SSD1306 display;
void ScreenInit();
void Menu();
void SelectEventSwitch();

#endif