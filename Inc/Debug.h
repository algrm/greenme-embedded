#ifndef DEBUG_H
#define DEBUG_H

#include "stm32f0xx.h"
#include "GpioDef.h"

/*==============================================================================
	DEFINITIONS
===============================================================================*/
#define DEBUG1_GPIO			GPIOA									
#define DEBUG1_PIN			GPIO_Pin_11

#define DEBUG2_GPIO			GPIOA									
#define DEBUG2_PIN			GPIO_Pin_12

/*==============================================================================
	MACROS
===============================================================================*/
#define DEBUG1_ON()				DEBUG1_GPIO->BSRR = DEBUG1_PIN;
#define DEBUG1_OFF()			DEBUG1_GPIO->BRR = DEBUG1_PIN;
#define DEBUG1_TOGGLE()			{if ( (Bit_RESET == (DEBUG1_GPIO->ODR & DEBUG1_PIN)) ) {DEBUG1_GPIO->BSRR=DEBUG1_PIN;} else {DEBUG1_GPIO->BRR=DEBUG1_PIN;}}

#define DEBUG2_ON()				DEBUG2_GPIO->BSRR = DEBUG2_PIN;
#define DEBUG2_OFF()			DEBUG2_GPIO->BRR = DEBUG2_PIN;
#define DEBUG2_TOGGLE()			{if ( (Bit_RESET == (DEBUG2_GPIO->ODR & DEBUG2_PIN)) ) {DEBUG2_GPIO->BSRR=DEBUG2_PIN;} else {DEBUG2_GPIO->BRR=DEBUG2_PIN;}}

/*==============================================================================
	FUNCTION PROTOTYPES
===============================================================================*/
extern void DebugPinsSetup( void );												// Sets up all debug pins
void DebugTimStart( void );
void DebugTimEnd( u8 StampVal );
void DebugTimStamp( u8 StampVal );
void DebugTimReset( void );
bool IsDebugAcqBufFull( void );

#endif	// DEBUG_H
