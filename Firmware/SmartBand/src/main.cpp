#include <Arduino.h>
#include <Adafruit_GFX.h> //OLED libraries
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include "MAX30105.h"  //MAX3010x library
#include "heartRate.h" //Heart rate calculating algorithm
#include "ESP32Servo.h"
#include "ArduinoJson.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "pic.h"
MAX30105 particleSensor;
Adafruit_MPU6050 mpu;
//定义引脚
#define Tonepin 2
#define Ledpin 23
#define Buttonpin1 5  
#define Buttonpin2 18 
#define Buttonpin3 19 
//提醒吃药喝水用变数
int setHour = -1;
int setMinute = -1;
//超过阈值声光报警用变数
bool Max30102WorkRight = 0;
//切换事件用变数
bool confirm = 0;
bool menu = 1;
int eventSwitch = 1;
bool button2Press = 0;
//計算心跳用變數
const byte RATE_SIZE = 10; //多少平均數量     10
byte rates[RATE_SIZE];	   //心跳陣列
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
//計算血氧用變數
double avered = 0;
double aveir = 0;
double sumirrms = 0;
double sumredrms = 0;
double SpO2 = 0;
double ESpO2 = 90.0; //初始值
double FSpO2 = 0.7;	 // filter factor for estimated SpO2
double frate = 0.95; // low pass filter for IR/red LED value to eliminate AC component
int i = 0;
int Num = 30;			  //取樣100次才計算1次
#define FINGER_ON 7000	  //紅外線最小量（判斷手指有沒有上）
#define MINIMUM_SPO2 90.0 //血氧最小量
//定时器中断用变数
int TimeCounter = 0;
hw_timer_t *timer1 = NULL;
hw_timer_t *timer2 = NULL;
//秒表
int stopWatchHour = 0;
int stopWatchMinute = 0;
int stopWatchSecon = 0;
bool stopWatchWorking = 0;
// oled初始化
#define SCREEN_WIDTH 128												  // OLED寬度
#define SCREEN_HEIGHT 64												  // OLED高度
#define OLED_RESET -1													  // Reset pin
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Declaring the display name (display)
// WiFi的初始化和连接
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// WiFi用变数
String weatherUrl = "https://api.seniverse.com/v3/weather/now.json?key=S92lT1RxhOcp7k2v_&location=huizhou&language=en&unit=c";
String weather;
String location;
String temperature;
int icon;
String UID = "477768935";
String followerUrl = "http://api.bilibili.com/x/relation/stat?vmid=" + UID; // 粉丝数
long follower = 0;															// 粉丝数
DynamicJsonDocument doc(1024);
int currentHour;
int currentMinute;
int monthDay;
int month;
int weekDay;
bool counts = 0; //获取时间变量
// 计步器用变数
int step = 0;
float thresholdValue = 3; //计步阈值
bool stepDown = false;   //用于检测方向，当加速度大于正阈值为0，小于负阈值为1
//连接WiFi
void WiFi_Connect()
{
	WiFi.begin("Mytest", "123456789");
	while (WiFi.status() != WL_CONNECTED)
	{ //这里是阻塞程序，直到连接成功
		showWifiConnet();
		// Serial.print(".");
	}
}
//	获取粉丝数
void GetBiliBiliFollowers()
{
	HTTPClient http;
	http.begin(followerUrl); // HTTP begin
	int httpCode = http.GET();

	if (httpCode > 0)
	{
		// httpCode will be negative on error
		// Serial.printf("HTTP Get Code: %d\r\n", httpCode);

		if (httpCode == HTTP_CODE_OK) // 收到正确的内容
		{
			String resBuff = http.getString();

			//	输出示例：{"mid":123456789,"following":226,"whisper":0,"black":0,"follower":867}}
			// Serial.println(resBuff);
			//	使用ArduinoJson_6.x版本，具体请移步：https://github.com/bblanchon/ArduinoJson
			deserializeJson(doc, resBuff); //开始使用Json解析
			follower = doc["data"]["follower"];
			// Serial.printf("Follers: %ld \r\n", follower);
		}
	}
	else
	{
		Serial.printf("HTTP Get Error: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
}
void RemainDrinkEat();
//获取时间
void GetTime()
{
	timeClient.update();
	// Serial.println(timeClient.getFormattedTime());
	unsigned long epochTime = timeClient.getEpochTime();
	// Serial.print("Epoch Time: ");
	// Serial.println(epochTime);
	//打印时间
	currentHour = timeClient.getHours();
	// Serial.print("Hour: ");
	// Serial.println(currentHour);
	currentMinute = timeClient.getMinutes();
	// Serial.print("Minutes: ");
	// Serial.println(currentMinute);
	weekDay = timeClient.getDay();
	// Serial.print("Week Day: ");
	// Serial.println(weekDay);
	//将epochTime换算成年月日
	struct tm *ptm = gmtime((time_t *)&epochTime);
	monthDay = ptm->tm_mday;
	// Serial.print("Month day: ");
	// Serial.println(monthDay);
	month = ptm->tm_mon + 1;
	// Serial.print("Month: ");
	// Serial.println(month);
	if ((currentHour == setHour) && (currentMinute == setMinute))
	{
		RemainDrinkEat();
	}
}
// 选择天气图标
void selectIcon(int i)
{
	switch (i)
	{
	case 0: // 白天晴天
	case 2:
		icon = 1;
		break;

	case 1: // 晚上晴天
	case 3:
		icon = 2;
		break;

	case 4: // 多云
	case 5:
	case 6:
	case 7:
	case 8:
	case 9:
		icon = 3;
		break;

	case 10: // 阵雨(太阳雨)
		icon = 4;
		break;

	case 11: // 雷阵雨
	case 12:
		icon = 5;
		break;

	case 13: // 小雨
		icon = 6;
		break;

	case 14: // 大到暴雨
	case 15:
	case 16:
	case 17:
	case 18:
		icon = 7;
		break;

	case 20: // 雪
	case 21:
	case 22:
	case 23:
	case 24:
	case 25:
		icon = 8;
		break;

	case 31: // 霾
		icon = 9;
		break;

	case 32: // 风
	case 33:
		icon = 10;
		break;

	default:
		icon = 11;
		break;
	}
}
//获取天气
void GetWeather()
{
	HTTPClient http;
	http.begin(weatherUrl); // HTTP begin
	int httpCode = http.GET();

	if (httpCode > 0)
	{
		// httpCode will be negative on error
		// Serial.printf("HTTP Get Code: %d\r\n", httpCode);

		if (httpCode == HTTP_CODE_OK) // 收到正确的内容
		{
			String resBuff = http.getString();
			deserializeJson(doc, resBuff); //开始使用Json解析
			JsonObject results_0 = doc["results"][0];

			JsonObject results_0_location = results_0["location"];
			const char *results_0_location_name = results_0_location["name"]; // "Huizhou"

			JsonObject results_0_now = results_0["now"];
			const char *results_0_now_text = results_0_now["text"]; // "Heavy rain"
			int weathercode = results_0_now["code"];
			const char *results_0_now_temperature = results_0_now["temperature"]; // "25"
			// int weathercode = results_0_now_code;
			location = results_0_location_name;
			weather = results_0_now_text;
			selectIcon(weathercode);
			temperature = results_0_now_temperature;
			// Serial.println(temperature);
			// Serial.println(weather);
			// Serial.println(location);
		}
	}
	else
	{
		Serial.printf("HTTP Get Error: %s\n", http.errorToString(httpCode).c_str());
	}
	http.end();
}
//显示时间和天气
void ShowTimeWeather()
{
	display.clearDisplay();
	display.setTextSize(4);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	if (currentHour < 10)
	{
		display.print("0"); //给小时数补零
	}
	display.print(currentHour);
	display.print(":");
	if (currentMinute < 10)
	{
		display.print("0"); //给分钟数补零
	}
	display.println(currentMinute);
	display.setTextSize(1);
	switch (weekDay)
	{
	case 1:
		display.print("Monday");
		break;
	case 2:
		display.print("Tuesday");
		break;
	case 3:
		display.print("Wednesday");
		break;
	case 4:
		display.print("Thursday");
		break;
	case 5:
		display.print("Friday");
		break;
	case 6:
		display.print("Saturday");
		break;
	case 0:
		display.print("Sunday");
		break;
	default:
		display.print("Unknown");
		break;
	}
	display.print(" ");
	display.print(month);
	display.print("/");
	display.println(monthDay);
	display.println(location);
	display.println(weather);
	display.print("temperature:");
	display.print(temperature);
	display.display();
	delay(1000);
	// Serial.println("ShowTimeWeather End");
}
//显示天气GIF
void ShowIcon()
{
	switch (icon)
	{
	case 1:
		showSun();
		break;
	case 2:
		showDayNight();
		break;
	case 3:
		showCloudy();
		break;
	case 4:
		showRainCloudy();
		break;
	case 5:
		showtStormy();
		break;
	case 6:
		showRainy();
		break;
	case 7:
		showTorrentialRain();
		break;
	case 8:
		showLightSnowy();
		break;
	case 9:
		showHaze();
		break;
	case 10:
		showWindy();
		break;

	default:
		break;
	}
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
//声光警报
void LedToneAlarm()
{
	for (i = 0; i < 5; i++)
	{
		tone(Tonepin, 440);
		digitalWrite(Ledpin, 1);
		delay(256);
		tone(Tonepin, 329);
		digitalWrite(Ledpin, 0);
		delay(256);
	}
	noTone(Tonepin); //停止聲音
	Max30102WorkRight = 0;
	//清除心跳數據
	for (byte rx = 0; rx < RATE_SIZE; rx++)
		rates[rx] = 0;
	beatAvg = 0;
	rateSpot = 0;
	lastBeat = 0;
	//清除血氧數據
	avered = 0;
	aveir = 0;
	sumirrms = 0;
	sumredrms = 0;
	SpO2 = 0;
	ESpO2 = 90.0;
}
//测心率血氧函数
void Max30102Measure()
{
	long irValue = particleSensor.getIR(); // Reading the IR value it will permit us to know if there's a finger on the sensor or not
	//是否有放手指
	if (irValue > FINGER_ON)
	{
		display.clearDisplay();								//清除螢幕
		display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE); //顯示小的心跳圖示
		display.setTextSize(2);								//設定文字大小
		display.setTextColor(WHITE);						//文字顏色
		display.setCursor(42, 10);							//設定游標位置
		display.print(beatAvg);
		display.println(" BPM");						  //顯示心跳數值
		display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE); //顯示氧氣圖示
		display.setCursor(42, 40);						  //設定游標位置
		//顯示血氧數值
		if (beatAvg > 30)
			display.print(String(ESpO2) + "%");
		else
			display.print("---- %");
		display.display(); //顯示螢幕
		//是否有心跳
		if (checkForBeat(irValue) == true)
		{
			display.clearDisplay();								//清除螢幕
			display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE); //顯示大的心跳圖示
			display.setTextSize(2);								//設定文字大小
			display.setTextColor(WHITE);						//文字顏色
			display.setCursor(42, 10);							//設定游標位置
			display.print(beatAvg);
			display.println(" BPM");						  //顯示心跳數值
			display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE); //顯示氧氣圖示
			display.setCursor(42, 40);						  //設定游標位置
			//顯示血氧數值
			if (beatAvg > 30)
				display.print(String(ESpO2) + "%");
			else
				display.print("---- %");
			display.display();	 //顯示螢幕
			tone(Tonepin, 1000); //發出聲音
			delay(10);
			noTone(Tonepin); //停止聲音
			Serial.print("beatAvg=");
			Serial.println(beatAvg);		  //將心跳顯示到序列
			long delta = millis() - lastBeat; //計算心跳差
			lastBeat = millis();
			beatsPerMinute = 60 / (delta / 1000.0); //計算平均心跳
			if (beatsPerMinute < 255 && beatsPerMinute > 20)
			{
				//心跳必須再20-255之間
				rates[rateSpot++] = (byte)beatsPerMinute; //儲存心跳數值陣列
				rateSpot %= RATE_SIZE;
				beatAvg = 0; //計算平均值
				for (byte x = 0; x < RATE_SIZE; x++)
					beatAvg += rates[x];
				beatAvg /= RATE_SIZE;
			}
		}
		//計算血氧
		uint32_t ir, red;
		double fred, fir;
		particleSensor.check(); // Check the sensor, read up to 3 samples
		if (particleSensor.available())
		{
			i++;
			red = particleSensor.getFIFOIR(); //讀取紅光
			ir = particleSensor.getFIFORed(); //讀取紅外線
			// Serial.println("red=" + String(red) + ",IR=" + String(ir) + ",i=" + String(i));
			fred = (double)red;									   //轉double
			fir = (double)ir;									   //轉double
			avered = avered * frate + (double)red * (1.0 - frate); // average red level by low pass filter
			aveir = aveir * frate + (double)ir * (1.0 - frate);	   // average IR level by low pass filter
			sumredrms += (fred - avered) * (fred - avered);		   // square sum of alternate component of red level
			sumirrms += (fir - aveir) * (fir - aveir);			   // square sum of alternate component of IR level
			if ((i % Num) == 0)
			{
				double R = (sqrt(sumredrms) / avered) / (sqrt(sumirrms) / aveir);
				SpO2 = -23.3 * (R - 0.4) + 100;
				ESpO2 = FSpO2 * ESpO2 + (1.0 - FSpO2) * SpO2; // low pass filter
				if (ESpO2 <= MINIMUM_SPO2)
					ESpO2 = MINIMUM_SPO2; // indicator for finger detached
				if (ESpO2 > 100)
					ESpO2 = 99.9;
				Serial.print("Oxygen % = ");
				Serial.println(ESpO2);
				sumredrms = 0.0;
				sumirrms = 0.0;
				SpO2 = 0;
				i = 0;
			}
			particleSensor.nextSample(); // We're finished with this sample so move to next sample
		}

		//超过限值声光报警
		if ((beatAvg > 50) && (ESpO2 > 96))
		{
			Max30102WorkRight = 1;
		}

		if ((Max30102WorkRight == 1) && ((beatAvg > 90) || (ESpO2 < 95) || (beatAvg < 50)))
		{

			LedToneAlarm();
		}
	}
	else
	{
		Max30102WorkRight = 0;
		//清除心跳數據
		for (byte rx = 0; rx < RATE_SIZE; rx++)
			rates[rx] = 0;
		beatAvg = 0;
		rateSpot = 0;
		lastBeat = 0;
		//清除血氧數據
		avered = 0;
		aveir = 0;
		sumirrms = 0;
		sumredrms = 0;
		SpO2 = 0;
		ESpO2 = 90.0;
		//显示手指图片
		display.clearDisplay();
		display.drawBitmap(32, 0, finger, 64, 64, 1);
		display.display();
		noTone(Tonepin);
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
//计步器
void pedometer()
{
	display.clearDisplay();
	display.setTextSize(3);
	display.setTextColor(WHITE);
	display.setCursor(0, 0);
	display.print("Step:");
	display.print(step);
	display.display();
	if (mpu.getMotionInterruptStatus())
	{
		/* Get new sensor events with the readings */
		sensors_event_t a, g, temp;
		mpu.getEvent(&a, &g, &temp);

		/* Print out the values */
		Serial.print("AccelX:");
		Serial.print(a.acceleration.x);
		Serial.print(",");
		Serial.print("AccelY:");
		Serial.print(a.acceleration.y);
		Serial.print(",");
		Serial.print("AccelZ:");
		Serial.println(a.acceleration.z);
		if ((a.acceleration.x< (-thresholdValue) || a.acceleration.y< (-thresholdValue)))
		{
			stepDown=1;
		}
		if ((a.acceleration.x> thresholdValue || a.acceleration.y> thresholdValue) && stepDown) 
		{
			step+=2;    //来回摆动算两步
			stepDown=0;
			Serial.print("step:");
			Serial.println(step);
		}
	}
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
void setup()
{
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Start the OLED display
	display.clearDisplay();
	display.display();
	Serial.begin(115200);
	delay(1000);

	Serial.print("WiFiConnecting.. ");
	WiFi_Connect();
	timeClient.begin();
	timeClient.setTimeOffset(28800); // + 1区 偏移3600， +8区 ：3600×8 = 28800

	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	pinMode(Ledpin, OUTPUT);
	pinMode(Buttonpin1, INPUT_PULLUP);
	pinMode(Buttonpin2, INPUT_PULLUP);
	pinMode(Buttonpin3, INPUT_PULLUP);
	attachInterrupt(Buttonpin1, Button1IntEvent, FALLING);
	attachInterrupt(Buttonpin2, Button2IntEvent, FALLING);
	attachInterrupt(Buttonpin3, Button3IntEvent, FALLING);
	// Try to initialize!
	if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
	{
		Serial.println("找不到MAX30102");
		while (1)
			;
	}
	if (!mpu.begin())
	{
		Serial.println("Failed to find MPU6050 chip");
		while (1)
		{
			delay(10);
		}
	}
	// setupt motion detection
	mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
	mpu.setMotionDetectionThreshold(1);
	mpu.setMotionDetectionDuration(20);
	mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
	mpu.setInterruptPinPolarity(true);
	mpu.setMotionInterrupt(true);

	byte ledBrightness = 0x7F; //亮度Options: 0=Off to 255=50mA         0x7F
	byte sampleAverage = 4;	   // Options: 1, 2, 4, 8, 16, 32            4
	byte ledMode = 2;		   // Options: 1 = Red only(心跳), 2 = Red + IR(血氧)        2
	// Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
	int sampleRate = 800; // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200         800
	int pulseWidth = 215; // Options: 69, 118, 215, 411        215
	int adcRange = 16384; // Options: 2048, 4096, 8192, 16384

	// Set up the wanted parameters
	particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings
	particleSensor.enableDIETEMPRDY();
	particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running  0x0A
	particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED

	timer1 = timerBegin(0, 80, true); //选用定时器0
	timerAttachInterrupt(timer1, &Timer1Event, true);
	timerAlarmWrite(timer1, 1000000, true); // 1s中断一次

	timer2 = timerBegin(1, 80, true); //选用定时器1
	timerAttachInterrupt(timer2, &Timer2Event, true);
	timerAlarmWrite(timer2, 30000000, true); // 20s中断一次,获取一次时间
	timerAlarmEnable(timer2);

	showCool();
	showCool();
	GetTime();
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