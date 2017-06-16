#include <stdio.h>
#include <inttypes.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "luxsensor.h"

//change adress:default 0x39
static const int TSL2561_ADDR_LOW = 0x29;
static const int TSL2561_ADDR_FLOAT = 0x39;
static const int TSL2561_ADDR_HIGH  = 0x49;

static const int TSL2561_CONTROL_POWERON = 0x03;
static const int TSL2561_CONTROL_POWEROFF = 0x00;

static const int TSL2561_GAIN_0X = 0x00;   //No gain
static const int TSL2561_GAIN_AUTO = 0x01;
static const int TSL2561_GAIN_1X = 0x02;
static const int TSL2561_GAIN_16X = 0x12; // (0x10)
static const int TSL2561_INTEGRATIONTIME_13MS = 0x00;   // 13.7ms
static const int TSL2561_INTEGRATIONTIME_101MS = 0x01; // 101ms
static const int TSL2561_INTEGRATIONTIME_402MS = 0x02; // 402ms

static const int TSL2561_READBIT = 0x01;
static const int TSL2561_COMMAND_BIT = 0x80;  //Must be 1
static const int TSL2561_CLEAR_BIT = 0x40;  //Clears any pending interrupt (write 1 to clear)
static const int TSL2561_WORD_BIT = 0x20;  // 1 = read/write word (rather than byte)
static const int TSL2561_BLOCK_BIT = 0x10;   // 1 = using block read/write

static const int TSL2561_REGISTER_CONTROL = 0x00;
static const int TSL2561_REGISTER_TIMING = 0x81;

static const int TSL2561_REGISTER_CHAN0_LOW = 0x8C;
static const int TSL2561_REGISTER_CHAN0_HIGH = 0x8D;
static const int TSL2561_REGISTER_CHAN1_LOW = 0x8E;
static const int TSL2561_REGISTER_CHAN1_HIGH = 0x8F;
//Delay getLux function
static const int LUXDELAY = 500;
static const int LIGHT_THRESHOLD = 10;  //光センサー閾値
static const int fd = 0;
//関数プロトタイプ宣言(static)
int getLux();

int luxsensor_initialize()
{
	//I2c setup
	fd = wiringPiI2CSetup(TSL2561_ADDR_FLOAT);
	//NOTE TSL2561_ADDR_FLOATをTSL2561_ADDR_LOWに置き換える必要あるかも
	if(fd == -1)
	{
		printf("WARNING! luxsensor wiringPiI2CSetup error\n");
		printf("fd = %d, errno=%d: %s\n", fd, errno, strerror(errno));
		return -1;
	}
	else
	{
		printf("luxsensor wiringPiI2CSetup success\n");
		printf("fd = %d, errno=%d: %s\n", fd, errno, strerror(errno));
	}
	wiringPiI2CWriteReg8(fd, TSL2561_COMMAND_BIT, TSL2561_CONTROL_POWERON);
	return 0;
}

int luxsensor_close()
{
	wiringPiI2CWriteReg8(fd, TSL2561_COMMAND_BIT, TSL2561_CONTROL_POWEROFF);
	return 0;
}

int getLux()
{
	//NOTE setup忘れたら値が振り切れる
	// Set timing (101 mSec)
	wiringPiI2CWriteReg8(fd, TSL2561_REGISTER_TIMING, TSL2561_GAIN_AUTO);
	//Wait for the conversion to complete
	delay(LUXDELAY);
	int visible_and_ir = wiringPiI2CReadReg16(fd, TSL2561_REGISTER_CHAN0_LOW);
	//CH0 photodiode:sensitive to both visible and infrared light
	return visible_and_ir;
}

int islight(){
	int lux=0;
	lux = getLux();
	if(lux>LIGHT_THRESHOLD) {
		printf("light:%d\n",lux);
		return 1;
	}
	else{
		return 0;
	}
}
