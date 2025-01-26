/*
 * Initializes the Joystick device and handles inputs from it.
 * Client code must call JoyStick_init() before using the module's functions,
 * and call JoyStick_destroy() to free resources.
 */
#ifndef JOYSTICK_H
#define JOYSTICK_H

enum DIRECTION {
    DIRECTION_NONE,
    DIRECTION_UP,
    DIRECTION_DOWN,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_PUSHED,
};

void JoyStick_init(void);
enum DIRECTION JoyStick_getDirection(void);
void JoyStick_destroy(void);

#endif

