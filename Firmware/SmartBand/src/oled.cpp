#include "oled.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Declaring the display name (display)
//切换事件用变数
bool confirm = 0;
bool menu = 1;
int eventSwitch = 1;
bool button2Press = 0;
void ScreenInit()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Start the OLED display
	display.clearDisplay();
	display.display();
}
//菜单
void Menu()
{
	display.clearDisplay();
	switch (eventSwitch)
	{
	case 1:
		display.drawBitmap(39, 7, pulseIcon, 50, 50, 1);
		break;
	case 2:
		display.drawBitmap(39, 7, timeIcon, 50, 50, 1);
		break;
	case 3:
		display.drawBitmap(32, 0, bilibiliIcon, 64, 64, 1);
		break;
	case 4:
		display.drawBitmap(39, 7, alarmClockIcon, 50, 50, 1);
		break;
	case 5:
		display.drawBitmap(39, 7, stopWatchIcon, 50, 50, 1);
		break;
	case 6:
		display.drawBitmap(32, 0, stepIcon, 64, 64, 1);
		break;
	default:
		break;
	}
	Serial.print("eventSwitch=");
	Serial.println(eventSwitch);
	display.display();
	if (button2Press)
	{
		if (eventSwitch < 6)
		{
			eventSwitch++;
		}
		else
		{
			eventSwitch = 1;
		}
		delay(200);
		button2Press = 0;
	}
	if (confirm)
	{
		delay(200);
		menu = 0;
		confirm = 0;
	}
}
//事件切换
void SelectEventSwitch()
{
	switch (eventSwitch)
	{
	case 1:
		Max30102Measure();
		break;
	case 2:
		GetWeather();
		ShowTimeWeather();
		delay(2000);
		ShowIcon();
		break;
	case 3:
		GetBiliBiliFollowers();
		ShowBilibiliFollower();
		break;
	case 4:
		Alarm();
		break;
	case 5:
		StopWatch();
		break;
	case 6:
		pedometer();
		break;
	default:
		Max30102Measure();
		break;
	}
}