#include "stm32f0xx.h"
#include "Uart1.h"
#include <string.h>
#include "StdTypes.h"
#include "GPIODef.h"
#include "Debug.h"

/*=============================================================================
	UART DEFINITIONS
=============================================================================*/
#define UART					USART1											// Base address for UART registers

/*=============================================================================
	DMA DEFINITIONS
=============================================================================*/
#define UART_RX_DMA_CHANNEL		DMA1_Channel3
#define UART_TX_DMA_CHANNEL		DMA1_Channel2

/*=============================================================================
	MACROS
=============================================================================*/
#define IS_RX_FRAME_READY()	(0 == (UART->CR1 & (USART_CR1_RXNEIE|USART_CR1_IDLEIE)))
#define IS_RX_IRQ_ENABLED()	(0 != (UART->CR1 & USART_CR1_RXNEIE) )

/*=============================================================================
	PRIVATE UART VARIABLES
=============================================================================*/
static u16 RxBufSize = 0;														//(Bytes) Reception buffer size

/*==============================================================================
	UART Interrupt handler
	
	Uart IRQ has 3 states
	
	- Waiting for first character of frame : only RX IRQ is enabled
	- Receiving frame : only IDLE IRQ enabled to detect when frame reception is finished
	- Not receiving : both IRQ disabled
	
	Detects first and last received characters of a frame on UART
==============================================================================*/
void USART1_IRQHandler(void)
{
if ( IS_RX_IRQ_ENABLED() )
	{	// Uart1 Rx IRQ enabled => First byte of frame detected
	UART->ICR = USART_ICR_IDLECF;												// Clear Idle detect flag
	USART_ITConfig(UART, USART_IT_IDLE, ENABLE);								// Enable Timeout IRQ
	USART_ITConfig(UART, USART_IT_RXNE, DISABLE);								// Disable RX byte ready IRQ
	}
else
	{	// Uart1 Rx IRQ disabled => Uart IDLE IRQ Enabled => End of frame detected
	DMA_Cmd( UART_RX_DMA_CHANNEL, DISABLE);
	USART_ITConfig(UART, USART_IT_IDLE, DISABLE);
	UART->CR1 &= ~USART_CR1_RE;
	}
}

/*==============================================================================
	Uart1 Rx Data Size
	
	Returns 0 untill a full frame is received ( terminated by one character time
		without reception )
	returns the data size otherwise
==============================================================================*/
u16 Uart1RxDataSize( void )
{
u16 Size = 0;
if ( IS_RX_FRAME_READY() )
	{
	Size = RxBufSize - DMA_GetCurrDataCounter( UART_RX_DMA_CHANNEL );
	
	if ( Size == 0 || (UART->ISR & (USART_FLAG_FE | USART_FLAG_PE )) )
		{	// abnormal end of reception or byte error : Re enable reception
		Uart1StartReception( (u8 *)UART_RX_DMA_CHANNEL->CMAR, RxBufSize );
		Size = 0;
		}
	}

return Size;
}

/*==============================================================================
	Uart Start Reception
==============================================================================*/
void Uart1StartReception( u8 *BufPtr, u16 RxMaxSize )
{
RxBufSize = RxMaxSize;

// Disable Uart receiver while resetting DMA data counter
UART->CR1 &= ~USART_CR1_RE;

// Clear Received data flag
USART_ReceiveData(UART);
UART->ICR = (USART_FLAG_FE | USART_FLAG_PE );									// Clear error flags

// Reset DMA RX data counter and size
DMA_Cmd( UART_RX_DMA_CHANNEL, DISABLE);
UART_RX_DMA_CHANNEL->CMAR = (u32)BufPtr;										// Set Dma memory start address
UART_RX_DMA_CHANNEL->CNDTR = RxMaxSize;
DMA_Cmd( UART_RX_DMA_CHANNEL, ENABLE);

// Enable Uart receiver
USART_ITConfig(UART, USART_IT_IDLE, DISABLE);
USART_ITConfig(UART, USART_IT_RXNE, ENABLE);
UART->CR1 |= USART_CR1_RE;
}

/*==============================================================================
	Uart Send
	
	Inputs : 
		BufPtr : pointer to data source
		UartSendSize : number of bytes to transmit
	Retval : 
		None
		
	Aborts any currently pending transmission
	Starts transmitting the first UartSendSize bytes of BufPtr
==============================================================================*/
void Uart1Send( u8 *BufPtr, u16 UartSendSize )
{
if ( UartSendSize != 0 )
	{
	USART_DMACmd(UART, USART_DMAReq_Tx, DISABLE);								// Disable UART TX DMA requests while reconfiguring
	DMA_Cmd( UART_TX_DMA_CHANNEL, DISABLE);										// Disable DMA while reconfiguring DMA
	UART_TX_DMA_CHANNEL->CMAR = (u32)BufPtr;									// Set Dma memory start address
	UART_TX_DMA_CHANNEL->CNDTR = UartSendSize;									// Load DMA transfer size
	DMA_Cmd( UART_TX_DMA_CHANNEL, ENABLE);										// Enable DMA transfer
	USART_DMACmd(UART, USART_DMAReq_Tx, ENABLE);								// Enable DMA Requests from UART module
	}
}

/*==============================================================================
	Is Uart Tx Complete
==============================================================================*/
bool IsUart1TxComplete( void )
{
if ( UART->ISR & USART_ISR_TC )
	return true;
else
	return false;
}
