#include "Stm32f0xx.h"
#include "dBAMeter.h"
#include "GPIODef.h"
#include "Tim.h"
#include "Debug.h"
#include "arm_math.h"
#include "Filters.h"
#include "Cfg.h"
#include "Flicker.h"
#include <float.h>
#include <math.h>
#include "definitions.h"
#include "Cfg.h"
#include "dBAMeter.h"



/*==============================================================================
	AUDIO and FFT DEFINITIONS
===============================================================================*/
#define LOWEST_FREQ				31.25											//(Hz) Lowest octave frequency 
#define N						128												// FFT Number of real samples

#define MAX_SATURATE_DB			140												//(dBA) maximum saturated octave dBA value

#define DB_TRHESHOLD			33.0											//(dBA) Start measuring low freq when total freq contribution under 190Hz is above this threshold

#define FFT_FORWARD	0															// 0 for standard forward FFT

/*==============================================================================
	ADC DEFINITIONS AND CONFIGURATION
===============================================================================*/
#define AUDIO_ADC						ADC1
#define AUDIO_ADC_PERIPH_ENABLE()		RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE);
#define AUDIO_ADC_RESET()				{ RCC->APB2RSTR |= RCC_APB2Periph_ADC1;RCC->APB2RSTR &= ~RCC_APB2Periph_ADC1;}	// Reset ADC then end ADC reset
#define ADC_8KHZ_AUDIO_CHANNEL			ADC_Channel_0							// Audio ADC channel dedicated to 250Hz to 8 kHZ audio
#define ADC_125HZ_AUDIO_CHANNEL			ADC_Channel_1							// Audio ADC channel low pass filtered for under 200Hz audio and 8kHz sampling
#define ADC_FLICKER_CHANNEL				ADC_Channel_8							// Flicker measurement ADC channel 

#define ADC_CFGR1_CFG	( \
/* Start conversion on external trigger rising edge */	ADC_ExternalTrigConvEdge_Rising | \
/* Use TIM15 output trigger as conversion trigger */	ADC_ExternalTrigConv_T15_TRGO | \
/* Data alignment */									ADC_DataAlign_Right | \
/* Data resolution */									ADC_Resolution_12b | \
/* Scan direction */									ADC_ScanDirection_Upward | \
/* DMA mode */											ADC_DMAMode_OneShot | \
/* DMA Request Enable/Disable */						ADC_CFGR1_DMAEN | \
/* Overwrite previous data when overrun */				ADC_CFGR1_OVRMOD)

typedef enum { START_LOW_FREQ_SAMPLING, START_HIGH_FREQ_SAMPLING }AudioSmpStart_t;

#define GAIN_DB		(23.52)													//(dB) =20*log(GH/GL) ( experimental 1dB added )

/*==============================================================================
	DMA DEFINITIONS AND CONFIGURATION
===============================================================================*/
#define AUDIO_DMA_CHANNEL		DMA1_Channel1
#define AUDIO_DMA_IT			DMA_IT_TC
#define AUDIO_DMA_IRQ_FLAG		DMA1_FLAG_TC1

typedef struct
{																				// Standard library type redefined because it used non ANSI u32 adresses instead of pointers
volatile u32 *DMA_PeripheralBaseAddr; 											// Specifies the peripheral base address for DMAy Channelx
u16 *DMA_MemoryBaseAddr;     													// Specifies the memory base address for DMAy Channelx
u32 DMA_DIR;                													// Specifies if the peripheral is the source or destination
u32 DMA_BufferSize;         													// Specifies the buffer size, in data unit, of the specified Channel. 
u32 DMA_PeripheralInc;      													// Specifies whether the Peripheral address register is incremented or not : DMA_PeripheralInc_Enable or Disable
u32 DMA_MemoryInc;          													// Specifies whether the memory address register is incremented or not : DMA_MemoryInc_Enable or Disable
u32 DMA_PeripheralDataSize; 													// Specifies the Peripheral data width. DMA_PeripheralDataSize_Byte, HalfWord or Word
u32 DMA_MemoryDataSize; 														// Specifies the Memory data width : DMA_MemoryDataSize_Byte, HalfWord or Word
u32 DMA_Mode;               													// Specifies the operation mode of the DMAy Channelx : DMA_Mode_Normal or DMA_Mode_Circular
u32 DMA_Priority;           													// Specifies the software priority for the DMAy Channelx : DMA_Priority_Low, Medium, High, VeryHigh
u32 DMA_M2M;                													// Specifies if the DMAy Channelx will be used in memory-to-memory transfer : DMA_M2M_Enable or Disable
}DMACfg_t;

