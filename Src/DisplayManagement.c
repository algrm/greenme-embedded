#include "DisplayManagement.h"
#include "StdTypes.h"
#include "Main.h"
#include "Display.h"
#include <string.h>
#include <stdio.h>
#include "Font.h"
#include "Time.h"
#include "utils.h"
#include "Main.h"
#include "Radio.h"
#include "definitions.h"

const u16  DISPLAY_REFRESH_DELAY_MS = 4000;						//(ms) default delay between two screen refresh


const char* temperature_label = "Temp.";
const char* temperature_unit = "degC";
const char* humidity_label = "Hum.";
const char* humidity_unit = "%HR";
const char* noise_label = "Bruit";
const char* noise_unit = "dBA";
const char* lux_label = "Lum.";
const char* lux_unit = "lux";
const char* klux_unit = "kLux";
const char* air_label = "Air";
const char* airVOC_unit = "COV";
const char* airCO2_unit = "CO2";

//private definitions
u8 displayMode = DISPLAY_MEASURES;
u8 lastDisplayMode = 0;
static u8 happyStatus;
char strBuffer[32];
static u32 happyTimer;
//static u8 iMeasure = 0;
u32 lastDisplayRefreshTick = 0;

static enum { TEMP, HUM, LIGHT, NOISE, AIR } activeDisplay = TEMP;

/**
Set current language
********************/
void initLocale()
{
	
	switch (boardConfig.lang)
	{
		case EN_GB:
			noise_label = "Noise";
			lux_label 	= "Lighting";
			airVOC_unit 		= "TVOC";
			humidity_unit = "%RH";
			break;
		case EN_US:
			noise_label = "Noise";
			lux_label 	= "Lighting";
			airVOC_unit 		= "TVOC";
			temperature_unit = "F";
			humidity_unit 	 = "%RH";
			break;
		default:
			//default is FR_FR;
			 temperature_label = "Temp.";
			 temperature_unit = "degC";
			 humidity_label = "Hum.";
			 humidity_unit = "%HR";
			 noise_label = "Bruit";
			 noise_unit = "dBA";
			 lux_label = "Lum.";
			 lux_unit = "lux";
			 klux_unit = "kLux";
			 air_label = "Air";
			 airVOC_unit = "COVT";
				airVOC_unit = "COV";
	}
}

/************************************************************
************************************************************/
void doDisplay(CubeData_t* Data, u8 lastOrientation, u8 warmingUp)
{
	switch (Data->Orientation)
	{
		case UPSIDE_DOWN:
			//display config, just once
				DisplaySetAngle(ANGLE_90);
				displayMode = DISPLAY_CONFIG;

			break;
		case UP:
				DisplaySetAngle(ANGLE_270);
				
				if ((boardConfig.pollUserEnabled == true) && (boardConfig.runtimeWithoutPoll_s > boardConfig.pollInterval_s))
				{
					displayMode = DISPLAY_QUESTION;
				}
				else if (Data->msgLength > 0)
				{
					displayMode = DISPLAY_MESSAGE;
				
				}
				else
				{
					displayMode = DISPLAY_MEASURES;
				}
			break;
		case TILTED_LEFT:		//on left side
			if (lastOrientation != TILTED_LEFT) {
				DisplaySetAngle(ANGLE_180);
				displayMode = DISPLAY_HAPPY;
			}
			break;
		case TILTED_RIGHT:			//on right side
			if (lastOrientation != TILTED_RIGHT) {
				DisplaySetAngle(ANGLE_0);
				displayMode = DISPLAY_UNHAPPY;
			}
			break;
		case TILTED_FRONT:			//on front side
			if (lastOrientation != TILTED_FRONT) {
				DisplaySetAngle(ANGLE_0);
				displayMode = DISPLAY_MEASURES;
			}
			break;
		case TILTED_BACK:			//on back side
			if (lastOrientation != TILTED_BACK) {
				DisplaySetAngle(ANGLE_0);
				displayMode = DISPLAY_MEASURES;
			}
			break;
		default:
			if (displayMode != DISPLAY_MEASURES)
			{
				displayMode = DISPLAY_MEASURES;
			}
		
	}
	
	////////////////////////////////////
	//DISPLAY_MEASURES, DISPLAY_CONFIG, DISPLAY_HAPPY, DISPLAY_UNHAPPY
	// 
	switch (displayMode)
	{
		case DISPLAY_MEASURES:
			DisplayMeasures(warmingUp, Data);
			break;
		case DISPLAY_HAPPY:
			ShowHappy(true, lastOrientation, Data);
			break;
		case DISPLAY_UNHAPPY:
			ShowHappy(false, lastOrientation, Data);
			break;
		case DISPLAY_CONFIG:
			if (displayMode != lastDisplayMode)
			{
				DisplayConfig();
			}
			break;
		case DISPLAY_QUESTION :
			if (displayMode != lastDisplayMode)
			{
				DisplayQuestion();
			}
			break;
		case DISPLAY_MESSAGE :
			//if (displayMode != lastDisplayMode)
			//{
				DisplayMessage(Data);
			//}
			break;
	}
	lastDisplayMode = displayMode;
}

