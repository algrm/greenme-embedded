#ifndef I2C1_H
#define I2C1_H

#include "I2C.h"

// I2C1 Functions 
bool I2C2Setup( I2CPins_t I2CPins, I2CBaudRate_t Baudrate );
void I2C2StartWrite( u8 SlaveAddr, u8 *DataPtr, u8 DataSize );
void I2C2StartRead( u8 SlaveAddr, u8 *DataPtr, u8 *DataSize );
I2CStatus_t I2C2Status( void );

#endif //I2C1_H
