#include "definitions.h"
#include "Watchdog.h"
#include "Time.h"
#include "Debug.h"
#include "Display.h"
#include "doDisplay.h"
#include "Radio.h"
#include "Font.h"
#include "Flicker.h"
#include <stdio.h>
#include <string.h>
#include "Uart1.h"
#include "dataStruct.h"
#include "Cfg.h"
#include "Eep25AA.h"
#include "ARMN8LW.h"
#include "time.h"
#include "utils.h"
#include "CCS811.h"
#include "CubeData.h"
#include "dBAMeter.h"
#include "Messaging.h"
#include "ARMN8LW.h"
#include "Init.h"
#include "I2CExtSensors.h"
#include "ColorSensor.h"
#include "crc.h"
#include "Averages.h"

/*==================================================================
	PRIVATE DEFINITIONS
===================================================================*/
#define MAX_MAIN_LOOP_TIME_MS		2000										//(ms) this is the maximum main loop time above which a watchdog reset may occur ( a watchdog reset MUST occur at approximately twice this value )
#define POWER_UP_TIME_MS 1000
/*==================================================================
	PRIVATE FUNCTIONS
===================================================================*/
u32 Wakeup(void);
void Sleep(Time_t Time, CubeData_t* Data);
void DoPostResetInit(CubeData_t *Data);
void UpdatePoll(Time_t Time, bool reset);
u32 PrepareMessage(CubeData_t *Data, Time_t Time);

/*==================================================================
	PRIVATE DATA
===================================================================*/
Averages_t avg;
u8 lastJoinStatus = 0;

/*==================================================================
		MAIN
===================================================================*/
int main(void)
{
	CubeData_t Data;
	Time_t Time;
	Color_t colors;
	CfgConfig_t *CfgConfigPtr;
	u8 noiseValx2 = 0;
	u32 lastTotalSec = 0;
	u32 displayElapsed = 0;
	pmReadResult_t *pmValues;
	
	
	AvgZero(&avg);
	
	#ifndef DEBUG_ON
	SetWatchdog(MAX_MAIN_LOOP_TIME_MS);									// Set maximum Main loop time above which watchdog may reset the processor
	#endif
		
	DebugPinsSetup();
	usTimerSetup();																	// Start us tick Timer for use as a time reference
	EepSetup();
	ARMN8Task();																		//initialize ARMN8 hardware immediately
	WaitMs( POWER_UP_TIME_MS );											// Max power up time valid for all peripherals
	DisplaySetup();

	Time.lastTickus = GetusTick();
	Time.nLoops = 0;
	Time.TotalSec = 0;
		
	//initialize parameters	
	I2CStdSensorsSetup();
	dBAMeterSetup();
	Data.Orientation = UP;
	Data.happy = 0;

	//run initialization
	DoPostResetInit(&Data);

	CfgConfigPtr = CfgConfigPointer();
	#ifdef DEBUG_ON
	CfgConfigPtr->showAirAsValue = false;
  #endif
	
	Time.lastTickus = GetusTick();
	//main loop
	while (1)
	{
		UpdateTime(&Time);
		//Data.LastOrientation = Data.Orientation;
		Data.Orientation = CubeOrientation();
		
			
		
		//if ((Time.TotalSec != lastTotalSec) || (Data.LastOrientation != Data.Orientation))
		if ((Time.TotalSec != lastTotalSec) )
		{
			lastTotalSec = Time.TotalSec;
			
			//read sensors
			if (isExtSensorMounted())
			{
				if (CfgConfigPtr->extSensorType == EXT_SENSOR_CO2)
					Data.CO2ppm = getCO2ppm();
				else if (CfgConfigPtr->extSensorType == EXT_SENSOR_PM)
				{
					pmValues = GetPmValues();
					Data.pm1 = pmValues->pm1x100;
					Data.pm2_5 = pmValues->pm2_5x100;
					Data.pm10 = pmValues->pm10x100;
				}
			}
			Data.VOCppb = TVoc_ppb();
			Data.Lux = Lux();
			Data.Tempx100 = Temperaturex100();
			Data.Hygrx100 = Hygrometryx100();
			colors = GetColors();
			Data.colors.Red = colors.Red;
			Data.colors.Green = colors.Green;
			Data.colors.Blue = colors.Blue;
			Data.colors.WhiteTus = colors.White;
			Data.colors.Flicker = Flicker();
			Data.Flicker = Flicker();
			if (!CfgConfigPtr->disableSound)
			{
				Data.dBAx100 = dBAx100();

				//compute rolling average 
				noiseValx2 = Data.dBAx100 * 2 / 100.0;
				Data.noiseAveragex2 = (noiseValx2 + Data.noiseAveragex2)/2;
			
				//compute max
				if (Data.noiseMaxx2 < noiseValx2)
					Data.noiseMaxx2 = noiseValx2;
			}
			
			//compute global averages
			AddPointToAverage(&Data, &avg);
			
			//display
			doDisplay(&Data, Time);
			
			//messaging
			PrepareMessage(&Data, Time);
			
			//poll management
			UpdatePoll(Time, false);

			Data.LastOrientation = Data.Orientation;
		} //end if (hzEvent)
		
		
		if (!CfgConfigPtr->disableSound)
			dBAUpdate();
		RadioTask();
		if (CfgConfigPtr->extSensorType == EXT_SENSOR_CO2)
			CO2Task();
		if (CfgConfigPtr->extSensorType == EXT_SENSOR_PM)
			ParticleTask();
		
		WatchdogRefresh();
		
	} //end while
}//end main()


