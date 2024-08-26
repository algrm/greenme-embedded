#ifndef I2C_EXT_SENSORS_H
#define I2C_EXT_SENSORS_H

#include "StdTypes.h"

typedef struct
	{
	u8 readError;
	u32 pm10x100;
	u32 pm2_5x100;
	u32 pm1x100;
	} pmReadResult_t;
	
void ParticleTask( void );
void CO2Task( void );
u16 getCO2ppm( void );
bool isExtSensorMounted( void ); 

pmReadResult_t* GetPmValues(void);


#endif // I2C_EXT_SENSORS_H
