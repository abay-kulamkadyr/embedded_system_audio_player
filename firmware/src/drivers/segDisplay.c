#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <string.h>
#include "../../Include/HardwareControlModule/segDisplay.h"

#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"

#define I2C_DEVICE_ADDRESS 0x20
#define REG_DIRA 0x00
#define REG_DIRB 0x01
#define REG_OUTA 0x14
#define REG_OUTB 0x15


#define gpioexportfile "/sys/class/gpio/export"
#define gpio44directionfile "/sys/class/gpio/gpio44/direction"
#define gpio61directionfile "/sys/class/gpio/gpio61/direction"
#define gpio44file "/sys/class/gpio/gpio44/value"
#define gpio61file "/sys/class/gpio/gpio61/value"


static int i2cFileDesc;

static unsigned bottompause=0X11;
static unsigned toppause=0X8E;

static unsigned char bottom14left[8]={0X00,0X00,0X00,0X00,0X00,0X20,0X21,0X21};
static unsigned char top15left[8]={0x04,0X04,0X04,0X04,0X04,0X04,0X04,0X84};


static unsigned char bottom14right[8]={0X00,0X00,0X00,0X80,0XA0,0XA0,0XA0,0XA0};
static unsigned char top15right[8]={0x00,0X04,0X06,0X06,0X06,0X06,0X06,0X06};


static void writegpio44value(int val);
static void writegpio61value(int val);
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr,
unsigned char value);



void DisplayP(char* disp){
	if (strcmp(disp,"ON")==0){

		writegpio44value(0);
		writegpio61value(0);
		i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1,I2C_DEVICE_ADDRESS);
		writeI2cReg(i2cFileDesc, REG_OUTA, bottompause);
		writeI2cReg(i2cFileDesc, REG_OUTB, toppause);
		writegpio61value(1);
		close(i2cFileDesc);
	}
	else{
		writegpio44value(0);
		writegpio61value(0);
	}
	
}




void DisplayProgressOnSeg(int cur,int max){
	struct timespec reqDelay={0,4700000};
	i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, 	I2C_DEVICE_ADDRESS);
	float progress=((float)cur/max)*100;
	int pos;
	if (progress<14){
		pos=0;
	}
	else if (progress>=14 && progress<28){
		pos=1;
	}
	else if (progress>=28 && progress<42){
		pos=2;
	}
	else if (progress>=42 && progress<56){
		pos=3;
	}
	else if (progress>=56 && progress<70){
		pos=4;
	}
	else if (progress>=70 && progress<84){
		pos=5;
	}
	else if (progress>=84 && progress<98){
		pos=6;
	}
	else{
		pos=7;
		
	}
	
	
	writegpio61value(0);
	writegpio44value(0);
		
	writeI2cReg(i2cFileDesc, REG_OUTA, bottom14left[pos]);
	writeI2cReg(i2cFileDesc, REG_OUTB, top15left[pos]);
	writegpio61value(1);
	nanosleep(&reqDelay,(struct timespec*) NULL);
	writegpio61value(0);
	writegpio44value(0);
	writeI2cReg(i2cFileDesc, REG_OUTA, bottom14right[pos]);
	writeI2cReg(i2cFileDesc, REG_OUTB, top15right[pos]);
	writegpio44value(1);
	nanosleep(&reqDelay,(struct timespec*) NULL);
	
	
	close(i2cFileDesc);
	return;
}


static void writegpio61value(int val){
	FILE* filew=fopen(gpio61file,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpio61file);
		exit(1);
	}
	int iswritten=fprintf(filew,"%d",val);
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	return;
}

static void writegpio44value(int val){
	FILE* filew=fopen(gpio44file,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpio44file);
		exit(1);
	}
	int iswritten=fprintf(filew,"%d",val);
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	return;
}


static int initI2cBus(char* bus, int address){
	int i2cFileDesc = open(bus, O_RDWR);
	int result = ioctl(i2cFileDesc, I2C_SLAVE, address);
	if (result < 0) {
		perror("I2C: Unable to set I2C device to slave address.");
		exit(1);
	}
	return i2cFileDesc;
}


static void writeI2cReg(int i2cFileDesc, unsigned char regAddr,
unsigned char value){
	unsigned char buff[2];
	buff[0] = regAddr;
	buff[1] = value;
	int res = write(i2cFileDesc, buff, 2);
	if (res != 2) {
		perror("I2C: Unable to write i2c register.");
		exit(1);
	}
}

void InitializeSegDisplay(void){
	struct timespec reqDelay={0,400000000};
	i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1,I2C_DEVICE_ADDRESS);
	
	FILE* filew;
	int iswritten;
	filew=fopen(gpioexportfile,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpioexportfile);
		exit(1);
	}
	iswritten=fprintf(filew,"%d",61);
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	
	filew=fopen(gpioexportfile,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpioexportfile);
		exit(1);
	}
	iswritten=fprintf(filew,"%d",44);
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	nanosleep(&reqDelay, (struct timespec *) NULL);
	filew=fopen(gpio44directionfile,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpio44directionfile);
		exit(1);
	}
	iswritten=fprintf(filew,"out");
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	
	filew=fopen(gpio61directionfile,"w");
	if (filew==NULL){
		printf("Error opening %s\n",gpio61directionfile);
		exit(1);
	}
	iswritten=fprintf(filew,"out");
	if (iswritten<=0){
		printf("Error writing data\n");
		exit(1);
	}
	fclose(filew);
	
	writeI2cReg(i2cFileDesc, REG_DIRA, 0x00);
	writeI2cReg(i2cFileDesc, REG_DIRB, 0x00);
	close(i2cFileDesc);
}
