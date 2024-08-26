#include "Messaging.h"
#include "CubeData.h"
#include "Cfg.h"
#include "string.h"
#include "definitions.h"
#include "Radio.h"
#include "dBAMeter.h"
#include "Averages.h"


/*************************************************************

makeRadioMessage: buid radio message

*****************************************************************/
void MakeRadioMessage(u8 msgType, CubeData_t* data, Averages_t* avg)
{
	CfgId_t *cfgIdPtr;
	CfgCalib_t *cfgCalibPtr;
	CfgConfig_t *cfgConfigPtr;
	u8 extMsgType;
	
	u8 radioSndBuffer[64];
	u8 i=0, j;
	u8 val;
	u32 result = 0;
	u16 val16 = 0;
	u8 batteryStatus = 0;
	u16* OctavesPtr;
	u16 tempx100, humx100, noiseMaxx2, noiseAvgx2, lux; 

	//enum {STATUS_NOT_UPRIGHT, STATUS_WARMING_UP, STATUS_RUNNING, STATUS_RECOVERING, STATUS_UNUSED, STATUS_STARTING_UP } runningState = STATUS_RUNNING;
	//#define STATUS_NOT_UPRIGHT  0			//board is not upright
	//#define STATUS_WARMING_UP   1			//board initialized 
	//#define STATUS_RUNNING      2			//board has reached nominal temperature
	//#define STATUS_RECOVERING   3			//board has been moved, manipulated 
	//#define STATUS_STARTING_UP  5			//board has just been powered up, initializing

	cfgIdPtr = CfgIdPointer();
	cfgCalibPtr = CfgCalibPointer();
	cfgConfigPtr = CfgConfigPointer();
	
	tempx100 = (u16) (data->Tempx100 + cfgCalibPtr->deltaTx100);
	humx100 = (u16) (data->Hygrx100 + cfgCalibPtr->deltaHx100);
	noiseMaxx2 = data->noiseMaxx2 + cfgCalibPtr->AudioCalibOffsetx100/50;
	noiseAvgx2 = avg->sum_noisex2 / avg->nb + cfgCalibPtr->AudioCalibOffsetx100/50;
	lux = (avg->sum_lux /avg->nb) * cfgCalibPtr->luxGainx100/100.0;
	
	batteryStatus = 0x80;
	
	//pack intro byte : feel (2 bits) + status (2 bits) + msgtype (4bits)
	val = 0;
	if (data->happy == 1)
	{
		val = 0x80;	//binary : 1000 0000
	}
	else if(data->happy == 2)
	{
		val = 0x40;	//binary : 0100 0000
	}
	
	//add running state (2 bits)
	//if feeling has been expressed, discard bad position status (0x00 status)
	if ((data->Orientation == UP) || (data->happy != 0))
	{
		val += 0x20; //binary : 0010 0000
	}
	
	//add message type (4 bits)
	val += msgType;
	memcpy(radioSndBuffer + i, &val, 1);
	i++;
	
	
	if (msgType == MSGV2_FEEL)
	{		
		//temperature
		memcpy(radioSndBuffer + i, &tempx100, 2);
		i+=2;
		
		//humidity
		memcpy(radioSndBuffer + i, &humx100, 2);
		i+=2;
		
		//dBA
		memcpy(radioSndBuffer + i, &(noiseMaxx2), 1);
		i+=1;
		
		//dBA again : no dba max when feel expressed
		val = data->noiseAveragex2 + cfgCalibPtr->AudioCalibOffsetx100/50;
		memcpy(radioSndBuffer + i, &val, 1);
		i+=1;

		//Lux
		memcpy(radioSndBuffer + i, &(lux), 2);
		i+=2;
		
		//colors
		val = avg->sum_colorR/avg->nb;
		memcpy(radioSndBuffer + i, &(val), 1);
		i+=1;
		val = avg->sum_colorG/avg->nb;
		memcpy(radioSndBuffer + i, &(val), 1);
		i+=1;
		val = avg->sum_colorB/avg->nb;
		memcpy(radioSndBuffer + i, &(val), 1);
		i+=1;
		val16 = avg->sum_colorW/avg->nb;
		memcpy(radioSndBuffer + i, &(val16), 2);	
		i+=2;
		//flicker
		result = avg->sum_flicker/avg->nb;
		memcpy(radioSndBuffer + i, &(result), 1);
		i+=1;

		//octaves
		OctavesPtr = GetOctaves();
		for (j=1; j<NB_OCTAVES; j++)
		{
			val = (u8) (OctavesPtr[j]*2.0/100.0);
			memcpy(radioSndBuffer + i, &val, 1);
			i++;
		}
	}
	else if ((msgType == MSGV2_SHORT) || (msgType == MSGV2_FULL))
	{
		//temperature
		memcpy(radioSndBuffer + i, &tempx100, 2);
		i+=2;

		//humidity
		memcpy(radioSndBuffer + i, &humx100, 2);
		i+=2;
			
		//dBAMax
		memcpy(radioSndBuffer + i, &noiseMaxx2, 1);
		i+=1;

		//dBAMoy
		memcpy(radioSndBuffer + i, &noiseAvgx2, 1);
		i+=1;

		//Lux
		memcpy(radioSndBuffer + i, &lux, 2);
		i+=2;

		if (msgType == MSGV2_FULL) {
			//colors
			val = avg->sum_colorR/avg->nb;
			memcpy(radioSndBuffer + i, &(val), 1);
			i+=1;
			val = avg->sum_colorG/avg->nb;
			memcpy(radioSndBuffer + i, &(val), 1);
			i+=1;
			val = avg->sum_colorB/avg->nb;
			memcpy(radioSndBuffer + i, &(val), 1);
			i+=1;
			val16 = avg->sum_colorW/avg->nb;
			memcpy(radioSndBuffer + i, &(val16), 2);	
			i+=2;
			//flicker
			result = avg->sum_flicker/avg->nb;
			memcpy(radioSndBuffer + i, &(result), 1);
			i+=1;

			//octaves
			OctavesPtr = GetOctaves();
			for (j=1; j<NB_OCTAVES; j++)
			{
				result = OctavesPtr[j-1] *2.0 /100.0;
				val = (u8) (result);
				memcpy(radioSndBuffer + i, &val, 1);
				i++;
			}
		}
	}
	else if (msgType == MSG_AUDIO)
	{
		//TODO !!
		/*noiseAvg = (u8) dBALastAvg_event*2;
		
		//dBA
		memcpy(radioSndBuffer + i, &(noiseAvg), 1);
		i+=1;
		
		lastEventTxTick = GetusTick();
		*/
	}
	
	//add V2 data
	if ((msgType == MSGV2_FEEL) || (msgType == MSGV2_FULL) || (msgType == MSGV2_SHORT))
	{
		memcpy(radioSndBuffer + i, &batteryStatus, 1);
		i += 1;
		
		
		if ((cfgIdPtr->CCS811FwVersion == 0) || (cfgConfigPtr->disableVOC == true))
		{
			if (cfgConfigPtr->extSensorType == EXT_SENSOR_CO2)
				extMsgType = MSGEXT_CO2_ONLY;
			else if (cfgConfigPtr->extSensorType == EXT_SENSOR_PM)
				extMsgType = MSGEXT_PM_ONLY;
		}
		else {
			if (cfgConfigPtr->extSensorType == EXT_SENSOR_CO2)
				extMsgType = MSGEXT_COV_CO2;
			else if (cfgConfigPtr->extSensorType == EXT_SENSOR_PM)
				extMsgType = MSGEXT_COV_PM;
			else 
				extMsgType = MSGEXT_COV_ONLY;
		}
		
		val = 0;
		if (extMsgType == MSGEXT_COV_CO2)	//both CO2 and COV are mounted
		{
			val = extMsgType;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
			val16 = (u16) data->VOCppb;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i +=2;
			val16 = (u16) data->CO2ppm;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i +=2;
		}
		else if (extMsgType == MSGEXT_COV_PM)	//both CO2 and COV are mounted
		{
			val = extMsgType;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
			val16 = (u16) data->VOCppb;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i +=2;
			val16 = data->pm1;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
			val16 = data->pm2_5;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
			val16 = data->pm10;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
		}
		else if (extMsgType == MSGEXT_COV_ONLY)							//only COV is mounted
		{
			val = extMsgType;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
			val16 = (u16) data->VOCppb;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
		}
		else if (extMsgType == MSGEXT_CO2_ONLY)										//only CO2 is mounted
		{
			val = extMsgType;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
			val16 = data->CO2ppm;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
		}
		else if (extMsgType == MSGEXT_PM_ONLY)										//only CO2 is mounted
		{
			val = extMsgType;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
			val16 = data->pm1;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
			val16 = data->pm2_5;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
			val16 = data->pm10;
			memcpy(radioSndBuffer + i, &(val16), 2);
			i += 2;
		}
		else 
		{
			//no extMsgType : default is 0
			val = 0;
			memcpy(radioSndBuffer + i, &val, 1);
			i += 1;
		}
	}
	
	RadioSend(radioSndBuffer, i);
	
	//reset dBA max
	data->noiseMaxx2 = data->dBAx100 / 100.0 * 2;		

}
