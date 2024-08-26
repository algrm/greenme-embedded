#include "ISL29035.h"
#include "I2C2.h"
#include "Debug.h"
#include "Filters.h"
#include "definitions.h"

/*=============================================================================
	PRIVATE DEFINITIONS
=============================================================================*/
#define ISL29035_ADDR	0x44													// Sensor's I2C address

#define CFG_REG_ADDR	0x00													// Configuration register address
#define LUX_REG_ADDR	0x02													// Lus value address

enum { MODE_POWER_DOWN, MODE_NOT_RECOMMENDED, MODE_IR_ONCE, MODE_RESERVED3, MODE_RESERVED4, MODE_ALS_CONT, MODE_IR_CONT };
enum { LUX_RANGE_1000, LUX_RANGE_4000, LUX_RANGE_16000, LUX_RANGE_64000 };
enum { RES_16_BITS, RES_12_BITS, RES_8_BITS, RES_4_BITS };

typedef struct
{
u8 RegAddr;
// COMMAND 1
u8 IRQPersist : 2; // IRQ unused
u8 IntStatus  : 1; // IRQ status bit : unused
u8 Reserved1  : 2; // Must be 0
u8 Mode       : 3; // See enum above
// COMMAND 2
u8 Range      : 2; //
u8 Res        : 2;
u8 Reserved2  : 4;
}ISL29035Cfg_t;

/*=============================================================================
	PRIVATE VARIABLES
=============================================================================*/
u32 LuxVal=0;																	//(Lux) Filtered Lux value
static const ISL29035Cfg_t Isl29035Cfg = 
{
CFG_REG_ADDR,
// COMMAND 1
0,             // don't care : IRQ unused
0,             // don't care status bit
0,             // Reserved must be 0
MODE_ALS_CONT, // Measure light continuously
// COMMAND 2
LUX_RANGE_64000,
RES_16_BITS,
0
};

/*=============================================================================
	LUX CFG

	reconfigures ISL29035 for 
		- continuous measurement
		- Full scale 64000 Lux range
		- 16 bit resolution ( 105 ms integration time )
=============================================================================*/
void LuxCfg( void )
{
I2C2StartWrite( ISL29035_ADDR, (u8 *)&Isl29035Cfg, sizeof(Isl29035Cfg) );
}

/*=============================================================================
	LUX TASK
	
	This task alternately
		- Sends a Read command
		- Gets and filters measured Lux value at next call
		
	Calls must be separated by enough time so that Read command is ready
	
	returns true when data has been read ( successfully or not )

=============================================================================*/
bool LuxTask( void )
{
static enum { SELECT, READ }NextCmd;
static const u8 RegAddr = LUX_REG_ADDR;
static u8 RxSize;																// First, I2C Data size to fetch THEN I2C Data size actually provided by the sensor
static u16 Data;																// I2C Data received from sensor

if ( NextCmd == SELECT )
	{
	I2C2StartWrite( ISL29035_ADDR, (u8 *)&RegAddr, sizeof(RegAddr) );
	NextCmd = READ;
	return false;
	}
else
	{// get last read value and apply filter
	if ( RxSize == sizeof(Data) )
		{
		LuxVal = Filter( LuxVal, Data, 16 );
		}
	// Start reading a new value
	RxSize = sizeof(Data);
	I2C2StartRead( ISL29035_ADDR, (u8 *)&Data, &RxSize );
	NextCmd = SELECT;
	return true;
	}
}

/*=============================================================================
	Lux
	
	returns last filtered Lux value
=============================================================================*/
u16 ISL29035Lux( void )
{
return LUX_GAIN_ADJUST*((64000*LuxVal)>>16);	// Lux = (Max Range/Max count)*LuxVal		GAIN_ADJUST is here to compensate for optical attenuation
}

