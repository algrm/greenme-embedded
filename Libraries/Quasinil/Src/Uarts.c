#include "Uarts.h"
#include "STM32F0xx.h"
#include "GPIODef.h"


#define STM32F030x8

#ifdef STM32F030x8
static const GPIODef_t TxPin[NB_UART_PIN_CFG] = 	{
	{  PA2, TYPE_UART2, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{  PA9, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{ PA14, TYPE_UART2, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{  PB6, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN }
												};
static const GPIODef_t RxPin[NB_UART_PIN_CFG] = 	{
	{  PA3, TYPE_UART2, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{ PA10, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{ PA15, TYPE_UART2, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{  PB7, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN }
												};
#else // STM32F030x4 x6
static const GPIODef_t TxPin[NB_UART_PIN_CFG] = 	{
	{  PA2, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{  PA9, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{ PA14, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN },
	{  PB6, TYPE_UART1, OUT_PP, SPEED_MEDIUM, INIT_OPEN }
												};
static const GPIODef_t RxPin[NB_UART_PIN_CFG] = 	{
	{  PA3, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{ PA10, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{ PA15, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN },
	{  PB7, TYPE_UART1, PULL_UP, SPEED_MEDIUM, INIT_OPEN }
												};
#endif

/*=============================================================================
	DMA DEFINITIONS
=============================================================================*/
#define UART1_RX_DMA_CHANNEL		DMA1_Channel3
#define UART1_TX_DMA_CHANNEL		DMA1_Channel2

#define UART2_RX_DMA_CHANNEL		DMA1_Channel5
#define UART2_TX_DMA_CHANNEL		DMA1_Channel4

#define AHB_DMA_PERIPH			RCC_AHBPeriph_DMA1

/*==============================================================================
	Private Data
==============================================================================*/
const NVIC_InitTypeDef Uart1IrqCfg = {USART1_IRQn, 3, ENABLE};
const NVIC_InitTypeDef Uart2IrqCfg = {USART2_IRQn, 3, ENABLE};

/*==============================================================================
	Uart Setup
	
	Inputs :
		BaudRate : any valid baud rate
		UartWordlength : WordLength8 or WordLenght9
		UartParity : NO_PARITY, PARITY_EVEN or PARITY_ODD
		
	Configures the Uart for given parameters and processor pinout
==============================================================================*/
bool UartSetup( UartPins_t UartPins, u32 BaudRate, UartWordLength_t WordLength, UartParity_t Parity, UartDMA_t DMA )// Configures the serial link
{
DMA_Channel_TypeDef *DMATxChannel,*DMARxChannel;
USART_TypeDef *Uart;
DMA_InitTypeDef DMA_InitStructure;
USART_InitTypeDef USART_InitStructure;

// GPIO Setup
GpioSetup( &TxPin[UartPins] );
GpioSetup( &RxPin[UartPins] );

switch( TxPin[UartPins].Type )
	{
	case TYPE_UART1 : 
		Uart = USART1;
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);				// Enable USART2 clock
		DMATxChannel = UART1_TX_DMA_CHANNEL;
		DMARxChannel = UART1_RX_DMA_CHANNEL;
		break;
	case TYPE_UART2 : 
		Uart = USART2;	
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);				// Enable USART2 clock
		DMATxChannel = UART2_TX_DMA_CHANNEL;
		DMARxChannel = UART2_RX_DMA_CHANNEL;
		break;
	default : return false;
	}

RCC_AHBPeriphClockCmd(AHB_DMA_PERIPH, ENABLE);									// Enable DMA clock

// UART CONFIGURATION
USART_DeInit(Uart);																// Reset USART to its default values
USART_InitStructure.USART_BaudRate = BaudRate;									// USART configuration
if ( WordLength == WordLength8 )
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
else
	USART_InitStructure.USART_WordLength = USART_WordLength_9b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
if ( Parity == NO_PARITY )
	USART_InitStructure.USART_Parity = USART_Parity_No;
else if ( Parity == PARITY_EVEN )
	USART_InitStructure.USART_Parity = USART_Parity_Even;
else
	USART_InitStructure.USART_Parity = USART_Parity_Odd;
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// Enable Receiver and transmitter
USART_Init(Uart, &USART_InitStructure);										

USART_OverrunDetectionConfig( Uart, USART_OVRDetection_Disable );
USART_SetReceiverTimeOut( Uart, 26 );										// End of frame if no new start bit 11 bits after last character
USART_ReceiverTimeOutCmd( Uart, ENABLE );

if ( Uart == USART1 )
	NVIC_Init((NVIC_InitTypeDef  *)&Uart1IrqCfg);
else
	NVIC_Init((NVIC_InitTypeDef  *)&Uart2IrqCfg);

// Configure Transmit DMA channel
if ( DMA & DMA_TX )
	{
	USART_DMACmd(Uart, USART_DMAReq_Tx, ENABLE);							// Enable DMA Requests for transmission

	DMA_DeInit( DMATxChannel );  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&Uart->TDR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init( DMATxChannel, &DMA_InitStructure);
	DMA_Cmd( DMATxChannel, ENABLE);
	}

if ( DMA & DMA_RX )
	{
	// Configure Receive DMA channel
	USART_DMACmd(Uart, USART_DMAReq_Rx, ENABLE);									// Enable DMA Requests from UART module

	DMA_DeInit( DMARxChannel );  
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&Uart->RDR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 0;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init( DMARxChannel, &DMA_InitStructure);
	DMA_Cmd( DMARxChannel, ENABLE);
	}

USART_Cmd(Uart, ENABLE);														// Enable USART
return true;
}
