#include "Stm32f0xx.h"
#include "Eep25AA.h"
#include "GPIODef.h"

/*=============================================================================
	EEPROM DEFINITIONS
=============================================================================*/
#define EEP_PAGE_SIZE 	64														// (bytes) write page size
#define EEP_SIZE 		(128*1024/8)											// (bytes) eeprom size

#define EEP_READ_CMD	0x03
#define EEP_WRITE_CMD	0x02
#define EEP_WRDI_CMD	0x04
#define EEP_WREN_CMD	0x06
#define EEP_RDSR_CMD	0x05
	#define EEP_SR_WIP_MSK	0x01
	#define EEP_SR_WEL_MSK	0x02
	#define EEP_SR_BP_MSK	0x0C
	#define EEP_SR_ALL_MSK	0x0F
#define EEP_WRSR_CMD	0x01

/*=============================================================================
	EEPROM SPI DEFINITIONS
=============================================================================*/
#define SPI 	SPI1

/*=============================================================================
	EEPROM PIN DEFINITIONS
=============================================================================*/
#define CS_GPIO			GPIOA	
#define CS_PIN			GPIO_Pin_15								

/*==============================================================================
	MACROS
===============================================================================*/
#define CS_1()				CS_GPIO->BSRR = CS_PIN;
#define CS_0()				CS_GPIO->BRR = CS_PIN;

/*=============================================================================
	PRIVATE DATA
=============================================================================*/
static const GPIODef_t MosiPin 	= { PB5, 	TYPE_SPI1, 	OUT_PP, SPEED_MEDIUM, INIT_0 };
static const GPIODef_t MisoPin 	= { PB4, 	TYPE_SPI1, 	OUT_PP, SPEED_MEDIUM, INIT_0 };
static const GPIODef_t SckPin 		= { PB3, 	TYPE_SPI1, 	OUT_PP, SPEED_MEDIUM, INIT_0 };
static const GPIODef_t CsPin 		= { PA15, 	TYPE_OUTPUT, 	OUT_PP, SPEED_MEDIUM, INIT_1 };

static const SPI_InitTypeDef SPI_InitStructure = 
{
SPI_Direction_2Lines_FullDuplex,
SPI_Mode_Master,
SPI_DataSize_8b,
SPI_CPOL_Low,																	// Idle SCLK state is 0
SPI_CPHA_1Edge,																	// SCLK rising edge validates MOSI
SPI_NSS_Soft,
SPI_BaudRatePrescaler_4,		// SPI Clk = 3MHz = PCLK/16
SPI_FirstBit_MSB,
7	
};

/*=============================================================================
	PRIVATE FUNCTIONS
=============================================================================*/
//static void EepTest( void );

/*=============================================================================
	EEP SETUP
	
	Configures the eeprom GPIOs
=============================================================================*/
void EepSetup( void )
{
// GPIO CONFIGURATION
GpioSetup( &MosiPin );
GpioSetup( &MisoPin );
GpioSetup( &SckPin );
GpioSetup( &CsPin );

// SPI Configuration
SPI_I2S_DeInit(SPI);
SPI_Init(SPI, (SPI_InitTypeDef *)&SPI_InitStructure);
SPI->CR2 |= SPI_CR2_FRXTH;
SPI_Cmd( SPI, ENABLE );															// Enable SPI

CS_0();
SPI_SendData8(SPI, EEP_WRSR_CMD);
SPI_SendData8(SPI, 0);															// Remove Array Write protection if it was enabled
while ( SPI->SR & SPI_SR_BSY );													// Wait untill transmission is finished
while ( SPI->SR & SPI_SR_RXNE )													// fetch last received byte
	{
	SPI_ReceiveData8(SPI);
	}
CS_1();
//EepTest();
}

/*=============================================================================
	Eep Rd Status
	
	returns the eeprom status; FF in case of error
=============================================================================*/
static u8 EepRdStatus( void )
{
CS_0();
while ( SPI->SR & SPI_SR_RXNE )													// purge Rx FIFO
	SPI_ReceiveData8(SPI);
// Note minimum 100ns between CS falling edge and CLK rising edge : 3 cycles at 24MHz : OK
SPI_SendData8(SPI, EEP_RDSR_CMD);
SPI_SendData8(SPI, 0xFF);
while ( SPI->SR & SPI_SR_BSY );													// Wait untill transmission is finished
SPI_ReceiveData8(SPI);															// Dump first byte
CS_1();
if ( SPI->SR & SPI_SR_RXNE )													// fetch last received byte
	return SPI_ReceiveData8(SPI);
else
	return 0xFF;																// Error case
}

