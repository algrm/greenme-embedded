#ifndef ARMN8LW_H
#define ARMN8LW_H

#include "StdTypes.h"

typedef enum { ARMN8_NOT_SETUP, ARMN8_POWERING_UP, ARMN8_IDLE, ARMN8_BUSY, ARMN8_DATA_READY, ARMN8_HWRESET_ASKED }ARMN8Status_t;

ARMN8Status_t ARMN8Task( void );												// ARMN8 Task status update
bool ARMN8StartSending( u8 *TxBuf, u8 TxSize , u16 Timeoutms );					// Sends a frame and switches ARMN8 to BUSY untill either an answer is received or timeout is elapsed
u8 ARMN8RxDataSize( void );														// Returns Current Rx Buffer size
u8 *ARMN8RxBufPtr( void );														// Returns Rx Buffer data pointer
void ARMN8PurgeRxBuffer( void );												// switches ARMN8 from DATA_RECEIVED to IDLE
void ARMN8Reset(void);

#endif //ARMN8LW_H
