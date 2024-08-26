#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "StdTypes.h"

/*==============================================================================
	FUNCTION PROTOTYPES
===============================================================================*/
extern void SetWatchdog( u32 Ms );												// Setup minimum watchdog period
extern void WatchdogRefresh( void );											// Refresh watchdog

#endif	// WATCHDOG_H
