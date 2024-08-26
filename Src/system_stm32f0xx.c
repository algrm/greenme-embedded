#include "stm32f0xx.h"
#include "StdTypes.h"

/*==============================================================================
	PRIVATE DEFINITIONS
===============================================================================*/
// Remove comment if quartz is to be used
#define USE_QUARTZ

/*==============================================================================
	GLOBAL VARIABLES
===============================================================================*/
u32 SystemCoreClock    = HSE_VALUE;
__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

/*==============================================================================
	PRIVATE FUNCTIONS
===============================================================================*/

/*=============================================================================
	SYSTEM INIT

	Ipnput,Output, Retval : None

	Setup the microcontroller clocks
	Initialize the Embedded Flash Interface, the PLL
  
  Note : This function should be used only after reset.

  Max Nb Cycles : not critical
=============================================================================*/
void SystemInit (void)
{    
RCC_DeInit();    

#ifdef USE_QUARTZ
// Enable External quartz oscillator
RCC_HSEConfig(RCC_HSE_ON);           											// Enable External external 16MHz quartz oscillator
while ( !RCC_WaitForHSEStartUp() );
#endif

FLASH_PrefetchBufferCmd(ENABLE);												// Enable Flash memory Prefetch Buffer
FLASH_SetLatency(FLASH_Latency_0);												// wait state for flash memory access 0:f<=24MHz	1:f<=48MHz	2:f>48MHz

RCC_HCLKConfig(RCC_SYSCLK_Div1);												// HCLK = SYSCLK = 24MHz
RCC_PCLKConfig(RCC_HCLK_Div2);													// PCLK = 12MHz
RCC_ADCCLKConfig(RCC_ADCCLK_PCLK_Div4);											// ADCCLK = 3MHz
RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);												// I2C1CLK = HSI = 8MHz
																				// I2C2CLK = PCLK = 12MHz
// Configure PLL
#ifdef USE_QUARTZ
RCC_PREDIV1Config(RCC_PREDIV1_Div4);											// PLLIN CLK = 4MHz = 16MHz/4
RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_6);								// PLLCLK = 24MHz = 4MHz*6 
#else
RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_6);							// PLLCLK = 24MHz = (HSI2/2)*6 = (8/2)*6
#endif
RCC_PLLCmd(ENABLE);
while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);							// Wait till PLL ready

RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);										// SYSCLK = PLLCLK = 24MHz
while (RCC_GetSYSCLKSource() != 0x08);											// Wait till PLL is used as system clock source

RCC->AHBENR |= RCC_AHBPeriph_CRC;												// Enable CRC module
}
																				// I2C2CLK = PCLK = 12MHz
