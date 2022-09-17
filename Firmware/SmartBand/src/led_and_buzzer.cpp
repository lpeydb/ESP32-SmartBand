#include "led_buzzer_button.h"

//提醒吃药喝水用变量
int setHour = -1;
int setMinute = -1;
//喝水吃药提示
void RemainDrinkEat()
{
	// Serial.println("REALYRemainDrinkEat");
	display.clearDisplay();
	display.drawBitmap(0, 0, DrinkWater, 64, 64, 1);
	display.drawBitmap(64, 0, Pills, 64, 64, 1);
	display.display();
	digitalWrite(Ledpin, 1);
	tone(Tonepin, 349);
	delay(4000);
	display.clearDisplay();
	display.display();
	digitalWrite(Ledpin, 0);
	noTone(Tonepin);
	setHour = -1;
	setMinute = -1;
}
void PinInit()
{
	pinMode(Ledpin, OUTPUT);
	pinMode(Buttonpin1, INPUT_PULLUP);
	pinMode(Buttonpin2, INPUT_PULLUP);
	pinMode(Buttonpin3, INPUT_PULLUP);
	attachInterrupt(Buttonpin1, Button1IntEvent, FALLING);
	attachInterrupt(Buttonpin2, Button2IntEvent, FALLING);
	attachInterrupt(Buttonpin3, Button3IntEvent, FALLING);
}