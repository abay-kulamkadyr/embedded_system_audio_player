#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include "joystick.h"
#include "../Utils/sleepMilliseconds.h"
#define EXPORT_FILE             "/sys/class/gpio/export"
#define MAX_PATH_SIZE           1024
#define UP_LINUX_NUMBER         26
#define RIGHT_LINUX_NUMBER      47
#define DOWN_LINUX_NUMBER       46
#define LEFT_LINUX_NUMBER       65
#define PUSHED_LINUX_NUMBER     27 
#define TOTAL_PINS_NUM          5

/*
File decriptors of the values for
each direction of the Joystick
*/
static FILE* joystickValue_left_fptr;
static FILE* joystickValue_right_fptr;
static FILE* joystickValue_up_fptr;
static FILE* joystickValue_down_fptr;
static FILE* joystickValue_pushed_fptr;

//Function prototypes
static int readGPIOValueFromFile(FILE* file_ptr);
static bool isDirectionPressed(FILE *dir_ptr);
void JoyStick_init()
{
    //Exporting pins
    int gpio_pins[TOTAL_PINS_NUM] = { 
        UP_LINUX_NUMBER, 
        DOWN_LINUX_NUMBER,
        LEFT_LINUX_NUMBER,
        RIGHT_LINUX_NUMBER,
        PUSHED_LINUX_NUMBER
    };
    // Use fopen() to open the file for write access.
    for (int i =0;i < 5;i++){

        FILE *pFile = fopen(EXPORT_FILE, "w");
        if (pFile == NULL) {
            printf("ERROR OPENING FILE: %s\n", EXPORT_FILE);
            exit(1);
        }
        fprintf(pFile, "%d", gpio_pins[i]);

        // Allow enough time for GPIO to complete exporting
        sleep_ms(500);
        // Close the file using fclose():
        fclose(pFile);
    }

    // setting up the pin direction for each of the pins to be output
    for(int i=0;i<TOTAL_PINS_NUM;i++)
    {
        char buff[MAX_PATH_SIZE];
        sprintf(buff, "/sys/class/gpio/gpio%d/direction", gpio_pins[i]);
        FILE *pDirectionFile= fopen(buff,"w");
        if(!pDirectionFile){
            printf("ERROR OPENING FILE: %s\n", buff);
            exit(-1);
        }
        fprintf(pDirectionFile,"in");
        fclose(pDirectionFile);
    }

    /*
    obtaining file descriptors for each gpio pin value readings
    */
    for(int i=0;i<TOTAL_PINS_NUM;i++){ 

        char buff[MAX_PATH_SIZE];
        sprintf(buff, "/sys/class/gpio/gpio%d/value", gpio_pins[i]);
        switch (i)
        {
        case 0:
            joystickValue_up_fptr= fopen(buff, "r");
            break;
        case 1:
            joystickValue_down_fptr=fopen(buff,"r");
            break;
        case 2:
            joystickValue_left_fptr=fopen(buff,"r");
            break;
        case 3:
            joystickValue_right_fptr=fopen(buff,"r");
            break;
        case 4: 
            joystickValue_pushed_fptr=fopen(buff,"r");
            break;
        }

    }
}
enum DIRECTION JoyStick_getDirection()
{
    return  isDirectionPressed(joystickValue_up_fptr)? DIRECTION_UP:
            isDirectionPressed(joystickValue_down_fptr)? DIRECTION_DOWN:
            isDirectionPressed(joystickValue_left_fptr)? DIRECTION_LEFT:
            isDirectionPressed(joystickValue_right_fptr)? DIRECTION_RIGHT:
            isDirectionPressed(joystickValue_pushed_fptr)? DIRECTION_PUSHED:
            DIRECTION_NONE;
}


/* 
Closes all file descriptors that were opened for each direction of the Joystick
*/
void JoyStick_destroy()
{
    fclose(joystickValue_left_fptr);
    fclose(joystickValue_right_fptr);
    fclose(joystickValue_up_fptr);
    fclose(joystickValue_down_fptr);
    fclose(joystickValue_pushed_fptr);
}

/*
Return true if the file pointed by dir_ptr contains a 0
Otherwise returns false
*/
static bool isDirectionPressed(FILE *file_ptr)
{
    return readGPIOValueFromFile(file_ptr)==0;
}

/*
Reads and returns a value from the file pointed by file_ptr
*/
static int readGPIOValueFromFile(FILE* file_ptr)
{
    assert(file_ptr);
    fseek(file_ptr, 0,SEEK_SET);
    fflush(file_ptr);
    int gpioReading=0;
    int itemsRead= fscanf(file_ptr, "%d", &gpioReading);
    if(itemsRead<0){
        printf("ERROR: UNABLE TO READ VALUES FROM GPIO PIN\n");
    }
    return gpioReading;
}