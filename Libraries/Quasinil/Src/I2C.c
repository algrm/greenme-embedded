#include "GPIODef.h"
#include "I2C.h"
#include "I2CPrivate.h"
#include "string.h"

/*=============================================================================
	I2C PIN SELECTION
=============================================================================*/
#define STM32F030x8

#ifdef STM32F030x8
static const GPIODef_t SclPin[NB_I2C_PIN_CFG] = 	{
	{ PA9, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB6, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB8, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB10, TYPE_I2C2,OUT_OD, SPEED_MEDIUM, INIT_OPEN }
												};
static const GPIODef_t SdaPin[NB_I2C_PIN_CFG] = 	{
	{ PA10, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB7,  TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB9,  TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB11, TYPE_I2C2, OUT_OD, SPEED_MEDIUM, INIT_OPEN }
												};
#else // STM32F030x4 x6
static const GPIODef_t SclPin[NB_I2C_PIN_CFG] = 	{
	{ PA9, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB6, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB8, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB10, TYPE_I2C2,OUT_OD, SPEED_MEDIUM, INIT_OPEN }
												};
static const GPIODef_t SdaPin[NB_I2C_PIN_CFG] = 	{
	{ PA10, TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB7,  TYPE_I2C1, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB9,  TYPE_I2C1
	, OUT_OD, SPEED_MEDIUM, INIT_OPEN },
	{ PB11, TYPE_I2C2, OUT_OD, SPEED_MEDIUM, INIT_OPEN }
												};
#endif	// STM32F030x8

/*=============================================================================
	I2C Pin Setup
	
	Temporarily configures I2C pins as logic I/Os
	Detects and tries to recdover from an I2C deadlock state
	Configures I2C pins for I2C operation
	
=============================================================================*/
bool I2CPinSetup( I2C_TypeDef *I2C, I2CPins_t I2CPins )
{
u32 i;
GPIODef_t OutSclPin;// = { PA9, TYPE_OUTPUT, OUT_OD, SPEED_MEDIUM, INIT_OPEN };
GPIODef_t InputSdaPin;// = { PA10, TYPE_LOGIC_INPUT, OUT_OD, SPEED_MEDIUM, INIT_OPEN },

// I2C EEPROM Deadlock recovery
// This is to recover from a deadlock due to I2C communication interruption by a reset for example
memcpy( &OutSclPin, &SclPin[I2CPins], sizeof(OutSclPin) );
OutSclPin.Type = TYPE_OUTPUT;
GpioSetup( &OutSclPin );
memcpy( &InputSdaPin, &SdaPin[I2CPins], sizeof(InputSdaPin) );
InputSdaPin.Type = TYPE_LOGIC_INPUT;
GpioSetup( &InputSdaPin );

// I2C Deadlock recovery
for ( i = 0; IsInputOff(&InputSdaPin) && i<10; i++ )
	{
	OutputReset( &SclPin[I2CPins] );
	OutputSet(   &SclPin[I2CPins] );
	}

// I2C GPIO Setup
GpioSetup( &SclPin[I2CPins] );
GpioSetup( &SdaPin[I2CPins] );

if ( SclPin[I2CPins].Type == TYPE_I2C1 )
	{
	if ( I2C == I2C1 )
		return true;
	}
else
	{
	if ( I2C == I2C2 )
		return true;
	}
return false;
}

