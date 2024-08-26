#ifndef CCS811_H
#define CCS811_H

#include "StdTypes.h"

bool CCS811Task( void );														// Updates gaz ppm value and starts a new measurement
u16 CCS811_eCO2_ppm( void );														// returns last CO2 estimated ppm value
u16 CCS811_TVoc_ppb( void );													// returns last VOC estimated ppb value
u16 CCS811_GetFwVersion(void);														//return firwmare version ; 0x0000 if not ready
void GazCfg( void );

#endif //CCS811_H
