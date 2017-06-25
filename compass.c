#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "compass.h"
#include "motor.h"

static const int ANGLE_OF_DEVIATION = -7.2;
static const int HMC5883L_ADDRESS = 0x1e; //I2C address
static const int MODE_REG = 0x02;
static const int MODE_CONTINUOUS = 0x00;
static const int MODE_SINGLE = 0x01;
static const int X_MSB_REG = 0x03;
static const int X_LSB_REG = 0x04;
static const int Z_MSB_REG = 0x05;
static const int Z_LSB_REG = 0x06;
static const int Y_MSB_REG = 0x07;
static const int Y_LSB_REG = 0x08;
static const double PI = 3.14159265;
static const double K_PARAMETER = 1.0;//地磁気の感度補正パラメータ
//calibration時の回転
static const int TURN_CALIB_POWER = 25;   //地磁気補正時turnするpower
static const int TURN_CALIB_MILLISECONDS = 75;   //地磁気補正時turnするmilliseconds
//周囲の今日磁場がある時の退避
static const int MAX_PWM_VAL = 100;
static const int ESCAPE_TIME = 1000;

static int fd = 0;
static int WPI2CWReg8 = 0;
int compass_initialize()
{
	//I2c setup
	fd = wiringPiI2CSetup(HMC5883L_ADDRESS);
	if(fd == -1)
	{
		printf("WARNING! compass wiringPiI2CSetup error\n");
		printf("fd = %d, errno=%d: %s\n", fd, errno, strerror(errno));
		return -1;
	}
	else
	{
		printf("compass wiringPiI2CSetup success\n");
		printf("fd = %d, errno=%d: %s\n", fd, errno, strerror(errno));
	}
	return 0;
}

//地磁気ロック対策のmode_change関数
static int compass_mode_change()
{
	WPI2CWReg8 = wiringPiI2CWriteReg8(fd,MODE_REG,MODE_SINGLE);
	if(WPI2CWReg8 == -1)
	{
		printf("compass write error register MODE_REG\n");
		printf("wiringPiI2CWriteReg8 = %d\n", WPI2CWReg8);
		printf("errno=%d: %s\n", errno, strerror(errno));
	}
	WPI2CWReg8 = wiringPiI2CWriteReg8(fd,MODE_REG,MODE_CONTINUOUS);
	if(WPI2CWReg8 == -1)
	{
		printf("compass write error register MODE_REG\n");
		printf("wiringPiI2CWriteReg8 = %d\n", WPI2CWReg8);
		printf("errno=%d: %s\n", errno, strerror(errno));
	}
	return 0;
}

static short read_out(int file,int msb_reg, int lsb_reg)
{
	uint8_t msb = 0;
	uint8_t lsb = 0;
	short i = 0;
	msb = wiringPiI2CReadReg8(file, msb_reg);
	lsb = wiringPiI2CReadReg8(file, lsb_reg);
	i = msb << 8| lsb;
	return i;
}

//通常のcompass読み取り関数 error時のみ表示するようにした
int compass_read(Cmps *compass_data)
{
	WPI2CWReg8 = wiringPiI2CWriteReg8(fd,MODE_REG,MODE_CONTINUOUS);
	if(WPI2CWReg8 == -1)
	{
		printf("Compass write error register MODE_REG\n");
		printf("wiringPiI2CWriteReg8 = %d\n", WPI2CWReg8);
		errno = -WPI2CWReg8;
		printf("errno=%d: %s\n", errno, strerror(errno));
	}
	/*uint8_t status_val = wiringPiI2CReadReg8(fd, 0x09);  とりあえずコメントアウトしておきます
	   printf("1st bit of status resister = %d\n", (status_val >> 0) & 0x01); //地磁気が正常ならここは1(死んでも1?)
	   printf("2nd bit of status resister = %d\n", (status_val >> 1) & 0x01); //地磁気が正常ならここは0(死んだら1)*/
	compass_data->x_value = (double)read_out(fd, X_MSB_REG, X_LSB_REG);
	compass_data->y_value = (double)read_out(fd, Y_MSB_REG, Y_LSB_REG);
	compass_data->z_value = (double)read_out(fd, Z_MSB_REG, Z_LSB_REG);
	return 0;
}

