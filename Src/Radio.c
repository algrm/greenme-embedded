#include "Radio.h"
#include "ARMN8LW.h"
#include <string.h>
#include <stdio.h>
#include "Debug.h"
#include "Main.h"
#include "Time.h"
#include "definitions.h"
#include "CCS811.h"
#include "Cfg.h"
#include "Uart2.h"
#include "crc.h"
#include "I2CStdSensors.h"
#include "dBAMeter.h"
#include "ColorSensor.h"



/*=============================================================================
	Private definitions
=============================================================================*/
//#define AT_CMD_MODE_TIMEOUT_MS			50										//(ms) maximum AT command mode request answer timeout
#define AT_CMD_MODE_TIMEOUT_MS			100										//(ms) maximum AT command mode request answer timeout

/*=============================================================================
	Types
=============================================================================*/

/*=============================================================================
	Private functions
=============================================================================*/

static void ProcessRx(u8* RxBuf, u8 Rxsize);
static u32 GetEmissionTimeout_ms(u8 payloadLen, u8 spreadingFactor);
static u8 AsciiCharToByte(char c);
static void SendConfig(void);
static void ProcessRx_SetValue(u8* RxBuf, u8 Rxsize);
/*=============================================================================
	Private variables
=============================================================================*/
static const u8 cmd_EnterATCmdMode[3] = {'+','+','+' };								// Enter AT command mode
static const char cmd_CfgEntryAck[0x29] = { '\r','\n','A','R','M','-',
	'L','O','R','A','W','A','N',' ','-',' ','W','e','l','c','o','m','e',' ',
	'i','n',' ','s','e','t','u','p',' ','m','o','d','e',' ','-','\r','\n' };    // Expected answer to AT command mode

static const u8 cmd_ReadClassC[7] = { 'A','T','M','0','0','0','\r' };
static const u8 cmd_ReadClassCAckStart[9] = { '\r','\n','M','0','0','0','=','0','9'};// Expected answer to ATM000 configuration request

static const u8 cmd_CfgClassC[10] = { 'A','T','M','0','0','0','=','0','9','\r' }; //class C
static const u8 cmd_CfgClassCAckStart[9] =  { '\r','\n','M','0','0','0','=','0','9'};// Expected answer to ATM000 configuration request
static const u8 cmd_CfgClassCAck_length = 22;	

static const u8 cmd_ReadLeds[7] = { 'A','T','M','0','0','2','\r' };
static const u8 cmd_ReadLedsAckStart[9] = { '\r','\n','M','0','0','2','=','1','7'};// Expected answer to ATM002 configuration request

static const u8 cmd_CfgLeds[10] = { 'A','T','M','0','0','2','=','1','7','\r' }; //class C
static const u8 cmd_CfgLedsAckStart[9] =  { '\r','\n','M','0','0','2','=','1','7'};// Expected answer to ATM002 configuration request
static const u8 cmd_CfgLedsAck_length = 22;	

static const u8 cmd_Cfg83[10] = { 'A','T','O','0','8','3','=','7','F','\r' };
static const u8 cmd_Cfg83AckStart[9] = { '\r','\n','O','0','8','3','=','7','F'};// Expected answer to Cfg83 configuration request

static const u8 cmd_Read83[7] = { 'A','T','O','0','8','3','\r' };
static const u8 cmd_Read83AckStart[9] = { '\r','\n','O','0','8','3','=','7','F'};// Expected answer to Cfg83 configuration request

static const u8 cmd_Save[5] = { 'A','T','O','S','\r' };
static const u8 cmd_SaveAckStart[4] = { '\r','\n','O','S'};// Expected answer to ATOS configuration request

static const u8 cmd_MSave[5] = { 'A','T','M','S','\r' };
static const u8 cmd_MSaveAckStart[15] = { '\r','\n','R','A','M',' ','=','>',' ','E','E','P','R','O','M'};// Expected answer to ATMS configuration request

static const u8 cmd_Reset[5] = { 'A','T','R','\r' };
//static const u8 cmd_ResetAckStart[4] = { '\r','\n','R'};// Expected answer to Cfg83 configuration request

static const u8 cmd_ExitATCmdMode[4] = { 'A','T','Q','\r' };						// Exit AT command mode
static const char cmd_CfgExitAckv2[] = { '\r','\n','\r','\n','Q','u','i','t',' '
							,'s','e','t','u','p',' ','m','o','d','e','\r','\n' };// Expected answer to AT command mode exit 
static const char cmd_CfgExitAckv3[] = { '\r','\n','Q','u','i','t',' '
							,'s','e','t','u','p',' ','m','o','d','e','\r','\n' };// Expected answer to AT command mode exit 
							
static const char cmd_devEui[7] = {'A','T','O','0','7','0','\r'};
static const u8 cmd_getDevEUIAck[7] = { '\r','\n','O','0','7','0','='};
static const u8 cmd_getDevEUIAckLength = 25;

