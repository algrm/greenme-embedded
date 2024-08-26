/*==============================================================================
	GPIODef.c
	Author : Benoit Chiron
	Version V1.00
	Date : 15/10/2014
	Brief : This file provides easy access to STM32F0C8 I/O configuration
===============================================================================*/
#include "stm32f0xx.h"
#include "GPIODef.h"

#define NB_AFx_PORTA	8														// Max Number of alternate functions per port
#define NB_AFx_PORTB	4														// Max number of alternate functions per port

/*==============================================================================
	Private variables
===============================================================================*/
static const GPIO_TypeDef* Port[1+PORT_NB_TYPES/0x10] = { GPIOA, GPIOB, GPIOC, GPIOD, GPIOD, GPIOF };

/*==============================================================================
	GPIO Setup
	
	Input : GPIO Defintion structure
	
	Initializes output ports
===============================================================================*/
bool GpioSetup( const GPIODef_t *GPIODef )
{
// STM32 F030C8 Alternate function list
const GpioType_t PA_AF[MAX_PINS_PER_PORT][NB_AFx_PORTA] = {
//			AF0            AF1            AF2            AF3            AF4            AF5            AF6            AF7
/* PA0 */ { TYPE_____,     TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA1 */ { TYPE_EVENTOUT, TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,	  TYPE_____     },
/* PA2 */ { TYPE_TIM15,    TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA3 */ { TYPE_TIM15,    TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA4 */ { TYPE_SPI1,     TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_TIM14,    TYPE_____,     TYPE_____,     TYPE_____     },
/* PA5 */ { TYPE_SPI1,     TYPE_____,      TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA6 */ { TYPE_SPI1,     TYPE_TIM3,      TYPE_TIM1,     TYPE_____,     TYPE_____,     TYPE_TIM16,    TYPE_EVENTOUT, TYPE_____     },
/* PA7 */ { TYPE_SPI1,     TYPE_TIM3,      TYPE_TIM1,     TYPE_____,     TYPE_TIM14,    TYPE_TIM17,    TYPE_EVENTOUT, TYPE_____     },
/* PA8 */ { TYPE_MCO,      TYPE_UART1,     TYPE_TIM1,     TYPE_EVENTOUT, TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA9 */ { TYPE_TIM15,    TYPE_UART1,     TYPE_TIM1,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA10*/ { TYPE_TIM17,    TYPE_UART1,     TYPE_TIM1,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA11*/ { TYPE_EVENTOUT, TYPE_UART1,     TYPE_TIM1,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA12*/ { TYPE_EVENTOUT, TYPE_UART1,     TYPE_TIM1,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA13*/ { TYPE_SWDAT,    TYPE_IR_OUT,    TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA14*/ { TYPE_SWCLK,    TYPE_UART2,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     },
/* PA15*/ { TYPE_SPI1,     TYPE_UART2,     TYPE_____,     TYPE_EVENTOUT, TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____     }
															};
const GpioType_t PB_AF[MAX_PINS_PER_PORT][NB_AFx_PORTB] = {
//			AF0            AF1            AF2            AF3
/* PB0 */ { TYPE_EVENTOUT, TYPE_TIM3,     TYPE_TIM1,     TYPE_____		},
/* PB1 */ { TYPE_TIM14,    TYPE_TIM3,     TYPE_TIM1,     TYPE_____		},
/* PB2 */ { TYPE_____,     TYPE_____,     TYPE_____,     TYPE_____		},
/* PB3 */ { TYPE_SPI1,     TYPE_EVENTOUT, TYPE_____,     TYPE_____		},
/* PB4 */ { TYPE_SPI1,     TYPE_TIM3,     TYPE_EVENTOUT, TYPE_____		},
/* PB5 */ { TYPE_SPI1,     TYPE_TIM3,     TYPE_TIM16,    TYPE_I2C1		},
/* PB6 */ { TYPE_UART1,    TYPE_I2C1,     TYPE_TIM16,    TYPE_____		},
/* PB7 */ { TYPE_UART1,    TYPE_I2C1,     TYPE_TIM17,    TYPE_____		},
/* PB8 */ { TYPE_____,     TYPE_I2C1,     TYPE_TIM16,    TYPE_____	    },
/* PB9 */ { TYPE_IR_OUT,   TYPE_I2C1,     TYPE_TIM17,    TYPE_EVENTOUT  },
/* PB10*/ { TYPE_____,     TYPE_I2C2,     TYPE_____,     TYPE_____		},
/* PB11*/ { TYPE_EVENTOUT, TYPE_I2C2,     TYPE_____,     TYPE_____		},
/* PB12*/ { TYPE_SPI2,     TYPE_EVENTOUT, TYPE_TIM1,     TYPE_____		},
/* PB13*/ { TYPE_SPI2,     TYPE_____,     TYPE_TIM1,     TYPE_____		},
/* PB14*/ { TYPE_SPI2,     TYPE_TIM15,    TYPE_TIM1,     TYPE_____		},
/* PB15*/ { TYPE_SPI2,     TYPE_TIM15,    TYPE_TIM1,     TYPE_TIM15	    }
															};
const u32 PortPeriph[1+PORT_NB_TYPES/0x10] = { RCC_AHBPeriph_GPIOA, RCC_AHBPeriph_GPIOB, RCC_AHBPeriph_GPIOC, RCC_AHBPeriph_GPIOD, RCC_AHBPeriph_GPIOD, RCC_AHBPeriph_GPIOF };
const GPIOSpeed_TypeDef Speeds[SPEED_NB_TYPES] = { GPIO_Speed_Level_1, GPIO_Speed_Level_2, GPIO_Speed_Level_3 };
const GPIOPuPd_TypeDef Pulls[PULL_NB_TYPES] = { GPIO_PuPd_NOPULL, GPIO_PuPd_UP, GPIO_PuPd_DOWN };
u8 PortNb = GPIODef->Port/0x10;
u8 PortPinNb = GPIODef->Port & 0x0F;
u8 AFx=0;
GPIO_TypeDef* GPIOx;
GPIO_InitTypeDef GPIOInit;

if ( GPIODef->Port >= PORT_NB_TYPES ) 
	return false;

GPIOx = (GPIO_TypeDef *)Port[PortNb];
RCC_AHBPeriphClockCmd( PortPeriph[PortNb], ENABLE );							// Enable clock for selected Port

// Fill GPIO_InitTypeDef structure
// Pin number
GPIOInit.GPIO_Pin = 1<<PortPinNb;
if ( GPIODef->InitState == INIT_0 )
	GPIOx->BRR = GPIOInit.GPIO_Pin;												// Default state at Init is 0 or closed
else
	GPIOx->BSRR = GPIOInit.GPIO_Pin;											// Default state at Init is 1 or open
	
// GPIO_Mode and GPIO_Type
GPIOInit.GPIO_OType = GPIO_OType_PP;
if ( GPIODef->Type < TYPE_AF )
	{
	switch( GPIODef->Type )
		{
		case TYPE_LOGIC_INPUT : GPIOInit.GPIO_Mode = GPIO_Mode_IN; break;
		case TYPE_ADC : 		GPIOInit.GPIO_Mode = GPIO_Mode_AN; 
								RCC_APB2PeriphClockCmd( RCC_APB2Periph_ADC1, ENABLE );
								break;
		case TYPE_OUTPUT : 		GPIOInit.GPIO_Mode = GPIO_Mode_OUT; break;
		default : while(1);
		}
	}
else
	{	// Alternate function for port
	switch ( GPIODef->Type )
		{
		case TYPE_SPI1 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_SPI1, ENABLE  ); break;
		case TYPE_SPI2 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI2, ENABLE  ); break;
		case TYPE_I2C1 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE  ); break;
		case TYPE_I2C2 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C2, ENABLE  ); break;
		case TYPE_UART1 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_USART1, ENABLE  ); break;
		case TYPE_UART2 : 	RCC_APB2PeriphClockCmd( RCC_APB1Periph_USART2, ENABLE  ); break;
		case TYPE_CEC : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_CEC, ENABLE  ); break;
		case TYPE_TIM1 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM1, ENABLE  ); break;
		case TYPE_TIM2 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE  ); break;
		case TYPE_TIM3 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM3, ENABLE  ); break;
		case TYPE_TIM14 : 	RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM14, ENABLE  ); break;
		case TYPE_TIM15 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM15, ENABLE  ); break;
		case TYPE_TIM16 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM16, ENABLE  ); break;
		case TYPE_TIM17 : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM17, ENABLE  ); break;
		case TYPE_IR_OUT : 	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM16, ENABLE  );
							RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM16, ENABLE  ); break;
		}
	GPIOInit.GPIO_Mode = GPIO_Mode_AF;
	if ( GPIOx == GPIOA )
		{
		for ( AFx=0; AFx<NB_AFx_PORTA && PA_AF[PortPinNb][AFx] != GPIODef->Type; AFx++ );	// Search for occurence of GPIODef->Type in AFx = f(PAx) table
		GPIO_PinAFConfig( GPIOA, PortPinNb, AFx );
		}
	else if ( GPIOx == GPIOB )
		{
		for ( AFx=0; AFx<NB_AFx_PORTB && PB_AF[PortPinNb][AFx] != GPIODef->Type; AFx++ );	// Search for occurence of GPIODef->Type in AFx = f(PAx) table
		GPIO_PinAFConfig( GPIOB, PortPinNb, AFx );
		}
	}
