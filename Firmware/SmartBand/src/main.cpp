#include <Arduino.h>
#include <ESP32Servo.h>
#include <Adafruit_Sensor.h>
#include "pic.h"
#include "max30102.h"
#include "oled.h"
#include "led_buzzer_button.h"
#include "mpu6050.h"
#include "network.h"
#include "timer.h"

void setup()
{
	Serial.begin(115200);

	ScreenInit();
	NetworkInit();
	PinInit();
	Max30102Init();
	MpuInit();
	TimeInit();

	showCool();
	showCool();

}

void loop()
{
	if (counts) //每隔一段时间获取一次时间
	{
		GetTime();
		counts = 0;
	}
	
	if (menu)
	{
		Menu();
	}
	else
	{
		SelectEventSwitch();
	}
}