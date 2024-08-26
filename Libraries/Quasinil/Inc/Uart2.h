#ifndef UART2_H
#define UART2_H

#include "Uarts.h"

void Uart2Send( u8 *BufPtr, u8 UartSendSize );									// Sends UartSendSize bytes from BufPtr memory
void Uart2StartReception( void );							// Starts reception of a RxMaxSize frame
u8 Uart2RxDataSize( void );														// Returns 0 untill a complete frame has been received, then, returns the frame size.
bool IsUart2TxComplete( void );													// returns true when UART is not transmitting
void Uart2PurgeRxBuffer( void );
u8 *Uart2RxBufPtr( void );

#endif //UART2_H
