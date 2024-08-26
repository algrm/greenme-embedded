#include <string.h>
#include <math.h>
#include <stm32f0xx.h>
#include "Eep25AA.h"
#include "Cfg.h"
#include "Time.h"
#include "Debug.h"



/*==============================================================================
	PRIVATE VARIABLES
===============================================================================*/
Cfg_t Cfg;																		// Product's configuration
CfgStatus_t CfgStatus;		//16 bytes
CfgId_t			CfgId;				//4 bytes
CfgCalib_t  CfgCalib;			//16 bytes
CfgConfig_t CfgConfig;		//12 bytes
CfgPoll_t CfgPoll;	//32 bytes


CfgStatus_t *CfgStatusPointer( void )
{
	static bool CfgInit = false;
	if ( !CfgInit )
		{
		if ( !EepPageRead((u8*) &CfgStatus, CFGSTATUS_PAGE_NB ,0, sizeof(CfgStatus) ) )
			{ while(1); } // Error reading eeprom : force reset
		CfgInit = true;
		}
	return &CfgStatus;
}

CfgId_t *CfgIdPointer( void )
{
	static bool CfgInit = false;
	if ( !CfgInit )
		{
		if ( !EepPageRead((u8*) &CfgId, CFGID_PAGE_NB ,0, sizeof(CfgId) ) )
			{ while(1); } // Error reading eeprom : force reset
		CfgInit = true;
		}
	return &CfgId;
}

CfgCalib_t *CfgCalibPointer( void )
{
	static bool CfgInit = false;
	if ( !CfgInit )
		{
		if ( !EepPageRead((u8*) &CfgCalib, CFGCALIB_PAGE_NB ,0, sizeof(CfgCalib) ) )
			{ while(1); } // Error reading eeprom : force reset
		CfgInit = true;
		}
	return &CfgCalib;
}

CfgConfig_t *CfgConfigPointer( void )
{
	static bool CfgInit = false;
	if ( !CfgInit )
		{
		if ( !EepPageRead((u8*) &CfgConfig, CFGCONFIG_PAGE_NB ,0, sizeof(CfgConfig) ) )
			{ while(1); } // Error reading eeprom : force reset
		CfgInit = true;
		}
	return &CfgConfig;
}

CfgPoll_t *CfgPollPointer( void )
{
	static bool CfgInit = false;
	if ( !CfgInit )
		{
		if ( !EepPageRead((u8*) &CfgPoll, CFGPOLL_PAGE_NB ,0, sizeof(CfgPoll) ) )
			{ while(1); } // Error reading eeprom : force reset
		CfgInit = true;
		}
	return &CfgPoll;
}

bool saveConfig()
{
	bool result = true;
	
	result &= EepPageWrite( CFGSTATUS_PAGE_NB, 0, (u8*) (&CfgStatus), sizeof(CfgStatus_t) );
	result &= EepPageWrite( CFGID_PAGE_NB,     0, (u8*) (&CfgId), sizeof(CfgId_t) );
	result &= EepPageWrite( CFGCONFIG_PAGE_NB, 0, (u8*) (&CfgConfig), sizeof(CfgConfig_t) );
	result &= EepPageWrite( CFGCALIB_PAGE_NB,  0, (u8*) (&CfgCalib), sizeof(CfgCalib_t) );
	result &= EepPageWrite( CFGPOLL_PAGE_NB,   0, (u8*) (&CfgPoll), sizeof(CfgPoll_t) );

	
	return result;
}

bool CfgStatusSaveToEep(void)
{
	return EepPageWrite( CFGSTATUS_PAGE_NB, 0, (u8*) (&CfgStatus), sizeof(CfgStatus_t) );
}
	
bool CfgIdSaveToEep(void)
{
	return EepPageWrite( CFGID_PAGE_NB, 0, (u8*) (&CfgId), sizeof(CfgId_t) );
}
bool CfgCalibSaveToEep(void)
{
	return EepPageWrite( CFGCALIB_PAGE_NB, 0, (u8*) (&CfgCalib), sizeof(CfgCalib_t) );
}
bool CfgConfigSaveToEep(void)
{
	return EepPageWrite( CFGCONFIG_PAGE_NB, 0, (u8*) (&CfgConfig), sizeof(CfgConfig_t) );
}
bool CfgPollSaveToEep(void)
{
	return EepPageWrite( CFGPOLL_PAGE_NB, 0, (u8*) (&CfgPoll), sizeof(CfgPoll_t) );
}



/*==============================================================================
	Check that config is properly written.
==============================================================================*/
bool CheckConf(void)
{
	u8 buffer[64];
	bool success = true;
	
	if ( !EepPageRead((u8*) &buffer, CFGSTATUS_PAGE_NB ,0, sizeof(CfgStatus) ) )
	{ while(1); } // Error reading eeprom : force reset
	if (0 != memcmp(buffer, &CfgStatus, sizeof(CfgStatus)))
		success = false;
	
	if (success)
	{
		if ( !EepPageRead((u8*) &buffer, CFGCONFIG_PAGE_NB ,0, sizeof(CfgConfig) ) )
		{ while(1); } // Error reading eeprom : force reset
		if (0 != memcmp(buffer, &CfgConfig, sizeof(CfgConfig)))
			success = false;
	}
	if (success)
	{
		if ( !EepPageRead((u8*) &buffer, CFGCALIB_PAGE_NB ,0, sizeof(CfgCalib) ) )
		{ while(1); } // Error reading eeprom : force reset
		if (0 != memcmp(buffer, &CfgCalib, sizeof(CfgCalib)))
			success = false;
	}

	if (success)
	{

		if ( !EepPageRead((u8*) &buffer, CFGPOLL_PAGE_NB ,0, sizeof(CfgPoll) ) )
		{ while(1); } // Error reading eeprom : force reset
		if (0 != memcmp(buffer, &CfgPoll, sizeof(CfgPoll)))
			success = false;
	}
	return success ;
}
