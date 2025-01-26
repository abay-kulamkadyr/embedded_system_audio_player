#include <stdio.h>
#include <stdlib.h>

#include "potentiometer.h"

#define A2D_VOLTAGE0_DIR "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define MAX_PATH_SIZE    1024

static FILE* pot_devFile;

void Potentiometer_Init(void)
{
    char filename[MAX_PATH_SIZE];
    sprintf(filename, "%s", A2D_VOLTAGE0_DIR);
    pot_devFile = fopen(filename, "r");
    if (!pot_devFile) {
        printf("Error opening: %s\n", filename);
        exit(-1);
    }
}

int Potentiometer_getReading(void)
{
    if (!pot_devFile) {
        printf("Potentiometer device file not open.\n");
        exit(-1);
    }
    fseek(pot_devFile, 0, SEEK_SET);
    fflush(pot_devFile);

    int reading = 0;
    if (fscanf(pot_devFile, "%d", &reading) < 0) {
        printf("Error reading from pot.\n");
    }
    return reading;
}

void Potentiometer_Destroy(void)
{
    fclose(pot_devFile);
}
