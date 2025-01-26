
/*
 * The module initializes and handles inputs from the Accelerometer device.
 * The client must call Accelerometer_init() before using the module,
 * and call Accelerometer_destroy() once finished to release resources.
 */
#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include <stdbool.h>
#include <stdint.h>

void    Accelerometer_init(void);

/*
 * Returns a raw signed 16-bit integer value from each axis.
 */
int16_t Accelerometer_getX_Value(void);
int16_t Accelerometer_getY_Value(void);
int16_t Accelerometer_getZ_Value(void);

bool    Accelerometer_isNewX_available(void);
bool    Accelerometer_isNewY_available(void);
bool    Accelerometer_isNewZ_availabe(void);

void    Accelerometer_destroy(void);

#endif

