#ifndef STM32F0_H
#define STM32F0_H

#include "StdTypes.h"

/*==============================================================================
	Added definitions to original STM32F0xx.h file
==============================================================================*/
#include "stm32f0xx.h"

/*==============================================================================
	Added exported types
==============================================================================*/
typedef struct
{
  __IO uint32_t CCR;          /*!< DMA channel x configuration register                                           */
  __IO uint32_t CNDTR;        /*!< DMA channel x number of data register                                          */
  __IO uint32_t CPAR;         /*!< DMA channel x peripheral address register                                      */
  __IO uint32_t CMAR;         /*!< DMA channel x memory address register                                          */
  __IO uint32_t Reserved;
}DMA_Ch_t;

typedef struct
{
__IO uint32_t ISR;          /*!< DMA interrupt status register,                            Address offset: 0x00 */
__IO uint32_t IFCR;         /*!< DMA interrupt flag clear register,                        Address offset: 0x04 */
__IO DMA_Ch_t Ch[8]; 
__IO struct
	{
	u32 C1S : 4;
	u32 C2S : 4;
	u32 C3S : 4;
	u32 C4S : 4;
	u32 C5S : 4;
	u32 C6S : 4;
	u32 C7S : 4;
	u32 Reserved : 4;
	}CSELR;         	// DMA channel x selection register
}DMA_t;

typedef struct
{
__IO u16 CR1;      /*!< SPI Control register 1 (not used in I2S mode),       Address offset: 0x00 */
u16 RESERVED0;    /*!< Reserved, 0x02                                                            */
__IO u16 CR2;      /*!< SPI Control register 2,                              Address offset: 0x04 */
u16  RESERVED1;    /*!< Reserved, 0x06                                                            */
__IO u16 SR;       /*!< SPI Status register,                                 Address offset: 0x08 */
u16  RESERVED2;    /*!< Reserved, 0x0A                                                            */
union
	{
	__IO u16 DR16;       /*!< SPI data register,                              Address offset: 0x0C */
	__IO u8  DR8;
	}DR;
u16  RESERVED3;    /*!< Reserved, 0x0E                                                            */
__IO u16 CRCPR;    /*!< SPI CRC polynomial register (not used in I2S mode),  Address offset: 0x10 */
u16  RESERVED4;    /*!< Reserved, 0x12                                                            */
__IO u16 RXCRCR;   /*!< SPI Rx CRC register (not used in I2S mode),          Address offset: 0x14 */
u16  RESERVED5;    /*!< Reserved, 0x16                                                            */
__IO u16 TXCRCR;   /*!< SPI Tx CRC register (not used in I2S mode),          Address offset: 0x18 */
u16  RESERVED6;    /*!< Reserved, 0x1A                                                            */ 
__IO u16 I2SCFGR;  /*!< SPI_I2S configuration register,                      Address offset: 0x1C */
u16  RESERVED7;    /*!< Reserved, 0x1E                                                            */
__IO u16 I2SPR;    /*!< SPI_I2S prescaler register,                          Address offset: 0x20 */
u16  RESERVED8;    /*!< Reserved, 0x22                                                            */    
}SPI_t;

typedef struct
{                           
__IO u32 TR;         /*!< RTC time register,                                        Address offset: 0x00 */
__IO u32 DR;         /*!< RTC date register,                                        Address offset: 0x04 */
__IO u32 CR;         /*!< RTC control register,                                     Address offset: 0x08 */
__IO u32 ISR;        /*!< RTC initialization and status register,                   Address offset: 0x0C */
__IO u32 PRER;       /*!< RTC prescaler register,                                   Address offset: 0x10 */
u32 RESERVED0;  /*!< Reserved,                                                 Address offset: 0x14 */
u32 RESERVED1;  /*!< Reserved,                                                 Address offset: 0x18 */
__IO u32 ALRMAR;     /*!< RTC alarm A register,                                     Address offset: 0x1C */
u32 RESERVED2;  /*!< Reserved,                                                 Address offset: 0x20 */
__IO u32 WPR;        /*!< RTC write protection register,                            Address offset: 0x24 */
__IO u32 SSR;        /*!< RTC sub second register,                                  Address offset: 0x28 */
__IO u32 SHIFTR;     /*!< RTC shift control register,                               Address offset: 0x2C */
__IO u32 TSTR;       /*!< RTC time stamp time register,                             Address offset: 0x30 */
__IO u32 TSDR;       /*!< RTC time stamp date register,                             Address offset: 0x34 */
__IO u32 TSSSR;      /*!< RTC time-stamp sub second register,                       Address offset: 0x38 */
__IO u32 CAL;        /*!< RTC calibration register,                                 Address offset: 0x3C */
__IO u32 TAFCR;      /*!< RTC tamper and alternate function configuration register, Address offset: 0x40 */
__IO u32 ALRMASSR;   /*!< RTC alarm A sub second register,                          Address offset: 0x44 */
u32 RESERVED3;  /*!< Reserved,                                                 Address offset: 0x48 */
u32 RESERVED4;  /*!< Reserved,                                                 Address offset: 0x4C */
__IO u32 BKP[5];																																// RTC Backup registers
}RTC_t;

/*==============================================================================
	Added memory map
==============================================================================*/

#undef FLASH_BASE
#define FLASH_BASE            	0x08000000 										// redefined for #if comparisons compatibility

#undef SRAM_BASE
#define SRAM_BASE             	0x20000000 										// redefined for #if comparisons compatibility

