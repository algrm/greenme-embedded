#ifndef RADIO_H
#define RADIO_H

#include "StdTypes.h"

typedef enum { RADIO_JOIN_UNKNOWN=0, RADIO_JOIN_SUCCESS=1, RADIO_JOIN_FAILURE=2 }RadioJoinStatus_t;

void RadioTask( void );
char* RadioGetDevEUI( void );
bool isRadioReady(void);
void RadioSend(u8 * buffer, u8 len);
bool HasJoinedNetwork(void);
u32 RadioGetFwVersion(void);

#define RADIOCMD_SET_CALIB  	0x04
#define RADIOCMD_SET_CFG			0x05
#define RADIOCMD_REQUEST_CFG	0x06
#define RADIOCMD_SET_DISPLAY	0x07
#define RADIOCMD_RESET				0x08
#define RADIOCMD_SET_VALUE		0x09
#define RADIOCMD_SET_ALERT		0x0A


#define MSG_SHORT		1				//quality, temp, hum, dbAMax, dBA moy, Lux
#define MSG_FULL		2				//quality, temp, hum, dbAMax, dBA moy, Lux
#define MSG_AUDIO		3				//quality, dbamax, dmaMoy, octaves[1..9]
#define MSG_FEEL		4				//expression du feel
#define MSGV2_SHORT	5				//version 2 : adds battery level
#define MSGV2_FULL	6				//version 2
#define MSGV2_FEEL	7				//version 2
#define MSGV2_CFG		0				//version 2		//configuration message

#define MSGEXT_NONE			0
#define MSGEXT_CO2_ONLY	1
#define MSGEXT_COV_ONLY	2
#define MSGEXT_COV_CO2	3
#define MSGEXT_PM_ONLY	4
#define MSGEXT_COV_PM	  5
#define MSGEXT_COV_CO2_PM	 6

#define MSG_VALUETYPE_TEMP	1
#define MSG_VALUETYPE_HYGR	2
#define MSG_VALUETYPE_DBA		3
#define MSG_VALUETYPE_LUX		4
#define MSG_VALUETYPE_COLORR	5
#define MSG_VALUETYPE_COLORG	6
#define MSG_VALUETYPE_COLORB	7
#define MSG_VALUETYPE_COLORW	8


#define RADIOMSG_HEADER1			0x65
#define RADIOMSG_HEADER2			0xef		//0Xee for version <=1.12, 0xef for version 2.5

#define RADIOMSG_CMD_SOFT_RESET	0x01
#define RADIOMSG_CMD_FACTORY_RESET	0x11

#define DISPLAY_MSG_MAX_LENGTH 30

typedef struct
{
	u16	header;
	u8  cmd;
	u8	shortMsgInterval_min;				//max 
	u8 longMessageInterval_min;		//max 60 min
	
	u8 showTemp		 								: 1;		 //display templerature vlua
	u8 showHygr			 							: 1;
	u8 showLux 										: 1;
	u8 showNoise 									: 1;
	u8 showAir 										: 1;
	u8 showToggle									: 1;			
	u8 showAirAsValue  						: 1;			//TODO: rendre possible un beacon sur QAI ?
	u8 disableSound								: 1;			//disable sound measurement

	u8 disableVOC									: 1;			//disable voc measurement
	u8 toggleMode 								: 1;
	u8 imgToggleLeft							: 3;			//img ID for left toggle
	u8 imgToggleRight							: 3;			//img ID for right toggle

	u8 imgToggleBack							: 3;
	u8 extSensorType							: 5;

	u8 text_toggle_left[10];
	u8 text_toggle_right[10];
	u8 text_acknowledgment[10];

	u8 lang;
	u8 eventMode;														//TODO: choose event mode (temperature, noise, etc.)									
	u8 eventFrom;
	u8 eventThreshold;	
	u8 eventWindow_s;
	u16 crc;	
}
RxCmdSetCfg_t;
static u8 RxCmdSetCfg_len = 45;


typedef struct
{
	u16	header;
	u8  cmd;
	u8 reboot;
	u16 crc;	
}
RxCmdReset_t;
static u8 RxCmdReset_len = 6;

typedef struct
{
	u16	header;
	u8  cmd;
	u8 alertMode;
	u16 thresholdx100;
	u16 crc;	
}
RxCmdSetAlert_t;
static u8 RxCmdSetAlert_len = 8;

typedef struct
{
	u16	header;
	u8  cmd;
	short	offsetTempx100;
	short	offsetHygrx100;
	u16		gainLuxx100;
  short deltadBAx100;
	u16		gainColorRx100;
	u16		gainColorGx100;
	u16		gainColorBx100;
	u16		gainColorWx100;
	u16 crc;	
}
RxCmdSetCalib_t;
static u8 RxCmdSetCalib_len = 21;

typedef struct
{
	u16	header;
	u8  cmd;
	u16 crc;	
}
RxCmdGetConfig_t;
static u8 RxCmdGetConfig_len = 5;


typedef struct
{
	u16	header;
	u8  cmd;
	u8  valueType;
	float	value;
	u16 crc;	
}
RxCmdSetValue_t;
static u8 RxCmdSetValue_len = 10;



typedef struct
{
	u16	header;						
	u8  cmd;
	
	u8 endOnToggle : 1;		
	u8 endOnReboot : 1;
	u8 responseChoices	 : 3;		//0: happy/not happy, 1: yes/no
	u8 acknowlegment	   : 3;		//0: ok, 1: "Sent !"
	
	u8 expiresAfter_hours;	//255 h max	
	u8 repeat_every_hours;	//255 h max
	u8 text[DISPLAY_MSG_MAX_LENGTH];							//30 bytes
	u16 crc;								//2 bytes
}
RxCmdSetMessage_t;
static u8 RxCmdSetMessage_len = 38;

#endif // RADIO_H
