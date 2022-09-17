#ifndef _NETWORK_H
#define _NETWORK_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include "pic.h"

extern String weather;
extern String location;
extern String temperature;
extern long follower;
extern int currentHour;
extern int currentMinute;
extern int monthDay;
extern int month;
extern int weekDay;
static void WiFiConnect();
void GetBiliBiliFollowers();
void GetTime();
void GetWeather();
void NetworkInit();

#endif