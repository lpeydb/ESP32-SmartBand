#include "timer.h"

bool counts = 0; //获取时间变量
//定时器中断用变数
int TimeCounter = 0;
hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;
//秒表
int stopWatchHour = 0;
int stopWatchMinute = 0;
int stopWatchSecon = 0;
bool stopWatchWorking = 0;
void TimeInit()
{
	timer1 = timerBegin(0, 80, true); //选用定时器0
	timerAttachInterrupt(timer1, &Timer1Event, true);
	timerAlarmWrite(timer1, 1000000, true); // 1s中断一次

	timer2 = timerBegin(1, 80, true); //选用定时器1
	timerAttachInterrupt(timer2, &Timer2Event, true);
	timerAlarmWrite(timer2, 30000000, true); // 20s中断一次,获取一次时间
	timerAlarmEnable(timer2);
}
//定时器中断事件
void IRAM_ATTR Timer1Event()
{
	Serial.println("Timer1Event");
	if (stopWatchSecon < 60)
	{
		stopWatchSecon++;
	}
	else
	{
		stopWatchSecon = 0;
		if (stopWatchMinute < 60)
		{
			stopWatchMinute++;
		}
		else
		{
			stopWatchMinute = 0;
			if (stopWatchHour < 60)
			{
				stopWatchHour++;
			}
		}
	}
}
void IRAM_ATTR Timer2Event()
{
	Serial.println("Timer2Event");
	counts = 1;
}
//按键中断事件
void Button1IntEvent()
{
	menu = 1;
	//Serial.println("Button1Int");
}
void Button2IntEvent()
{
	button2Press = 1;
	Serial.println("Button2Int");
}
void Button3IntEvent()
{
	confirm = 1;
}
//显示闹钟
void Alarm()
{
	GetTime();
	int Hour = currentHour;
	int Minute = currentMinute;
	while (1)
	{
		display.clearDisplay();
		display.drawBitmap(0, 0, he, 16, 16, WHITE);
		display.drawBitmap(18, 0, shui, 16, 16, WHITE);
		display.drawBitmap(36, 0, chi, 16, 16, WHITE);
		display.drawBitmap(54, 0, yao, 16, 16, WHITE);
		display.setTextSize(2);
		display.setTextColor(WHITE);
		display.setCursor(0, 18);
		if (Hour < 10)
		{
			display.print("0"); //给小时数补零
		}
		display.print(Hour);
		display.print(":");
		if (Minute < 10)
		{
			display.print("0"); //给分钟数补零
		}
		display.print(Minute);
		display.display();
		if (button2Press)
		{
			Minute++;
			if (Minute >= 60)
			{
				Hour++;
				Minute = 0;
			}
			delay(200);
			button2Press = 0;
		}
		if (confirm) 
		{
			if ((Hour > currentHour) || (Minute > currentMinute))    //判断是否有设置时间（不能设置和当前时间一模一样）
			{
				setHour = Hour;
				setMinute = Minute;
			}
			menu = 1;
			delay(200);
			confirm = 0;
			break;
		}
		if (menu)
		{
			break;
		}
	}
}
//秒表
void StopWatch()
{
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	if (stopWatchHour < 10)
	{
		display.print("0"); //给小时数补零
	}
	display.print(stopWatchHour);
	display.print(":");
	if (stopWatchMinute < 10)
	{
		display.print("0"); //给小时数补零
	}
	display.print(stopWatchMinute);
	display.print(":");
	if (stopWatchSecon < 10)
	{
		display.print("0"); //给小时数补零
	}
	display.print(stopWatchSecon);
	display.display();
	if (confirm)
	{
		delay(200);
		confirm = 0;
		stopWatchWorking = !stopWatchWorking;
		if (stopWatchWorking)
		{
			timerAlarmEnable(timer1); //	使能定时器
			Serial.println("timerAlarmEnable");
		}
		else
		{
			timerAlarmDisable(timer1);
			Serial.println("timerAlarmDisable");
		}
	}
	if (menu)
	{
		timerAlarmDisable(timer1);
		stopWatchHour = 0;
		stopWatchMinute = 0;
		stopWatchSecon = 0;
	}
}