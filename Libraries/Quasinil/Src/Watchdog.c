#include "stm32f0xx.h"
#include "Watchdog.h"

#define LSI_MIN_FREQ	30000													//(Hz)
#define LSI_MAX_FREQ	60000													//(Hz)

#define MAX_WDG_RELOAD_VAL	0x0FFF

/*==============================================================================
	Independant Watchdog Setup
	
	Input: MinWDGTimeMs	minimum watchdog time for which Watchdog MUST not activate ( it MUST activate for twice this time )
	
	Output, Retval : None

	Watchdog setup for period Tw : 2,18s < Tw < 4,37s
	Note : watchdog is hardware enabled and cannot be disabled ( see file STM32F10xOPT.s )
	
	Execution time : not critical
==============================================================================*/
void SetWatchdog( u32 MinWDGTimeMs )
{
u32 Divider = MinWDGTimeMs * (LSI_MAX_FREQ/1000);
u32 Prescaler;

DBGMCU_Config( DBGMCU_IWDG_STOP, ENABLE );										// Stop IWDG counter when core halted ( breakpoint )
IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);									// Enable write access to IWDG_PR and IWDG_RLR registers

Divider /= 4;
Prescaler=IWDG_Prescaler_4;
while ( (Divider>=MAX_WDG_RELOAD_VAL) && (Prescaler<IWDG_Prescaler_256) )
	{
	Divider /= 2;
	Prescaler++;
	}
Divider++;																		// Round up watchdog time
if ( Divider > MAX_WDG_RELOAD_VAL )
	{	while ( 1 );	}														// Error case : Watchdog time too big : infinite loop

IWDG_SetPrescaler( Prescaler );
IWDG_SetReload(Divider);
IWDG_ReloadCounter();															// Reload IWDG counter
IWDG_Enable();
}

/*==============================================================================
	Independant Watchdog Refresh
	
	Reset watchdog timer tiemout to predefined value
==============================================================================*/
void WatchdogRefresh( void )
{
IWDG_ReloadCounter();															// Reload IWDG counter
}








//#include "Watchdog.tst"
