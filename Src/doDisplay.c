#include "doDisplay.h"
#include "Time.h"
#include "Display.h"
#include "Font.h"
#include <stdio.h>
#include <string.h>
#include "CubeData.h"
#include "definitions.h"
#include "Radio.h"
#include "Cfg.h"

/*==================================================================
	PRIVATE DEFINITIONS
===================================================================*/
#define  DISPLAY_REFRESH_DELAY_MS	1000										//(ms) default delay between two screen refresh

#define HAPPY_STATUS_START 6
#define FEEL_UNKNOWN 	0x00
#define FEEL_HAPPY 		0x01
#define FEEL_UNHAPPY 	0x10



/*==================================================================
	PRIVATE FUNCTIONS
===================================================================*/
static void DisplayMeasures( CubeData_t *Data, u8 Sec);
static void DisplayConfig(void);
static void ShowCountDown(bool happy, CubeData_t *data);
static void DisplayMessage(u8 *str);
static void ShowToggle(CubeData_t* Data);
static bool ShowAlert(CubeData_t* Data);




/*==================================================================
	PRIVATE DATA
===================================================================*/
static const u8 FR_FR_tem_labl[] = {0x54,0x65,0x6D,0x70,0x82,0x72,0x61,0x74,0x75,0x72,0x65,0x00};
static const u8 FR_FR_tem_unit[] = {0xA6,0x43,0x00};
static const u8 FR_FR_hum_labl[] = {0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x82,0x00};
static const u8 FR_FR_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 FR_FR_noi_labl[] = {0x42,0x72,0x75,0x69,0x74,0x00};
static const u8 FR_FR_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 FR_FR_lux_labl[] = {0x90,0x63,0x6C,0x61,0x69,0x72,0x65,0x6D,0x65,0x6E,0x74,0x00};
static const u8 FR_FR_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 FR_FR_air_labl[] = {0x41,0x69,0x72,0x00};
static const u8 FR_FR_air_unit[] = {0x43,0x4F,0x56,0x20,0x72,0x65,0x6C,0x2E,0x00};
static const u8 FR_FR_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 FR_FR_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 EN_GB_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x00};
static const u8 EN_GB_tem_unit[] = {0xA6,0x43,0x00};
static const u8 EN_GB_hum_labl[] = {0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x00};
static const u8 EN_GB_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 EN_GB_noi_labl[] = {0x4E,0x6F,0x69,0x73,0x65,0x00};
static const u8 EN_GB_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 EN_GB_lux_labl[] = {0x4C,0x69,0x67,0x68,0x74,0x69,0x6E,0x67,0x00};
static const u8 EN_GB_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 EN_GB_air_labl[] = {0x41,0x69,0x72,0x00};
static const u8 EN_GB_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 EN_GB_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 EN_GB_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 EN_US_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x65,0x00};
static const u8 EN_US_tem_unit[] = {0xA6,0x46,0x00};
static const u8 EN_US_hum_labl[] = {0x48,0x75,0x6D,0x69,0x64,0x69,0x74,0x79,0x00};
static const u8 EN_US_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 EN_US_noi_labl[] = {0x4E,0x6F,0x69,0x73,0x65,0x00};
static const u8 EN_US_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 EN_US_lux_labl[] = {0x4C,0x69,0x67,0x68,0x74,0x69,0x6E,0x67,0x00};
static const u8 EN_US_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 EN_US_air_labl[] = {0x41,0x69,0x72,0x00};
static const u8 EN_US_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 EN_US_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 EN_US_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 DE_DE_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x00};
static const u8 DE_DE_tem_unit[] = {0xA6,0x43,0x00};
static const u8 DE_DE_hum_labl[] = {0x46,0x65,0x75,0x63,0x68,0x74,0x69,0x67,0x6B,0x65,0x69,0x74,0x00};
static const u8 DE_DE_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 DE_DE_noi_labl[] = {0x54,0x6F,0x6E,0x00};
static const u8 DE_DE_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 DE_DE_lux_labl[] = {0x42,0x65,0x6C,0x65,0x75,0x63,0x68,0x74,0x75,0x6E,0x67,0x00};
static const u8 DE_DE_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 DE_DE_air_labl[] = {0x4C,0x75,0x66,0x74,0x00};
static const u8 DE_DE_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 DE_DE_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 DE_DE_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 NL_NL_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x75,0x72,0x00};
static const u8 NL_NL_tem_unit[] = {0xA6,0x43,0x00};
static const u8 NL_NL_hum_labl[] = {0x56,0x6F,0x63,0x68,0x74,0x69,0x67,0x68,0x65,0x69,0x64,0x00};
static const u8 NL_NL_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 NL_NL_noi_labl[] = {0x4C,0x61,0x77,0x61,0x61,0x69,0x00};
static const u8 NL_NL_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 NL_NL_lux_labl[] = {0x56,0x65,0x72,0x6C,0x69,0x63,0x68,0x74,0x69,0x6E,0x67,0x00};
static const u8 NL_NL_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 NL_NL_air_labl[] = {0x4C,0x75,0x63,0x68,0x74,0x00};
static const u8 NL_NL_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 NL_NL_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 NL_NL_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 PT_PT_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x61,0x00};
static const u8 PT_PT_tem_unit[] = {0xA6,0x43,0x00};
static const u8 PT_PT_hum_labl[] = {0x55,0x6D,0x69,0x64,0x61,0x64,0x65,0x00};
static const u8 PT_PT_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 PT_PT_noi_labl[] = {0x53,0x6F,0x61,0x72,0x00};
static const u8 PT_PT_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 PT_PT_lux_labl[] = {0x49,0x6C,0x75,0x6D,0x69,0x6E,0x61,0x87,0x61,0x6F,0x00};
static const u8 PT_PT_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 PT_PT_air_labl[] = {0x41,0x72,0x00};
static const u8 PT_PT_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 PT_PT_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 PT_PT_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

