#include <stdio.h>
#include <math.h>
#include <gps.h>
#include <wiringPi.h>

loc_t data;//gpsのデータを確認するものをグローバル変数宣言

int main()
{
	gps_init();
	while(1)
	{
		gps_avail();
		gps_location(&data);
		printf("time:%f\nlatitude:%f\nlongitude:%f\naltitude:%f\n",
		       data.time,data.latitude, data.longitude,data.altitude);
		delay(9000);
		gps_flush();
	}
	return 0;
}
