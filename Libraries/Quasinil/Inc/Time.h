#ifndef TIME_H
#define TIME_H

#include "StdTypes.h"

typedef struct
{
u16 nLoops;										// Hundredth of current second
u32 TotalSec;
u32 lastTickus;
}Time_t;

/*==============================================================================
	FUNCTION PROTOTYPES
===============================================================================*/
void usTimerSetup( void );														// Setup Microsecond timer
u32 GetusTick( void );
void Waitus( u16 us );
void WaitMs( u16 Ms );
u32 Elapsed_us( u32 Startus );
void UpdateTime(Time_t *time);

#endif	// TIME_H
