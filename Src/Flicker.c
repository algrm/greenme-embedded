#include "Flicker.h"
#include "Filters.h"
#include <math.h>

/*==============================================================================
	PRIVATE DEFINTIONS
==============================================================================*/
#define FLICKER_SMP_FREQ		200												//(Hz) Flicker analog sampling rate
#define MAINS_FLICKER_FREQ		50												//(Hz) Mains frequency used to detect mains flickering

#if FLICKER_SMP_FREQ != MAINS_FLICKER_FREQ*(FLICKER_SMP_FREQ/MAINS_FLICKER_FREQ)
	#error Flicker Smp freq must be a multiple of mains freq
#endif

/*==============================================================================
	Private Data
==============================================================================*/
static u32 SmpSum;																// = last Sum 1 to 256( Smp(i) )
static u32 SquareSum;															// = last Sum 1 to 256( Smp(i)*Smp(i) )

/*==============================================================================
	Transfer Flicker Smp
	
	Inputs : 
		SmpPtr : 12 bits Analog samples buffer: Audio and photodiode
		N : number of samples in buffer

	Flicker sampling rate must be a multiple of mains frequency
	Buffer sample size must cover at least one mains period
		
	- Converts ADC sampling rate to Flicker sampling rate by computing a mean value.
	- Computes Flicker samples sum and square sum over 256 samples
	
	note : Flicker is computed over 256 samples with 4 samples over 20ms every low audio freq scan period ( around 50ms ) ( which makes a total around 3s )
==============================================================================*/
void TransferFlickerSmp( AdcSmp_t *SmpPtr, u16 N )
{
static u32 PendingSmpSum=0;
static u32 PendingSmp2Sum=0;
static u8 n=0;
u32 Smp=0;
u32 i,j;

if ( AUDIO_SMP_FREQL > MAINS_FLICKER_FREQ*N )
	return;																		// We don't have enough samples to compute mains flicker

for ( j=(FLICKER_SMP_FREQ/MAINS_FLICKER_FREQ); j-- != 0; )
	{
	Smp = 0;
	for ( i=0; i<(AUDIO_SMP_FREQL/FLICKER_SMP_FREQ); i++ )
		{ 
		Smp += SmpPtr->Flicker;
		SmpPtr++;
		}
	Smp /= (AUDIO_SMP_FREQL/FLICKER_SMP_FREQ);
	if ( n == 0 )
		{
		SmpSum = PendingSmpSum;
		PendingSmpSum = 0;
		SquareSum = PendingSmp2Sum;
		PendingSmp2Sum = 0;
		}
	PendingSmpSum += Smp;
	PendingSmp2Sum += Smp*Smp;
	n++;
	}
}

/*==============================================================================
	Flicker
	
	returns the standard deviation as a percentage of mean light
	
	Note : the percentage cannot be computed if mean value is null. 0 is returned in this case
==============================================================================*/
u8 Flicker( void )
{
float Mean;
float Variance;
float Sigma;
float Flicker_0_100;

Mean = (float)SmpSum/256;
Variance = (float)SquareSum/256 - Mean*Mean;
Flicker_0_100=0;

if ( Variance >= 0 )
	Sigma = _sqrt(Variance);
else
	Sigma = _sqrt(-Variance);

if ( Mean == 0 )
	return 0;
else
	Flicker_0_100 = ((float)(Sigma*100)/Mean);

if ( Flicker_0_100 < 0 )
	Flicker_0_100 = 0;
else if ( Flicker_0_100 > 100 )
	Flicker_0_100 = 100;

return Flicker_0_100;
}
