#ifndef DMA_H
#define DMA_H

/*=============================================================================
	DMA DEFINITIONS
=============================================================================*/

#define ADC_DMA_CHANNEL_BASE				DMA1_Channel1_BASE

#define UART1_TX_DMA_CHANNEL_BASE		DMA1_Channel2_BASE

#define I2C1_RX_DMA_CHANNEL_BASE			DMA1_Channel3_BASE		// à mettre à jour dans le code à la prochaine modif

#define I2C2_TX_DMA_CHANNEL_BASE			DMA1_Channel4_BASE

#define SPI2_TX_DMA_CHANNEL_BASE			DMA1_Channel5_BASE



// DMA CCR REGISTER ADDED DEFINITIONS
#define DMA_CCR_DIR_MEM_TO_PERIPHERAL		DMA_CCR_DIR
#define DMA_CCR_DIR_PERIPHERAL_TO_MEM		0

#define DMA_CCR_PINC_DISABLED				0
#define DMA_CCR_PINC_ENABLED				DMA_CCR_PINC

#define DMA_CCR_MINC_DISABLED				0
#define DMA_CCR_MINC_ENABLED				DMA_CCR_MINC

#define DMA_CCR_CIRC_DISABLED				0
#define DMA_CCR_CIRC_ENABLED				DMA_CCR_CIRC

#define DMA_CCR_PSIZE_8BITS					0
#define DMA_CCR_PSIZE_16BITS				DMA_CCR_PSIZE_0
#define DMA_CCR_PSIZE_32BITS	            DMA_CCR_MSIZE_1

#define DMA_CCR_MSIZE_8BITS					0
#define DMA_CCR_MSIZE_16BITS				DMA_CCR_MSIZE_0
#define DMA_CCR_MSIZE_32BITS				DMA_CCR_MSIZE_1

#define DMA_CCR_PL_VERY_HIGH				DMA_CCR_PL
#define DMA_CCR_PL_HIGH						DMA_CCR_PL_1
#define DMA_CCR_PL_MEDIUM					DMA_CCR_PL_0
#define DMA_CCR_PL_LOW						0

#define DMA_CCR_M2M_DISABLED				0
#define DMA_CCR_M2M_ENABLED					DMA_CCR_MEM2MEM





#endif
