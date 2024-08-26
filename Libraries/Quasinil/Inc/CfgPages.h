#ifndef CFG_FLASH_PAGES_H
#define CFG_FLASH_PAGES_H

#include "StdTypes.h"
#include "FlashWr.h"

typedef struct
{
u32 U32[DATA_FLASH_WORD32_SIZE-1];
u32 ValidCfgTag;
}CfgPage_t;

/*==============================================================================
	FUNCTIONS PROTOTYPES
===============================================================================*/
bool CfgFlashDataRead( void );													// Reads flash data from currently valid flash data page
bool CfgFlashDataUpdate( void );												// Updates flash data so that current valid flash data page equals data in RAM pointed by Ptr
bool CfgFlashPagesTest( void );													// Reserved for module tests
bool IsCfgFlashUpToDate(  void );
void SetVolatileCfgAddress( void *NewCfgPtr );

#endif // CFG_FLASH_PAGES_H
