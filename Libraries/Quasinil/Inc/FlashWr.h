#ifndef FLASHWR_H
#define FLASHWR_H

#include "StdTypes.h"
#include <stdint.h>

#define DATA_FLASH_WORD32_SIZE	(u16)0x100

extern bool FlashPageErase( const void *Ptr );										// Erases the page pointed to by Ptr
extern void FlashPageSave( uint32_t *Ptr );										// Saves to RAM flash page pointed to by Ptr
extern bool FlashProgram( const u32 *Dest, uint32_t *Src, uint16_t NbWords32 );// Programs Nb 32 bit Words from Src to Des
extern bool FlashProgramFromSaved( uint32_t *Dest, uint16_t NbWord32 );		// Programs Nb 32 bit words from last saved Dest address to Dest Address
extern bool FlashWord32Write( uint32_t Data, const uint32_t *Dest );			// Programs one 32 bit Data word at Dest address
extern bool FlashWord16Write( uint16_t Data, const uint16_t *Dest );			// Programs one 16 bit Data word at Dest address
extern bool FlashWrTest( void );												// Reserved for module testing

#endif	// FLASHWR_H
