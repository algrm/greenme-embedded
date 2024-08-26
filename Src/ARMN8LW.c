#include "Stm32f0xx.h"
#include "ARMN8LW.h"
#include "Uart2.h"
#include "GpioDef.h"
#include "Tim.h"
#include <string.h>

/*=============================================================================
	Private definitions
=============================================================================*/
#define RADIO_START_TIME_MS		  1200												//(ms) Minimum time before module is operationnal after leaving reset ( undefined by ATIM : experimental )
//#define RADIO_START_TIME_MS		1500												//(ms) Minimum time before module is operationnal after leaving reset ( undefined by ATIM : experimental )

#define ARMN8_TIMER_FREQ	1000												//(Hz) ARMN8 timer clock frequency
#define ARMN8_RESET_TIME_MS	500												//(Hz) ARMN8 reset time

#define ARMN8_TIMER			TIM17
#define ARMN8_TIM_PERIPH_ENABLE()	RCC_APB2PeriphClockCmd( RCC_APB2Periph_TIM17, ENABLE);
#define ARMN8_TIMER_DBG	DBGMCU_TIM17_STOP										// Debug timer used to stop timer when core halted ( breakpoint )

/*=============================================================================
	Types
=============================================================================*/

/*=============================================================================
	Private variables
=============================================================================*/
static const GPIODef_t RadioRstPin = { PA5, TYPE_OUTPUT, OUT_PP, SPEED_MEDIUM, INIT_0 };	// Start with Reset pin active ( low level )
static ARMN8Status_t Status = ARMN8_NOT_SETUP;



/*=============================================================================
	Private functions
=============================================================================*/

/*=============================================================================
	ARMN8LW HARDWARE SETUP
	
	called first time ARMN8 Task is used
	
	Configures ARMN8 module IOs and timeout timer
=============================================================================*/
static void ARMN8LWHardwareSetup( void )
{
	u32 TimFreq;																	// Requested timer frequency
	u16 TimPrescaler = 1;

	UartSetup( UART_A2A3, 19200, WordLength8, NO_PARITY, NO_DMA );

	GpioSetup( &RadioRstPin );														// Setup GPIO and start reset

	// Timeout timer
	ARMN8_TIM_PERIPH_ENABLE();
	DBGMCU_Config( ARMN8_TIMER_DBG, ENABLE );										// Stop timer counter when core halted

	ARMN8_TIMER->CR1 = 0;															// stop timer and deinit

	TimFreq = ARMN8_TIMER_FREQ;
	TimPrescaler = ComputeTimPrescaler( ARMN8_TIMER, &TimFreq );
	if ( TimFreq != ARMN8_TIMER_FREQ )
		{ while(1); }	// Should never occur
	if ( TimPrescaler == 0 )
		ARMN8_TIMER->PSC = 0;																// Prescaler too low : set prescaler to maximum speed
	else
		ARMN8_TIMER->PSC = TimPrescaler-1;									// Set prescaler for timer resolution
	ARMN8_TIMER->ARR = 0xFFFF;														// Set timer period
	ARMN8_TIMER->RCR = 0;																	// Repetition counter unused
	ARMN8_TIMER->EGR = TIM_PSCReloadMode_Immediate;				// Reset prescaler and repetition counter internal values
	ARMN8_TIMER->SR = 0;																	// Clear all IRQ flags
	ARMN8_TIMER->DIER = TIM_IT_Update;										// Enable Update IRQ
	ARMN8_TIMER->CR1 = TIM_CR1_ARPE | TIM_CounterMode_Up;	// Configure timer : do not start counting

	// Note : Rst pin must stay at 0 a few microseconds ( Timer init time condidered enough ) ( unspecified by ATIM )
	OutputSet( &RadioRstPin );														// Exit reset mode
}

/*=============================================================================
	Start Timer
	
	Triggers a timeout at now + ms time
=============================================================================*/
static void StartTimer( u16 ms )
{
	ARMN8_TIMER->ARR = ms;															// Reset timer counter
	ARMN8_TIMER->CNT = 0;															// Reset timer counter
	ARMN8_TIMER->SR = 0;															// Clear all IRQ flags
	ARMN8_TIMER->CR1 = TIM_CR1_CEN;													// Start counting
}

/*=============================================================================
	Is Timer Elapsed
	
	returns true if last StartTimer time has elapsed
=============================================================================*/
static bool IsTimerElapsed( void )
{
if ( ARMN8_TIMER->SR & TIM_SR_UIF )
	{
	ARMN8_TIMER->CR1 &= ~TIM_CR1_CEN;											// Time is elaspeds : Disable timer
	return true;
	}
else
	{
	return false;
	}
}

