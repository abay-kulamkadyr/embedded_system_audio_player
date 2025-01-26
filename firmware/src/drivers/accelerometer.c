
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "accelerometer.h"

#define I2C_ACCELEROMETER_ADDRESS  0x1C
#define CTRL_REG1                  0x2A
#define STAT_REG                   0x00
#define I2CDRV_LINUX_BUS1          "/dev/i2c-1"
#define BYTES_TO_READ              7

#define REG_XMSB 1
#define REG_XLSB 2
#define REG_YMSB 3
#define REG_YLSB 4
#define REG_ZMSB 5
#define REG_ZLSB 6

static int i2cFileDesc;

// Forward declarations
static int  initI2CBus(char *bus, int address);
static void writeI2CReg(unsigned char regAddr, unsigned char value);
static char* readI2CReg(unsigned char regAddr);

void Accelerometer_init(void)
{
    i2cFileDesc = initI2CBus(I2CDRV_LINUX_BUS1, I2C_ACCELEROMETER_ADDRESS);
    // Put accelerometer in ACTIVE mode
    writeI2CReg(CTRL_REG1, 0x01);
}

int16_t Accelerometer_getX_Value(void)
{
    char* data = readI2CReg(STAT_REG);
    int16_t value = (data[REG_XMSB] << 8) | data[REG_XLSB];
    free(data);
    return value;
}

int16_t Accelerometer_getY_Value(void)
{
    char* data = readI2CReg(STAT_REG);
    int16_t value = (data[REG_YMSB] << 8) | data[REG_YLSB];
    free(data);
    return value;
}

int16_t Accelerometer_getZ_Value(void)
{
    char* data = readI2CReg(STAT_REG);
    int16_t value = (data[REG_ZMSB] << 8) | data[REG_ZLSB];
    free(data);
    return value;
}

bool Accelerometer_isNewX_available(void) { return false; }
bool Accelerometer_isNewY_available(void) { return false; }
bool Accelerometer_isNewZ_availabe(void)  { return false; }

void Accelerometer_destroy(void)
{
    close(i2cFileDesc);
}

// Private helpers
static int initI2CBus(char *bus, int address)
{
    int fd = open(bus, O_RDWR);
    if (fd < 0) {
        perror("I2C: Unable to open bus");
        exit(1);
    }
    int result = ioctl(fd, I2C_SLAVE, address);
    if (result < 0) {
        perror("I2C: Unable to set slave address");
        exit(1);
    }
    return fd;
}

static void writeI2CReg(unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2] = {regAddr, value};
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write register");
        exit(-1);
    }
}

static char* readI2CReg(unsigned char regAddr)
{
    int res = write(i2cFileDesc, &regAddr, sizeof(regAddr));
    if (res != sizeof(regAddr)) {
        perror("I2C: Write failed");
        exit(1);
    }
    char* value = (char*)malloc(BYTES_TO_READ);
    res = read(i2cFileDesc, value, BYTES_TO_READ);
    if (res != BYTES_TO_READ) {
        perror("I2C: Read failed");
        exit(1);
    }
    return value;
}