/**********************************************
DisplayConfig
show configuration (id, fw number)
***********************************************/
void DisplayConfig()
{
	char* serial = RadioGetDevEUI();

	DisplayBlack();
	strBuffer[8] = '\0';
	sprintf(strBuffer, "radio id:");
	DrawText(strBuffer, true, 0, 0);
	
	memcpy(strBuffer, serial+8, 4);
	strBuffer[4] = '\0';
	DrawText(strBuffer, false, 28, 16);
	memcpy(strBuffer, serial+12, 4);
	strBuffer[4] = '\0';
	DrawText(strBuffer, false, 56, 16);
	
	sprintf(strBuffer, "rev %.2f", FWVERSION);
	DrawText(strBuffer, true, 100, 0);
	
	DisplayRefresh();
}


/*********************************************************
* left / right toggle interaction
**********************************************************/
void ShowHappy(bool happy, u8 lastOrientation, CubeData_t* Data)
{
	u32 elapsed;
	if ((Data->Orientation != lastOrientation) )
	{
		//first time : init timer
		happyStatus = HAPPY_STATUS_START;
		happyTimer = GetusTick();
		DisplayBlack();
		if (displayMode == DISPLAY_HAPPY)
		{
			DrawHappy(true);
		}
		else {
			DrawHappy(false);
		}
	}
	
	elapsed = GetElapsedus(happyTimer);
	
	if (elapsed > 7000000) {
		displayMode = DISPLAY_MEASURES;
		DisplaySetAngle(ANGLE_270);
	}
	else if (elapsed > 5000000){
		if (happyStatus != 0){
			setFeel(happy, GetusTick());		//update feel value in main process
			DisplayBlack();
			DrawOK();
			DisplayRefresh();
			happyStatus = 0;
		}
	}
	else if ((elapsed > (HAPPY_STATUS_START - happyStatus)*1000000))
	{
		//display count down
		DrawEmpty(0, 104, 20, 20);
		sprintf(strBuffer, "%d", (happyStatus -1));
		DrawText(strBuffer, true, 104, 4);
		DisplayRefresh();
		happyStatus--;
	}
}


