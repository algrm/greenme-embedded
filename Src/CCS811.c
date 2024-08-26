#include "CCS811.h"
#include "I2C2.h"
#include "GpioDef.h"
#include "Endianess.h"
#include "Filters.h"
#include "HDC1080.h"
#include "Debug.h"
//#include "ccs811_firmware.h"
#include "Time.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/

#define CCS811_I2C_ADDR	0x5A

//registers
#define STATUS_REG 			0x00
#define MEAS_MODE_REG 	0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 				0x05
#define NTC_REG 				0x06
#define THRESHOLDS 			0x10
#define HW_ID_REG 			0x20
#define HW_VERSION 			0x21
#define FW_APP_REG 			0x24
#define ERR_ID_REG 			0xE0
#define APP_REG_BOOT_APP 0xF2
#define VERIFY 					0xF3
#define APP_START_REG 	0xF4
#define SW_RESET 				0xFF

#define GPIO_WAKE 			0x5
#define DRIVE_MODE_IDLE 0x0
#define DRIVE_MODE_1SEC 0x10
#define DRIVE_MODE_10SEC 0x20
#define DRIVE_MODE_60SEC 0x30
#define INTERRUPT_DRIVEN 0x8
#define THRESHOLDS_ENABLED 0x4

/*=============================================================================
	PRIVATE DATA
=============================================================================*/
static const GPIODef_t SleepPin = { PB2, TYPE_OUTPUT, OUT_PP, SPEED_MEDIUM, INIT_0 };	// Start with CCS811 active

static const u8 AppStart[1] = {APP_START_REG};									// Application start command
static const u8 StatusSelect[1] = {STATUS_REG};									// Select Status register command
static const u8 ErrorSelect[1] = {ERR_ID_REG};									// Select Status register command
static const u8 MeasSelect[1] = {ALG_RESULT_DATA};								// Select measurement register command
static const u8 ModeCfg[2] = {MEAS_MODE_REG, DRIVE_MODE_10SEC};					// Set Mode command
static const u8 ResetSelect[1] = {SW_RESET};
static const u8 getFwVersion[1] = {FW_APP_REG};

static s32 eCO2ppm=0;
static u16 Vocppb=0;

static u8 ccs811Error[1];
static u8 ccs811Status[1];
static u8 fwVersion[2] = {0,0};


