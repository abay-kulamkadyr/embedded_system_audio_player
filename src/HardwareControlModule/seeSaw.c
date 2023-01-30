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

#include "seeSaw.h"
#include "../Utils/sleepMilliseconds.h"
int i2cBus_fd;
#define CONFIG_PIN_COMMAND      "config-pin"
#define I2C2_SCL_PIN            "P9_19"
#define I2C2_SDA_PIN            "P9_20"
#define I2C_MODE                "i2c"

#define I2CDRV_LINUX_BUS1       "/dev/i2c-2"
#define NEO_TRELLIS_ADDRESS     0x2e
#define STATUS_BASE_REG         0x00
#define STATUS_RESET_REG        0xff
#define STATUS_SWRST_REG        0x7f
typedef struct bytearray{
    unsigned char* buff;
    size_t buff_size;
}bytearray;
/******************************************************
 * FUNCTION PROTOTYPES
 ******************************************************/
static int initI2CBus(char *bus,unsigned int address);
static int readI2C(void * buff, size_t buff_size);
static int writeI2C(void *buff, size_t buff_size);


static void resetAllRegisters(void);
static bytearray* buildByteArray(va_list args, size_t n);
static void freeByteArray(bytearray* byteArray);
/*
 *Constructs an array of bytes from passed arguments
 ******************************************************************/
static bytearray* buildByteArray(va_list args, size_t n) {
    bytearray* barr = (bytearray*)malloc(sizeof(bytearray));
    barr->buff = (unsigned char*)malloc(sizeof(unsigned char) * n);
    barr->buff_size = n;

    for (int i = 0; i < n; i++) {
        barr->buff[i] = va_arg(args, unsigned int);
    }
    return barr;
}
void freeByteArray(bytearray* byteArray)
{
    free(byteArray->buff);
    free(byteArray);
}
 //*****************************************************************/

int initI2CBus(char *bus,unsigned int address)
{
    int i2cFileDesc= open(bus, O_RDWR);
    int result= ioctl(i2cFileDesc,I2C_SLAVE, address);
    if(result<0) {
        perror("I2C: Unable to set I2C device to salve address");
        exit(1);
    }
    return i2cFileDesc;
}
static int writeI2C(void *buff, size_t buff_size)
{
    int bytesWritten = write(i2cBus_fd, buff, buff_size);
    return bytesWritten;
}
static int readI2C(void * buff, size_t buff_size)
{
    int bytesRead = read(i2cBus_fd, buff, buff_size);
    return bytesRead;
}

static void resetAllRegisters(void)
{
    SeeSaw_Write(3, STATUS_BASE_REG,STATUS_SWRST_REG, STATUS_RESET_REG);
    sleep_ms(1000);
}

/******************************************************
 * APIs
 ******************************************************/
void SeeSaw_Init(void)
{
    //Configure P19 and P20 headers to be in i2c mode
    system(CONFIG_PIN_COMMAND" "I2C2_SDA_PIN" "I2C_MODE);
    system(CONFIG_PIN_COMMAND" "I2C2_SCL_PIN" "I2C_MODE);
    sleep_ms(500);
    //open i2c-2 bus
    i2cBus_fd = initI2CBus(I2CDRV_LINUX_BUS1, NEO_TRELLIS_ADDRESS);
    resetAllRegisters();
}

int SeeSaw_Write(size_t bytes_num, ...)
{
    va_list args;
    va_start(args, bytes_num);
    bytearray* array = buildByteArray(args, bytes_num);
    va_end(args);
    int bytesWritten = writeI2C(array->buff, array->buff_size);
    if (bytesWritten != bytes_num) {
        printf("Error during i2cWrite: %s\n", strerror(errno));
    }
    freeByteArray(array);
    return bytesWritten; 
}
int SeeSaw_Read(unsigned char module_base_reg,unsigned char module_func_reg, unsigned char *buff, size_t buff_size)
{
    //before initiating a read, we need to write to base address followed by function register first
    SeeSaw_Write(2, module_base_reg, module_func_reg);
    int bytesRead = readI2C(buff, buff_size);
    return bytesRead;
}
unsigned char SeeSaw_byteRead (unsigned char module_base, unsigned char module_func)
{
    unsigned char byteToReturn[1];
    
    SeeSaw_Write(2, module_base, module_func);
    
    readI2C(&byteToReturn, 1);

    return byteToReturn[0];
}
void SeeSaw_Destroy(void)
{
    close(i2cBus_fd);
}