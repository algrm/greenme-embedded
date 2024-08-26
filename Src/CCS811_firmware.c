#include "CCS811.h"
#include "I2C2.h"
#include "GpioDef.h"
#include "Endianess.h"
#include "Filters.h"
#include "HDC1080.h"
#include "Debug.h"
#include "ccs811_firmware.h"
#include "Time.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/

#define CCS811_I2C_ADDR	0x5A

#define STATUS_REG 0x00
#define MEAS_MODE_REG 0x01
#define ALG_RESULT_DATA 0x02
#define ENV_DATA 0x05
#define NTC_REG 0x06
#define THRESHOLDS 0x10
#define HW_ID_REG 0x20
#define HW_VERSION 0x21
#define ERR_ID_REG 0xE0
#define APP_REG_BOOT_APP 0xF2
#define VERIFY 0xF3
#define APP_START_REG 0xF4
#define SW_RESET 0xFF

#define GPIO_WAKE 0x5
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

static const u8 StatusSelect[1] = {STATUS_REG};									// Select Status register command

//firmware update update variable
static int updateStep = 0;
static int doUpdate = false;



/*=============================================================================
	CCS811 firmware upload
	
	performs firmware update
=============================================================================*/
bool CCS811_upload_firmware (void)
{
	//static const u8 SwReset[5] = {0xff, 0x11, 0xe5, 0x72, 0x8A};
	static const u8 Erase[5] = {0xf1, 0xe7, 0xa7, 0xe6, 0x09};
	static const u8 Verify[1] = {VERIFY};
	static const u8 reqHwId[1] = {HW_ID_REG};
	static u8 hwid[1];
	static u8 RxSize = 0;
	static u8 Status[1];
	u8 payload[9];
	int n,i,j;
	
	if (doUpdate == false)
		return false;
	
	//u8* firmware = ccs811_firmware_1_0_0;
	
	if (updateStep == 0)
	{
		//wake up
		GpioSetup(&SleepPin);
		updateStep++;
		return false;
	}
	else if (updateStep == 1)
	{
		//get HW version
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)reqHwId, sizeof(reqHwId) );
		updateStep++;
		return false;
	}
	else if (updateStep == 2)
	{
		RxSize = sizeof(hwid);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)hwid, &RxSize );
		updateStep++;
		return false;
	}
	else if (updateStep == 3)
	{
		if (hwid[0] == 0x81)
		{
			doUpdate = true;
		}
		else 
		{
			doUpdate = false;
		}
		updateStep++;
		return false;
	}
	else if (updateStep == 4)
	{
	//reset
		//I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)SwReset, sizeof(SwReset) );
		updateStep++;
		return false;
	}
	else if (updateStep == 5)
	{
	//check that ccs811 is in bootloader mode
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)StatusSelect, sizeof(StatusSelect) );
		updateStep++;
		return false;
	}
	else if (updateStep == 6)
	{

		RxSize = sizeof(Status);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)Status, &RxSize );
		updateStep++;
		return false;
	}
	else if (updateStep == 7)
	{
		//bootloader ?
		if ((Status[0] & 0x80) == 0x00)
		{
				doUpdate = true;
		}
		else {
				doUpdate = false;
		}
		updateStep++;
		return true;
	}
	else if (updateStep == 8)
	{
		//erase
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)Erase, sizeof(Erase) );
		WaitMs(300);
		updateStep++;
		return true;
	}
	else if (updateStep == 9)
	{
		//check APP_VALID bit[4] of the status (0x00) is cleared
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)StatusSelect, sizeof(StatusSelect) );
		updateStep++;
		return false;
	}
	else if (updateStep == 10)
	{
		//check APP_VALID bit[4] of the status (0x00) is cleared
		RxSize = sizeof(Status);
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)Status, &RxSize );
		updateStep++;
		return false;
	}
	else if (updateStep == 11)
	{
		//check bit 4
		if ((Status[0] & 0x10) == 0x00)
		{
				doUpdate = true;
		}
		else 
		{
			doUpdate = false;
		}
		updateStep++;
		return false;
	}	
	else if (updateStep == 12)
	{
		if (doUpdate) 
		{
			n = sizeof(ccs811_firmware);
			payload[0] = 0xf2;
			for (i=0; i<n; i=i+8)
			{
				for (j=0; j<8; j++)
				{
					payload[j+1] = ccs811_firmware[i+j];
				}
				//write to I2C
				I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)payload, 9 );
				WaitMs(20);
			}
			
		}
		updateStep++;
		return true;
	}	
	else if (updateStep == 13)
	{
		I2C2StartWrite( CCS811_I2C_ADDR, (u8 *)Verify, sizeof(Verify) );
		WaitMs(70);
		updateStep++;
		return true;
	}
	else if (updateStep == 14)
	{
		//send status request
		I2C2StartRead( CCS811_I2C_ADDR, (u8 *)Status, &RxSize );
		updateStep++;
		return false;
	}
	else if (updateStep == 15)
	{
		//read status
		if ((Status[0] & 0x10) == 0x10)
		{
				doUpdate = true;
		}
		updateStep++;
		return false;
	}	
	
	doUpdate = false;
	return false;

}