#undef PERIPH_BASE
#define PERIPH_BASE           	0x40000000 										// redefined for #if comparisons compatibility

// Added Peripheral memory map
#define DMA2_BASE			(AHBPERIPH_BASE + 0x400)
	#define DMA2_Channel1_BASE	(DMA2_BASE + 0x00000008)
	#define DMA2_Channel2_BASE	(DMA2_BASE + 0x0000001C)
	#define DMA2_Channel3_BASE	(DMA2_BASE + 0x00000030)
	#define DMA2_Channel4_BASE	(DMA2_BASE + 0x00000044)
	#define DMA2_Channel5_BASE	(DMA2_BASE + 0x00000058)

#define DMA2                ((DMA_t *) DMA2_BASE)
#define DMA2_Channel1       ((DMA_Channel_TypeDef *) DMA2_Channel1_BASE)
#define DMA2_Channel2       ((DMA_Channel_TypeDef *) DMA2_Channel2_BASE)
#define DMA2_Channel3       ((DMA_Channel_TypeDef *) DMA2_Channel3_BASE)
#define DMA2_Channel4       ((DMA_Channel_TypeDef *) DMA2_Channel4_BASE)
#define DMA2_Channel5       ((DMA_Channel_TypeDef *) DMA2_Channel5_BASE)

/*==============================================================================
	Peripheral redefinitions
===============================================================================*/
#undef SPI1
#define SPI1				((SPI_t *) SPI1_BASE)

#undef SPI2
#define SPI2				((SPI_t *) SPI2_BASE)

#undef  DMA1
#define DMA1            ((DMA_t *) DMA1_BASE)

#undef RTC
#define RTC					((RTC_t *) RTC_BASE)

/*==============================================================================
	Added Registers bits and bit groups definitions
===============================================================================*/
// GPIO
#define GPIO_MODER_IN			0
#define GPIO_MODER_OUT			1
#define GPIO_MODER_AF			2
#define GPIO_MODER_AN			3

#define GPIO_OTYPER_OT_PP		0
#define GPIO_OTYPER_OT_OD		1

#define GPIO_OSPEEDR_MEDIUM		1
#define GPIO_OSPEEDR_FAST		2
#define GPIO_OSPEEDR_HIGH		3

#define GPIO_PUPDR_NO_PULL		0
#define GPIO_PUPDR_PULL_UP		1
#define GPIO_PUPDR_PULL_DOWN	2

// RTC
#define RTC_TAFCR_PC15MODE					0x00800000
#define RTC_TAFCR_PC15VALUE				0x00400000
#define RTC_TAFCR_PC14MODE					0x00200000
#define RTC_TAFCR_PC14VALUE				0x00100000
#define RTC_TAFCR_PC13MODE					0x00080000
#define RTC_TAFCR_PC13VALUE				0x00040000

// RCC
#define RCC_AHBENR_DMA2EN					0x00000002        					// DMA2 clock enable

#define RCC_CFGR3_USART1SW_PCLK				0
#define RCC_CFGR3_USART1SW_SYSCLK			(RCC_CFGR3_USART1SW_0)
#define RCC_CFGR3_USART1SW_LSE				(RCC_CFGR3_USART1SW_1)
#define RCC_CFGR3_USART1SW_HSI				(RCC_CFGR3_USART1SW_1 | RCC_CFGR3_USART1SW_0)

//USART
#define USART_CR1_PCE_PARITY_DISABLED				0
#define USART_CR1_PCE_PARITY_ENABLED				USART_CR1_PCE

#define USART_CR1_PS_EVEN							0
#define USART_CR1_PS_ODD							USART_CR1_PS

#define USART_CR2_STOP_1_BIT						0
#define USART_CR2_STOP_2_BITS						USART_CR2_STOP_1
#define USART_CR2_STOP_1_BIT_5						(USART_CR2_STOP_0 | USART_CR2_STOP_1)

// ADC
#define  ADC_CFGR2_ADCCLK_MASK      				0xC0000000
#define  ADC_CFGR2_ADCCLK_14MHZ      				0x00000000
#define  ADC_CFGR2_ADCCLK_PCLKDIV2    			0x40000000
#define  ADC_CFGR2_ADCCLK_PCLKDIV4      		0x80000000

// SPI
#define SPI_CR1_PCLK_DIV2							0x0000
#define SPI_CR1_PCLK_DIV4							0x0008
#define SPI_CR1_PCLK_DIV8							0x0010
#define SPI_CR1_PCLK_DIV16							0x0018
#define SPI_CR1_PCLK_DIV32							0x0020
#define SPI_CR1_PCLK_DIV64							0x0028
#define SPI_CR1_PCLK_DIV128						0x0030
#define SPI_CR1_PCLK_DIV256						0x0038

#define SPI_CR2_4BITS								0x0300
#define SPI_CR2_5BITS								0x0400
#define SPI_CR2_6BITS								0x0500
#define SPI_CR2_7BITS								0x0600
#define SPI_CR2_8BITS								0x0700
#define SPI_CR2_9BITS								0x0800
#define SPI_CR2_10BITS								0x0900
#define SPI_CR2_11BITS								0x0A00
#define SPI_CR2_12BITS								0x0B00
#define SPI_CR2_13BITS								0x0C00
#define SPI_CR2_14BITS								0x0D00
#define SPI_CR2_15BITS								0x0E00
#define SPI_CR2_16BITS								0x0F00

#endif


