#include "Init.h"
#include "CubeData.h"
#include "Cfg.h"
#include "Time.h"
#include "definitions.h"
#include "doDisplay.h"
#include "ARMN8LW.h"
#include "Radio.h"
#include "string.h"
#include "CCS811.h"
#include "Watchdog.h"
#include <stdio.h>
#include "CCS811.h"
#include "I2CExtSensors.h"



/*********************************************************************
* Initialize board (ARMN8 setup, get version numbers, init EEPROM)
**********************************************************************/
void InitializeBoard(CubeData_t *Data)
{
	CfgId_t *CfgIdPtr;
	CfgCalib_t *CfgCalibPtr;
	CfgConfig_t *CfgConfigPtr;
	CfgStatus_t *CfgStatusPtr;
	CfgPoll_t *CfgPollPtr;
	char str[32] ;
	u32 startTick;
	
	CfgIdPtr = CfgIdPointer();
		
	#ifdef DEBUG_MODE
	if (true)
	//if (CfgIdPtr->fwVersion != FWVERSION)
	#else
	if (CfgIdPtr->fwVersion != (u16) FWVERSION) 
	//if (true)
	#endif
	{
		CfgCalibPtr  = CfgCalibPointer();
		CfgConfigPtr = CfgConfigPointer();
		CfgStatusPtr = CfgStatusPointer();
		CfgPollPtr = CfgPollPointer();
		
		//sprintf(str, "%2d %2d %2d %2d", sizeof(CfgCalib_t), sizeof(CfgConfig_t), sizeof(CfgStatus_t), sizeof(CfgDisplay_t));
		//DisplaySingleString(str, 11);

		DisplaySingleString("starting setup", 14);
		CfgStatusPtr->IsRadioConfigured = false;
		DisplaySingleString("radio setup", 11);
		
		//long wait ; we want to prevent join collision
		sprintf(str, "board init");
		DisplaySingleString(str, 13);
		startTick = GetusTick();
		WaitMs(100);
		while (Elapsed_us(startTick) < 5*1000000)
		{
			RadioTask();
			WaitMs(1);
			WatchdogRefresh();
		}
		
		//by now radio should be started up
		CfgIdPtr->armn8FwVersion = RadioGetFwVersion();
		if (CfgIdPtr->armn8FwVersion < REQUIRED_ARMN8_MIN_FW)
		{
			DisplaySingleString("fw: ATIM", 8);
			while (1)
			{
				WatchdogRefresh();
			}
		}
		
		DisplaySingleString("CCS811 setup", 12);
		WaitMs(10);
		//read CCS811 version
		CfgIdPtr->CCS811FwVersion = CCS811_GetFwVersion();
		if (CfgIdPtr->CCS811FwVersion < REQUIRED_CCS811_MIN_FW)
		{
			DisplaySingleString("fw: CCS", 7);
			while (1)
			{
				WatchdogRefresh();
			}
		}
		
		WatchdogRefresh();
		DisplaySingleString("EEP setup", 9);
		WaitMs(10);
		DisplaySingleString("configuring", 10);
		WaitMs(10);
		//read board firmware version
		CfgIdPtr->fwVersion = (u16) FWVERSION;
		CfgConfigPtr->shortMessageInterval_min =  10;
		CfgConfigPtr->longMessageInterval_min =  60;
		CfgConfigPtr->eventMode = 0;
		
		CfgConfigPtr->showTemp 	= true;
		CfgConfigPtr->showHygr 	= true;
		CfgConfigPtr->showLux 	= true;
		CfgConfigPtr->showNoise = true;
		CfgConfigPtr->showAir 	= true;
		CfgConfigPtr->disableVOC 	= false;	//TODO: unused ?
		CfgConfigPtr->disableSound 	= false; 
		CfgConfigPtr->showAirAsValue = false;
		
		
		CfgConfigPtr->lang = FR_FR;
		//TODO for next major update: add custom images
		CfgConfigPtr->imgToggleLeft = 0;		//default : happy
		CfgConfigPtr->imgToggleRight = 0;		//default : unhappy
		CfgConfigPtr->imgToggleBack = 0;		//default : none
		CfgConfigPtr->text_toggle_left[0] = 0;
		CfgConfigPtr->text_toggle_right[0] = 0;
		CfgConfigPtr->text_acknowledgment[0] = 0;
		CfgConfigPtr->toggleMode = TOGGLE_MODE_IMG;
		
		CfgConfigPtr->alertMode = 0;
		CfgConfigPtr->alertThresholdx100 = 0;
		CfgConfigPtr->extSensorType = EXT_SENSOR_NONE;
		if (isExtSensorMounted())
			CfgConfigPtr->extSensorType = EXT_SENSOR_CO2;		//default to CO2 sensor
		

		CfgCalibPtr->luxGainx100 =        (s16)  		1*100;
		CfgCalibPtr->deltaTx100 =         (s16)     0*100;	
		CfgCalibPtr->deltaHx100 = (s16) 0;
		CfgCalibPtr->AudioCalibOffsetx100 = 0;
		
		CfgStatusPtr->IsBoardInitialized = true;
		CfgStatusPtr->pollNextStart_hour = 0xff;		//max value: never starts
		CfgStatusPtr->pollTimeRemaining_hour = 0;
		CfgPollPtr->pollDuration_hour = 0;	//never
		CfgPollPtr->pollRepeat_hour = (u8) 7*24; //1 week
		CfgPollPtr->PollEndsOnReboot = true;
		CfgPollPtr->pollEndsOnToggle = true;
		CfgPollPtr->pollMode = POLLMODE_TEXT;
		CfgPollPtr->text[0]  = 0;
		
		//save config
		CfgStatusSaveToEep();
		CfgIdSaveToEep();
		CfgCalibSaveToEep();
		CfgConfigSaveToEep();
		CfgPollSaveToEep();
		if (!CheckConf())
		{
			DisplaySingleString("setup failed", 12);
			while (1)
			{
				WatchdogRefresh();
			}
		}
		
		DisplaySingleString("setup done.", 11);
		WaitMs(500);
		
		//TODO: self-test here ? (temperature, etc. ?)
	}
}





