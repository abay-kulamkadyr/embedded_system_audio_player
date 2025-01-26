#include <stdio.h>
#include <stdlib.h>
#include "../../Include/HardwareControlModule/potentiometer.h"
#define A2D_VOLTAGE0_DIR "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define MAX_PATH_SIZE 1024

static FILE* pot_devFile;
void Potentiometer_Init()
{
    char pot_filename[MAX_PATH_SIZE];
    sprintf(pot_filename, A2D_VOLTAGE0_DIR);

    pot_devFile= fopen(pot_filename, "r");
    if (pot_devFile==NULL)
    {
        printf("FILEOPEN ERROR: Unable to open file for read: %s\n", pot_filename);
        exit(-1);
    }
}
int Potentiometer_getReading()
{
    if(pot_devFile==NULL)
    {
        printf("Voltage0 (potentiometer) device file is not open\n");
        exit(-1);
    }
    //moving the file pointer to the beginning and flushing the file pointer
    fseek(pot_devFile, 0, SEEK_SET);
    fflush(pot_devFile);
    int a2dReading=0;
    int itemsRead=fscanf(pot_devFile, "%d", &a2dReading);
    if(itemsRead<0){
        printf("ERROR: Unable to read values from voltage input file.\n");
    }
    return a2dReading;
}
void Potentiometer_Destroy()
{
    fclose(pot_devFile);
}