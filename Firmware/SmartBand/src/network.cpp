#include "network.h"

long follower = 0; // 粉丝数
int currentHour;
int currentMinute;
int monthDay;
int month;
int weekDay;
// WiFi的初始化和连接
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// WiFi用变数
String weather;
String location;
String temperature;
//连接WiFi
static void WiFiConnect()
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
	String UID = "477768935";
	String followerUrl = "http://api.bilibili.com/x/relation/stat?vmid=" + UID; // 粉丝链接
	DynamicJsonDocument doc(1024);

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
//获取天气
void GetWeather()
{
	String weatherUrl = "https://api.seniverse.com/v3/weather/now.json?key=S92lT1RxhOcp7k2v_&location=huizhou&language=en&unit=c";
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
			DynamicJsonDocument doc(1024);
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
void NetworkInit()
{
	Serial.print("WiFiConnecting.. ");
	WiFiConnect();
	timeClient.begin();
	timeClient.setTimeOffset(28800); // + 1区 偏移3600， +8区 ：3600×8 = 28800

	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	GetTime();
}