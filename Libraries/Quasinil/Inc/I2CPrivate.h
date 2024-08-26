#ifndef I2C_PRIVATE_H
#define I2C_PRIVATE_H

#include "stm32f0xx.h"
#include "I2C.h"
#include "GPIODef.h"

bool I2CPinSetup( I2C_TypeDef *I2C, I2CPins_t I2CPins );


#endif // I2C_PRIVATE_H
