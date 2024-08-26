#ifndef GPIO_DEF_H
#define GPIO_DEF_H

#include "StdTypes.h"

#define MAX_PINS_PER_PORT	0x10

typedef enum { 	PA0=0x00, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PA8, PA9, PA10, PA11, PA12, PA13, PA14, PA15, 
				PB0=0x10, PB1, PB2, PB3, PB4, PB5, PB6, PB7, PB8, PB9, PB10, PB11, PB12, PB13, PB14, PB15,
				PC= 0x20, 														   PC13=PC+13, PC14, PC15,
				PD= 0x30,
				PF0=0x50, PF1, 				 PF6=PF0+6, PF7, PORT_NB_TYPES }Port_t;
typedef enum { 	TYPE_LOGIC_INPUT, TYPE_ADC, TYPE_OUTPUT, 
				TYPE_AF, TYPE_EVENTOUT, TYPE_SPI1, TYPE_SPI2, TYPE_I2C1, TYPE_I2C2, TYPE_UART1, TYPE_UART2, TYPE_CEC, TYPE_TIM1, TYPE_TIM2, TYPE_TIM3, TYPE_TIM14, TYPE_TIM15, TYPE_TIM16, TYPE_TIM17, 
				TYPE_MCO, TYPE_SWCLK, TYPE_SWDAT, TYPE_IR_OUT, TYPE_____, TYPE_NB_TYPES	}GpioType_t;
typedef enum { PULL_NONE, PULL_UP, PULL_DOWN, PULL_NB_TYPES, OUT_PP, OUT_OD }PullType_t;
typedef enum { SPEED_MEDIUM, SPEED_FAST, SPEED_HIGH, SPEED_NB_TYPES }Speed_t;
typedef enum { INIT_0, INIT_1, INIT_OPEN }InitState_t;

typedef struct
{
Port_t				Port;
GpioType_t			Type;
PullType_t 			Pull;														// PULL_NONE, PULL_UP or PULL_DOWN
Speed_t				Speed;														// SPEED_MEDIUM, SPEED_FAST or SPEED_HIGH
InitState_t			InitState;
}GPIODef_t;

bool GpioSetup( const GPIODef_t *GPIODef );
bool IsInputOn( const GPIODef_t *GPIODef );
bool IsInputOff( const GPIODef_t *GPIODef );
void OutputSet( const GPIODef_t *GPIODef );
void OutputReset( const GPIODef_t *GPIODef );
void Output( const GPIODef_t *GPIODef, u8 NewState );

#endif
