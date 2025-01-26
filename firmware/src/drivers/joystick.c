#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include "joystick.h"
#include "sleep_milliseconds.h"

#define EXPORT_FILE      "/sys/class/gpio/export"
#define MAX_PATH_SIZE    1024

#define UP_LINUX_NUMBER     26
#define RIGHT_LINUX_NUMBER  47
#define DOWN_LINUX_NUMBER   46
#define LEFT_LINUX_NUMBER   65
#define PUSHED_LINUX_NUMBER 27
#define TOTAL_PINS_NUM      5

static FILE* joystickValue_left_fptr;
static FILE* joystickValue_right_fptr;
static FILE* joystickValue_up_fptr;
static FILE* joystickValue_down_fptr;
static FILE* joystickValue_pushed_fptr;

static int  readGPIOValueFromFile(FILE* file_ptr);
static bool isDirectionPressed(FILE *dir_ptr);

void JoyStick_init(void)
{
    int gpio_pins[TOTAL_PINS_NUM] = {
        UP_LINUX_NUMBER,
        DOWN_LINUX_NUMBER,
        LEFT_LINUX_NUMBER,
        RIGHT_LINUX_NUMBER,
        PUSHED_LINUX_NUMBER
    };

    // Export pins
    for (int i = 0; i < TOTAL_PINS_NUM; i++) {
        FILE* pFile = fopen(EXPORT_FILE, "w");
        if (!pFile) {
            printf("ERROR: Opening file: %s\n", EXPORT_FILE);
            exit(1);
        }
        fprintf(pFile, "%d", gpio_pins[i]);
        fclose(pFile);
        sleep_ms(200);
    }

    // Set direction to "in"
    for (int i = 0; i < TOTAL_PINS_NUM; i++) {
        char buff[MAX_PATH_SIZE];
        sprintf(buff, "/sys/class/gpio/gpio%d/direction", gpio_pins[i]);
        FILE *pDirectionFile = fopen(buff, "w");
        if (!pDirectionFile) {
            printf("ERROR: Opening direction file: %s\n", buff);
            exit(-1);
        }
        fprintf(pDirectionFile, "in");
        fclose(pDirectionFile);
    }

    // Open file descriptors
    for (int i = 0; i < TOTAL_PINS_NUM; i++) {
        char buff[MAX_PATH_SIZE];
        sprintf(buff, "/sys/class/gpio/gpio%d/value", gpio_pins[i]);
        switch (i) {
            case 0: joystickValue_up_fptr    = fopen(buff, "r"); break;
            case 1: joystickValue_down_fptr  = fopen(buff, "r"); break;
            case 2: joystickValue_left_fptr  = fopen(buff, "r"); break;
            case 3: joystickValue_right_fptr = fopen(buff, "r"); break;
            case 4: joystickValue_pushed_fptr= fopen(buff, "r"); break;
        }
    }
}

enum DIRECTION JoyStick_getDirection(void)
{
    if (isDirectionPressed(joystickValue_up_fptr))    return DIRECTION_UP;
    if (isDirectionPressed(joystickValue_down_fptr))  return DIRECTION_DOWN;
    if (isDirectionPressed(joystickValue_left_fptr))  return DIRECTION_LEFT;
    if (isDirectionPressed(joystickValue_right_fptr)) return DIRECTION_RIGHT;
    if (isDirectionPressed(joystickValue_pushed_fptr))return DIRECTION_PUSHED;
    return DIRECTION_NONE;
}

void JoyStick_destroy(void)
{
    fclose(joystickValue_left_fptr);
    fclose(joystickValue_right_fptr);
    fclose(joystickValue_up_fptr);
    fclose(joystickValue_down_fptr);
    fclose(joystickValue_pushed_fptr);
}

// Private
static bool isDirectionPressed(FILE *file_ptr)
{
    return (readGPIOValueFromFile(file_ptr) == 0);
}

static int readGPIOValueFromFile(FILE* file_ptr)
{
    assert(file_ptr);
    fseek(file_ptr, 0, SEEK_SET);
    fflush(file_ptr);
    int gpioReading = 0;
    int itemsRead = fscanf(file_ptr, "%d", &gpioReading);
    if (itemsRead < 0) {
        printf("ERROR: Reading GPIO\n");
    }
    return gpioReading;
}

