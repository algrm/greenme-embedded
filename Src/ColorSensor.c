#include "ColorSensor.h"
#include "I2C2.h"
#include "Debug.h"
#include "Filters.h"

/*=============================================================================
	PRIVATE DEFINITIONS
=============================================================================*/
#define TCS34725_ADDR	0x29													// Sensor's I2C address

#define CMD_MSK		0x80
#define AUTO_INC	0x20
#define CFG_REG_ADDR	(CMD_MSK | AUTO_INC | 0x00)										// Configuration register address
#define ID_REG_ADDR		(CMD_MSK | AUTO_INC | 0x12)										// ID register address

enum { MODE_POWER_DOWN, MODE_NOT_RECOMMENDED, MODE_IR_ONCE, MODE_RESERVED3, MODE_RESERVED4, MODE_ALS_CONT, MODE_IR_CONT };
enum { LUX_RANGE_1000, LUX_RANGE_4000, LUX_RANGE_16000, LUX_RANGE_64000 };
enum { RES_16_BITS, RES_12_BITS, RES_8_BITS, RES_4_BITS };

typedef struct
{
u8 RegAddr;
// ENABLE REG Low bits first
u8 PON      : 1; // Power On 	0:OFF 1:ON
u8 AEN      : 1; // Adc Enable  0:OFF 1:ON
u8 Reserved1: 2; // Must be 0
u8 WEN      : 1; // Wait Enable 0:Wait Timer disabled 1:Wait Timer enabled 
u8 AIEN     : 1; // Interrupt Enable 0:interrupt disabled 1:interrupt enabled
u8 Reserved2: 2;
// COMMAND 2
u8 ATime;        // (256-Val)*2,4ms integration time
u8 WTime;
}TCS34725Cfg_t;

/*=============================================================================
	PRIVATE VARIABLES
=============================================================================*/
u32 WhiteVal=0;																	// Filtered Red value
u32 RedVal=0;																	// Filtered Red value
u32 GreenVal=0;																	// Filtered Red value
u32 BlueVal=0;																	// Filtered Red value
static const TCS34725Cfg_t TCS34725Cfg = 
{
CFG_REG_ADDR,
// ENABLE
1,             // PON
1,             // AEN
0,             // Reserved must be 0
1,             // WEN
0,             // AIEN
0,             // Reserved must be 0
// ATIME
256-64,        // ATIME = 154ms = (256-Val)*2,4ms integration time
// WTIME
256-85,			// WTIME = 204ms = (256-Val)*2,4ms wait time between conversions
};

/*=============================================================================
	RGB CFG
=============================================================================*/
#include "Time.h"
#include "Debug.h"
void RgbCfg( void )
{
I2C2StartWrite( TCS34725_ADDR, (u8 *)&TCS34725Cfg, sizeof(TCS34725Cfg) );
}

/*=============================================================================
	RGB TASK
	
	This task alternately
		- Sends a Read command
		- Gets and filters measured Lux value at next call
		
	Calls must be separated by enough time so that Read command is ready
	
	returns true when data has been read ( successfully or not )

=============================================================================*/
bool RgbTask( void )
{
static enum { SELECT, READ, INIT }NextCmd=INIT;
static const u8 RegAddr = ID_REG_ADDR;
static u8 RxSize;																// First, I2C Data size to fetch THEN I2C Data size actually provided by the sensor
static struct
{
u8 ID;
u8 Status;
u16 Clear;
u16 Red;
u16 Green;
u16 Blue;
}Data;

if ( NextCmd == SELECT )
	{
	I2C2StartWrite( TCS34725_ADDR, (u8 *)&RegAddr, sizeof(RegAddr) );
	NextCmd = READ;
	return false;
	}
else if ( NextCmd == READ )
	{// get last read value and apply filter
	if ( (RxSize==sizeof(Data)) && (Data.ID == 0x44) && (0x11==(Data.Status&0x11)) )
		{
		WhiteVal = Filter( WhiteVal, Data.Clear, 16 );
		RedVal   = Filter(   RedVal, Data.Red,  16 );
		GreenVal = Filter( GreenVal, Data.Green, 16 );
		BlueVal  = Filter(  BlueVal, Data.Blue, 16 );
		}

	// Start reading a new value
	RxSize = sizeof(Data);
	I2C2StartRead( TCS34725_ADDR, (u8 *)&Data, &RxSize );
	NextCmd = SELECT;
	return true;
	}
else
	{
	RgbCfg();
	NextCmd = SELECT;
	return true;
	}
}

/*=============================================================================
	GetRedColor
	
	returns last filtered Red value scaled to [0-255]
=============================================================================*/
u32 Sum;
Color_t GetColors( void )
{
	Color_t Colors;
	u32 Red, Green, Blue;

	Colors.White = WhiteVal;	
	Sum = RedVal + GreenVal + BlueVal;

	if ( WhiteVal == 0 )
		{ Red = Green = Blue = 0; }
	else
		{
		Red = (RedVal*256)/WhiteVal;
		if ( Red < 256 )
			Colors.Red = Red;
		else
			Colors.Red = 255;

		Green = (GreenVal*256)/WhiteVal;
		if ( Green < 256 )
			Colors.Green = Green;
		else
			Colors.Green = 255;
			
		Blue = (BlueVal*256)/WhiteVal;
		if ( Blue < 256 )
			Colors.Blue = Blue;
		else
			Colors.Blue = 255;
		}
	return Colors;
}