/*地磁気のキャリブレーション補正用のログを取るためにprintをコメントアウトしたもの*/
int compass_read_scatter(Cmps *data)
{
	//WriteReg8
	WPI2CWReg8 = wiringPiI2CWriteReg8(fd,MODE_REG,MODE_CONTINUOUS);
	/*if(WPI2CWReg8 == -1)
	   {
	        printf("Compass write error register MODE_REG\n");
	        printf("wiringPiI2CWriteReg8 = %d\n", WPI2CWReg8);
	        errno = -WPI2CWReg8;
	        printf("errno=%d: %s\n", errno, strerror(errno));
	   }
	   else
	   {
	        printf("Compass write register:MODE_REG\n");
	   }*/
	data->x_value = (double)read_out(fd, X_MSB_REG, X_LSB_REG);
	data->y_value = (double)read_out(fd, Y_MSB_REG, Y_LSB_REG);
	data->z_value = (double)read_out(fd, Z_MSB_REG, Z_LSB_REG);
	return 0;
}

//10個の配列の中の数値を昇順にsort
static int compass_sort(double *list)
{
	int i,j;
	double tmp_value = 0;
	for (i=0; i<10; i++)
	{
		for (j=i+1; j<10; j++)
		{
			if (list[i] > list[j])
			{
				tmp_value = list[i];
				list[i] = list[j];
				list[j] = tmp_value;
			}
		}
	}
	return 0;
}

//10個の配列の値のうちmaxとmin(左端と右端)以外の8つの平均値を計算
static double get_compass_average(double *list)
{
	int i;
	double sum = 0;
	for(i=1; i<9; i++)
	{
		sum += list[i];
	}
	return sum/8;
}

//Cmps構造体に地磁気データ10個中左端右端の2個以外の８個の平均を格納
int compass_mean(Cmps *data)
{
	int i;
	double compass_xlist[10];
	double compass_ylist[10];
	for(i=0; i<10; i++)
	{
		compass_mode_change();
		compass_read(data);
		compass_xlist[i] = data->x_value;
		compass_ylist[i] = data->y_value;
	}
	compass_sort(compass_xlist);
	compass_sort(compass_ylist);
	data->x_value = get_compass_average(compass_xlist);
	data->y_value = get_compass_average(compass_ylist);
	return 0;
}

int handle_compass_error(Cmps *data)//地磁気が-1になった時に使う
{
	compass_initialize();
	printf("compass reinitialized\n");
	compass_mode_change();
	compass_mean(data);
	printf("\n");
	return 0;
}
int handle_compass_error_two(Cmps *data)//地磁気が-4096になった時使う　モーター回して近くの磁場を一応避ける
{
	compass_initialize();
	printf("compass reinitialized\n");
	compass_mode_change();
	motor_forward(MAX_PWM_VAL);
	delay(ESCAPE_TIME);
	compass_mean(data);
	printf("\n");
	return 0;
}
int compass_value_initialize(Cmps *compass_init)
{
	compass_init->x_value = 0;
	compass_init->y_value = 0;
	compass_init->z_value = 0;
	return 0;
}

//偏角を考慮を考慮して計算
static double cal_deviated_angle(double theta_degree)
{
	double true_theta = 0;
	true_theta = theta_degree + ANGLE_OF_DEVIATION;
	if (true_theta > 360)
	{
		true_theta = true_theta - 360;
	}
	else if(true_theta<0)
	{
		true_theta = true_theta+ 360;
	}
	else
	{
		true_theta = true_theta;
	}
	return true_theta;
}