/****************************************************
affichage des mesures sur l'écran toutes les x secondes
****************************************************/
void DisplayMeasures(u8 warmingUp, CubeData_t* Data )
{
	u8 airQuality;
	u32 elapsed_us = GetElapsedus(lastDisplayRefreshTick);
	bool doRefresh = false;
	//float val;
	

	//// display refresh
	if (elapsed_us > DISPLAY_REFRESH_DELAY_MS*1000) {

		
		
		//do not display temperature if we still are in warm up 
		if (warmingUp && activeDisplay == TEMP)
		{
			activeDisplay = NOISE;
		}
		

		//change display
		if (activeDisplay == TEMP)
		{
			if (boardConfig.showTemp){
				DisplayBlack();
				DrawText((char*) temperature_label, true, 4, 2);
				DrawText((char*) temperature_unit, false, 128 - 24, 2);
				if (boardConfig.lang == EN_US)
					DrawMainNumber(floatToString(strBuffer, Data->Tempx100/100.0*9/5.0+32));
				else
					DrawMainNumber(floatToString(strBuffer, Data->Tempx100/100.0));
				doRefresh = true;
			}
			
		}
		else if (activeDisplay == HUM){
			if (boardConfig.showHygr) 
			{
				DisplayBlack();
				DrawText((char*) humidity_label, true, 4, 2);
				DrawText((char*) humidity_unit, false, 128 - 24, 2);
				DrawMainNumber(floatToString(strBuffer, Data->Hygrx100/100.00));
				doRefresh = true;
			}
		}
		else if (activeDisplay == NOISE){
			if (boardConfig.showNoise) 
			{
				DisplayBlack();
				DrawText((char*) noise_label, true, 4, 2);
				DrawText((char*) noise_unit, false, 128 - 24, 2);
				DrawMainNumber(floatToString(strBuffer, Data->dBA/100.0));
				doRefresh = true;
			}
			
		}
		else if (activeDisplay == LIGHT){
			if (boardConfig.showLux) 
			{
				DisplayBlack();
				DrawText((char*) lux_label, true, 4, 2);
				if (Data->Lux < 1000) {
					DrawText((char*) lux_unit, false, 128 - 24, 2);
					DrawMainNumber(floatToString(strBuffer, Data->Lux));
				}
				else {
					DrawText((char*) klux_unit, false, 128 - 24, 2);
					DrawMainNumber(floatToString(strBuffer, Data->Lux/1000.0));
				}
				doRefresh = true;
			}
			
		}
		else if (activeDisplay == AIR){
			if (boardConfig.showAir) 
			{
				if (isCO2Mounted())
				{
					if (Data->CO2ppm > 1000)
						airQuality = 1;
					else if (Data->CO2ppm > 600)
						airQuality = 2;
					else 
						airQuality = 3;
					
					DisplayBlack();
					DrawText((char*) air_label, true, 4, 2);
					DrawText((char*) airCO2_unit, false, 128 - 24, 2);
					DrawStars(airQuality);
				}
				else {
					if (Data->eCO2ppm > 1500)
						airQuality = 1;
					else if (Data->eCO2ppm > 800)
						airQuality = 2;
					else 
						airQuality = 3;
					
					DisplayBlack();
					DrawText((char*) air_label, true, 4, 2);
					DrawText((char*) airVOC_unit, false, 128 - 24, 2);
					DrawStars(airQuality);
				}
				doRefresh = true;

			}
			
		}

		if (doRefresh) {
			if (!HasJoinedNetwork())
				DrawNoSignal();
			DisplayRefresh();
			lastDisplayRefreshTick = GetusTick();
		}
		
		
		activeDisplay++;
		
		if (activeDisplay > 4) {
			activeDisplay = TEMP;
		}
	}

}

void DisplayQuestion()
{
	DisplayBlack();
	DrawQuestion();
	DisplayRefresh();
}

void DisplayMessage(CubeData_t *Data)
{
	u32 elapsed_us = GetElapsedus(lastDisplayRefreshTick);

	//// display refresh
	if (elapsed_us > DISPLAY_REFRESH_DELAY_MS*1000) {	
		DisplayBlack();
		DrawText((char*) Data->msgContent, true, 4, 2);
		lastDisplayRefreshTick = GetusTick();
		DisplayRefresh();
	}
}

u8 getDisplayMode()
{
	return displayMode;
}

void setDisplayMode(u8 mode)
{
	displayMode = mode;
}

u8 getLastDisplayRefreshTick()
{
	return lastDisplayRefreshTick;
}

void setLastDisplayRefreshTick(u32 ticks)
{
	lastDisplayRefreshTick = ticks;
}
