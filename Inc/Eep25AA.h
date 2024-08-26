#ifndef EEP25AA_H
#define EEP25AA_H

#include "StdTypes.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/

/*=============================================================================
	FUNCTIONS	
=============================================================================*/
void EepSetup( void );																												// Initializes EEPROM interface
bool EepPageRead( u8 *DstPtr, u8 PageNb, u8 PageStartPos, u8 NbBytesToRead );	// Reads NbBytes from Start Position inside page PageNb
bool EepPageWrite( u8 PageNb, u8 PageStartPos, u8 *SrcPtr, u8 NbBytesToWrite );	// Writes NbBytes to Start Position inside page PageNb

#endif //EEP25AA_H
