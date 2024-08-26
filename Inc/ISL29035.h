#ifndef ISL29035_H
#define ISL29035_H

#include "StdTypes.h"

u16 ISL29035Lux( void );														// Filtered Lux value
void LuxCfg( void );															// Configures Luxmeter sensor
bool LuxTask( void );															// Configures, and updates Lux sensor value

#endif // ISL29035_H

