#include "stm32f0xx.h"
#include "FlashWr.h"

/*==============================================================================
	PRIVATE DEFINITIONS
===============================================================================*/
#define MAX_WORD32_PROG_SIZE  	(u16)0x400  									// 2 KBytes

/*==============================================================================
	PRIVATE VARIABLES
===============================================================================*/

/*==============================================================================
	Flash Page Erase

	Input : FlashPtr : Pointer to flash memory page that needs to be erased
	Retval : true:flash page was successfully erased	false otherwise

	Erases required flash page
	
	Execution time <= 40ms ( flash page erasure time )
==============================================================================*/
bool FlashPageErase( const void *Ptr )
{
uint8_t NbTry = 3;
bool Result = false;
uint32_t *FlashPagePtr;

FlashPagePtr = (uint32_t *)((uint32_t)Ptr & ~(DATA_FLASH_WORD32_SIZE*4-1));		// Compute page start address

while ( Result == false && NbTry-- != 0 )
	{
	FLASH_Unlock();  															// Unlock the Flash Program Erase controller
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);		// Clear All pending flags
	if ( FLASH_COMPLETE == FLASH_ErasePage( (u32)FlashPagePtr ) )
		Result = true;
	FLASH_Lock();
	}
return Result;
}

/*==============================================================================
	Flash Program

	Inputs : 
		Dest : 32 bit aligned pointer to flash memory that must be written.	
			range : all 32 bit aligned flash addresses
		Src : 32 bits aligned pointer to data memory from which data will be copied
		NbWords32 : number of 32 bit words to copy to flash memory [ 0 <-> Flash page size ]
	Output : None
	Retval : true if Flash memory data equals Src memory data

	Copies the required number of 32 bits words from RAM to flash memory
	Verifies after programming that dest data equals source data
	
	returns false if ...
		- Word size is too big
		- Dest is not 32 bit word aligned
		- Program verify fails
	
	Execution time  ~= 16 bit programming time(70us) * Nb words + glue
		1 word < 142us
		16 words < 2,3ms
		256 words < 37ms
==============================================================================*/
bool FlashProgram( const u32 *Dest, uint32_t *Src, uint16_t NbWords32 )
{
uint32_t *U32Src;
const u32 *U32Dst;
const u32 *U32DstEnd = Dest + NbWords32;

if ( NbWords32 > MAX_WORD32_PROG_SIZE )
	return false;															// Size error
if ( (uint32_t)Dest & 0x00000003 != 0 )
	return false;															// Word32 alignment error

/////////// Program ////////
FLASH_Unlock();  															// Unlock the Flash Program Erase controller
FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);	// Clear All pending flags
U32Src = Src;
for ( U32Dst = Dest; U32Dst < U32DstEnd; U32Dst++ )
	{
	FLASH_ProgramWord( (u32)U32Dst, *U32Src++ );
	}
FLASH_Lock();

////////// Verify /////////
while ( Dest < U32DstEnd )
	{
	if ( *Dest++ != *Src++ )
		return false;
	}
return true;
}

/*==============================================================================
	Flash Word32 Write

	Input : Dest : pointer to destination flash memory 32 bit word
	Retval : Dest memory was successfully written to

	Writes Data to required address memory and checks that this word was 
	successfully written

	Max Execution time : 141us = 2*70us + glue
==============================================================================*/
bool FlashWord32Write( uint32_t Data, const uint32_t *Dest )
{
FLASH_Unlock();  															// Unlock the Flash Program Erase controller
FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);	// Clear All pending flags
FLASH_ProgramWord( (u32)Dest, Data );
FLASH_Lock();
if ( *Dest == Data )
	return true;
else
	return false;
}

/*==============================================================================
	Flash Word16 Write

	Input : Dest : pointer to destination flash memory 16 bit word ( 16 bit aligned )
	Retval : true if wrtie was successfully verified
			false if alignment error or program error or verify error

	Writes Data to required address memory and checks that this word was 
	successfully written

	Max Execution time : 72us = 70us + glue
==============================================================================*/
bool FlashWord16Write( uint16_t Data, const uint16_t *Dest )
{
if ( (uint32_t)Dest & 0x00000001 != 0 )
	return false;															// Word16 alignment error
	
FLASH_Unlock();  															// Unlock the Flash Program Erase controller
FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);	// Clear All pending flags
FLASH_ProgramHalfWord( (u32)Dest, Data );
FLASH_Lock();
if ( *Dest == Data )
	return true;
else
	return false;
}











//#include "FlashWr.tst"													// Include this file only when using module test function
