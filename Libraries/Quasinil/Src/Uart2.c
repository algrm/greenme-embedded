#include "stm32f0xx.h"
#include "Uart2.h"
#include <string.h>
#include "StdTypes.h"
#include "GPIODef.h"
#include "Debug.h"

/*=============================================================================
	UART DEFINITIONS
=============================================================================*/
#define UART					USART2											// Base address for UART registers

/*=============================================================================
	DMA DEFINITIONS
=============================================================================*/

/*=============================================================================
	PRIVATE UART VARIABLES
=============================================================================*/
static u8 RxBuf[254];															// Radio Uart receive buffer

static u8 TxBufSize = 0;														//(Bytes) Reception buffer size
static u8 RxSize=0;
static u8 TxBufPos=0;
static u8 *TxBufPtr;

/*==============================================================================
	UART Interrupt handler
	
	Uart IRQ has 3 states
	
	- Waiting for first character of frame : only RX IRQ is enabled
	- Receiving frame : only IDLE IRQ enabled to detect when frame reception is finished
	- Transmitting : both IRQ disabled ( at end of transmission : IRQ switches to receive mode )
	
	Detects first and last received characters of a frame on UART
==============================================================================*/
u16 ISR;
void USART2_IRQHandler(void)
{
static u8 IrqRxBufPos = 0;
u8 ReceivedByte;

if ( UART->CR1&USART_CR1_TXEIE )
	{
	if ( UART->ISR&USART_ISR_TXE )
		{
		UART->TDR = TxBufPtr[TxBufPos++];
		if ( TxBufPos >= TxBufSize )
			{
			IrqRxBufPos = 0;
			UART->ICR = (USART_ICR_IDLECF|USART_FLAG_FE | USART_FLAG_PE );		// Clear error flags and idle detect flag
			// Clear Received data flag
			if ( UART->RDR );													// Dummy Receive data register read  to clear buffered data and RXNE flag
			if ( UART->RDR );													// Dummy Receive data register read  to clear buffered data and RXNE flag

			// Enable Uart receiver
			UART->CR1 &= ~(USART_CR1_IDLEIE|USART_CR1_TXEIE);					// Disable transmitter and IDLE detect IRQ
			UART->CR1 |= USART_CR1_RXNEIE|USART_CR1_RE;							// Enable UART receiver and receive IRQ
			}
		}
	}
else
	{
	if ( UART->ISR&USART_ISR_RXNE )
		{	// Rx Data ready
		UART->ICR |= USART_ICR_IDLECF;												// Clear Idle detect flag
		ReceivedByte = UART->RDR;													// Always read Receive data register to clear RXNE flag
		if ( IrqRxBufPos < sizeof(RxBuf) )
			RxBuf[IrqRxBufPos++] = ReceivedByte;
		UART->CR1 |= USART_CR1_IDLEIE;												// Enable end of frame IDLE detect IRQ
		}
	else if ( UART->CR1 & USART_CR1_IDLEIE )
		{
		if ( UART->ISR&USART_ISR_IDLE )
			{	// Idle end of frame detected
			RxSize = IrqRxBufPos;
//DEBUG
IrqRxBufPos = 0;
//				
			UART->ICR |= USART_ICR_IDLECF;											// Clear IDLE IRQ Flag
			UART->CR1 &= ~USART_CR1_IDLEIE;											// Disable IDLE detect until next Start of frame detected
			}
		}
	}
}

/*==============================================================================
	Uart1 Rx Data Size
	
	Returns 0 until a full frame is received ( terminated by one character time
		without reception )
	returns the data size otherwise
==============================================================================*/
u8 Uart2RxDataSize( void )
{
return RxSize;
}

/*==============================================================================
	Uart Start Reception
==============================================================================*/
void Uart2StartReception( void )
{
RxSize = 0;

UART->CR1 &= ~USART_CR1_RE;

// Clear Received data flag
if ( UART->RDR );																// Dummy Receive data register read  to clear buffered data and RXNE flag
if ( UART->RDR );																// Dummy Receive data register read  to clear buffered data and RXNE flag
UART->ICR = (USART_FLAG_FE | USART_FLAG_PE );									// Clear error flags

// Enable Uart receiver
UART->ICR |= USART_ICR_IDLECF;													// Clear Idle detect flag
UART->CR1 &= ~USART_CR1_IDLEIE;													// Disable IDLE detect
UART->CR1 |= USART_CR1_RXNEIE|USART_CR1_RE;										// Enable UART receiver and receive IRQ
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
void Uart2Send( u8 *SrcBufPtr, u8 UartSendSize )
{
if ( UartSendSize != 0 )
	{
	UART->CR1 &= ~(USART_CR1_RE|USART_CR1_IDLEIE|USART_CR1_RXNEIE|USART_CR1_TXEIE);	// Disable Receiver IDLE detect and RX IRQ
	TxBufSize = UartSendSize;
	TxBufPtr = SrcBufPtr;
	TxBufPos = 0;
	RxSize = 0;																	// Purge Rx data buffer before transmitting
	UART->CR1 |= USART_CR1_TXEIE;
	}
}

/*==============================================================================
	Is Uart Tx Complete
==============================================================================*/
bool IsUart2TxComplete( void )
{
if ( UART->ISR & USART_ISR_TC )
	return true;
else
	return false;
}

/*==============================================================================
	Uart2 Purge RxBuffer
==============================================================================*/
void Uart2PurgeRxBuffer( void )
{
RxSize = 0;
}

/*==============================================================================
	Uart2 Rx Buf Ptr
==============================================================================*/
u8 *Uart2RxBufPtr( void )
{
return RxBuf;
}