static const char cmd_getSF[7] = {'A','T','O','2','1','8','\r'};
static const u8 cmd_getSFAck[4] = { '\r','\n','=','0'};

static const char cmd_getJoined[7] = {'A','T','O','2','0','1','\r'};
static const u8 cmd_getJoinedAck[4] = { '\r','\n','=','0'};

static const char cmd_disableLogs[9] = {'A','T','M','0','4','=','0','0','\r'};
static const u8 cmd_getdisableLogsAck[4] = { '\r','\n','=','0'};

static const char cmd_getFwInfo[4] = {'A','T','V','\r'};
static const u8 cmd_getFwInfoAckv2[] = { '\r','\n','\r','\n','R', 'e', 'v', ':'};
static const u8 cmd_getFwInfoAckv3[] = { '\r','\n','A','R','M','-','N','8','-','L','W',' ','R', 'e', 'v', ':','3','.','2','.','3'};
//static const u8 cmd_getFwInfoAckv5[] = { '\r','\n','A','R','M','-','N','8','-','L','W',' ','R', 'e', 'v', ':','5','.','1','.','1'};

bool readyToSend = false;
static u32 fwVersion; 
static u8 outBox[32];
static u8 outBoxLen = 0;
static u8 currentSF;
static bool joined = false;
static u32 lastStatusUpdate = 0;
static u32 lastTx = 0;
static u8 majorVersion = 2;
static u8 RxMsg[64];