/*=============================================================================
	CCS811 TASK
	
	This task initializes the CCS811 application mode and then
	- scans the STATUS until a value is ready, then, reads this value
	- writes temperature and humidity data to environment data register
=============================================================================*/
bool CCS811Task( void )
{
	static enum { SLEEP_END, APP_START, MODE_WRITE, SELECT_STATUS, START_READ_STATUS, READ_STATUS, START_READ_MEAS, READ_MEAS, START_READ_FWVERSION, READ_FWVERSION, START_READ_ERROR, READ_ERROR, SWRESET }NextCmd = SLEEP_END;
	static u8 RxSize;

	static u8 EnvData[5];
	static struct
	{
		u16 eCO2ppm;
		u16 Vocppb;
	} Regs;

	if ( NextCmd == SLEEP_END )
	{
		GpioSetup(&SleepPin);
		NextCmd = SELECT_STATUS;
		return false;
	}
	else if ( NextCmd == APP_START )
	{
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)AppStart, sizeof(AppStart) );
		NextCmd = MODE_WRITE;
		return true;
	}
	else if ( NextCmd == MODE_WRITE )
	{
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)ModeCfg, sizeof(ModeCfg) );
		NextCmd = SELECT_STATUS;
		return false;
	}
	else if ( NextCmd == SWRESET )
	{
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)ResetSelect, sizeof(ResetSelect) );
		NextCmd = SELECT_STATUS;
		return false;
	}
	else if ( NextCmd == SELECT_STATUS )
	{
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)StatusSelect, sizeof(StatusSelect) );
		NextCmd = START_READ_STATUS;
		return true;
	}
	else if ( NextCmd == START_READ_STATUS )
	{
		RxSize = sizeof(ccs811Status);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)ccs811Status, &RxSize );
		NextCmd = READ_STATUS;
		return false;
	}
	else if ( NextCmd == READ_STATUS )
	{
		if ( RxSize == sizeof(ccs811Status) )
			{
			if ( (ccs811Status[0] & 0x99) == 0x90 )	//app mode, firwmare present, no data
			{
				if (fwVersion[0] == 0)
				{
					I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)getFwVersion, sizeof(getFwVersion) );
					NextCmd = START_READ_FWVERSION;
				}
				else {
					I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)StatusSelect, sizeof(StatusSelect) );
					NextCmd = START_READ_STATUS;
				}
			}
			else if ( (ccs811Status[0] & 0x99) == 0x98 )	//app mode, firwmare present, data ready
			{
				I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)MeasSelect, sizeof(MeasSelect) );
				NextCmd = START_READ_MEAS;
			}
			else if ( (ccs811Status[0] & 0x81) == 0x00 )	//are we in boot more? if yes go to app mode
			{
				I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)AppStart, sizeof(AppStart) );
				NextCmd = MODE_WRITE;
			}
			else if ((ccs811Status[0] & 0x01) == 0x01)
			{
				ccs811Error[0] = 0;
				I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)ErrorSelect, sizeof(ErrorSelect) );
				NextCmd = START_READ_ERROR;
			}
			else
			{// Abnormal status : reset
				NextCmd = SWRESET;
			}
		}
		return true;
	}
	else if ( NextCmd == START_READ_FWVERSION )
	{
		RxSize = sizeof(fwVersion);
		I2C2StartRead( CCS811_I2C_ADDR, (u8*) fwVersion, &RxSize );
		NextCmd = READ_FWVERSION;
		//return true;
	}
	else if ( NextCmd == READ_FWVERSION )
	{
		NextCmd = START_READ_STATUS;
		//return true;
	}
	
	else if ( NextCmd == START_READ_ERROR )
	{
		RxSize = sizeof(ccs811Error);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *) ccs811Error,  &RxSize);
		NextCmd = READ_ERROR;
	}
	else if ( NextCmd == READ_ERROR ){
		//restart app
		if (ccs811Error[0] != 0xFF) {
			NextCmd = SWRESET;
		}
	}
	else if ( NextCmd == START_READ_MEAS )
	{
		RxSize = sizeof(Regs);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)&Regs.eCO2ppm, &RxSize );
		NextCmd = READ_MEAS;
		return false;
	}
	else
	{// Last command was a read measurement request
		u16 Temp;
		u16 Hum;
		if ( RxSize == sizeof(Regs) )
		{
			eCO2ppm = HTONS(Regs.eCO2ppm);
			Vocppb = HTONS(Regs.Vocppb);
		}
		// Write environment data ( small execution time optimization is still possible here )
		EnvData[0] = ENV_DATA;
		Hum = 512.0*HDC1080Hygr();
		EnvData[2] = (u8)(Hum&0xFF);
		EnvData[1] = (u8)(Hum>>8);
		Temp = 512.0*(HDC1080Temp()+25.0);
		EnvData[4] = (u8)(Temp&0xFF);
		EnvData[3] = (u8)(Temp>>8);
		I2C2StartWrite( CCS811_I2C_ADDR, EnvData, sizeof(EnvData) );
		NextCmd = SELECT_STATUS;
		return true;
	}
	
	return true;
}


/*=============================================================================
	CCS811 eCO2 ppm
	
	returns the filtered Gaz pmm value
=============================================================================*/
u16 CCS811_eCO2_ppm( void )
{
if ( eCO2ppm < 0 )
	eCO2ppm = 0;
else if ( eCO2ppm > UINT16_MAX )
	eCO2ppm = UINT16_MAX;
return (u16)eCO2ppm;
}

/*=============================================================================
	CCS811 TVOC ppb
	
	returns the filtered Voc ppb estimated value
=============================================================================*/
u16 CCS811_TVoc_ppb( void )
{
return Vocppb;
}


u16 CCS811_GetFwVersion()
{
	return (fwVersion[0] << 8) | fwVersion[1];
}