static const u8 ES_ES_tem_labl[] = {0x54,0x65,0x6D,0x70,0x65,0x72,0x61,0x74,0x75,0x72,0x61,0x00};
static const u8 ES_ES_tem_unit[] = {0xA6,0x43,0x00};
static const u8 ES_ES_hum_labl[] = {0x48,0x75,0x6D,0x65,0x64,0x61,0x64,0x00};
static const u8 ES_ES_hum_unit[] = {0x25,0x48,0x52,0x00};
static const u8 ES_ES_noi_labl[] = {0x52,0x75,0x69,0x64,0x6F,0x00};
static const u8 ES_ES_noi_unit[] = {0x64,0x42,0x41,0x00};
static const u8 ES_ES_lux_labl[] = {0x69,0x6C,0x75,0x6D,0x69,0x6E,0x61,0x63,0x69,0xA2,0x6E,0x00};
static const u8 ES_ES_lux_unit[] = {0x6C,0x75,0x78,0x00};
static const u8 ES_ES_air_labl[] = {0x41,0x69,0x72,0x65,0x00};
static const u8 ES_ES_air_unit[] = {0x52,0x65,0x6C,0x2E,0x20,0x56,0x4F,0x43,0x00};
static const u8 ES_ES_co2_labl[] = {0x43,0x4f,0x32,0x00};
static const u8 ES_ES_co2_unit[] = {0x70,0x70,0x6d,0x20,0x43,0x4f,0x32,0x00};

//special texts
static const u8 FR_FR_text_thanks[] = {0x4d,0x65,0x72,0x63,0x69,0x20,0x21,0x00};
static const u8 EN_GB_text_thanks[] = {0x54,0x68,0x61,0x6e,0x6b,0x73,0x21,0x00};
static const u8 EN_US_text_thanks[] = {0x54,0x68,0x61,0x6e,0x6b,0x73,0x21,0x00};
static const u8 DE_DE_text_thanks[] = {0x44,0x61,0x6e,0x6b,0x65,0x21,0x00};
static const u8 NL_NL_text_thanks[] = {0x44,0x61,0x6e,0x6b,0x21,0x00};
static const u8 PT_PT_text_thanks[] = {0x4f,0x62,0x72,0x69,0x67,0x61,0x64,0x6f,0x21,0x00};
static const u8 ES_ES_text_thanks[] = {0x47,0x72,0x61,0x63,0x69,0x61,0x73,0x21,0x00};
																			           
static const u8 FR_FR_text_sent[] = {0x45,0x6e,0x76,0x6f,0x79,0x82,0x2e,0x00};
static const u8 EN_GB_text_sent[] = {0x53,0x65,0x6e,0x74,0x2e,0x00};
static const u8 EN_US_text_sent[] = {0x53,0x65,0x6e,0x74,0x2e,0x00};
static const u8 DE_DE_text_sent[] = {0x47,0x65,0x73,0x63,0x68,0x69,0x63,0x6b,0x74,0x2e,0x00};
static const u8 NL_NL_text_sent[] = {0x56,0x65,0x72,0x7a,0x6f,0x6e,0x64,0x65,0x6e,0x2e,0x00};
static const u8 PT_PT_text_sent[] = {0x45,0x6e,0x76,0x69,0x61,0x64,0x6f,0x2e,0x00};
static const u8 ES_ES_text_sent[] = {0x45,0x6e,0x76,0x69,0x61,0x64,0x6f,0x2e,0x00};
static const u8 FR_FR_text_yes[] = {0x4f,0x55,0x49,0x00};
static const u8 EN_GB_text_yes[] = {0x59,0x45,0x53,0x00};
static const u8 EN_US_text_yes[] = {0x59,0x45,0x53,0x00};
static const u8 DE_DE_text_yes[] = {0x59,0x41,0x00};
static const u8 NL_NL_text_yes[] = {0x4a,0x41,0x00};
static const u8 PT_PT_text_yes[] = {0x53,0x49,0x00};
static const u8 ES_ES_text_yes[] = {0x53,0x49,0x00};
static const u8 FR_FR_text_no[] = {0x4e,0x4f,0x4e,0x00};
static const u8 EN_GB_text_no[] = {0x4e,0x4f,0x00};
static const u8 EN_US_text_no[] = {0x4e,0x4f,0x00};
static const u8 DE_DE_text_no[] = {0x4e,0x45,0x49,0x4e,0x00};
static const u8 NL_NL_text_no[] = {0x4e,0x45,0x45,0x4e,0x00};
static const u8 PT_PT_text_no[] = {0x4e,0x41,0x4f,0x00};
static const u8 ES_ES_text_no[] = {0x4e,0x4f,0x00};


