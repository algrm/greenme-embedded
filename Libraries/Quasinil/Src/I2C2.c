#include "Stm32f0xx.h"
#include "I2CPrivate.h"
#include "I2C2.h"
#include "Debug.h"

#define I2C	I2C2

/*=============================================================================
	I2C TIMING CONFIGURATION

	SCLK Period = 10us = SCLL=5us + tSync1=4/4MHz=500ns + SCLH=4us + tsync2=4/4MHz
	SCLK Freq = 100kHz

=============================================================================*/
#define I2C_TX_DMA_CHANNEL			DMA1_Channel4


/*=============================================================================
	PRIVATE DATA
=============================================================================*/
u8 *RxDataPtr;																	// Data address where IRQ should copy received data when reception is done
u8 *RxSize;																		// RxSize address that sould be updated when reception is done

/*=============================================================================
	I2C IRQ Handler

	Triggered only by reception IRQ
	if no error, stores received data to destination address and increments 
		RxSize
=============================================================================*/
void I2C2_IRQHandler( void )
{
u8 NBytes = (I2C->CR2 & I2C_CR2_NBYTES)>>16;

if ( I2C->ISR & (I2C_ISR_BERR | I2C_ISR_NACKF) )
	{
	if ( I2C->RXDR );	// Dump received data
	}
else if ( *RxSize < NBytes )
	{
	RxDataPtr[(*RxSize)++] = I2C->RXDR;
	}
}

/*=============================================================================
	I2C2 Setup
	
=============================================================================*/
bool I2C2Setup( I2CPins_t I2CPins, I2CBaudRate_t BaudRate )
{
const DMA_InitTypeDef DMACfg = 
{
(u32)(&I2C->TXDR),			// Peripheral Address
0,							// Memory base address ( unloaded at this stage
DMA_DIR_PeripheralDST,		// Memory to perpheral transfer
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
const NVIC_InitTypeDef I2C2IrqCfg = { I2C2_IRQn, 3, ENABLE };

I2C_DeInit(I2C);															// Reset all I2C registers to default ( among which Clock stretching enabled )
if ( I2CPinSetup(I2C, I2CPins) )
	{
	// I2C Configuration
	I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;			// Suppresses 50ns spikes
	I2C_InitStructure.I2C_DigitalFilter = 0x00;								// No digital filter
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	// I2C2 is clocked by PCLK
	I2C_InitStructure.I2C_Timing = BaudRate;
	I2C_Init(I2C, &I2C_InitStructure);
	
	// Disable I2C IRQ
	I2C->CR1 &= ~I2C_IT_TXI;												// Disable I2C Transmit IRQ
	I2C->CR1 &= ~I2C_IT_RXI;												// Disable I2C Receive IRQ

	NVIC_Init((NVIC_InitTypeDef *)&I2C2IrqCfg);
	
	I2C_DMACmd(I2C, I2C_DMAReq_Tx, ENABLE);								// Enable DMA Requests for transmission

	DMA_DeInit( I2C_TX_DMA_CHANNEL );  
	DMA_Init( I2C_TX_DMA_CHANNEL, (DMA_InitTypeDef *)&DMACfg);

	I2C->CR1 |= I2C_CR1_PE;												// Enable I2C

	return true;
	}
else
	{
	return false;
	}
}

/*=============================================================================
	I2C2 Start Write
	
=============================================================================*/
void I2C2StartWrite( u8 SlaveAddr, u8 *DataPtr, u8 DataSize )
{
I2C_TransferHandling(I2C, SlaveAddr<<1, DataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Write);	// Configure slave address, nbytes, reload and generate start

if ( DataSize != 0 )
	{
	I2C_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;										// Disable DMA while reconfiguring DMA
	I2C_TX_DMA_CHANNEL->CMAR = (u32)DataPtr;									// Set Dma memory start address
	I2C_TX_DMA_CHANNEL->CNDTR = DataSize;										// Load DMA transfer size
	I2C_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;										// Enable DMA transfer
	}
}

/*=============================================================================
	I2C2 Read
=============================================================================*/
void I2C2StartRead( u8 SlaveAddr, u8 *DataPtr, u8 *DataSize )
{
I2C->CR1 &= ~I2C_CR1_PE;														// Disable I2C and clear all errors
RxDataPtr = DataPtr;
I2C->CR1 |= I2C_CR1_PE;															// Enable I2C
I2C_TransferHandling(I2C, SlaveAddr<<1, *DataSize, I2C_AutoEnd_Mode, I2C_Generate_Start_Read );	// Configure slave address, nbytes, reload and generate start
RxSize = DataSize;																// This where IRQ will store the number of bytes received
*RxSize = 0;																	// RxSize is 0 untill recpetion is done
I2C->CR1 |= I2C_IT_RXI;															// Enable Receive IRQ
}

