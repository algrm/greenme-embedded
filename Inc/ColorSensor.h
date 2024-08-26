#ifndef COLOR_SENSOR_H
#define COLOR_SENSOR_H

#include <StdTypes.h>

typedef struct 
{
u8 Red;																			// Red ratio of white light 255:99.6%
u8 Green;																		// Green ratio of white light 255:99.6%
u8 Blue;																		// Blue ratio of white light 255:99.6%
u8 Unused;																		// Unused ( for 32 bit operations )
u16 White;	
}Color_t;

void ColorSensorSetup( void );													// Configures color sensor and starts measurement
bool RgbTask( void );
void RgbCfg( void );
Color_t GetColors( void );														// returns last color measurements

#endif // COLOR_SENSOR