/*==============================================================================
	SAMPLING TIMER DEFINITIONS
===============================================================================*/
#define AUDIO_SMP_TIMER					TIM15
#define AUDIO_SMP_TIMER_FREQ			4000000									//(Hz) Audio sampling timer clock frequency
#define AUDIO_SMP_TIM_PERIPH_ENABLE()	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM15, ENABLE);
#define AUDIO_SMP_TIMER_DBG	DBGMCU_TIM15_STOP									// Debug timer used to stop timer when core halted ( breakpoint )

/*==============================================================================
	Private data
===============================================================================*/
static const GPIODef_t MIC8kHzPin =    { PA0, TYPE_ADC, PULL_NONE, SPEED_MEDIUM, INIT_OPEN };
static const GPIODef_t MIC125HzPin =   { PA1, TYPE_ADC, PULL_NONE, SPEED_MEDIUM, INIT_OPEN };
static const GPIODef_t FlickerPin =   { PB0, TYPE_ADC, PULL_NONE, SPEED_MEDIUM, INIT_OPEN };
static const GPIODef_t MICMaxGainPin = { PA4, TYPE_OUTPUT, PULL_NONE, SPEED_MEDIUM, INIT_1 };
static u16 OctavedBAx100[NB_OCTAVES];
static AdcSmp_t AdcSmp[N];															// N 12 bits samples
static float SmpFloat[N];														// N Floating point samples ( 512 bytes = 128*4 )
static float Bins[2*N];															// 2*N Floating point FFT bins ( 2048 bytes = 128*4*4 )

static const DMACfg_t AudioDMACfg = 
{
&AUDIO_ADC->DR,																	// Peripheral Address
&AdcSmp[0].Audio,																// Memory base address
DMA_DIR_PeripheralSRC,															// Peripheral to Memory transfer
N,																				// Buffer size 
DMA_PeripheralInc_Disable,
DMA_MemoryInc_Enable,
DMA_PeripheralDataSize_HalfWord,
DMA_MemoryDataSize_HalfWord,
DMA_Mode_Normal,
DMA_Priority_Low,
DMA_M2M_Disable
};

/*==============================================================================
	OCTAVES DBA UPDATE
	
	Inputs : 
		FirstBin : Pointer to first bin to sum up
		Last Bin : Pointer to  last bin to sum up
		OctaveNb : Octave number to which bins summation must be set
		ReducedGain : when true, gain has been reduced for a high amplitude signal
		
	Sums up FirstBin to LastBin FFT results and computes the resulting raw dB level
	this dB level is a non standard dB level
	converts dB to dBA by removing dBA offset for required octave
	then adds the calibration dB offset
===============================================================================*/
void OctavedBAUpdate( float *FirstBin, float *LastBin, Octave_t OctaveNb, bool ReducedGain )
{// Octave Number                      dc      1      2      3  ||  4     5       6      7      8      9
//  Octave Frequency                        31.5Hz   63Hz  125Hz  250Hz  500Hz   1kHz   2kHz   4kHz   8kHz
const s16 dBAOffset[NB_OCTAVES] =   { 1100,  -2380, -2380, -2960, -2120, -1400, -1330, -1800, -2700, -3100 };
CfgCalib_t *CfgPtr = (CfgCalib_t *)CfgCalibPointer();
float OctaveSum;
float *CurBin;
u32 dBx100=0;

OctaveSum = 0;
for ( CurBin=FirstBin; CurBin<=LastBin; CurBin++ )
	{	OctaveSum += *CurBin;	}
dBx100 = (u32)(100*8.0*log(OctaveSum)) + dBAOffset[OctaveNb];					// 8 instead of 10 for unknown gain non linearity compensation probably noise level uncompensated
if ( ReducedGain )
	{	dBx100 += (u32)(90.0*GAIN_DB);	}										// 90 instead of 100 : same reason as above

// Low pass filter
dBx100 = (79*OctavedBAx100[OctaveNb] + dBx100)*0.0125;							// Low pass filter on octaves
if ( dBx100 > 100*MAX_SATURATE_DB )
	dBx100 = 100*MAX_SATURATE_DB;
OctavedBAx100[OctaveNb] = dBx100;
}

