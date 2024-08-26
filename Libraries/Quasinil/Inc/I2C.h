#ifndef I2C_H
#define I2C_H

#include "StdTypes.h"

typedef enum { I2C_A9A10, I2C_B6B7, I2C_B8B9, I2C_B10B11, NB_I2C_PIN_CFG	}I2CPins_t;
typedef enum { I2C_IDLE, I2C_TRANSMITTING, I2C_RECEIVING, I2C_ERROR, I2C_NB_STATUS }I2CStatus_t;
typedef enum { I2C_BAUDRATE_100K=0x20420F13 }I2CBaudRate_t;
#define I2C_TIMING_100K          0x20420F13
// 31:28 PRESC		Timing Prescaler		(3-1)	2	=> I2CCLK = 4MHz = 12MHz/3 
// 27:24 Reserved							0		0
// 23:20 SCLDEL		Data Setup Time			(5-1)	4		
// 19:16 SDADEL		Data Hold time			(3-1)	2
// 15:8	 SCLH		Scl High period			(16-1)	0F	=> SCLH = 16/4MHz = 4us
//  7:0	 SCLL		Scl Low Period			(20-1)	13	=> SCLL = 20/4MHz = 5us

#endif
