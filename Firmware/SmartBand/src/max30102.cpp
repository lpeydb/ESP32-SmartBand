#include "max30102.h"

MAX30105 particleSensor;
const byte RATE_SIZE = 10; //多少平均數量     10
byte rates[RATE_SIZE];     //心跳陣列
bool Max30102WorkRight = 0;
//計算心跳用變數
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
double FSpO2 = 0.7;  // filter factor for estimated SpO2
double frate = 0.95; // low pass filter for IR/red LED value to eliminate AC component
void Max30102Init()
{
    // Try to initialize!
    while (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
    {
        Serial.println("找不到MAX30102");
    }
    byte ledBrightness = 0x7F; //亮度Options: 0=Off to 255=50mA         0x7F
    byte sampleAverage = 4;    // Options: 1, 2, 4, 8, 16, 32            4
    byte ledMode = 2;          // Options: 1 = Red only(心跳), 2 = Red + IR(血氧)        2
    // Options: 1 = IR only, 2 = Red + IR on MH-ET LIVE MAX30102 board
    int sampleRate = 800; // Options: 50, 100, 200, 400, 800, 1000, 1600, 3200         800
    int pulseWidth = 215; // Options: 69, 118, 215, 411        215
    int adcRange = 16384; // Options: 2048, 4096, 8192, 16384
    // Set up the wanted parameters
    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); // Configure sensor with these settings
    particleSensor.enableDIETEMPRDY();
    particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running  0x0A
    particleSensor.setPulseAmplitudeGreen(0);  // Turn off Green LED
}
//测心率血氧函数
void Max30102Measure()
{
    int i = 0;
    int Num = 30;                          //取樣100次才計算1次
    long irValue = particleSensor.getIR(); // Reading the IR value it will permit us to know if there's a finger on the sensor or not
    //是否有放手指
    if (irValue > FINGER_ON)
    {
        display.clearDisplay();                             //清除螢幕
        display.drawBitmap(5, 5, logo2_bmp, 24, 21, WHITE); //顯示小的心跳圖示
        display.setTextSize(2);                             //設定文字大小
        display.setTextColor(WHITE);                        //文字顏色
        display.setCursor(42, 10);                          //設定游標位置
        display.print(beatAvg);
        display.println(" BPM");                          //顯示心跳數值
        display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE); //顯示氧氣圖示
        display.setCursor(42, 40);                        //設定游標位置
        //顯示血氧數值
        if (beatAvg > 30)
            display.print(String(ESpO2) + "%");
        else
            display.print("---- %");
        display.display(); //顯示螢幕
        //是否有心跳
        if (checkForBeat(irValue) == true)
        {
            display.clearDisplay();                             //清除螢幕
            display.drawBitmap(0, 0, logo3_bmp, 32, 32, WHITE); //顯示大的心跳圖示
            display.setTextSize(2);                             //設定文字大小
            display.setTextColor(WHITE);                        //文字顏色
            display.setCursor(42, 10);                          //設定游標位置
            display.print(beatAvg);
            display.println(" BPM");                          //顯示心跳數值
            display.drawBitmap(0, 35, O2_bmp, 32, 32, WHITE); //顯示氧氣圖示
            display.setCursor(42, 40);                        //設定游標位置
            //顯示血氧數值
            if (beatAvg > 30)
                display.print(String(ESpO2) + "%");
            else
                display.print("---- %");
            display.display();   //顯示螢幕
            tone(Tonepin, 1000); //發出聲音
            delay(10);
            noTone(Tonepin); //停止聲音
            Serial.print("beatAvg=");
            Serial.println(beatAvg);          //將心跳顯示到序列
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
            fred = (double)red;                                    //轉double
            fir = (double)ir;                                      //轉double
            avered = avered * frate + (double)red * (1.0 - frate); // average red level by low pass filter
            aveir = aveir * frate + (double)ir * (1.0 - frate);    // average IR level by low pass filter
            sumredrms += (fred - avered) * (fred - avered);        // square sum of alternate component of red level
            sumirrms += (fir - aveir) * (fir - aveir);             // square sum of alternate component of IR level
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
//声光警报
void LedToneAlarm()
{
	for (int i = 0; i < 5; i++)
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