/*==============================================================================
	dBAx100
	
	Computes dBA from all octaves
	
	returns dBA value
===============================================================================*/
u16 dBAx100( void )
{
u32 i;
u16 MaxdBx100 = 0;
u16 dBAx100;
u16 Deltax100;

// Initialize dBA with maximum octave level
for ( i=1; i < NB_OCTAVES; i++ )
	{
	if ( OctavedBAx100[i] > MaxdBx100 )
		{
		MaxdBx100 = OctavedBAx100[i];
		}
	}

// Add octaves according to difference with maximum octave ( Fast dB add Table method )
dBAx100 = MaxdBx100 - 3*100;	// -3 so that when Max power octave is encountered, it may be added ( +3dB )
for ( i=1; i < NB_OCTAVES; i++ )
	{
	Deltax100 = MaxdBx100 - OctavedBAx100[i];
	if ( OctavedBAx100[i] < (DB_TRHESHOLD*100) )
		{
		if ( Deltax100 <= 0 )
			dBAx100 += 3*100;
		}
	else if ( Deltax100 <= 8*100 )
		{
		if ( Deltax100 <= 2*100 )
			{
			if ( Deltax100 <= 1*100 )
				dBAx100 += 300 -Deltax100/2;									// 0 - 1   -> 3.0 - 2.5
			else
				dBAx100 += 290 -Deltax100*4/10;									// 1 - 2   -> 2.5 - 2.1
			}
		else 
			{
			if ( Deltax100 <= 5*100 )
				dBAx100 += 270 -Deltax100*3/10;									// 2 - 5   -> 2.1 - 1.2
			else
				dBAx100 += 220 -Deltax100*2/10;									// 5 - 8   -> 1.2 - 0.6
			}
		}
	else if ( Deltax100 <= 10*100 )
		{
		dBAx100 += 140 -Deltax100/10;											// 8 - 10  -> 0.6 - 0.4
		}
	else if ( Deltax100 <= 16*100 )
		{
		dBAx100 += 90 -Deltax100*5/100;											// 10 - 16 -> 0.4 - 0.1
		}		
	}
return dBAx100;
}

/*==============================================================================
	Returns raw octaves
===============================================================================*/
u16* GetOctaves(void)
{
	return OctavedBAx100;
}
/*==============================================================================
	Prepare FFT Samples
	
	converts u16 samples to float for best FFT resolution
	
	Hamming filter not implemented ( replaced by a low frequency detection )
		
	Reduces gain when signal clipping detected
	returns to normal gain when clipping condition is no longer present
		
	return true if Gain is reduced for large signals
===============================================================================*/
bool PrepareFFTSamples( void )
{
u32 i;
u16 Smp;
static bool MinGain=false;
bool RetVal = MinGain;
static u16 HighAmpFilter=0;

if ( MinGain )
	{// Minimum gain setting : high amplitude audio signal
	u16 NbHighAmplitudes=0;
	for ( i=0; i<N; i++ )
		{
		Smp = AdcSmp[i].Audio;
		SmpFloat[i] = Smp;
		if ( Smp < (0x800 -0x80)  )
			NbHighAmplitudes++;
		}
	if ( NbHighAmplitudes < 5 )
		{
		if ( ++HighAmpFilter > 6 )
			{// Switch back to High gain only if signal has been low for quite some time
			HighAmpFilter = 0;
			MinGain = false;
			OutputSet( &MICMaxGainPin );
			}
		}
	else
		{
		HighAmpFilter = 0;
		}
	}
else
	{// Maximum gain setting
	u16 NbSaturated=0;
	for ( i=0; i<N; i++ )
		{
		Smp = AdcSmp[i].Audio;
		SmpFloat[i] = Smp;
		if ( Smp < 0x100 )															// We just test low saturation as saturation should be symetricaal
			NbSaturated++;
		}
	if ( NbSaturated > 5 )
		{
		MinGain = true;
		OutputReset( &MICMaxGainPin );
		}
	}
return RetVal;
}

