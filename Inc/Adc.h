#ifndef ADC_H
#define ADC_H

#include <StdTypes.h>

#define AUDIO_SMP_FREQH			32000											//(Hz) High audio frequency band sampling frequency
#define AUDIO_SMP_FREQL			4000											//(Hz) Low Audio frequency band sampling frequency

typedef struct
{
u16 Audio;
u16 Flicker;
}AdcSmp_t;

#endif //ADC_H
