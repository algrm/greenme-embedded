#include "stm32f0xx.h"             										// STM32F10x Library Definitions
#include "Time.h"
#include "Tim.h"
#include "Debug.h"

/*==============================================================================
	US TIMER DEFINITIONS
===============================================================================*/
#define US_TIMER_FREQ					1000000									//(Hz) us timer clock frequency

#define US_LOW16_TIMER_PERIPH			RCC_APB2Periph_TIM16
#define US_LOW16_TIMER					TIM16									// Timer used for Low 16 bit counter
#define US_LOW16_TIMER_DBG				DBGMCU_TIM16_STOP						// Debug timer used to stop timer when core halted ( breakpoint )
#define US_LOW16_TIMER_IRQ				TIM16_IRQn

/*==============================================================================
	PRIVATE VARIABLES
===============================================================================*/
u16 TmrHigh16=0;

/*==============================================================================
	US HIGH 16 bits TIMER IRQ
===============================================================================*/
void TIM16_IRQHandler( void )
{
US_LOW16_TIMER->SR &= ~TIM_IT_Update;											// Clear Overflow flag
TmrHigh16++;
}

/*=============================================================================
	US TIMER SETUP
=============================================================================*/
void usTimerSetup( void )
{
NVIC_InitTypeDef TimeIrqCfg = { US_LOW16_TIMER_IRQ, 3, ENABLE };
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
u32 TimFreq = US_TIMER_FREQ;
u16 TimLowPrescaler = 1;

// Enable peripheral clocks
RCC_APB2PeriphClockCmd( US_LOW16_TIMER_PERIPH, ENABLE);

DBGMCU_Config( US_LOW16_TIMER_DBG, ENABLE );									// Stop TMR counter when core halted ( breakpoint )

TimLowPrescaler = ComputeTimPrescaler( US_LOW16_TIMER, &TimFreq );
if ( TimFreq != US_TIMER_FREQ )
	{
	while ( 1 );																// Error : could not set exact frequency : infinite loop
	}

///////////// LOW 16 bit TIMER SETUP /////////////
TIM_ARRPreloadConfig( US_LOW16_TIMER, ENABLE);

if ( TimLowPrescaler == 0 )
	TIM_TimeBaseStructure.TIM_Prescaler = 0;									// Prescaler too low : set prescaler to maximum speed
else
	TIM_TimeBaseStructure.TIM_Prescaler = TimLowPrescaler-1;					// Set prescaler for timer resolution
TIM_TimeBaseStructure.TIM_Period = (u16)0xFFFF;									// Set Max period
TIM_TimeBaseStructure.TIM_ClockDivision = 0;									// Use Timer clock / 1
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;						// Note : repetitionCounter NA for TMR1
TIM_TimeBaseInit( US_LOW16_TIMER, &TIM_TimeBaseStructure);

NVIC_Init((NVIC_InitTypeDef *)&TimeIrqCfg);

US_LOW16_TIMER->SR &= ~TIM_IT_Update;											// Clear Overflow flag
US_LOW16_TIMER->DIER |= TIM_IT_Update;											// Enable Overflow IRQ

TIM_Cmd( US_LOW16_TIMER, ENABLE );
}

/*==============================================================================
	WAIT uS

	Waits N us before returning control
	
	Note : resolution is 1 us processing time so, Waitus(1) duration is between 0 and 1us + processing time
==============================================================================*/
void Waitus( u16 us )
{
u16 CurTick = US_LOW16_TIMER->CNT;
while ( us-- )
	{
	while ( CurTick == US_LOW16_TIMER->CNT );
	CurTick++;
	}
}

/*==============================================================================
	WAIT MS

	Waits N ms before returning control
==============================================================================*/
void WaitMs( u16 Ms )
{
while ( Ms > 65 )
	{
	Waitus(65000);
	Ms -= 65;
	}
Waitus(Ms*1000);
}

/*==============================================================================
	GET us TICK

	Note : this us tick counter overflows after 4294,967296 seconds

	returns the tick timer counter value 0 us to 4294,967296 seconds
==============================================================================*/
u32 GetusTick( void )
{
	union
		{
		u32 Tim32;
		struct
			{
			u16 L;
			u16 H;
			}Tim16;
		}Cnt1,Cnt2;
	Cnt1.Tim16.H = TmrHigh16;
	Cnt1.Tim16.L = US_LOW16_TIMER->CNT;
	Cnt2.Tim16.H = TmrHigh16;
	Cnt2.Tim16.L = US_LOW16_TIMER->CNT;

	if ( Cnt1.Tim16.H == Cnt2.Tim16.H )
		return Cnt1.Tim32;														// No counter overflow between H1L1 load and H2L2 load => no overflow between H1 and L1
	else
		return Cnt2.Tim32;														// counter overflow between H1 load and H2 load => no overflow betweeh H2 and L2
}

/*==============================================================================
	Elapsed us
	
	Input : Startus Tick counter from which to comute elapsed time

	returns the number of microseconds elapsed since Startus
==============================================================================*/
u32 Elapsed_us( u32 Startus )
{
	u32 us = GetusTick();
	return us - Startus;
}

void UpdateTime(Time_t* time)
{
	u32 sec, lastSec;
	//u32 elapsed;
	u32 now = GetusTick();
	//elapsed = Elapsed_us(time->lastTickus);
	lastSec = time->lastTickus / 1000000;
	sec = now / 1000000;
	if ((sec == 0) && (lastSec == 4294))
	{
		time->nLoops++;
	}
	time->TotalSec = sec + time->nLoops * 4296;
	time->lastTickus = now;

}
