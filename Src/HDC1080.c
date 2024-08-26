#include "HDC1080.h"
#include "I2C2.h"
#include "Endianess.h"
#include "Debug.h"
#include "Filters.h"
#include "definitions.h"

/*=============================================================================
	PRIVATE DEFINITIONS
=============================================================================*/

#define HDC1080_ADDR		0x40												// HDC1080 I2C slave address

#define TEMP_REG_ADDR		0x00												// Temperature measurement register address
#define HUM_REG_ADDR		0x01												// Humidity measurement reegister address
#define CFG_REG_ADDR		0x02												// Configuration register address

/*=============================================================================
	PRIVATE VARIABLES
=============================================================================*/
static u32 FilteredTempCode=0;													// Filtered temperature code returned by Hdc1080
static u32 FilteredRHCode=0;													// Filtered Humidity code returned by Hdc1080
static bool IsHeaterForcedOn = false;											// true when Humidity purge Heater command is active

/*=============================================================================
	Hum Temp Cfg
	
	reconfigures the sensor on the go
	
	deals with Heater ON/OFF requests
=============================================================================*/
void HumTempCfg( void )
{
static const u8 HeaterOnCfg[3] = { CFG_REG_ADDR, 0x36, 0x00 };
static const u8 HeaterOffCfg[3] = { CFG_REG_ADDR, 0x16, 0x00 };

if ( IsHeaterForcedOn )
	{// Heater ON mode
	I2C2StartWrite( HDC1080_ADDR, (u8 *)HeaterOnCfg, sizeof(HeaterOnCfg) );			// Turn heater On
	}
else
	{// Writes maximum resolution and heating power off to chip's Cfg
	I2C2StartWrite( HDC1080_ADDR, (u8 *)HeaterOffCfg, sizeof(HeaterOffCfg) );
	}
}

/*=============================================================================
	Hum Temp Task Done
	
	sends humidity and temperature measurement commands
	reads measured values
	
	returns true if I2C bus is available for another peripheral
=============================================================================*/
bool HumTempTask( void )
{
static enum { SELECT, READ }NextCmd = SELECT;
static u8 RegAddr;
static u8 RxSize;
static bool Init = false;
static struct
{
u16 Temp;
u16 Hum;
}Data;

if ( NextCmd == SELECT )
	{
	RegAddr = TEMP_REG_ADDR;
	I2C2StartWrite( HDC1080_ADDR, &RegAddr, sizeof(RegAddr) );
	NextCmd = READ;
	return false;
	}
else
	{
	if ( RxSize == sizeof(Data) )
		{// RxSize data bytes received
		if ( !Init )
			{
			FilteredTempCode = HTONS(Data.Temp);
			FilteredRHCode = HTONS(Data.Hum);
			Init = true;
			}
		else
			{
			FilteredTempCode = Filter(FilteredTempCode,HTONS(Data.Temp),8);
			FilteredRHCode = Filter(FilteredRHCode,HTONS(Data.Hum),8);
			}
		}
	// Start reading a new value
	RxSize = sizeof(Data);
	I2C2StartRead( HDC1080_ADDR, (u8 *)&Data, &RxSize );
	NextCmd = SELECT;
	return true;
	}
}

/*=============================================================================
	TEMPERATURE
	
	returns filtered temperature ( °C )
=============================================================================*/
float HDC1080Temp( void )
{
return (165.0/65536.0)*(float)FilteredTempCode - 40.0 + TEMPERATURE_OFFSET;
}

/*=============================================================================
	HUMIDITY
	
	returns relative humidity (%)
=============================================================================*/
float HDC1080Hygr( void )
{
return (100.0/65536.0)*(float)FilteredRHCode;
}

/*=============================================================================
	PURGE HUMIDITY START
	
	starts heating humidity sensor to purge humidity
=============================================================================*/
void PurgeHumidityStart( void )
{
IsHeaterForcedOn = true;
}

/*=============================================================================
	PURGE HUMIDITY STOP
	
	stops humidity sensor heater
=============================================================================*/
void PurgeHumidityStop( void )
{
IsHeaterForcedOn = false;
}
