#include <stdio.h>
#include <wiringPi.h>
#include "gut.h"

static const int first_gut_pin = 10;
static const int second_gut_pin = 9;
static const int electric_on_time = 3000; //ms・3秒間


int cut_initialize(){

	if(wiringPiSetupGpio() == -1)
	{
		printf("error wiringPi setup\n");
		return -1;
	}
	else
	{
		printf("wiringPisetup success\n");
	}
	pinMode(gut_pin, OUTPUT);
	return 0;
}

int cutInterupt(void)
{
	digitalWrite(first_gut_pin,0);
	digitalWrite(second_gut_pin,0);
}

int first_cut(){
	int i=0;

	printf("first nichrome cut now\n");

	for(i=0; i<2; i++)
	{
		//念のため二回流す
		printf("%d\n", i);
		digitalWrite(first_gut_pin,1);
		delay(electric_on_time);
		digitalWrite(second_gut_pin,0);
		delay(2000);
	}
	return 0;
}

int second_cut(){
	int i=0;

	printf("second nichrome cut now\n");

	for(i=0; i<2; i++)
	{
		//念のため二回流す
		printf("%d\n", i);
		digitalWrite(second_gut_pin,1);
		delay(electric_on_time);
		digitalWrite(second_gut_pin,0);
		delay(2000);
	}
	return 0;
}
