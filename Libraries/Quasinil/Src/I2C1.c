#include "Stm32f0xx.h"
#include "I2CPrivate.h"
#include "I2C1.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/
#define I2C	I2C1

#define I2C1_RX_DMA_CHANNEL		DMA1_Channel3

/*=============================================================================
	PRIVATE DATA
=============================================================================*/
u8 TxDataSize = 0;
u8 *TxDataPtr = 0;

/*=============================================================================
	I2C1 Setup
	
	
=============================================================================*/
bool I2C1Setup( I2CPins_t I2CPins, I2CBaudRate_t BaudRate )
{
static const DMA_InitTypeDef DMARxCfg = 
{
(u32)(&I2C->RXDR),			// Peripheral Address
0,							// Memory base address ( unloaded at this stage
DMA_DIR_PeripheralSRC,		// Peripheral to memory transfer
0,							// BUffer size empty at this stage
DMA_PeripheralInc_Disable,
DMA_MemoryInc_Enable,
DMA_PeripheralDataSize_HalfWord,
DMA_MemoryDataSize_Byte,
DMA_Mode_Normal,
DMA_Priority_Low,
DMA_M2M_Disable
};

I2C_InitTypeDef  I2C_InitStructure; 

if ( I2CPinSetup( I2C1, I2CPins ) )
	{
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;				// Suppresses 50ns spikes
	I2C_InitStructure.I2C_DigitalFilter = 0x00;									// No digital filter
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_DeInit(I2C);															// Reset all I2C registers to default ( among which Clock stretching enabled )
	RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
	I2C_InitStructure.I2C_Timing = BaudRate;
	I2C_Init(I2C, &I2C_InitStructure);
	
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;											// Enable DMA clock
	I2C->CR1 |= I2C_CR1_RXDMAEN;												// Enable DMA Requests for reception

	DMA_DeInit( I2C1_RX_DMA_CHANNEL );  
	DMA_Init( I2C1_RX_DMA_CHANNEL, (DMA_InitTypeDef *)&DMARxCfg);

	I2C->CR1 |= I2C_CR1_PE;														// Enable I2C
	return true;
	}
else
	{
	return false;
	}
}

/*=============================================================================
	I2C1 Start Write
	
=============================================================================*/
void I2C1StartWrite( u8 SlaveAddr, u8 *DataPtr, u8 DataSize )
{
I2C->CR1 &= ~I2C_CR1_PE;													// Disable I2C and clear all errors
I2C->CR1 |= I2C_CR1_PE;														// Enable I2C

I2C_TransferHandling(I2C, SlaveAddr<<1, DataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);	// Configure slave address, nbytes, reload and generate start

TxDataSize = DataSize;
TxDataPtr = DataPtr;
}

/*=============================================================================
	I2C1 Start Write _ NO STOP
	
=============================================================================*/
void I2C1StartWriteNoStop( u8 SlaveAddr, u8 *DataPtr, u8 DataSize )
{
	I2C->CR1 &= ~I2C_CR1_PE;													// Disable I2C and clear all errors
	I2C->CR1 |= I2C_CR1_PE;														// Enable I2C

	I2C_TransferHandling(I2C, SlaveAddr<<1, DataSize, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);	// Configure slave address, nbytes, reload and generate start

	TxDataSize = DataSize;
	TxDataPtr = DataPtr;
}

/*=============================================================================
	Is I2C1 Write Done
	
=============================================================================*/
bool IsI2C1WriteDone( void )
{
if ( I2C->ISR & I2C_ISR_STOPF )
	return true;
else if ( I2C->ISR & I2C_ISR_TXIS )
	{
	if ( TxDataSize != 0 )
		{
		TxDataSize--;
		I2C->TXDR = *TxDataPtr++;
		return false;
		}
//	else if ( I2C->ISR & I2C_ISR_BUSY )
//		return false;
	else
		return true;
	}
else
	{
	return false;
	}
}

/*=============================================================================
	I2C1 Start Read

	Starts reading NewDataSize Bytes to DataPtr from SlaveAddr
	
	This function launches the Read.
	It exits while read is in progress
	Use I2C1RxDataReady to check for completion
=============================================================================*/
void I2C1StartRead( u8 SlaveAddr, u8 *DataPtr, u8 NewDataSize )
{
if ( NewDataSize != 0 )
	{
	I2C->CR1 &= ~I2C_CR1_PE;													// Disable I2C and clear all errors
	I2C->CR1 |= I2C_CR1_PE;														// Enable I2C

	I2C1_RX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;									// Disable DMA while reconfiguring DMA
	I2C1_RX_DMA_CHANNEL->CMAR = (u32)DataPtr;									// Set Dma memory start address
	I2C1_RX_DMA_CHANNEL->CNDTR = NewDataSize;										// Load DMA transfer size
	I2C1_RX_DMA_CHANNEL->CCR |= DMA_CCR_EN;										// Enable DMA transfer

	I2C_TransferHandling(I2C, SlaveAddr<<1, NewDataSize, I2C_AutoEnd_Mode,
													I2C_Generate_Start_Read );	// Configure slave address, nbytes, reload and generate start
	}
}

/*=============================================================================
	I2C1 Rx Data Ready
	
	returns true if read transfer is finished
=============================================================================*/
bool I2C1RxDataReady( void )
{
if ( I2C1_RX_DMA_CHANNEL->CNDTR == 0 )
	return true;
else
	return false;
}