// GPIO Speed
if ( GPIODef->Speed >= SPEED_NB_TYPES )
	return false;
GPIOInit.GPIO_Speed = Speeds[GPIODef->Speed];

// Pull up / Pull down / OD / PP
if ( GPIODef->Pull < PULL_NB_TYPES )
	GPIOInit.GPIO_PuPd = Pulls[GPIODef->Pull];
else
	{
	GPIOInit.GPIO_PuPd = Pulls[PULL_NONE];
	if ( GPIODef->Pull == OUT_PP )
		GPIOInit.GPIO_OType = GPIO_OType_PP;
	else if ( GPIODef->Pull == OUT_OD )
		GPIOInit.GPIO_OType = GPIO_OType_OD;
	else
		return false;
	}
GPIO_Init( GPIOx, &GPIOInit );
return true;
}

/*==============================================================================
	Is Input On
===============================================================================*/
bool IsInputOn( const GPIODef_t *GPIODef )
{
u16 GPIOPin = GPIODef->Port & 0x0F;
GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)Port[GPIODef->Port/0x10];

if ( GPIOx->IDR & (1<<GPIOPin) )
	return true;
else
	return false;
}

/*==============================================================================
	Is Input On
===============================================================================*/
bool IsInputOff( const GPIODef_t *GPIODef )
{
u16 GPIOPin = GPIODef->Port & 0x0F;
GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)Port[GPIODef->Port/0x10];

if ( GPIOx->IDR & (1<<GPIOPin) )
	return false;
else
	return true;
}

/*==============================================================================
	OutputSet
===============================================================================*/
void OutputSet( const GPIODef_t *GPIODef )
{
u16 GPIOPin = GPIODef->Port & 0x0F;
GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)Port[GPIODef->Port/0x10];

GPIOx->BSRR = 1<<GPIOPin;
}

/*==============================================================================
	Output Reset
===============================================================================*/
void OutputReset( const GPIODef_t *GPIODef )
{
u16 GPIOPin = GPIODef->Port & 0x0F;
GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)Port[GPIODef->Port/0x10];

GPIOx->BRR = 1<<GPIOPin;
}

/*==============================================================================
	Output
===============================================================================*/
void Output( const GPIODef_t *GPIODef, u8 NewState )
{
u16 GPIOPin = GPIODef->Port & 0x0F;
GPIO_TypeDef *GPIOx = (GPIO_TypeDef *)Port[GPIODef->Port/0x10];

if ( NewState == 0 )
	GPIOx->BRR = 1<<GPIOPin;
else
	GPIOx->BSRR = 1<<GPIOPin;
}
