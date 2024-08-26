#include "stm32f0xx.h"
#include "Tim.h"

/*==============================================================================
	OPTIMAL TIM PRESCALER
===============================================================================*/
u16 OptimalTimPrescaler( float Period, u32 *TimFreq )
{
RCC_ClocksTypeDef Clocks;
u16 TimPrescaler = 1;

// Compute best timer resolution for required sampling frequency
RCC_GetClocksFreq(&Clocks);

// Get PCLK frequency and compute the timer prescaler and period
if ( Clocks.PCLK_Frequency == Clocks.HCLK_Frequency )
	{
	TimPrescaler = ((u32)(Clocks.PCLK_Frequency*Period))>>15;
	if ( TimPrescaler == 0 )
		TimPrescaler = 1;
	*TimFreq = Clocks.PCLK_Frequency/TimPrescaler;
	}
else
	{
	TimPrescaler = ((u32)(Clocks.PCLK_Frequency*Period))>>14;
	if ( TimPrescaler == 0 )
		TimPrescaler = 1;
	*TimFreq = 2*Clocks.PCLK_Frequency/TimPrescaler;
	}
return TimPrescaler;
}

/*==============================================================================
	COMPUTE TIM PRESCALER
===============================================================================*/
u16 ComputeTimPrescaler( TIM_TypeDef* TIMx, u32 *TimFreq )
{
RCC_ClocksTypeDef Clocks;
u16 TimPrescaler = 1;

if ( *TimFreq == 0 )
	return 0xFFFF;
	
// Get PCLK frequency and compute the timer prescaler and period
RCC_GetClocksFreq(&Clocks);

// Compute best timer resolution for required Period
if ( Clocks.PCLK_Frequency == Clocks.HCLK_Frequency )
	{
	TimPrescaler = Clocks.PCLK_Frequency/(*TimFreq);
	if ( TimPrescaler == 0 )
		TimPrescaler = 1;
	*TimFreq = Clocks.PCLK_Frequency/TimPrescaler;
	}
else
	{
	TimPrescaler = Clocks.PCLK_Frequency*2/(*TimFreq);
	if ( TimPrescaler == 0 )
		TimPrescaler = 1;
	*TimFreq = 2*Clocks.PCLK_Frequency/TimPrescaler;
	}
return TimPrescaler;
}
