#ifndef MAIN_H
#define MAIN_H

#include "StdTypes.h"
#include "I2CStdSensors.h"
#include "I2CExtSensors.h"
#include "ColorSensor.h"
#include "dBAMeter.h"
#include "Radio.h"

/*==============================================================================
	DEFINITIONS
===============================================================================*/
#define NB_CHANNELS		16
#define NB_MAX_MASTERS	32

//#define HAPPY_STATUS_START 6
//#define FEEL_UNKNOWN 	0x00
//#define FEEL_HAPPY 		0x01
//#define FEEL_UNHAPPY 	0x10

typedef struct 
{
	u16 addr;
	u32 serial;
} radioNode_t;

#endif




