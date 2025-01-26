#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "see_saw.h"
#include "../../utils/sleep_milliseconds.h"

#define CONFIG_PIN_COMMAND  "config-pin"
#define I2C2_SCL_PIN        "P9_19"
#define I2C2_SDA_PIN        "P9_20"
#define I2C_MODE            "i2c"

#define I2CDRV_LINUX_BUS2   "/dev/i2c-2"
#define NEO_TRELLIS_ADDRESS 0x2E
#define STATUS_SWRST_REG    0x7F
#define STATUS_RESET_REG    0xFF

static int i2cBus_fd;

static int  initI2CBus(char *bus, unsigned int address);
static int  writeI2C(void *buff, size_t buff_size);
static int  readI2C(void *buff, size_t buff_size);
static void resetAllRegisters(void);

void SeeSaw_Init(void)
{
    // Configure P9_19 / P9_20 as i2c
    char cmdBuf[128];
    sprintf(cmdBuf, "%s %s %s", CONFIG_PIN_COMMAND, I2C2_SDA_PIN, I2C_MODE);
    system(cmdBuf);
    sprintf(cmdBuf, "%s %s %s", CONFIG_PIN_COMMAND, I2C2_SCL_PIN, I2C_MODE);
    system(cmdBuf);

    sleep_ms(500);
    i2cBus_fd = initI2CBus(I2CDRV_LINUX_BUS2, NEO_TRELLIS_ADDRESS);
    resetAllRegisters();
}

int SeeSaw_Write(size_t bytes_num, ...)
{
    va_list args;
    va_start(args, bytes_num);

    unsigned char* array = (unsigned char*)malloc(bytes_num);
    for (size_t i = 0; i < bytes_num; i++) {
        array[i] = (unsigned char)va_arg(args, int);
    }
    va_end(args);

    int bytesWritten = writeI2C(array, bytes_num);
    if (bytesWritten != (int)bytes_num) {
        printf("Error writing i2c: %s\n", strerror(errno));
    }
    free(array);
    return bytesWritten;
}

int SeeSaw_Read(unsigned char module_base_reg, unsigned char module_func_reg,
                unsigned char *buff, size_t buff_size)
{
    SeeSaw_Write(2, module_base_reg, module_func_reg);
    return readI2C(buff, buff_size);
}

unsigned char SeeSaw_byteRead(unsigned char module_base, unsigned char module_func)
{
    unsigned char byteRet[1];
    SeeSaw_Write(2, module_base, module_func);
    readI2C(byteRet, 1);
    return byteRet[0];
}

void SeeSaw_Destroy(void)
{
    close(i2cBus_fd);
}

// Private
static void resetAllRegisters(void)
{
    // Write reset command
    SeeSaw_Write(3, 0x00, STATUS_SWRST_REG, STATUS_RESET_REG);
    sleep_ms(1000);
}

static int initI2CBus(char *bus, unsigned int address)
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

static int writeI2C(void *buff, size_t buff_size)
{
    return write(i2cBus_fd, buff, buff_size);
}

static int readI2C(void *buff, size_t buff_size)
{
    return read(i2cBus_fd, buff, buff_size);
}