//地磁気のxy座標から方角を計算
double calc_compass_angle(double x,double y)
{
	double cal_theta = 0;
	cal_theta = atan2(-y*K_PARAMETER,x)*(180/PI);
	if(cal_theta  < -90)  //詳しい計算方法はkndまで
	{
		cal_theta = -cal_theta - 90;
	}
	else
	{
		cal_theta = 270 - cal_theta;
	}
	return cal_deviated_angle(cal_theta);
}


//6軸を用いた方角の計算
double cal_deg_acclcompass(double x, double y,double z,
                           double sin_phi, double sin_psi,
                           double cos_phi, double cos_psi)
{
	double y1 = 0;//y1~x3は見やすさと計算のために用意した物理的に意味はない変数
	double y2 = 0;
	double x1 = 0;
	double x2 = 0;
	double x3 = 0;
	double cal_theta = 0;
	y1 = z*sin_phi;
	y2 = y*cos_phi;
	x1 = x*cos_psi;
	x2 = y*sin_psi*sin_phi;
	x3 = z*sin_psi*cos_phi;
	cal_theta = atan2((y1 - y2)*K_PARAMETER,x1 + x2 + x3)*(180.0/PI);
	if(cal_theta  < -90)  //詳しい計算方法はkndまで
	{
		cal_theta = -cal_theta - 90;
	}
	else
	{
		cal_theta = 270 - cal_theta;
	}
	return cal_deviated_angle(cal_theta);
}

/*******************************************/
/***以下はマシンによる自動地磁気calibration用****/
/*******************************************/
static int compass_offset_initialize(Cmps_offset *compass_offset, Cmps *compass_data)
{
	compass_value_initialize(compass_data);
	compass_read(compass_data);
	compass_offset->compassx_offset_max = compass_data->x_value;
	compass_offset->compassx_offset_min = compass_data->x_value;
	compass_offset->compassy_offset_max = compass_data->y_value;
	compass_offset->compassy_offset_min = compass_data->y_value;
	compass_offset->compassx_offset = 0;
	compass_offset->compassy_offset = 0;
	return 0;
}

static int maxmin_compass(Cmps_offset *compass_offset, Cmps *compass_data)
{
	if(compass_data->x_value > compass_offset->compassx_offset_max)
	{
		compass_offset->compassx_offset_max = compass_data->x_value;
	}
	else if(compass_data->x_value < compass_offset->compassx_offset_min)
	{
		compass_offset->compassx_offset_min = compass_data->x_value;
	}

	if(compass_data->y_value > compass_offset->compassy_offset_max)
	{
		compass_offset->compassy_offset_max = compass_data->y_value;
	}
	else if(compass_data->y_value < compass_offset->compassy_offset_min)
	{
		compass_offset->compassy_offset_min = compass_data->y_value;
	}
	return 0;
}

static int mean_compass_offset(Cmps_offset *compass_offset)
{
	compass_offset->compassx_offset = (compass_offset->compassx_offset_max + compass_offset->compassx_offset_min)/2;
	compass_offset->compassy_offset = (compass_offset->compassy_offset_max + compass_offset->compassy_offset_min)/2;
	printf("x_offset=%f, y_offset=%f\n", compass_offset->compassx_offset
	       ,compass_offset->compassy_offset);
	return 0;
}

static int rotate_to_calib(Cmps *compass_data)
{
	compass_value_initialize(compass_data);
	motor_right(TURN_CALIB_POWER);
	delay(TURN_CALIB_MILLISECONDS);
	motor_stop();
	delay(2000);
	compass_read(compass_data);
	printf( "compass_x= %f, compass_y= %f\n",compass_data->x_value
	        ,compass_data->y_value);
	delay(50);
	return 0;
}

int cal_maxmin_compass(Cmps_offset *compass_offset,Cmps *compass_data)
{
	int i = 0;
	compass_offset_initialize(compass_offset,compass_data);
	for(i = 0; i<75; i++)
	{
		rotate_to_calib(compass_data);
		maxmin_compass(compass_offset,compass_data);
	}
	mean_compass_offset(compass_offset);
	return 0;
}
