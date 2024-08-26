#ifndef UART1_H
#define UART1_H

#include "Uarts.h"

void Uart1Send( u8 *BufPtr, u16 UartSendSize );									// Sends UartSendSize bytes from BufPtr memory
void Uart1StartReception( u8 *BufPtr, u16 RxMaxSize );							// Starts reception of a RxMaxSize frame
u16 Uart1RxDataSize( void );													// Received data size
bool IsUart1TxComplete( void );													// returns true when UART is not transmitting

#endif //UART1_H