/*==============================================================================
	START ACQ
===============================================================================*/
void StartAcq( AudioSmpStart_t Start )
{
AUDIO_SMP_TIMER->CR1 &= ~TIM_CR1_CEN;											// Stop audio sampling timer
AUDIO_ADC->CR &= ADC_CR_ADEN;													// Disable AUDIO_ADC
AUDIO_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;											// Disable DMA

if ( Start == START_LOW_FREQ_SAMPLING )
	{
	AUDIO_SMP_TIMER->ARR = (AUDIO_SMP_TIMER_FREQ/AUDIO_SMP_FREQL)-1;			// Swap to Low audio frequency sampling
	AUDIO_ADC->CHSELR = ADC_125HZ_AUDIO_CHANNEL | ADC_FLICKER_CHANNEL;			// Select low pass filtered aduio ADC channel to be acquired
	}
else
	{
	AUDIO_SMP_TIMER->ARR = (AUDIO_SMP_TIMER_FREQ/AUDIO_SMP_FREQH)-1;			// Swap to High audio frequency sampling
	AUDIO_ADC->CHSELR = ADC_8KHZ_AUDIO_CHANNEL | ADC_FLICKER_CHANNEL;			// Select high audio frequency ADC channel to be acquired
	}
DMA1->IFCR |= AUDIO_DMA_IRQ_FLAG;												// Clear IRQ flag
AUDIO_DMA_CHANNEL->CMAR = (u32)AdcSmp;
AUDIO_DMA_CHANNEL->CNDTR = 2*N;
AUDIO_ADC->CR |= ADC_CR_ADEN;													// Reenable AUDIO_ADC
AUDIO_SMP_TIMER->CR1 |= TIM_CR1_CEN;											// Start audio sampling
AUDIO_DMA_CHANNEL->CCR |= DMA_CCR_EN;											// Start DMA reception as soon as DMA is re enabled
AUDIO_ADC->CR |= ADC_CR_ADSTART;												// Start conversion as soon as trigger event occurs
}

/*==============================================================================
	DBA UPDATE
	
	Updates the audio octaves with a high frequency priority.
	When low frequency bin of high freq FFT shows some low frequencies,
	swaps to low frequency sampling 
	
	Worse case measured duration at 24MHz : 17,2ms
===============================================================================*/
void dBAUpdate( void )
{
arm_rfft_instance_f32 RFFTCfg;
arm_cfft_radix4_instance_f32 CFFTCfg;
bool ReducedGain;

if ( DMA1->ISR & AUDIO_DMA_IRQ_FLAG )
	{
	arm_rfft_init_f32( &RFFTCfg,  &CFFTCfg, N, FFT_FORWARD, 1); 				// Initialize the CFFT/CIFFT module
	ReducedGain = PrepareFFTSamples();
	if ( AUDIO_ADC->CHSELR == (ADC_125HZ_AUDIO_CHANNEL|ADC_FLICKER_CHANNEL) )
		{
		TransferFlickerSmp( AdcSmp, N);				
		StartAcq( START_HIGH_FREQ_SAMPLING );									// Begin sampling high frequencies while computing FFT and flicker on low frequencies
		arm_rfft_f32( &RFFTCfg, SmpFloat, Bins );
		arm_cmplx_mag_f32(SmpFloat, Bins, 5);
		OctavedBAUpdate( &Bins[1],&Bins[1],OCTAVE_31HZ, ReducedGain);
		OctavedBAUpdate( &Bins[2],&Bins[2],OCTAVE_63HZ, ReducedGain);
		OctavedBAUpdate( &Bins[3],&Bins[5],OCTAVE_125HZ, ReducedGain);
		}
	else
		{
		StartAcq( START_LOW_FREQ_SAMPLING );									// Begin sampling low frequencies while computing FFT on high frequencies
		arm_rfft_f32( &RFFTCfg, SmpFloat, Bins );
		arm_cmplx_mag_f32(SmpFloat, Bins, 45);
		OctavedBAUpdate( &Bins[1],&Bins[1],OCTAVE_250HZ, ReducedGain);
		OctavedBAUpdate( &Bins[2],&Bins[2],OCTAVE_500HZ, ReducedGain);
		OctavedBAUpdate( &Bins[3],&Bins[5],OCTAVE_1KHZ, ReducedGain);
		OctavedBAUpdate( &Bins[6],&Bins[11],OCTAVE_2KHZ, ReducedGain);
		OctavedBAUpdate( &Bins[12],&Bins[22],OCTAVE_4KHZ, ReducedGain);
		OctavedBAUpdate( &Bins[23],&Bins[45],OCTAVE_8KHZ, ReducedGain);
		}
	}
}

