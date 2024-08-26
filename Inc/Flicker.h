#ifndef FLICKER_H
#define FLICKER_H

#include "Adc.h"

void TransferFlickerSmp( AdcSmp_t *SmpPtr, u16 N );
u8 Flicker( void );

#endif // FLICKER_H