void DoPostResetInit(CubeData_t *Data)
{
  CfgStatus_t *CfgStatusPtr;
	CfgPoll_t *CfgPollPtr;
		
		//this is a reboot: initialize modules
		if (Data->isBoardInitialized == false)
		{
			InitializeBoard(Data);
			Data->isBoardInitialized = true;
		}
		
		//perform ARMN8 hardware setup
		//RadioInit();
		
		//draw GreenMe logo ("G")
		ShowGreenMe();
		WaitMs(400);
		Data->Orientation = UP;
		
		//set default values
		Data->happy = 3;														
		Data->LastOrientation = Data->Orientation;

		//cancel poll is needed
		CfgStatusPtr = CfgStatusPointer();
		CfgPollPtr = CfgPollPointer();
		if (CfgPollPtr->PollEndsOnReboot)
		{
			CfgStatusPtr->pollTimeRemaining_hour = 0;
			CfgStatusSaveToEep();
		}
}



u32 PrepareMessage(CubeData_t *Data, Time_t Time)
{
	CfgStatus_t *CfgStatusPtr;
	CfgConfig_t *CfgConfigPtr;
	CfgPoll_t *CfgPollPtr;
	u32 radioQueryStartTick = 0;
	
	CfgStatusPtr = CfgStatusPointer();
	CfgConfigPtr = CfgConfigPointer();
	CfgPollPtr = CfgPollPointer();
	
	//force message as soon as join status has changed
	if (CfgStatusPtr->IsRadioJoined != lastJoinStatus)
	{
		Time.TotalSec = CfgConfigPtr->shortMessageInterval_min * 60;
		lastJoinStatus = CfgStatusPtr->IsRadioJoined;
	}
		
	#ifdef CALIB_MODE
		CfgConfigPtr->shortMessageInterval_min = 2;
	#endif 
		
	//radio messaging
	if (Data->happy != 0)			//this is a feedback message	
	{
		//use rolling average 
		MakeRadioMessage(MSGV2_FEEL, Data, &avg);
		RadioTask();
		
		//reset averages and max 
		AvgReset(&avg, Data);
		Data->noiseMaxx2 = 0;

		//clear feedback
		Data->happy = 0;
		
		//reset poll
		CfgStatusPtr->pollNextStart_hour = CfgPollPtr->pollRepeat_hour;
		CfgStatusSaveToEep();
	}
	else if ((CfgConfigPtr->longMessageInterval_min != 0) &&
					 (Time.TotalSec % (60*CfgConfigPtr->longMessageInterval_min) == 0))
	{
		MakeRadioMessage(MSGV2_FULL, Data, &avg);
		
		//reset averages and max
		AvgReset(&avg, Data);
		Data->noiseMaxx2 = 0;

	}
	else if ((CfgConfigPtr->shortMessageInterval_min != 0) &&
						(Time.TotalSec % (60*CfgConfigPtr->shortMessageInterval_min) == 0))
	{
		MakeRadioMessage(MSGV2_SHORT, Data, &avg);
		RadioTask();
		
		//reset averages and max
		AvgReset(&avg, Data);
		Data->noiseMaxx2 = 0;
	}
	return radioQueryStartTick;
}


/*********************************************************
* Update poll timer and save to eeprom
**********************************************************/
void UpdatePoll(Time_t Time, bool reset)
{
	bool saveToEep = false;
	CfgStatus_t *CfgStatusPtr;
	CfgPoll_t *CfgPollPtr = CfgPollPointer();
	
	CfgStatusPtr = CfgStatusPointer();
	
	//poll: this function must be called at the end ; eeprom needs time to write.
	//CfgStatusPtr = CfgStatusPointer();
	if (Time.TotalSec % 3600 == 0)
	{
		saveToEep = false;
		
		//decrease poll remaining time
		if (CfgStatusPtr->pollTimeRemaining_hour > 0)
		{
			CfgStatusPtr->pollTimeRemaining_hour --;
			saveToEep |= true;
		}
		
		//decrease poll time before next start
		if (CfgStatusPtr->pollNextStart_hour > 1) {
				CfgStatusPtr->pollNextStart_hour --;
				saveToEep |= true;
		}
		else {
			if (CfgStatusPtr->pollNextStart_hour == 1)
			{
				CfgStatusPtr->pollNextStart_hour --;
				CfgStatusPtr->pollNextStart_hour = CfgPollPtr->pollRepeat_hour;
				CfgStatusPtr->pollTimeRemaining_hour = CfgPollPtr->pollDuration_hour;
				saveToEep |= true;
			}
		}
		
		if (saveToEep)
			CfgStatusSaveToEep();
	}
}



