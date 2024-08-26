#ifndef UARTS_H
#define UARTS_H

#include "GPIODef.h"

typedef enum { UART1, UART2 }UartNb_t;
typedef enum { UART_A2A3, UART_A9A10, UART_A14A15, UART_B6B7, NB_UART_PIN_CFG	}UartPins_t;
typedef enum{ NO_PARITY, PARITY_EVEN, PARITY_ODD}UartParity_t;					// Allowed parity types
typedef enum{ WordLength8, WordLength9}UartWordLength_t;						// Allowed number of bits ( mind : 9 bits when using parity )
typedef enum{ NO_DMA=0, DMA_RX=1, DMA_TX=2, DMA_RX_TX=3 }UartDMA_t;				// Use of DMA

bool UartSetup( UartPins_t UartPins, u32 BaudRate, UartWordLength_t WordLength, UartParity_t Parity, UartDMA_t DMA );// Configures the serial link

#endif // UARTS_H

