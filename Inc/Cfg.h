#ifndef CFG_H
#define CFG_H

#include <StdTypes.h>

static const u8 POLLMODE_ICON = 0x00;
static const u8 POLLMODE_TEXT = 0x01;


/*==============================================================================
	TYPEDEFS
	
	Configuration data structure
===============================================================================*/
#define CFGID_PAGE_NB     	0x00
#define CFGSTATUS_PAGE_NB 	0x01
#define CFGCALIB_PAGE_NB  	0x02	
#define CFGCONFIG_PAGE_NB 	0x03	
#define CFGPOLL_PAGE_NB  		0x04

#define TOGGLE_MODE_IMG		0x00
#define TOGGLE_MODE_TEXT	0x01

#define POLL_RESPONSE_HAPPY	  0x00
#define POLL_RESPONSE_YESNO	  0x01
#define POLL_RESPONSE_GOODBAD	0x02

#define POLL_ACK_OK					0x00
#define POLL_ACK_THANKS			0x01
#define POLL_ACK_SENT				0x02

#define EXT_SENSOR_NONE			0x00
#define EXT_SENSOR_CO2			0x01
#define EXT_SENSOR_PM				0x02


typedef struct	
{
	//id
	u16 fwVersion;										//2 bytes major-minor, e.g. 2.01 = 0x0201 
	u16	CCS811FwVersion;
	u32 armn8FwVersion;
	u8  devEUI[8];										//radio devEUI (chars)
	//u8	bleMac[6];										//Bluetooth Mac, if present : 6 bytes only	
	//u16 unused;
} CfgId_t;		//16 bytes
static const u8 CfgIdSize = 16;

typedef struct	//careful! must fit words in memory!
{
	u8 IsBoardInitialized   	    : 1;		//initial configuration is done
	u8 IsRadioConfigured  		    : 1;		//radio initial configuration is done
	u8 IsRadioJoined		  		    : 1;		//radio initial configuration is done
	u8 unused1										: 5;
	u8 pollNextStart_hour;
	u8 pollTimeRemaining_hour;
	u8 unused2;
} CfgStatus_t;		//total size : 4 bytes
static const u8 CfgStatusSize = 4;

typedef struct	
{
	u16 luxGainx100;									 //lux calibration
	s16 deltaTx100;										 //temperature calibration
	s16 deltaHx100;										 //humdity calibration
	s16 AudioCalibOffsetx100;				   //dBA calibration
	u16 gainColorRx100;
	u16 gainColorGx100;
	u16 gainColorBx100;
	u16 gainColorWx100;
} CfgCalib_t;	
static const u8 CfgCalibSize = 16;

typedef struct	//careful! must fit words in memory!
{
	//configuration
	u8 shortMessageInterval_min;		//max 255 min
	u8 longMessageInterval_min;		//max 255 min
	
	u8 showTemp		 								: 1;		 //display templerature vlua
	u8 showHygr			 							: 1;
	u8 showLux 										: 1;
	u8 showNoise 									: 1;
	u8 showAir 										: 1;
	u8 showToggle									: 1;			
	u8 showAirAsValue  						: 1;			
	u8 disableSound								: 1;			//disable sound measurement

	
	u8 disableVOC									: 1;			//disable voc measurement
	u8 toggleMode									: 1;			//text or image
	u8 imgToggleLeft							: 3;			//img ID for left toggle
	u8 imgToggleRight							: 3;			//img ID for right toggle
	
	u8 imgToggleBack							: 3;			//0: show config, 1+ : to be implemented
	u8 extSensorType							: 5;			//external sensor type (co2, particle...)
	
	u8 text_toggle_left[10];
	u8 text_toggle_right[10];
	u8 text_acknowledgment[10];
	
	u8 lang;
	
	u8 eventMode;														//TODO: choose event mode (temperature, noise, etc.)									
	u8 eventFrom;
	u8 eventThreshold;	
	u8 eventWindow_s;
	
	u8 alertMode;							//0:none; 1:temp; 2:noise; 3:TVOC; 4:CO2; 5:PM10; 6:PM10 
	u16 alertThresholdx100;

} CfgConfig_t;	//total size: 43 bytes
static const u8 CfgConfigSize = 43;


typedef struct	
{
	u8 pollMode										: 1;						//text or image
	u8 pollEndsOnToggle						: 1;						//if true, message disappear after response 
	u8 PollEndsOnReboot						: 1;						//message disappear after reboot
	u8 unused											: 5;

	u8 pollResponseChoices				: 4;						//preset response choices (happy/not happy, yes/no)
	u8 pollAcknowlegment					: 4;						//show ok, thank you, sent!

	u8 pollRepeat_hour;											//show poll every xx h; poll enabled if duration != 0
	u8 pollDuration_hour;										//show poll during xx hours
																					//poll duration can be 1 to 31 hours
																					//poll can be deactivated by toggle, reboot or can expire by itself
																					//poll can be a text or a question mark

	u8 text[32];									 //text itself (ascii, 1 byte per char)
} CfgPoll_t;	//total size : 36 bytes
static const u8 CfgPollSize = 36;

typedef struct
{
	CfgStatus_t status;
	CfgId_t id;
	CfgCalib_t calib;
	CfgConfig_t config;
	CfgPoll_t poll;
} Cfg_t;

/*==============================================================================
	FUNCTIONS
===============================================================================*/
bool CfgStatusSaveToEep(void);
bool CfgIdSaveToEep(void);
bool CfgCalibSaveToEep(void);
bool CfgConfigSaveToEep(void);
bool CfgPollSaveToEep(void);
extern CfgStatus_t *CfgStatusPointer( void );						// Returns a pointer to configuration
extern CfgId_t *CfgIdPointer( void );										// Returns a pointer to configuration
extern CfgCalib_t *CfgCalibPointer( void );							// Returns a pointer to configuration
extern CfgConfig_t *CfgConfigPointer( void );						// Returns a pointer to configuration
extern CfgPoll_t *CfgPollPointer( void );					// Returns a pointer to configuration
	
bool CheckConf(void);

#endif	// CFG_H
