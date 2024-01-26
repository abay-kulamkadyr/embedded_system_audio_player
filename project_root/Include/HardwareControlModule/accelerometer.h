/*
The module initializes and handles inputs from 
the Accelerometer device. 
The client must call Accelerometer_init() before 
using the module's functionalities 
and call Accelerometer_destroy() once finished using the module
to release all the resources the module was using 
*/
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H
#include <stdbool.h>
#include <stdint.h>

void    Accelerometer_init(void);
/*
Returns a raw signed 16 bit integer value 
from the X-axis of the accelerometer register
*/
int16_t Accelerometer_getX_Value(void);
/*
Returns a raw signed 16 bit integer value 
from the Y-axis of the accelerometer register
*/
int16_t Accelerometer_getY_Value(void);
/*
Returns a raw signed 16 bit integer value 
from the Z-axis of the accelerometer register
*/
int16_t Accelerometer_getZ_Value(void);
bool    Accelerometer_isNewX_available(void);
bool    Accelerometer_isNewY_available(void);
bool    Accelerometer_isNewZ_availabe(void);

//frees all the resources used by the module
void    Accelerometer_destroy(void);

#endif