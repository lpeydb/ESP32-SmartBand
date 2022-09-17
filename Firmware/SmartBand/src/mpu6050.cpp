#include "mpu6050.h"

Adafruit_MPU6050 mpu;
// 计步器用变数
int step = 0;
float thresholdValue = 3; //计步阈值
bool stepDown = false;   //用于检测方向，当加速度大于正阈值为0，小于负阈值为1
void MpuInit()
{
	while (!mpu.begin())
	{
		Serial.println("Failed to find MPU6050 chip");
	}
	// setupt motion detection
	mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
	mpu.setMotionDetectionThreshold(1);
	mpu.setMotionDetectionDuration(20);
	mpu.setInterruptPinLatch(true); // Keep it latched.  Will turn off when reinitialized.
	mpu.setInterruptPinPolarity(true);
	mpu.setMotionInterrupt(true);
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