char strBuffer[16];
u8 locale = FR_FR;
const u8* tem_labl;
const u8* tem_unit;
const u8* hum_labl;
const u8* hum_unit;
const u8* noi_labl;
const u8* noi_unit;
const u8* lux_labl;
const u8* lux_unit;
const u8* air_labl;
const u8* air_unit;
const u8* co2_labl;
const u8* co2_unit;
const u8* text_thanks;
const u8* text_sent;
const u8* text_YES;
const u8* text_NO;

bool alert_on = true;
bool alert_showingValue = false;

void SetLocale()
{
		CfgConfig_t* cfgConfigPtr = CfgConfigPointer();
		switch (cfgConfigPtr->lang)
		{
			case FR_FR:
				tem_labl = FR_FR_tem_labl;
				tem_unit = FR_FR_tem_unit;
				hum_labl = FR_FR_hum_labl;
				hum_unit = FR_FR_hum_unit;
				noi_labl = FR_FR_noi_labl;
				noi_unit = FR_FR_noi_unit;
				lux_labl = FR_FR_lux_labl;
				lux_unit = FR_FR_lux_unit;
				air_labl = FR_FR_air_labl;
				air_unit = FR_FR_air_unit;
				co2_labl = FR_FR_co2_labl;
				co2_unit = FR_FR_co2_unit;
				text_thanks = FR_FR_text_thanks;
				text_sent = FR_FR_text_sent;
				text_YES = FR_FR_text_yes;
				text_NO = FR_FR_text_no;

				break;
			case EN_US:
				tem_labl = EN_US_tem_labl;
				tem_unit = EN_US_tem_unit;
				hum_labl = EN_US_hum_labl;
				hum_unit = EN_US_hum_unit;
				noi_labl = EN_US_noi_labl;
				noi_unit = EN_US_noi_unit;
				lux_labl = EN_US_lux_labl;
				lux_unit = EN_US_lux_unit;
				air_labl = EN_US_air_labl;
				air_unit = EN_US_air_unit;
				co2_labl = EN_US_co2_labl;
				co2_unit = EN_US_co2_unit;
				text_thanks = EN_US_text_thanks;
				text_sent = EN_US_text_sent;
				text_YES = EN_US_text_yes;
				text_NO = EN_US_text_no;
				break;
			case EN_GB:
				tem_labl = EN_GB_tem_labl;
				tem_unit = EN_GB_tem_unit;
				hum_labl = EN_GB_hum_labl;
				hum_unit = EN_GB_hum_unit;
				noi_labl = EN_GB_noi_labl;
				noi_unit = EN_GB_noi_unit;
				lux_labl = EN_GB_lux_labl;
				lux_unit = EN_GB_lux_unit;
				air_labl = EN_GB_air_labl;
				air_unit = EN_GB_air_unit;
				co2_labl = EN_GB_co2_labl;
				co2_unit = EN_GB_co2_unit;
				text_thanks = EN_GB_text_thanks;
				text_sent = EN_GB_text_sent;
				text_YES = EN_GB_text_yes;
				text_NO = EN_GB_text_no;

				break;
			case DE_DE:
				tem_labl = DE_DE_tem_labl;
				tem_unit = DE_DE_tem_unit;
				hum_labl = DE_DE_hum_labl;
				hum_unit = DE_DE_hum_unit;
				noi_labl = DE_DE_noi_labl;
				noi_unit = DE_DE_noi_unit;
				lux_labl = DE_DE_lux_labl;
				lux_unit = DE_DE_lux_unit;
				air_labl = DE_DE_air_labl;
				air_unit = DE_DE_air_unit;
				co2_labl = DE_DE_co2_labl;
				co2_unit = DE_DE_co2_unit;
				text_thanks = DE_DE_text_thanks;
				text_sent = DE_DE_text_sent;
				text_YES = DE_DE_text_yes;
				text_NO = DE_DE_text_no;

				break;
			case PT_PT:
				tem_labl = PT_PT_tem_labl;
				tem_unit = PT_PT_tem_unit;
				hum_labl = PT_PT_hum_labl;
				hum_unit = PT_PT_hum_unit;
				noi_labl = PT_PT_noi_labl;
				noi_unit = PT_PT_noi_unit;
				lux_labl = PT_PT_lux_labl;
				lux_unit = PT_PT_lux_unit;
				air_labl = PT_PT_air_labl;
				air_unit = PT_PT_air_unit;
				co2_labl = PT_PT_co2_labl;
				co2_unit = PT_PT_co2_unit;
				text_thanks = PT_PT_text_thanks;
				text_sent = PT_PT_text_sent;
				text_YES = PT_PT_text_yes;
				text_NO = PT_PT_text_no;
				break;
			case NL_NL:
				tem_labl = NL_NL_tem_labl;
				tem_unit = NL_NL_tem_unit;
				hum_labl = NL_NL_hum_labl;
				hum_unit = NL_NL_hum_unit;
				noi_labl = NL_NL_noi_labl;
				noi_unit = NL_NL_noi_unit;
				lux_labl = NL_NL_lux_labl;
				lux_unit = NL_NL_lux_unit;
				air_labl = NL_NL_air_labl;
				air_unit = NL_NL_air_unit;
				co2_labl = NL_NL_co2_labl;
				co2_unit = NL_NL_co2_unit;
				text_thanks = NL_NL_text_thanks;
				text_sent = NL_NL_text_sent;
				text_YES = NL_NL_text_yes;
				text_NO = NL_NL_text_no;
				break;
			case ES_ES:
				tem_labl = ES_ES_tem_labl;
				tem_unit = ES_ES_tem_unit;
				hum_labl = ES_ES_hum_labl;
				hum_unit = ES_ES_hum_unit;
				noi_labl = ES_ES_noi_labl;
				noi_unit = ES_ES_noi_unit;
				lux_labl = ES_ES_lux_labl;
				lux_unit = ES_ES_lux_unit;
				air_labl = ES_ES_air_labl;
				air_unit = ES_ES_air_unit;
				co2_labl = ES_ES_co2_labl;
				co2_unit = ES_ES_co2_unit;
				text_thanks = ES_ES_text_thanks;
				text_sent = ES_ES_text_sent;
				text_YES = ES_ES_text_yes;
				text_NO = ES_ES_text_no;
				break;
			default: 	//default to EN_GB
				tem_labl = EN_GB_tem_labl;
				tem_unit = EN_GB_tem_unit;
				hum_labl = EN_GB_hum_labl;
				hum_unit = EN_GB_hum_unit;
				noi_labl = EN_GB_noi_labl;
				noi_unit = EN_GB_noi_unit;
				lux_labl = EN_GB_lux_labl;
				lux_unit = EN_GB_lux_unit;
				air_labl = EN_GB_air_labl;
				air_unit = EN_GB_air_unit;
				co2_labl = EN_GB_co2_labl;
				co2_unit = EN_GB_co2_unit;
				text_thanks = EN_GB_text_thanks;
				text_sent = EN_GB_text_sent;
				text_YES = EN_GB_text_yes;
				text_NO = EN_GB_text_no;
				break;
		}
}

