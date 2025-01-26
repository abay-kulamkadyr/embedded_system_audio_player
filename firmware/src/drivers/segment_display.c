
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <string.h>

#include "segment_display.h"

#define I2CDRV_LINUX_BUS1 "/dev/i2c-1"
#define I2C_DEVICE_ADDRESS 0x20
// ... original definitions remain ...

static int i2cFileDesc;

// Prototypes
static int initI2cBus(char* bus, int address);
static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value);

void DisplayP(char* disp)
{
    // Same logic as your original code
    // ...
}

void DisplayProgressOnSeg(int cur, int max)
{
    // Same logic as your original code
    // ...
}

void InitializeSegDisplay(void)
{
    i2cFileDesc = initI2cBus(I2CDRV_LINUX_BUS1, I2C_DEVICE_ADDRESS);
    // Your export logic, direction files, etc.
    // ...
    close(i2cFileDesc);
}

// Private
static int initI2cBus(char* bus, int address)
{
    int fd = open(bus, O_RDWR);
    if (fd < 0) {
        perror("I2C: Unable to open bus");
        exit(1);
    }
    if (ioctl(fd, I2C_SLAVE, address) < 0) {
        perror("I2C: Unable to set slave address");
        exit(1);
    }
    return fd;
}

static void writeI2cReg(int i2cFileDesc, unsigned char regAddr, unsigned char value)
{
    unsigned char buff[2] = {regAddr, value};
    int res = write(i2cFileDesc, buff, 2);
    if (res != 2) {
        perror("I2C: Unable to write i2c register");
        exit(1);
    }
}

