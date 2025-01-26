#ifndef _NEO_TRELLIS_LEDS
#define _NEO_TRELLIS_LEDS
#include <stdbool.h>
typedef struct color
{
    unsigned char red;
    unsigned char blue;
    unsigned char green;
}color;

void NeoTrellis_LEDs_Init(void);
void NeoTrellis_LEDs_SetPixel_to_Color(unsigned int index, color color);
void NeoTrellis_LEDs_SetAllLEDs_to(color color);
void NeoTrellis_LEDs_TurnAllLEDs_off(void);
void NeoTrellis_LEDs_TurnLED_off(unsigned int index);
void NeoTrellis_LEDs_Destroy(void);
void NeoTrellis_LEDs_UpdateTrellisBuff (void);
void NeoTrellis_LEDs_RippleEffect(int index, color color);
int NeoTrellis_Keys_getPushedButtonIndex (void);

#endif