/*==================================================================
	do Display
  must be called every second
===================================================================*/
u8 doDisplay( CubeData_t *Data, Time_t Time)
{
	CfgStatus_t *cfgStatusPtr;
	CfgPoll_t *cfgPollPtr;
	cfgStatusPtr = CfgStatusPointer();
	cfgPollPtr = CfgPollPointer();
	
	//orientation change detection
	if (Data->Orientation != Data->LastOrientation)
	{
		Data->tiltSecCounter = 0;
		//orientation has changed
		switch (Data->Orientation)
		{
			case UPSIDE_DOWN:
				Data->displayMode = DISPLAY_CONFIG;
				break;
			case UP:
				Data->displayMode = DISPLAY_TEMP;
				break;
			case TILTED_LEFT:		//on left side
				Data->tiltSecCounter = 0;
				Data->displayMode = DISPLAY_HAPPY;
				break;
			case TILTED_RIGHT:			//on right side
				Data->tiltSecCounter = 0;
				Data->displayMode = DISPLAY_UNHAPPY;
				break;
			default:
				//default : keep last display mode
				Data->displayMode = Data->displayMode;	//TODO: useless !
		}
	}

	// DISPLAY_MEASURES, DISPLAY_CONFIG, DISPLAY_HAPPY, DISPLAY_UNHAPPY
	if (Data->displayMode == DISPLAY_HAPPY)
		ShowCountDown(true, Data);
	else if (Data->displayMode == DISPLAY_UNHAPPY)
		ShowCountDown(false, Data);
	else if (Data->displayMode == DISPLAY_CONFIG) 
		DisplayConfig();			
	else if ((cfgStatusPtr->pollTimeRemaining_hour != 0) && (cfgPollPtr->pollDuration_hour != 0))
	{
		if (cfgPollPtr->pollMode == POLLMODE_TEXT)
			DisplayMessage(cfgPollPtr->text);
		else
		{
			DisplayBlack();
			DrawImageFast(16,16, ICON_QUESTION);
			DisplayRefresh();
		}
	}
	else
	{
		if (!ShowAlert(Data))
		{
			//change every 5 s
		if ((Time.TotalSec%5 == 0) || (Data->Orientation != Data->LastOrientation)) 
			DisplayMeasures(Data, Time.TotalSec );
		}
	}
		
	//Data->LastOrientation = Data->Orientation;
	return Data->Orientation;	// return Displayed orientation
}