/*=============================================================================
	EEP Page Read
	
	Inputs :
		DstPtr : destination address pointer
		ReadAddr : Eeprom read address start
		Size : Number of bytes to copy
	
	returns true if operation succeeded
	
	Copies Size bytes from Eeprom Address Addr to DstPtr area
	
=============================================================================*/
bool EepPageRead( u8 *DstPtr, u8 PageNb, u8 PageStartPos, u8 NbBytesToRead )
{
u16 StartAddr = PageNb*EEP_PAGE_SIZE + PageStartPos;

if ( NbBytesToRead == 0 )														// This is useful to always have a continuous SPI read with no time lost between bytes
	return true;
else if ( StartAddr + NbBytesToRead > EEP_SIZE )
	return false;

// Wait untill last write command completes
while ( EEP_SR_WIP_MSK & EepRdStatus() );	

// Read Data
CS_0();
// Note minimum 100ns between CS falling edge and CLK rising edge
SPI_SendData8(SPI, EEP_READ_CMD);
SPI_SendData8(SPI, StartAddr/0x100);
SPI_SendData8(SPI, StartAddr&0xFF);
while ( SPI->SR & SPI_SR_BSY );									// Wait until last transmission is finished
while ( SPI->SR & SPI_SR_RXNE )													// dump receive FIFO
	{	SPI_ReceiveData8(SPI);	}

SPI_SendData8(SPI, 0xFF);
while ( --NbBytesToRead != 0 )
	{
	SPI_SendData8(SPI, 0xFF);
	while ( 0 == (SPI->SR & SPI_SR_RXNE) );
	*DstPtr++ = SPI_ReceiveData8(SPI);
	}
while ( SPI->SR & SPI_SR_BSY );									// Wait until last transmission is finished
if ( SPI_SR_FRLVL_0 != (SPI->SR & SPI_SR_FRLVL) )
	return false;																// Rx FIFO should hold exacly 1 byte at this stage
else
	*DstPtr++ = SPI_ReceiveData8(SPI);
CS_1();

return true;
}

/*=============================================================================
	EEP Page Write
	
	Inputs :
		DstPtr : destination address pointer
		Addr : Eeprom read address start
		Size : Number of bytes to copy
		
	returns true if operation succeeded

	Copies Size bytes from Eeprom Address Addr to DstPtr area
	
	Max duration if no eeprom write pending when called : 200us @ HCLK24MHz; SCLK 3MHz
	
=============================================================================*/
bool EepPageWrite( u8 PageNb, u8 PageStartPos, u8 *SrcPtr, u8 NbBytesToWrite )
{
u16 StartAddr = PageNb*EEP_PAGE_SIZE + PageStartPos;

if ( NbBytesToWrite == 0 )
	return true;
else if ( StartAddr + NbBytesToWrite > EEP_SIZE )
	return false;

// Wait until last write command completes
while ( EEP_SR_WIP_MSK & EepRdStatus() );	

// Write Enable Eep										
CS_0();
SPI_SendData8(SPI, EEP_WREN_CMD);
while ( SPI->SR & SPI_SR_BSY );									// Wait until transmission is finished
CS_1();

// Write Data to Eep										
CS_0();
SPI_SendData8(SPI, EEP_WRITE_CMD);
SPI_SendData8(SPI, StartAddr/0x100);
SPI_SendData8(SPI, StartAddr&0xFF);
while ( NbBytesToWrite-- != 0 )
	{
	while ( SPI->SR & SPI_SR_FTLVL_1 );											// Wait until some room is available in SPI FIFO
	SPI_SendData8(SPI, *SrcPtr++ );
	}
while ( SPI->SR & SPI_SR_BSY );									// Wait until transmission is finished
CS_1();

// Note : Write enable will be automatically switched off when page write completes : max 5ms after CS raises
return true;
}


/*=============================================================================
	EEP Test
=============================================================================*/
/*
#include <string.h>
static void EepTest( void )
{
const u8 TestData[128] = {
 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
 0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
 0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
 0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
 0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
 0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
 0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
 0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,0x7F
};
u8 Data[64];

// ecriture 64 octets page 0 puis immediatement ecriture 64 octets page 255
EepPageWrite( 0, 0, (u8 *)&TestData[0], 64 );
EepPageWrite( 255, 0, (u8 *)&TestData[64], 64 );

// Lecture page 0 puis comparaison
EepPageRead( &Data[0], 0, 0, 64 );
if ( 0 != memcmp( &TestData[0], &Data[0], 64 ) )
	{ while(1); }

// Lecture page 255 puis comparaison
EepPageRead( &Data[0], 255, 0, 64 );
if ( 0 != memcmp( &TestData[64], &Data[0], 64 ) )
	{ while(1); }
	
// Ecriture puis lecture 1 octet en position 0 page 0
EepPageWrite( 0, 0, (u8 *)&TestData[10], 1 );
EepPageRead( (u8 *)&Data[0], 0, 0, 3 );
if ( Data[0] != TestData[10] || Data[1] != TestData[1] )
	{ while (1); }

// Ecriture puis lecture 3 octet en position 61 page 255
EepPageWrite( 255, 61, (u8 *)&TestData[10], 3 );
EepPageRead( (u8 *)&Data[0], 255, 61, 3 );
if ( Data[0] != TestData[10] || Data[1] != TestData[11] || Data[2] != TestData[12] ) 
	{ while (1); }	// Erreur de Verification

// Lecture de l'octet précédent la zone que l'on vient d'écrire
EepPageRead( (u8 *)&Data[0], 255, 60, 1 );
if ( Data[0] != TestData[64+60] )
	{while (1); }

}
*/
