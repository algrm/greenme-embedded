/*==============================================================================
	IMPORTANT NOTE
	Cfg0 and Cfg1 are set to a fixed address
	Option Target IROM1 must stop before this specific flash area to prevent the 
	linker from putting some code or init data in this area.
===============================================================================*/
#include "CfgPages.h"
#include <string.h>
#include "Cfg.h"
#include "DefaultCfg.h"

/*==============================================================================
	PRIVATE DEFINITIONS
===============================================================================*/
#define PROG_TAG		0x55555555												// Tells that the corresponding flash page holds some valid configuration data
#define TAG_SIZE		4

/*==============================================================================
	PRIVATE VARIABLES
===============================================================================*/
static Cfg_t *CfgPtr;

/*==============================================================================
	DATA FLASH UPDATE

	Inputs :
		Ptr : Pointer to the RAM data area that we want to save
		Size : Size of the RAM data area
	Retval :
		true if the Data in RAM was successfully saved to flash memory
		false otherwise
		
	Saves Data from RAM to free configuration flash page

	Max execution time : 77ms = 1ms + max page erase time 40ms + 256*141us
===============================================================================*/
bool CfgFlashDataUpdate( void )
{
if ( CfgPage[0].ValidCfgTag == PROG_TAG )
	{																			// Page 0 holds some valid data
	if ( 0 == memcmp( CfgPtr, (void *)CfgPage[0].U32, sizeof(*CfgPtr) ) )
		{
		return true;															// Page 0 is already up to date
		}
	else
		{ 																		// Data in page 0 is different from data in RAM
		FlashPageErase( &CfgPage[1].ValidCfgTag );
		FlashPageErase( &CfgPage[1].ValidCfgTag );
		FlashProgram( CfgPage[1].U32, CfgPtr->U32, sizeof(*CfgPtr) );			// Copy RAM data to flash memory
		if ( 0 == memcmp( CfgPage[1].U32, CfgPtr->U32, sizeof(*CfgPtr) ) )
			{
			if ( FlashWord32Write( PROG_TAG, &CfgPage[1].ValidCfgTag ) )
				{
				if ( FlashPageErase( &CfgPage[0].ValidCfgTag ) &&
					 FlashPageErase( &CfgPage[0].ValidCfgTag ))	// Erase page 0 that is now no longer usefull
					{ return true; }
				}
			}
		}
	}
else if ( CfgPage[1].ValidCfgTag == PROG_TAG )
	{																			// Page 1 holds some valid data
	if ( 0 == memcmp( CfgPtr, CfgPage[1].U32, sizeof(*CfgPtr) ) )
		{
		return true;
		}
	else
		{ 																		// Data in page 1 is different from data in RAM
		FlashPageErase( &CfgPage[0].ValidCfgTag );
		FlashPageErase( &CfgPage[0].ValidCfgTag );
		FlashProgram( CfgPage[0].U32, CfgPtr->U32, sizeof(*CfgPtr) );			// Copy RAM data to flash memory
		if ( 0 == memcmp( CfgPage[0].U32, CfgPtr, sizeof(*CfgPtr) ) )
			{
			if ( FlashWord32Write( PROG_TAG, &CfgPage[0].ValidCfgTag ) )
				{
				if ( FlashPageErase( &CfgPage[1].ValidCfgTag ) &&
					 FlashPageErase( &CfgPage[1].ValidCfgTag ))	// Erase page 1 that is now no longer usefull
					{ return true; }
				}
			}
		}
	}
else
	{																			// No valid data page
	FlashPageErase( &CfgPage[1].ValidCfgTag );
	FlashPageErase( &CfgPage[1].ValidCfgTag );
	FlashProgram( CfgPage[1].U32, CfgPtr->U32, sizeof(*CfgPtr) );	// Copy RAM data to flash memory
	if ( 0 == memcmp( CfgPage[1].U32, CfgPtr, sizeof(*CfgPtr) ) )	
		{
		if ( FlashWord32Write( PROG_TAG, &CfgPage[1].ValidCfgTag ) )
			{
			if ( FlashPageErase( &CfgPage[0].ValidCfgTag ) &&
				 FlashPageErase( &CfgPage[0].ValidCfgTag ))		// Erase page 0 that is now no longer usefull
				{ return true; }
			}
		}
	}
return false;
}

/*==============================================================================
	DATA FLASH READ

	Inputs :

	Retval : true:if copy was successfull false otherwise

	Reads some data in flash memory and copies it to RAM.
	If unused page las 32 bit word is not erased, erase the whole unused flash page

	Max execution time : not critical
===============================================================================*/
bool CfgFlashDataRead( void )
{
if ( CfgPage[0].ValidCfgTag == PROG_TAG )
	{
	memcpy( CfgPtr, CfgPage[0].U32, sizeof(*CfgPtr) );
	if ( CfgPage[1].ValidCfgTag != PROG_TAG )											// This is in case a reset occured while writing to flash, before page erasure
		{
		FlashPageErase( CfgPage[1].U32 );
		FlashPageErase( &CfgPage[1].ValidCfgTag );
		}
	return true;
	}
else if ( CfgPage[1].ValidCfgTag == PROG_TAG )
	{
	memcpy( CfgPtr, CfgPage[1].U32, sizeof(*CfgPtr) );
	if ( CfgPage[0].ValidCfgTag != PROG_TAG )											// This is in case a reset occured while writing to flash, before page erasure
		{
		FlashPageErase( CfgPage[0].U32 );
		FlashPageErase( &CfgPage[0].ValidCfgTag );
		}
	return true;
	}
else
	{																		// No valid data page, try nevertheless to read data from page 0
	memcpy( CfgPtr, CfgPage[0].U32, sizeof(*CfgPtr) );
	return false;
	}
}

/*==============================================================================
	 Is Cfg Flash Up to date

	Inputs :
		Ptr : Pointer to the RAM data area where data has to be copied to
	Retval : true:if copy was successfull false otherwise

	Reads some data in flash memory and copies it to RAM.
	If unused page las 32 bit word is not erased, erase the whole unused flash page

	Max execution time : not critical
===============================================================================*/
bool IsCfgFlashUpToDate( void )
{
if ( CfgPage[0].ValidCfgTag == PROG_TAG )
	{
	if ( 0 == memcmp( CfgPage[0].U32, CfgPtr, sizeof(*CfgPtr) ) )
		return true;
	}
else
	{
	if ( 0 == memcmp( CfgPage[1].U32, CfgPtr, sizeof(*CfgPtr) ) )
		return true;
	}
return false;
}

/*==============================================================================
	Set Volatile Cfg Address
===============================================================================*/
void SetVolatileCfgAddress( void *NewCfgPtr )
{
CfgPtr = (Cfg_t *)NewCfgPtr;
}