/*==============================================================================
	dBA Set Gain
	
	Inputs : G ( 0:Low gain; 1:normal gain )
===============================================================================*/
void SetGain( int G )
{
if ( G==0 )
	OutputReset( &MICMaxGainPin );
else
	OutputSet( &MICMaxGainPin );
}

/*==============================================================================
	DBA METER SETUP
	
	Sets up dBA meter pins and peripherals
	
	returns a pointer on audio octaves
===============================================================================*/
u16 *dBAMeterSetup( void )
{
u32 TimFreq;
u16 TimPrescaler;

//return OctavedBA;
// PORT CONFIGURATION
GpioSetup( &MIC8kHzPin );
GpioSetup( &MIC125HzPin );
GpioSetup( &FlickerPin );
GpioSetup( &MICMaxGainPin );

// DMA SETUP
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1 , ENABLE);
AUDIO_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;
DMA_Init( AUDIO_DMA_CHANNEL, (DMA_InitTypeDef *)&AudioDMACfg );

// SAMPLING TIMER SETUP
AUDIO_SMP_TIM_PERIPH_ENABLE();
AUDIO_SMP_TIMER->CR1 = 0;														// stop timer and deinit
DBGMCU->CR |= AUDIO_SMP_TIMER_DBG;												// Stop timer counter when core halted

TIM_SelectOutputTrigger( AUDIO_SMP_TIMER, TIM_TRGOSource_Update );				// Timer period end trigs a new analog conversion

TimFreq = AUDIO_SMP_TIMER_FREQ;
TimPrescaler = ComputeTimPrescaler( AUDIO_SMP_TIMER, &TimFreq );
if ( TimPrescaler == 0 )
	AUDIO_SMP_TIMER->PSC = 0;													// Prescaler too low : set prescaler to maximum speed
else
	AUDIO_SMP_TIMER->PSC = TimPrescaler-1;										// Set prescaler for timer resolution
AUDIO_SMP_TIMER->ARR = (AUDIO_SMP_TIMER_FREQ/AUDIO_SMP_FREQH)-1;				// Set timer period
AUDIO_SMP_TIMER->RCR = 0;														// Repetition counter unused
AUDIO_SMP_TIMER->EGR = TIM_PSCReloadMode_Immediate;								// Reset prescaler and repetition counter internal values
AUDIO_SMP_TIMER->SR = 0;
AUDIO_SMP_TIMER->DIER = 0;														// disable all timer IRQs
AUDIO_SMP_TIMER->CR1 = TIM_CR1_ARPE | TIM_CounterMode_Up | TIM_CR1_CEN;			// Configure timer and start counting

// ADC SETUP
AUDIO_ADC_PERIPH_ENABLE();
AUDIO_ADC_RESET();
AUDIO_ADC->CFGR1 = ADC_CFGR1_CFG;												// Configure ADC
ADC_GetCalibrationFactor(AUDIO_ADC);
AUDIO_ADC->SMPR = ADC_SampleTime_28_5Cycles;									// Total Sampling time = 13,7us = (28,5 + 12,5  )/3MHz => Sampling period > 27,4us = 2*SmpTime
AUDIO_ADC->CR |= ADC_CR_ADEN;													// Enable AUDIO_ADC

while(!ADC_GetFlagStatus(AUDIO_ADC, ADC_FLAG_ADRDY)); 

AUDIO_ADC->CR |= ADC_CR_ADSTP;													// Stop conversion
while ( AUDIO_ADC->CR & ADC_CR_ADSTP );											// Wait for current conversion to stop
AUDIO_ADC->CR |= ADC_CR_ADSTART;												// Start conversion as soon as trigger event occurs

StartAcq( START_LOW_FREQ_SAMPLING );									// Begin sampling high frequencies while computing FFT on low frequencies
return OctavedBAx100;
}

