#include "Debug.h"
#include "Tim.h"
#include "GPIODef.h"

/*==============================================================================
	DEBUG TIMER DEFINITIONS
===============================================================================*/
#define DEBUG_TIMER_FREQ			1000000										//(Hz) Debug timer clock frequency is 1 us

#define ENABLE_DEBUG_TMR_CLCK();	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM14, ENABLE);
#define DEBUG_TIMER_STOP_STATUS()	DBGMCU_Config( DBGMCU_TIM14_STOP, ENABLE );	// Stop TMR counter when core halted ( breakpoint )
#define DEBUG_TIMER					TIM14

#define CR1_TIM_ENABLE()			{DEBUG_TIMER->CR1 |= 0x0001;}
#define CR1_TIM_DISABLE()			{DEBUG_TIMER->CR1 &= 0xFFFE;}

/*==============================================================================
	Global Data
	
	This is only for debug purposes
==============================================================================*/
u8 DebugAcqListPos=0;
u32 DebugAcqList[16];
u16 DebugMaxDuration=0;

/*==============================================================================
	Private functions
==============================================================================*/
static void DebugTimerSetup( void );

/*==============================================================================
	Debug setup

	Input, Output, Retval : None
	
	Uncomment usefull sections when necessary

	Max Nb Cycles : not critical
==============================================================================*/
void DebugPinsSetup( void )
{
const GPIODef_t DBG1Pin = { PA11, TYPE_OUTPUT, OUT_PP, SPEED_MEDIUM, INIT_0 };
const GPIODef_t DBG2Pin = { PA12, TYPE_OUTPUT, OUT_PP, SPEED_MEDIUM, INIT_0 };
const GPIODef_t DBG3Pin = { PB4 , TYPE_OUTPUT, OUT_PP, SPEED_MEDIUM, INIT_0 };

GpioSetup( &DBG1Pin );
GpioSetup( &DBG2Pin );
GpioSetup( &DBG3Pin );

DebugTimerSetup();
}

/*=============================================================================
	DEBUG TIM START
=============================================================================*/
void DebugTimStart( void )
{
CR1_TIM_ENABLE();
}

/*=============================================================================
	DEBUG TIM End
=============================================================================*/
void DebugTimEnd( u8 StampVal )
{
CR1_TIM_DISABLE();
if ( DEBUG_TIMER->CNT > DebugMaxDuration )
	DebugMaxDuration = DEBUG_TIMER->CNT;
DebugAcqList[DebugAcqListPos++] = StampVal*1000000000 + DEBUG_TIMER->CNT;
DebugAcqListPos &= 0x0F;
DEBUG_TIMER->CNT = 0;
}

/*=============================================================================
	DEBUG TIM End
=============================================================================*/
void DebugTimStamp( u8 StampVal )
{
DebugAcqList[DebugAcqListPos++] = StampVal*1000000000 + DEBUG_TIMER->CNT;
DebugAcqListPos &= 0x0F;
}

/*=============================================================================
	DEBUG TIM Reset
=============================================================================*/
void DebugTimReset( void )
{
DebugTimEnd(0);
for ( DebugAcqListPos=0; DebugAcqListPos<sizeof(DebugAcqList); DebugAcqListPos++ )
	{
	DebugAcqList[DebugAcqListPos] = 0;
	}
DebugAcqListPos = 0;
DebugMaxDuration = 0;
}

/*=============================================================================
	Is Debug Acq List Full
=============================================================================*/
bool IsDebugAcqBufFull( void )
{
static bool Init = false;

if ( DebugAcqListPos == 0 )
	{
	if ( Init )
		return true;
	}
else
	{
	Init = true;
	}
return false;
}

/*=============================================================================
	DEBUG TIMER SETUP
=============================================================================*/
static void DebugTimerSetup( void )
{
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
u32 TimClkFreq = DEBUG_TIMER_FREQ;
u16 TimPrescaler = 1;

// Enable peripheral clocks
ENABLE_DEBUG_TMR_CLCK();

DEBUG_TIMER_STOP_STATUS();

TimPrescaler = ComputeTimPrescaler( DEBUG_TIMER, &TimClkFreq );

TIM_TimeBaseStructure.TIM_Prescaler = TimPrescaler-1;							// Set prescaler for timer resolution
TIM_TimeBaseStructure.TIM_Period = (u16)0xFFFF;									// Set Max period
TIM_TimeBaseStructure.TIM_ClockDivision = 0;									// Use Timer clock / 1
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;						// Note : repetitionCounter NA for TMR1
TIM_TimeBaseInit( DEBUG_TIMER, &TIM_TimeBaseStructure);

DebugTimReset();
}

