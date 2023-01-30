#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "accelerometer.h"

#define I2C_ACCELEROMETER_ADDRESS  0x1C

#define CTRL_REG1           0x2A
#define STAT_REG            0x00    
#define I2CDRV_LINUX_BUS1   "/dev/i2c-1"
#define BYTES_TO_READ       7
#define REG_XMSB            1     
#define REG_XLSB            2
#define REG_YMSB            3
#define REG_YLSB            4
#define REG_ZMSB            5
#define REG_ZLSB            6
/*
    Helper function prototypes
*/
static void writeI2CReg(unsigned char regAddr, unsigned char value);
static int initI2CBus(char *bus, int address);
static char* readI2CReg(unsigned char regAddr);

//File descriptor of the slave i2c device (Accelerometer)
static int i2cFileDesc;

void Accelerometer_init(void)
{
   //Opening the Accelerometer device   
   i2cFileDesc= initI2CBus(I2CDRV_LINUX_BUS1, I2C_ACCELEROMETER_ADDRESS);
   
   /*
    Drive 0x01 to CTRL_REG1 
    to set the mode of accelerometer to ACTIVE 
    so we can read acceleration values from the X-Y-Z axis 
   */
   writeI2CReg(CTRL_REG1, 0x01);

}
int16_t Accelerometer_getX_Value(void)
{
    /*
    Starting from the STAT_REG, i.e. register 0x00 
    of the accelerometer, read the first 7 bytes
    REG_XMSB is the index in the byte array,
    which indicates the 12 MSBs of the X-axis and
    REG_XLSB is the index which indicates 4 LSBs of the X-axis 
    */
    char* readbytes=readI2CReg(STAT_REG);
    int16_t value =(readbytes[REG_XMSB]<<8 | readbytes[REG_XLSB]);
    free(readbytes);
    return value;

}
int16_t Accelerometer_getY_Value(void)
{
    /*
    Starting from the STAT_REG, i.e. register 0x00 
    of the accelerometer, read the first 7 bytes 
    REG_YMSB is the index in the byte array,
    which indicates the 12 MSBs of the Y-axis and
    REG_YLSB is the index which indicates 4 LSBs of the Y-axis 
    */
    char* readbytes=readI2CReg(STAT_REG);
    int16_t value =(readbytes[REG_YMSB]<<8 | readbytes[REG_YLSB]);
    free(readbytes);
    return value;
}
int16_t Accelerometer_getZ_Value(void)
{
    /*
    Starting from the STAT_REG, i.e. register 0x00 
    of the accelerometer, read the first 7 bytes 
    REG_ZMSB is the index in the byte array,
    which indicates the 12 MSBs of the Z-axis and
    REG_ZLSB is the index which indicates 4 LSBs of the Z-axis 
    */
    char* readbytes=readI2CReg(STAT_REG);
    
    int16_t value =(readbytes[REG_ZMSB]<<8 | readbytes[REG_ZLSB]);
    free(readbytes);
    return value;
}
bool Accelerometer_isNewX_available(void)
{
    return false;
}
bool Accelerometer_isNewY_available(void)
{
    return false;
}
bool Acceleremeter_isNewZ_availabe(void)
{
    return false;
}
void Accelerometer_destroy(void)
{
    close(i2cFileDesc);
}


int initI2CBus(char *bus, int address)
{
    int i2cFileDesc= open(bus, O_RDWR);
    int result= ioctl(i2cFileDesc,I2C_SLAVE, address);
    if(result<0){
        perror("I2C: Unable to set I2C device to salve address");
        exit(1);
    }
    return i2cFileDesc;
}
/*
Reads BYTES_TO_READ number of bytes starting from regAddr
and returns an array of chars of the read bytes
*/
char* readI2CReg(unsigned char regAddr)
{
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    
    if (res != sizeof(regAddr)) {
        perror("I2C: Unable to write to i2c register.");
        exit(1);
    }
    // Now read 2 bytes from the register
    char* value= (char*) malloc (sizeof(char)* BYTES_TO_READ);

    res = read(i2cFileDesc, value, BYTES_TO_READ);
    if (res != BYTES_TO_READ) {
        perror("I2C: Unable to read from i2c register");
        exit(1);
    }
    return value;
}
void writeI2CReg(unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2];
    buff[0]= regAddr;
    buff[1]=value;
    int res= write(i2cFileDesc, buff, 2);
    if(res!=2){
        perror("I2C: Unable to write i2c register.");
        exit(-1);
    }

}