/****************************************************
affichage des alertes
****************************************************/
bool ShowAlert(CubeData_t *Data)
{
	CfgConfig_t *cfgConfigPtr;
	bool showAlert = false;
	char charBuffer[16];
	u8 len;
	cfgConfigPtr = CfgConfigPointer();

	if (cfgConfigPtr->alertMode != 0)
	{
		if ((cfgConfigPtr->alertMode == 4) && (Data->CO2ppm*100 > cfgConfigPtr->alertThresholdx100 ))
		{
			showAlert = true;
			
		if (alert_showingValue)
		{
			DisplayBlack();
			len = sprintf(charBuffer, "CO2:");
			DrawText((u8*) charBuffer, len, 64, 32, FONT_ROBOTO24, ALIGN_CENTER);
			len = sprintf(charBuffer, "%d ppm", Data->CO2ppm); 
			DrawText((u8*) charBuffer, len, 64, 64, FONT_ROBOTO24, ALIGN_CENTER);
		}
		else
		{
			DisplayWhiteBorders();
			DrawImageFast(16,16, ICON_AEREZ);
		}
			DisplayRefresh();
			alert_showingValue = !alert_showingValue;
		}
	}
	return showAlert;
}

void DisplayToggle(CubeData_t *Data)
{
	bool pollRunning;
	CfgStatus_t *cfgStatusPtr;
	CfgPoll_t *cfgPollPtr;
	CfgConfig_t * cfgConfigPtr;
	
	cfgStatusPtr = CfgStatusPointer();
	cfgPollPtr = CfgPollPointer();
	cfgConfigPtr = CfgConfigPointer();
	
	pollRunning = (bool) ((cfgStatusPtr->pollTimeRemaining_hour != 0) && (cfgPollPtr->pollDuration_hour != 0));
	if (pollRunning)
	{
	
	}
	else 
	{
		//do standard toggle
		if (cfgConfigPtr->showToggle)
		{
			if (cfgConfigPtr->toggleMode == TOGGLE_MODE_IMG)
			{
				if (Data->Orientation == TILTED_LEFT)
					ShowCountDown(true, Data);
				else
					ShowCountDown(false, Data);
			}
			else 
			{
				//if (Data->Orientation == TILTED_LEFT)
				//	ShowToggleText(true, Data);
				//else
				//	ShowToggleText(false, Data);
			}
		}
	}
	
}