/*=============================================================================
	ARMN8 Task
	
	This task has 5 states and must be called as often as possible ( every 10 ms for example )
	 
	ARMN8_NOT_SETUP : initial state before ARMN IOs, UART and timer are initialized
		ARMN8 immediately and automatically leaves this state to go to ARMN8_POWERING_UP state
	ARMN8_POWERING_UP : initial state while ARMN8 powers up after its reset has been released
		when power up time has elapsed, ARMN8 goes to ARMN8_IDLE state
	ARMN8_IDLE : Default state : no data received and no data being sent, and no timeout pending
		We leave this state when_
		- some data is received ->ARMN8_DATA_READY
		- calling  ARMN8StartSending -> ARMN8_BUSY
	ARMN8_BUSY : ARMN8 enters this state when we start sending some data to the module
		We leave this state when 
		- a full frame is received -> ARMN8_DATA_READY
		- timeout is elapsed -> ARMN8_IDLE
	ARMN8_DATA_READY : A full frame has been received and is ready in RxBuf
		We leave this state when 
		- calling ARMN8PurgeRxBuffer -> ARMN8_IDLE
		- calling ARMN8StartSending -> ARMN8_BUSY
	
	It returns the current ARMN8 state	
=============================================================================*/
ARMN8Status_t ARMN8Task( void )
{
switch( Status )
	{
	case ARMN8_HWRESET_ASKED:
		if (IsTimerElapsed())
			Status = ARMN8_NOT_SETUP;
		break;
	case ARMN8_NOT_SETUP :
		///DEBUG
	//break;
	///
	ARMN8LWHardwareSetup();
	StartTimer( RADIO_START_TIME_MS );
  Status = ARMN8_POWERING_UP;
		break;
	case ARMN8_POWERING_UP :
		if ( IsTimerElapsed() )
			{
			Uart2StartReception();
			Status = ARMN8_IDLE;
			}
		break;
	case ARMN8_IDLE :
		if ( 0 != Uart2RxDataSize() )
			Status = ARMN8_DATA_READY;
		break;
	case ARMN8_BUSY :
		if ( 0 != Uart2RxDataSize() )
			Status = ARMN8_DATA_READY;
		else if ( IsTimerElapsed() )
			{
			Status = ARMN8_IDLE;
			}
		break;
	case ARMN8_DATA_READY :
		if ( 0 == Uart2RxDataSize() )
			Status = ARMN8_IDLE;
		break;
	}
return Status;
}

/*=============================================================================
	ARMN8 Start Sending
	
	Inputs :
		TxBuf : Pointer to the data to transmit
		TxSize : number of bytes to transmit
		Timeoutms : Timeout during which Task stays BUSY if no answer is received
	
	Starts sending a frame to ARMN8 module
	If TxSize != 0, this action purges any data previously received
	
=============================================================================*/
bool ARMN8StartSending( u8 *TxBuf, u8 TxSize , u16 Timeoutms )
{
if ( (Status==ARMN8_IDLE) || (Status==ARMN8_DATA_READY) )
	{
	if ( TxSize != 0 )
		{ 
		Uart2Send( TxBuf, TxSize);
		StartTimer( Timeoutms );
		Status = ARMN8_BUSY;
		}
	return true;
	}
else
	{
	return false;
	}
}

/*=============================================================================
	ARMN8 Rx Data Size
 
	returns 0 until a full frame has been received
	returns the frame size when a full frame has been received.
	
	Note : reception is never stopped. A new frame may overwrite current frame.
=============================================================================*/
u8 ARMN8RxDataSize( void )
{
return Uart2RxDataSize();
}

/*=============================================================================
	ARMN8 Rx Buf Ptr
 
	returns ARMN8 reception buffer address
=============================================================================*/
u8 *ARMN8RxBufPtr( void )
{
return Uart2RxBufPtr();
}

/*=============================================================================
	ARMN8 Purge Rx Buffer
	
	purges Rx buffer. 
	if buffer empty, ARMN8 task swaps from ARMN_DATA_READY to ARMN8_IDLE
=============================================================================*/
void ARMN8PurgeRxBuffer( void )
{
	Uart2PurgeRxBuffer();
}

/*=============================================================================
	ARMN8 Purge Rx Buffer
	



=============================================================================*/
void ARMN8Reset( void )
{
	Status = ARMN8_HWRESET_ASKED;
	GpioSetup( &RadioRstPin );
	StartTimer(ARMN8_RESET_TIME_MS);
}


