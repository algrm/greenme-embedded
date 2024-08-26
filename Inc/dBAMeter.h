#ifndef DBA_METER_H
#define DBA_METER_H

#include <StdTypes.h>

typedef enum { OCTAVE_DC, OCTAVE_31HZ, OCTAVE_63HZ, OCTAVE_125HZ, OCTAVE_250HZ, OCTAVE_500HZ,OCTAVE_1KHZ, OCTAVE_2KHZ, OCTAVE_4KHZ, OCTAVE_8KHZ, NB_OCTAVES }Octave_t;

u16 *dBAMeterSetup( void );
void dBAMeterCalibration( float *OctaveCalVal );
void dBAUpdate( void );
u16 dBAx100( void );
u16* GetOctaves(void);

#endif // DBA_METER_H