void DisplaySingleString(char* str, u8 len)
{
	DisplayBlack();
	DrawText((u8*) str, len,126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
	DisplayRefresh();
	while (!IsDisplayRefreshed());
}

/****************************************************
affichage des mesures sur l'écran toutes les x secondes
****************************************************/
static void DisplayMeasures( CubeData_t *Data, u8 Sec)
{
	char charBuffer[32];
	u8 x = 0;
	Font_t *font;
	CfgConfig_t* cfgConfigPtr;
	CfgCalib_t*  cfgCalibPtr;
	CfgStatus_t* cfgStatusPtr;
	//u8 i=0;
	u8 len = 0;
	float val = 0;
	u8 currentDisplayMode;

	//read config
	cfgConfigPtr = CfgConfigPointer();
	cfgStatusPtr = CfgStatusPointer();
	
	//if nothing should be displayed, show logo
	if ((!cfgConfigPtr->showTemp) &&
			(!cfgConfigPtr->showHygr) &&
			(!cfgConfigPtr->showLux) &&
			(!cfgConfigPtr->showNoise) &&
			(!cfgConfigPtr->showAir)) 
	{
		DisplayBlack();
		ShowGreenMe();
		DisplayRefresh();
		return;
	}
	
	DisplayBlack();
	SetLocale();
	
	if (Data->displayMode == DISPLAY_TEMP)
	{
		cfgCalibPtr = CfgCalibPointer();
		DrawText((u8*) tem_labl, strlen((const char*) tem_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
		
		val = Data->Tempx100 + cfgCalibPtr->deltaTx100;
		
		if (cfgConfigPtr->lang == EN_US)
		{
			len = sprintf(charBuffer, "%.1f", val/100.0 * 9/5.0 + 32); 
		}
		else {
			len = sprintf(charBuffer, "%.1f", val/100.0); 
		}
		DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);

		DrawText((u8*) tem_unit, strlen((const char*) tem_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
	}
	else if (Data->displayMode == DISPLAY_HYGR)
	{
		cfgCalibPtr = CfgCalibPointer();
		DrawText((u8*) hum_labl, strlen((const char*) hum_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
		
		//len = sprintf(charBuffer, "%.1f", (Data->Hygrx100)/100.0); 
		len = sprintf(charBuffer, "%.1f", (Data->Hygrx100 + cfgCalibPtr->deltaHx100)/100.0); 
		DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);

		DrawText((u8*) hum_unit, strlen((const char*) hum_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
	}
	else if (Data->displayMode == DISPLAY_NOISE)
	{
		cfgCalibPtr = CfgCalibPointer();
		DrawText((u8*) noi_labl, strlen((const char*) noi_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
		
		len = sprintf(charBuffer, "%.1f", (Data->dBAx100 + cfgCalibPtr->AudioCalibOffsetx100)/100.0); 
		DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);
		DrawText((u8*) noi_unit, strlen((const char*) noi_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
	}
	else if (Data->displayMode == DISPLAY_LUX)
	{
		cfgCalibPtr = CfgCalibPointer();
		DrawText((u8*) lux_labl, strlen((const char*) lux_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
		val = Data->Lux * cfgCalibPtr->luxGainx100/100.0;
		if (val < 0)
			len = sprintf(charBuffer, "%d", 0); // char / is translated to k in font
		else if (val > 1999)
			len = sprintf(charBuffer, "%.1f/", val/1000.0); // char / is translated to k in font
		else
			len = sprintf(charBuffer, "%d", (u16) val); 
		DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);
		DrawText((u8*) lux_unit, strlen((const char*) lux_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
	}
	else if (Data->displayMode == DISPLAY_AIR)
	{
		
		if (cfgConfigPtr->extSensorType == EXT_SENSOR_CO2)
		{
			DrawText((u8*) co2_labl, strlen((const char*) co2_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
			DrawText((u8*) co2_unit, strlen((const char*) co2_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);
			if (!cfgConfigPtr->showAirAsValue)
			{
				charBuffer[0] = ':';
				if (Data->CO2ppm > 1500)
				{
					DrawText((u8*) charBuffer, 1, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
				else if (Data->CO2ppm > 800)
				{
					charBuffer[1] = ':';
					DrawText((u8*) charBuffer, 2, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
				else 
				{
					charBuffer[1] = ':';
					charBuffer[2] = ':';
					DrawText((u8*) charBuffer, 3, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
			}
			else 
			{
				len = sprintf(charBuffer, "%d", Data->CO2ppm); 
				DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);
			}
			
		}
		else {
			
			DrawText((u8*) air_labl, strlen((const char*) air_labl),4, 2, FONT_ROBOTO24, ALIGN_LEFT);
			DrawText((u8*) air_unit, strlen((const char*) air_unit),126, 127-32, FONT_ROBOTO24, ALIGN_RIGHT);

			if (!cfgConfigPtr->showAirAsValue)
			{
				//TODO!! revoir ces seuils
				charBuffer[0] = ':';
				if (Data->VOCppb > 100)
				{
					DrawText((u8*) charBuffer, 1, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
				else if (Data->VOCppb > 30)
				{
					charBuffer[1] = ':';
					DrawText((u8*) charBuffer, 2, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
				else 
				{
					charBuffer[1] = ':';
					charBuffer[2] = ':';
					DrawText((u8*) charBuffer, 3, 16, 48, FONT_SONYSKETCH32, ALIGN_LEFT);
				}
			}
			else 
			{
				len = sprintf(charBuffer, "%d", Data->VOCppb); 
				DrawText((u8*) charBuffer, len, 64, 48, FONT_SONYSKETCH32, ALIGN_CENTER);
			}
			
		}
	}
	currentDisplayMode = Data->displayMode;

	while (true)
	{
		Data->displayMode++;
		if (Data->displayMode > 4)
			Data->displayMode = 0;
		
		//back to beginning: stop loop
		if (Data->displayMode == currentDisplayMode)
			break;
		
		if 		  ((Data->displayMode == DISPLAY_TEMP) && (cfgConfigPtr->showTemp == true))
			break;
		else if ((Data->displayMode == DISPLAY_HYGR) && (cfgConfigPtr->showHygr == true))
			break;
		else if ((Data->displayMode == DISPLAY_NOISE) && (cfgConfigPtr->showNoise == true) && (cfgConfigPtr->disableSound == false))
			break;
		else if ((Data->displayMode == DISPLAY_LUX) && (cfgConfigPtr->showLux == true))
			break;
		else if ((Data->displayMode == DISPLAY_AIR) && (cfgConfigPtr->showAir == true))
			break;
	}
	
	//icons
	x = 0;
	if (cfgStatusPtr->IsRadioJoined  != true)
	{
		//no network
		font = GetFont(FONT_SMALLICONS);
		DrawChar(x, 111, 0, font);
		x+= font->widths[0] + 1;
	}

	DisplayRefresh();
}

/***************************************************
Show configuration values (Lora ID, firmware version, etc.)
*****************************************************/
void DisplayConfig()
{
	char charBuffer[32];
	u8 y = 8;
	CfgId_t *cfgIdPtr = CfgIdPointer();
	
	DisplayBlack();

	sprintf(charBuffer, "Lora id:");
	DrawText180((u8*) charBuffer, 8, 64, 8, ALIGN_CENTER);
	
	y=28;
	sprintf(charBuffer, "%02x%02x%02x%02x", cfgIdPtr->devEUI[0], cfgIdPtr->devEUI[1], cfgIdPtr->devEUI[2], cfgIdPtr->devEUI[3] );
	DrawText180((u8*) charBuffer, 8, 64, y, ALIGN_CENTER);
	
	y=48;
	sprintf(charBuffer, "%02x%02x%02x%02x", cfgIdPtr->devEUI[4], cfgIdPtr->devEUI[5], cfgIdPtr->devEUI[6], cfgIdPtr->devEUI[7] );
	DrawText180((u8*) charBuffer, 8, 64, y, ALIGN_CENTER);
	
	y=88;
	sprintf(charBuffer, "Fw: %0d.%02d", cfgIdPtr->fwVersion >> 8, cfgIdPtr->fwVersion & 0x00FF);
	DrawText180((u8*) charBuffer, 8, 64, y, ALIGN_CENTER);
	
	DisplayRefresh();
}


static void DisplayMessage(u8 *str)
{
	DisplayBlack();
	DrawMultilineText(str, strlen( (char *) str));
	DisplayRefresh();
	while (!IsDisplayRefreshed());
}


/***************************************************
Show feedback positive or negative
*****************************************************/
static void ShowCountDown(bool happy, CubeData_t *Data)
{
	bool pollRunning = false;
	
	CfgStatus_t *cfgStatusPtr = CfgStatusPointer();
	CfgConfig_t *cfgConfigPtr = CfgConfigPointer();
	CfgPoll_t *cfgPollPtr = CfgPollPointer();
	Font_t* font;
	font = GetFont(FONT_ROBOTO24);
	pollRunning = (bool) ((cfgStatusPtr->pollTimeRemaining_hour != 0) && (cfgPollPtr->pollDuration_hour != 0));
	
	if (Data->LastOrientation == UP)
	{
		//first loop since position change : initialize timer
		Data->tiltSecCounter = 0;
		Data->happy = 0;
		
		DisplayBlack();
		ShowToggle(Data);
		
		if (Data->Orientation == TILTED_LEFT)
			DrawNumber90(2, 96, 0x30 + (5 - Data->tiltSecCounter ));
		else
			DrawNumber270(96, 2, 0x30 + (5 - Data->tiltSecCounter ));
		
		DisplayRefresh();
	}
	else {
		Data->tiltSecCounter ++;

		if (Data->tiltSecCounter > 7)
		{
			//over: return to normal display mode
			Data->tiltSecCounter = 8;
			Data->displayMode = DISPLAY_TEMP;

		}
		else if (Data->tiltSecCounter == 5)
		{
			//show acknowlegdment
			DisplayBlack();
			if (pollRunning)
			{
				if (cfgPollPtr->pollAcknowlegment == POLL_ACK_OK)
				{
					if (Data->Orientation == TILTED_LEFT)
						DrawImageFast(50, 16, ICON_OK_LEFT);
					else
						DrawImageFast(50, 16, ICON_OK_RIGHT);
				}
				else if (cfgPollPtr->pollAcknowlegment == POLL_ACK_SENT)
				{
					if (Data->Orientation == TILTED_LEFT)
						DrawTextAngle((char*) text_sent, font, true, 40, 1, ANGLE_270);
					else
						DrawTextAngle((char*) text_sent, font, true, 40, 1, ANGLE_90);
				}
				else if (cfgPollPtr->pollAcknowlegment == POLL_ACK_THANKS)
				{
					if (Data->Orientation == TILTED_LEFT)
						DrawTextAngle((char*) text_thanks, font, true, 40, 1, ANGLE_270);
					else
						DrawTextAngle((char*) text_thanks, font, true, 40, 1, ANGLE_90);
				}
			}
			else 
			{
				if (cfgConfigPtr->toggleMode == TOGGLE_MODE_IMG)
				{
					if (Data->Orientation == TILTED_LEFT)
						DrawImageFast(50, 16, ICON_OK_LEFT);
					else
						DrawImageFast(50, 16, ICON_OK_RIGHT);
				}
				else 
				{
					if (Data->Orientation == TILTED_LEFT)
						DrawTextAngle((char*) cfgConfigPtr->text_acknowledgment, font, ALIGN_CENTER, 40, 1, ANGLE_270);
					else
						DrawTextAngle((char*) cfgConfigPtr->text_acknowledgment, font, ALIGN_CENTER, 40, 1, ANGLE_90);
				}
			}
			
			if (Data->Orientation == TILTED_LEFT)
			{
				Data->happy = 1;
			}
			else {
				Data->happy = 2;
			}
			
			//disable poll display
			if (cfgPollPtr->pollEndsOnToggle)
			{
				cfgStatusPtr->pollTimeRemaining_hour = 0;
				CfgStatusSaveToEep();
			}
			DisplayRefresh();
		}
		else if (Data->tiltSecCounter < 5) {
			//show proposed response + countdown
			DisplayBlack();
			ShowToggle(Data);
			if (Data->displayMode == DISPLAY_HAPPY)
				DrawNumber90(2, 96, 0x30 + (5 - Data->tiltSecCounter ));
			else 
				DrawNumber270(96, 2, 0x30 + (5 - Data->tiltSecCounter ));
			DisplayRefresh();
		}
	}
}



/**
* Show Greenme icon
*/
void ShowGreenMe()
{
	u32 start = GetusTick() ;
	DisplayBlack();
	DrawImageFast(24,16,ICON_G);
	DisplayRefresh();
	WaitMs(100);
}

/**
* Show Toggle display
*/
void ShowToggle( CubeData_t* Data)
{
	CfgStatus_t *cfgStatusPtr = CfgStatusPointer();
	CfgConfig_t *cfgConfigPtr = CfgConfigPointer();
	CfgPoll_t *cfgPollPtr = CfgPollPointer();
	//u8 *text;
	bool pollRunning = false;
	Font_t* font;
	font = GetFont(FONT_ROBOTO24);
	pollRunning = (bool) ((cfgStatusPtr->pollTimeRemaining_hour != 0) && (cfgPollPtr->pollDuration_hour != 0));
	
	if (pollRunning)
		{
			if (cfgPollPtr->pollResponseChoices == POLL_RESPONSE_HAPPY)
			{
				if (Data->Orientation == TILTED_LEFT)
					DrawHappy(true);
				else
					DrawHappy(false);
			}
			else {
				//response yes no
				if (Data->Orientation == TILTED_LEFT)
					DrawTextAngle((char*) text_YES, font, ALIGN_CENTER, 40, 1, ANGLE_270);
				else
					DrawTextAngle((char*) text_NO, font, ALIGN_CENTER, 40, 1, ANGLE_90);
			}
					
		}
		else 
		{
			if (cfgConfigPtr->toggleMode == TOGGLE_MODE_IMG)
			{
				if (Data->Orientation == TILTED_LEFT)
					DrawHappy(true);
				else
					DrawHappy(false);
			}
			else 
			{
				if (Data->Orientation == TILTED_LEFT)
					DrawTextAngle((char*) cfgConfigPtr->text_toggle_left, font, ALIGN_CENTER, 40, 1, ANGLE_270);
				
				else
					DrawTextAngle((char*) cfgConfigPtr->text_toggle_right, font, ALIGN_CENTER, 40, 1, ANGLE_90);

			}
		}
}

//helper function (test only)
void TestDisplay(void)
{
	Font_t* font;
			//DisplaySetup();
			DisplayBlack();

	DisplayBlack();
	DisplayRefresh();
	while(!IsDisplayRefreshed());
	
	font = GetFont(FONT_ROBOTO24);
	DrawTextAngle((char*) FR_FR_tem_labl, font, true, 16, 0, ANGLE_270);
	DisplayRefresh();
	/*DrawCharAngle(4,4, 'A', font, ANGLE_90);
	DisplayRefresh();
	*/
	/*//DrawCharAngle(4,4, font->firstChar, font, ANGLE_0);
	for (i=0; i<40; i++)
	{
		SetPixel(i,i*2, WHITE, ANGLE_270);
	}
	DisplayRefresh();
	*/
	
	while(1);
}