/*=============================================================================
	Radio Task
	
	Pour test, cette tache
	- Attend que les ports de communication soient initialisés puis
	- passe l'ARMN8 en mode commande AT
	- vérifie que le module répond "... Welcome ect."
	- Configure le registre 75 à 0
	- Vérifie la réponse "ATO075=00"
	
	- quitte le mode commande AT
	- vérifie la réponse "Quit setup mode"
	- Passe RadioStatus à RADIO_IDLE
=============================================================================*/
void RadioTask( void )
{
u8 *RxBuf;
u8 RxSize;
u8 i;
CfgStatus_t *CfgStatus;
CfgId_t * CfgId;
	
static enum { RADIO_RST, RADIO_GOTO_CFG_MODE, RADIO_CHECK_M00, RADIO_CFG_M00, RADIO_CHECK_M02, RADIO_CFG_M02, RADIO_ATMS, RADIO_ATMS_ACK, RADIO_CFG75, RADIO_CFG76, RADIO_CFG77, RADIO_CHK83, RADIO_CFG83, RADIO_GETADDR, RADIO_GOTO_DATA_MODE, RADIO_CFGSAVE, RADIO_IDLE, RADIO_GETSF, RADIO_GETJOINED, RADIO_GETFWINFO, RADIO_GOTO_CMD_MODE }RadioStatus;

switch ( ARMN8Task() )
{

	case ARMN8_IDLE :
		if ( RadioStatus == RADIO_RST )
		{	
			ARMN8StartSending( (u8 *)cmd_EnterATCmdMode, sizeof(cmd_EnterATCmdMode), AT_CMD_MODE_TIMEOUT_MS );
			RadioStatus = RADIO_GOTO_CFG_MODE;
		}
		else if ( RadioStatus == RADIO_IDLE )
		{
			//only send data if network has been joined ; stall otherwise
			if (joined == true)
			{
				if (outBoxLen != 0)	//check if we have messages to send
				{	
						ARMN8StartSending( (u8*) outBox, outBoxLen, 1000 );
						outBoxLen = 0;
						lastTx = GetusTick();
				}
			}
			else {
				if (((lastStatusUpdate == 0) || ((GetusTick() - lastStatusUpdate) > 5000000)))		//must update status (joined AND sf) before sending, at least every xx us
				{
					//entrer command mode to get status update
					ARMN8StartSending( (u8 *)cmd_EnterATCmdMode, sizeof(cmd_EnterATCmdMode), AT_CMD_MODE_TIMEOUT_MS );
					RadioStatus = RADIO_GOTO_CMD_MODE;
				}
			}
		}
		else
		{// Error case should never happen
			RadioStatus = RADIO_RST;
			//ARMN8Reset();
		}
		break;

	case ARMN8_DATA_READY :
		RxBuf = ARMN8RxBufPtr();
		RxSize = ARMN8RxDataSize();
	
		if ( RadioStatus == RADIO_GOTO_CFG_MODE )
		{
			RadioStatus = RADIO_RST;											// Retry full reset configuration if answer is not exactly what's expected
			if ( (RxSize == sizeof(cmd_CfgEntryAck)) && ( 0 == memcmp( RxBuf, cmd_CfgEntryAck, sizeof(cmd_CfgEntryAck) ) ) )
			{
				ARMN8StartSending( (u8 *)cmd_ReadClassC, sizeof(cmd_ReadClassC), AT_CMD_MODE_TIMEOUT_MS );
				RadioStatus = RADIO_CHECK_M00;
			}
		}
		else if ( RadioStatus == RADIO_GOTO_CMD_MODE )
		{
			RadioStatus = RADIO_RST;											// Retry full reset configuration if answer is not exactly what's expected
			if ( (RxSize == sizeof(cmd_CfgEntryAck)) && ( 0 == memcmp( RxBuf, cmd_CfgEntryAck, sizeof(cmd_CfgEntryAck) ) ) )
			{
				ARMN8StartSending( (u8 *)cmd_getJoined, sizeof(cmd_getJoined), AT_CMD_MODE_TIMEOUT_MS );
				RadioStatus = RADIO_GETJOINED;
			}
		}
		else if ( RadioStatus == RADIO_CHECK_M00 )	//class C
		{
			if (RxSize >= sizeof(cmd_ReadClassCAckStart))
			{
				if ( 0 == memcmp( RxBuf, cmd_ReadClassCAckStart, 9 ) )
				{
					ARMN8StartSending((u8 *)cmd_ReadLeds, sizeof(cmd_ReadLeds), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CHECK_M02;
					//ARMN8StartSending((u8 *)cmd_Read83, sizeof(cmd_Read83), AT_CMD_MODE_TIMEOUT_MS);
					//RadioStatus = RADIO_CHK83;

				}
				else {
					ARMN8StartSending((u8 *)cmd_CfgClassC, sizeof(cmd_CfgClassC), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CFG_M00;
				}
			}
		}
		else if ( RadioStatus == RADIO_CFG_M00 )	
		{
		
			if (RxSize == cmd_CfgClassCAck_length)
				if( 0 == memcmp( RxBuf, cmd_CfgClassCAckStart, sizeof(cmd_CfgClassCAckStart) ) )
				{
					
					ARMN8StartSending((u8 *) cmd_ReadLeds, sizeof(cmd_ReadLeds), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CHECK_M02;
					//save to eeprom
					//ARMN8StartSending((u8 *) cmd_MSave, sizeof(cmd_MSave), AT_CMD_MODE_TIMEOUT_MS);
					//RadioStatus = RADIO_ATMS;
				}
			
		}
		///
		else if ( RadioStatus == RADIO_CHECK_M02 )	//Led config
		{
			if (RxSize >= sizeof(cmd_ReadLedsAckStart))
			{
				if ( 0 == memcmp( RxBuf, cmd_ReadLedsAckStart, 9 ) )
				{
					ARMN8StartSending((u8 *)cmd_Read83, sizeof(cmd_Read83), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CHK83;

				}
				else {
					ARMN8StartSending((u8 *)cmd_CfgLeds, sizeof(cmd_CfgLeds), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CFG_M02;
				}
			}
		}
		else if ( RadioStatus == RADIO_CFG_M02 )	
		{
		
			if (RxSize == cmd_CfgLedsAck_length)
				if( 0 == memcmp( RxBuf, cmd_CfgLedsAckStart, sizeof(cmd_CfgLedsAckStart) ) )
				{
					//save to eeprom
					ARMN8StartSending((u8 *) cmd_MSave, sizeof(cmd_MSave), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_ATMS;
				}
			
		}
		///
		else if ( RadioStatus == RADIO_ATMS )	//save
		{
			if ((RxSize >= sizeof(cmd_MSaveAckStart)) && ( 0 == memcmp( RxBuf, cmd_MSaveAckStart, sizeof(cmd_MSaveAckStart) ) ) )
			{
				ARMN8StartSending((u8 *)cmd_Read83, sizeof(cmd_Read83), AT_CMD_MODE_TIMEOUT_MS);
				RadioStatus = RADIO_CHK83;

			}
		}
		else if ( RadioStatus == RADIO_CHK83 )	//check setup
		{
			if ( (RxSize > sizeof(cmd_Read83AckStart)) && ( 0 == memcmp( RxBuf, cmd_Read83AckStart, 5 ) ) )
			{
				//check if we have expected config
				if ( 0 == memcmp( RxBuf, cmd_Read83AckStart, sizeof(cmd_Read83AckStart)))
				{
					//ok : jump to get addr
					ARMN8StartSending((u8 *)cmd_devEui, sizeof(cmd_devEui), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_GETADDR;
				}
				else {
					//not ok: configure OTAA
					ARMN8StartSending((u8 *)cmd_Cfg83, sizeof(cmd_Cfg83), AT_CMD_MODE_TIMEOUT_MS);
					RadioStatus = RADIO_CFG83;
				}
			}
		}
		else if ( RadioStatus == RADIO_CFG83 )	//ABP/OTA
		{
			if ( (RxSize > sizeof(cmd_Cfg83AckStart)) && ( 0 == memcmp( RxBuf, cmd_Cfg83AckStart, sizeof(cmd_Cfg83AckStart) ) ) )
			{
					
				ARMN8StartSending((u8 *)cmd_Save, sizeof(cmd_Save), AT_CMD_MODE_TIMEOUT_MS);
				RadioStatus = RADIO_CFGSAVE;
			}
		}
		else if ( RadioStatus == RADIO_CFGSAVE )	//save and reset
		{
			if ( (RxSize > sizeof(cmd_SaveAckStart)) && ( 0 == memcmp( RxBuf,cmd_SaveAckStart, sizeof(cmd_SaveAckStart) ) ) )
			{
					
				ARMN8StartSending((u8 *)cmd_Reset, sizeof(cmd_Reset), AT_CMD_MODE_TIMEOUT_MS);
				RadioStatus = RADIO_RST;
			}
		}
		else if ( RadioStatus == RADIO_GETADDR )	//get devEUI
		{
			if ( (RxSize == cmd_getDevEUIAckLength) && ( 0 == memcmp( RxBuf, cmd_getDevEUIAck, sizeof(cmd_getDevEUIAck) ) ) )
			{
				CfgId = CfgIdPointer();
				//read device ID in reverse order 
					for (i=0; i<16; i=i+2)
						CfgId->devEUI[7 - i/2] = (AsciiCharToByte(RxBuf[7+i]) << 4) + AsciiCharToByte(RxBuf[7+i+1]);
				
				ARMN8StartSending( (u8 *)cmd_getFwInfo, sizeof(cmd_getFwInfo), AT_CMD_MODE_TIMEOUT_MS );
				RadioStatus = RADIO_GETFWINFO;
			}
		}
		else if ( RadioStatus == RADIO_GETFWINFO )	//get firmware information
		{
			if ( RxSize > 19   )
			{
				fwVersion = 0;
				if ( 0 == memcmp( RxBuf, cmd_getFwInfoAckv2, sizeof(cmd_getFwInfoAckv2)))
				{
					//memcpy(fwinfo, RxBuf+8, 12);
					fwVersion = 0x02000000;
					fwVersion  +=  (((RxBuf[8] - 48) * 10) + (RxBuf[9] - 48)) << 16;
					majorVersion = 2;
				}
				else if ( 0 == memcmp( RxBuf, cmd_getFwInfoAckv3, sizeof(cmd_getFwInfoAckv3)))
				{

					fwVersion  += ((u8) RxBuf[16] - 48) << 24;
					fwVersion  += ((u8) RxBuf[18] - 48) << 16;
					fwVersion  += ((u8) RxBuf[20] - 48) << 8;
					//memcpy(fwinfo, RxBuf+16, 12); 
					majorVersion = 3;
				}
				//else if ( 0 == memcmp( RxBuf, cmd_getFwInfoAckv5, sizeof(cmd_getFwInfoAckv5)))
				//{
				//	fwVersion  += ((u8) RxBuf[16] - 48) << 24;
				//	fwVersion  += ((u8) RxBuf[18] - 48) << 16;
				//	fwVersion  += ((u8) RxBuf[20] - 48) << 8;
				//	//memcpy(fwinfo, RxBuf+16, 12); 
				//	majorVersion = 5;
				//}
			}
			ARMN8StartSending( (u8 *)cmd_getJoined, sizeof(cmd_getJoined), AT_CMD_MODE_TIMEOUT_MS );
			RadioStatus = RADIO_GETJOINED;
		}
		else if ( RadioStatus == RADIO_GETJOINED )	//get joined status
		{
			if ( (RxSize == 7) && ( 0 == memcmp( RxBuf, cmd_getJoinedAck, sizeof(cmd_getJoinedAck) ) ) )
			{
				if (RxBuf[4] == '1')
				{
					joined = true;
				}
				else {
					joined = false;
				}
				lastStatusUpdate = GetusTick();
				CfgStatus = CfgStatusPointer();
				CfgStatus->IsRadioConfigured = true;
				CfgStatus->IsRadioJoined = joined;
				// Nothing more to configure : exit CFG mode
				ARMN8StartSending( (u8 *)cmd_ExitATCmdMode, sizeof(cmd_ExitATCmdMode), AT_CMD_MODE_TIMEOUT_MS );
				RadioStatus = RADIO_GOTO_DATA_MODE;
			}
		}
		else if ( RadioStatus == RADIO_GOTO_DATA_MODE )
		{
			if (
				((majorVersion == 2) && (RxSize==sizeof(cmd_CfgExitAckv2)) && ( 0 == memcmp( RxBuf, cmd_CfgExitAckv2, sizeof(cmd_CfgExitAckv2))))
				||	
				((majorVersion >= 3) && (RxSize==sizeof(cmd_CfgExitAckv3)) && ( 0 == memcmp( RxBuf, cmd_CfgExitAckv3, sizeof(cmd_CfgExitAckv3))))
			)
			{
				// Correct Cfg mode exit : ready to send and receive radio data
				readyToSend = true;
				RadioStatus = RADIO_IDLE;
			}
		}
		else if ( RadioStatus == RADIO_IDLE )
		{
			// Radio Data received while in idle mode
			if (RxSize >= 5)
			{
				memcpy(RxMsg, RxBuf, sizeof(RxMsg));
				ARMN8PurgeRxBuffer();
				Uart2StartReception();
				if (( RxMsg[0] == RADIOMSG_HEADER1 ) && ( RxMsg[1] == RADIOMSG_HEADER2 ))
				{
						ProcessRx(RxMsg, RxSize);
				}
			}
			
		}
		ARMN8PurgeRxBuffer();	// Purge Rx buffer and go back to Idle mode
		break;
	default :// Probably ARMN8 busy sending or waiting for an answer
		break;
	}
}


bool isRadioReady(void)
{
	return readyToSend;
}


void RadioSend(u8 *buffer, u8 len)
{
	if (joined)
	{
	memcpy(outBox, buffer, len);
	outBoxLen = len;
	}
}


bool HasJoinedNetwork()
{
	return joined;
}


/**
Returns rough estimation of total time to wait (transmission + european duty cycle regulatory silence) in ms
**/
u32 GetEmissionTimeout_ms(u8 payloadLen, u8 dataRate)
{
	u8 debugdr = dataRate;
	if (dataRate != 0)
	{
		dataRate = debugdr;
	}
		/*
		0: DR_0 = SF12 - BW125
	1: DR_1 = SF11 - BW125
	2: DR_2 = SF10 - BW125
	3: DR_3 = SF9 - BW125
	4: DR_4 = SF8 - BW125
	5: DR_5 = SF7 - BW125
	6: DR_6 = SF7 - BW250
		*/
	
	if (dataRate > 6)
		dataRate = 6;
	
	//add lora header len
	payloadLen += 13;
	switch (dataRate)
	{
		case 6:
			return payloadLen*65+1664;
		case 5:
			return payloadLen*132+3228;
		case 4:
			return payloadLen*246+5322;
		case 3:
			return payloadLen*453+10545;
		case 2:
			return payloadLen*825+20991;
		case 1:
			return payloadLen*1487+41883;
		case 0:
			return payloadLen*2646+83667;
		default : 
			return 60000; //should never happen
	}
	
}

u32 RadioGetFwVersion()
{
 return fwVersion;
			
}

/***
Process downlink messages.
***/
void ProcessRx(u8* RxBuf, u8 RxSize)
{
	u8 cmd = RxBuf[2];
	u8 i=0;
	u8 j=0;
	u8 val=0;
	u16 val16;
	u16 crc = 0;
	RxCmdSetCalib_t cmdSetCalib;
	RxCmdSetCfg_t cmdSetCfg;
	RxCmdSetMessage_t cmdSetMessage;
	RxCmdReset_t cmdReset;
	RxCmdSetAlert_t cmdSetAlert;
	
	CfgConfig_t *cfgConfig;
	CfgCalib_t *cfgCalib;
	CfgPoll_t *cfgPoll;
	CfgStatus_t *cfgStatus;
	CfgId_t *cfgId;
	Color_t colors;
	
	cfgConfig = CfgConfigPointer();
	
	//configuration command received : parse and apply
	if (cmd == RADIOCMD_SET_CFG) {
		if (RxSize == RxCmdSetCfg_len)
		{
			//check CRC
			cmdSetCfg.crc = (RxBuf[RxCmdSetCfg_len - 1] << 8) +  RxBuf[RxCmdSetCfg_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdSetCfg.crc)
				return;
			
			//read buffer
			i = 3;
			cmdSetCfg.shortMsgInterval_min = RxBuf[i];
			i++;
			cmdSetCfg.longMessageInterval_min = RxBuf[i];
			i++;
			val = RxBuf[i];
			cmdSetCfg.showTemp = val >> 7;
			cmdSetCfg.showHygr = (val & 0x40) >> 6;
			cmdSetCfg.showLux = (val & 0x20) >> 5;
			cmdSetCfg.showNoise = (val & 0x10) >> 4;
			cmdSetCfg.showAir = (val & 0x08) >> 3;
			cmdSetCfg.showToggle = (val & 0x04) >> 2;
			cmdSetCfg.showAirAsValue = (val & 0x02) >> 1;
			cmdSetCfg.disableSound = (val & 0x01);
			i++;
			val = RxBuf[i];
			cmdSetCfg.disableVOC = val >> 7;
			cmdSetCfg.toggleMode = (val & 0x40) >> 6;
			cmdSetCfg.imgToggleLeft = (val & 0x56) >> 3;
			cmdSetCfg.imgToggleRight = (val & 0x07) ;
			i++;
			val = RxBuf[i];
			cmdSetCfg.imgToggleBack = val >> 5;
			cmdSetCfg.extSensorType = val & 0x1f;
			i++;
			
			for (j=0; j<10; j++)
			{
				cmdSetCfg.text_toggle_left[j] = RxBuf[i];
				i++;
			}
			for (j=0; j<10; j++)
			{
				cmdSetCfg.text_toggle_right[j] = RxBuf[i];
				i++;
			}
			for (j=0; j<10; j++)
			{
				cmdSetCfg.text_acknowledgment[j] = RxBuf[i];
				i++;
			}
			
			cmdSetCfg.lang = RxBuf[i];
			i++;
			cmdSetCfg.eventMode = RxBuf[i];
			i++;
			cmdSetCfg.eventFrom = RxBuf[i];
			i++;
			cmdSetCfg.eventThreshold = RxBuf[i];
			i++;
			cmdSetCfg.eventWindow_s = RxBuf[i];
			i++;
			
			//
			cfgConfig = CfgConfigPointer();
			cfgConfig->showAirAsValue = cmdSetCfg.showAirAsValue;
			cfgConfig->disableSound = cmdSetCfg.disableSound;
			cfgConfig->disableVOC = cmdSetCfg.disableVOC;
			
			cfgConfig->eventFrom = cmdSetCfg.eventFrom;
			cfgConfig->eventMode = cmdSetCfg.eventMode;
			cfgConfig->eventThreshold = cmdSetCfg.eventThreshold;
			cfgConfig->eventWindow_s = cmdSetCfg.eventWindow_s;
			cfgConfig->imgToggleBack = cmdSetCfg.imgToggleBack;
			cfgConfig->imgToggleLeft = cmdSetCfg.imgToggleLeft;
			cfgConfig->imgToggleRight = cmdSetCfg.imgToggleRight;
			memcpy(cfgConfig->text_acknowledgment, cmdSetCfg.text_acknowledgment, sizeof(cfgConfig->text_acknowledgment));
			memcpy(cfgConfig->text_toggle_left, cmdSetCfg.text_toggle_left, sizeof(cfgConfig->text_toggle_left));
			memcpy(cfgConfig->text_toggle_right, cmdSetCfg.text_toggle_right, sizeof(cfgConfig->text_toggle_right));
			cfgConfig->lang = cmdSetCfg.lang;
			cfgConfig->longMessageInterval_min = cmdSetCfg.longMessageInterval_min;
			cfgConfig->shortMessageInterval_min = cmdSetCfg.shortMsgInterval_min;
			cfgConfig->showAir = cmdSetCfg.showAir;
			cfgConfig->showHygr = cmdSetCfg.showHygr;
			cfgConfig->showLux = cmdSetCfg.showLux;
			cfgConfig->showNoise = cmdSetCfg.showNoise;
			cfgConfig->showTemp = cmdSetCfg.showTemp;
			cfgConfig->showToggle = cmdSetCfg.showToggle;
			cfgConfig->toggleMode = cmdSetCfg.toggleMode;
			cfgConfig->extSensorType = cmdSetCfg.extSensorType;
			
			CfgConfigSaveToEep();
		}
	}
	else if (cmd == RADIOCMD_RESET)		//calibration values
	{
		if (RxSize == RxCmdReset_len)
		{
			
			cmdReset.crc = (RxBuf[RxCmdReset_len - 1] << 8) +  RxBuf[RxCmdReset_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdReset.crc)
				return;
			
			cmdReset.reboot = RxBuf[3];
			
			if (cmdReset.reboot == RADIOMSG_CMD_SOFT_RESET)
			{
				//force reboot
				while(1);
			}
			else if (cmdReset.reboot == RADIOMSG_CMD_FACTORY_RESET)
			{
				//force factory reset at next startup
				cfgId = CfgIdPointer();
				cfgStatus = CfgStatusPointer();
				cfgId->fwVersion = 0x00;
				cfgStatus->IsBoardInitialized = 0;
				CfgStatusSaveToEep();
				CfgIdSaveToEep();
				//force reboot
				while(1);
			}
				
		}
	}
	else if (cmd == RADIOCMD_SET_ALERT)		//calibration values
	{
		if (RxSize == RxCmdSetAlert_len)
		{
			
			cmdSetAlert.crc = (RxBuf[RxCmdSetAlert_len - 1] << 8) +  RxBuf[RxCmdSetAlert_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdSetAlert.crc)
				return;
			
			i = 3;
			cmdSetAlert.alertMode = RxBuf[i];
			i++;
			
			if ((cmdSetAlert.alertMode != 0) && (cmdSetAlert.alertMode != 4))
				return; //TODO: other modes are not supported
			
			cmdSetAlert.thresholdx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			
			cfgConfig->alertMode = cmdSetAlert.alertMode;
			cfgConfig->alertThresholdx100 = cmdSetAlert.thresholdx100;
			CfgConfigSaveToEep();

				
		}
	}
	else if (cmd == RADIOCMD_SET_CALIB)		//calibration values
	{
		if (RxSize == RxCmdSetCalib_len)
		{
			//check CRC
			cmdSetCalib.crc = (RxBuf[RxCmdSetCalib_len - 1] << 8) +  RxBuf[RxCmdSetCalib_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdSetCalib.crc)
				return;
			
			i = 3;
			cmdSetCalib.offsetTempx100 = (short) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			
			cmdSetCalib.offsetHygrx100 = (short) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			
			cmdSetCalib.gainLuxx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			
			cmdSetCalib.deltadBAx100 = (short) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			cmdSetCalib.gainColorRx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			cmdSetCalib.gainColorGx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			cmdSetCalib.gainColorBx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			cmdSetCalib.gainColorWx100 = (u16) ((RxBuf[i] << 8) + RxBuf[i+1]);
			i += 2;
			
			cfgCalib = CfgCalibPointer();
			
			cfgCalib->AudioCalibOffsetx100 = cmdSetCalib.deltadBAx100;
			cfgCalib->deltaHx100 = cmdSetCalib.offsetHygrx100;
			cfgCalib->deltaTx100 = cmdSetCalib.offsetTempx100;
			cfgCalib->luxGainx100 = cmdSetCalib.gainLuxx100;
			cfgCalib->gainColorRx100 = cmdSetCalib.gainColorRx100;
			cfgCalib->gainColorGx100 = cmdSetCalib.gainColorGx100;
			cfgCalib->gainColorBx100 = cmdSetCalib.gainColorBx100;
			cfgCalib->gainColorWx100 = cmdSetCalib.gainColorWx100;
			CfgCalibSaveToEep();
		}
	}
	else if (cmd == RADIOCMD_REQUEST_CFG)		//configuration settings request
	{
		if (RxSize >= RxCmdGetConfig_len)
		{
			//TO BE IMPLEMENTED
		}
	}
	else if (cmd == RADIOCMD_SET_VALUE)		//configuration settings request
	{
		ProcessRx_SetValue(RxBuf, RxSize);
	}
	else if (cmd == RADIOCMD_SET_DISPLAY) {
		if (RxSize == RxCmdSetMessage_len)			//a bit dangerous but the RX buffer seems to keep messages
		{
			//check CRC
			cmdSetMessage.crc = (RxBuf[RxCmdSetMessage_len - 1] << 8) +  RxBuf[RxCmdSetMessage_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdSetMessage.crc)
				return; 
			
			//bit fields are not properly unpacked, correct
			i = 3;
			val = RxBuf[i];
			cmdSetMessage.endOnToggle = (val & 0x80) >> 7;
			cmdSetMessage.endOnReboot = (val & 0x40) >> 6;
			cmdSetMessage.responseChoices = (val & 0x38) >> 3;
			cmdSetMessage.acknowlegment = val & 0x07;
			i++;
			cmdSetMessage.expiresAfter_hours = RxBuf[i];
			i++;
			cmdSetMessage.repeat_every_hours = RxBuf[i];
			i++;
			for (j=0; j < DISPLAY_MSG_MAX_LENGTH; j++)
			{
				cmdSetMessage.text[j] = RxBuf[i];
				i++;
			}
			
			cfgPoll = CfgPollPointer();
			cfgConfig = CfgConfigPointer();
			cfgStatus = CfgStatusPointer();
			
			cfgPoll->pollMode = POLLMODE_TEXT;
			
			cfgPoll->PollEndsOnReboot  = cmdSetMessage.endOnReboot;
			cfgPoll->pollEndsOnToggle  = cmdSetMessage.endOnToggle;
			cfgPoll->pollResponseChoices = (cmdSetMessage.responseChoices > 2) ? 0:cmdSetMessage.responseChoices;
			cfgPoll->pollAcknowlegment = (cmdSetMessage.acknowlegment > 2) ? 0 : cmdSetMessage.acknowlegment;
			
			cfgPoll->pollDuration_hour = cmdSetMessage.expiresAfter_hours;
			cfgPoll->pollRepeat_hour = cmdSetMessage.repeat_every_hours;
			
			cfgStatus->pollTimeRemaining_hour = cfgPoll->pollDuration_hour;
			
			memcpy(&(cfgPoll->text), &(cmdSetMessage.text), sizeof(cmdSetMessage.text));
			
			//save config
			CfgStatusSaveToEep();
			CfgConfigSaveToEep();
			CfgPollSaveToEep();
		}
	}
}

void ProcessRx_SetValue(u8* RxBuf, u8 RxSize)
{
	RxCmdSetValue_t cmdSetValue;
	u16 crc, val16;
	CfgCalib_t* cfgCalib;
	Color_t colors;
	u8 i;
	union {
			float f;
			unsigned long ul;
	 } u;
	
	 if (RxSize == RxCmdSetValue_len)
		{
			//check CRC
			cmdSetValue.crc = (RxBuf[RxCmdSetValue_len - 1] << 8) +  RxBuf[RxCmdSetValue_len - 2];
			crc = Crc16_ccit_false(RxBuf, RxSize - 2);
			if (crc != cmdSetValue.crc)
				return;
			
			cfgCalib = CfgCalibPointer();
			
			i = 3;
			cmdSetValue.valueType = RxBuf[3];
			i++;
			
			u.ul = (RxBuf[i+3] << 24) | (RxBuf[i+2] << 16) | (RxBuf[i+1] << 8) | RxBuf[i];
			cmdSetValue.value = u.f;
	
			if (cmdSetValue.valueType == MSG_VALUETYPE_TEMP)
			{
				cfgCalib->deltaTx100 = (cmdSetValue.value*100) - Temperaturex100();
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_HYGR)
			{
				cfgCalib->deltaHx100 = (cmdSetValue.value*100) - Hygrometryx100();
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_DBA)
			{
				cfgCalib->AudioCalibOffsetx100 = (cmdSetValue.value*100) - dBAx100();
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_LUX)
			{
				val16 = Lux();
				if (val16 > 0)
					cfgCalib->luxGainx100 = ((cmdSetValue.value) / (float)val16) * 100;
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_COLORR)
			{
				colors = GetColors();
				val16 = colors.Red;
				if (val16 > 0)
					cfgCalib->gainColorRx100 = ((cmdSetValue.value) / (float)val16) * 100;
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_COLORG)
			{
				colors = GetColors();
				val16 = colors.Green;
				if (val16 > 0)
					cfgCalib->gainColorGx100 = ((cmdSetValue.value) / (float)val16) * 100;
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_COLORB)
			{
				colors = GetColors();
				val16 = colors.Blue;
				if (val16 > 0)
					cfgCalib->gainColorBx100 = ((cmdSetValue.value) / (float)val16) * 100;
			}
			else if (cmdSetValue.valueType == MSG_VALUETYPE_COLORW)
			{
				colors = GetColors();
				val16 = colors.White;
				if (val16 > 0)
					cfgCalib->gainColorWx100 = ((cmdSetValue.value) / (float)val16) * 100;
			}
			CfgCalibSaveToEep();
		}
}

void SendConfig()
{
	// TO BE IMPLEMENTED
			/*
			i=0;
			TxBuf[i] = MSGV2_CFG;
			i++;
			
			//firmware version (e.g. 1.14) : major (1) 
			val = (u8) FWVERSION;
			memcpy(TxBuf + i, &val, 1);
			i += 1;
			
			//firmware version : minor (14)
			val16 = (u16) (FWVERSION - val)*100;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			
			//CCS811 firwmare version (eg. 0x1000)
			val16 = CCS811_GetFwVersion();
			TxBuf[i] = (u8) FWVERSION;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			
			//ATIM fw version (eg. '3.2.3' converted to 0x030203)
			memcpy(TxBuf + i, &fwVersion, 3);
			i += 3;
			
			//configuration parameters
			val16 = cfgConfig->shortMessageInterval_min*60;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;

			val16 = cfgConfig->longMessageInterval_hour*3600;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;

//TODO
			//val = cfgConfig->msgSoundEventEnabled;
			//memcpy(TxBuf + i, &val, 1);
			i += 1;

			//val = cfgConfig->.msgSoundEventNbSamples;
			//memcpy(TxBuf + i, &val, 1);
			i += 1;

			val = cfgConfig->eventThreshold;
			memcpy(TxBuf + i, &val, 1);
			i += 1;

			val = cfgConfig->eventFrom;
			memcpy(TxBuf + i, &val, 1);
			i += 1;

			val = 0;
			if (cfgConfig->showTemp)
				val = 0x01;
			if (cfgConfig->showHygr)
				val += 0x02;
			if (cfgConfig->showLux)
				val += 0x04;
			if (cfgConfig->showNoise)
				val += 0x08;
			if (cfgConfig->showAir)
				val += 0x10;
			if (cfgConfig->showToggle)
				val += 0x20;
			if (cfgConfig->BLEBeaconEnabled)
				val += 0x40;
			if (cfgConfig->pollMode)
				val += 0x80;
			memcpy(TxBuf + i, &val, 1);
			i += 1;
			
			val16 = cfgConfig->pollDuration_hour * >> 16;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			val16 = (u16) cfgConfig->.pollInterval_s ;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			
			val16 = cfgConfig->.pollDuration_s >> 16;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			val16 = (u16) cfgConfig->.pollDuration_s ;
			memcpy(TxBuf + i, &val16, 2);
			i += 2;
			val = cfgConfig->.lang;
			memcpy(TxBuf + i, &val, 1);
			i += 1;
			
			//send now
			RadioSend(TxBuf, i);
			*/
			
}


u8 AsciiCharToByte(char c)
{
	u8 b = 0;
	if ((c > 47) && (c < 58)) //number
	{
		b = (u8) c - 48;
	}
	else if ((c > 64) && (c < 71)) //capital letters
	{
		b = (u8) c - 65 + 0x0A;
	}
	else if ((c > 96) && (c < 103)) //letters
	{
		b = (u8) c - 97 + 0x0A;
	}
	return b;
}
