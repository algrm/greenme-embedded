#ifndef I2C1_H
#define I2C1_H

#include "I2C.h"

// I2C1 Functions 
bool I2C1Setup( I2CPins_t I2CPins, I2CBaudRate_t BaudRate );
void I2C1StartWrite( u8 SlaveAddr, u8 *DataPtr, u8 DataSize  );
bool IsI2C1WriteDone( void );
void I2C1StartRead( u8 SlaveAddr, u8 *DataPtr, u8 NewDataSize );
bool I2C1RxDataReady( void );
void I2C1StartWriteNoStop( u8 SlaveAddr, u8 *DataPtr, u8 DataSize );

#endif //I2C1_H
