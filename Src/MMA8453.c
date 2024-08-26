#include "MMA8453.h"
#include "I2C2.h"
#include "Debug.h"
#include "Endianess.h"
#include "Filters.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/
#define MMA8453_ADDR	0x1C													// I2C Address

#define FILTER_SIZE		8														// Basic Low pass filter divider

/*=============================================================================
	MMA8453 STATUS / XYZ REGISTERS / HYSTERESIS
=============================================================================*/
#define STATUS_REG_ADDR	0x00													// MMA8453 STATUS regsiter address
#define MMA_8453_G_VAL		256.0												//(counts) sensor counts for 1G
#define ONE_DIV_MMA_8453_G_VAL	(1.0/MMA_8453_G_VAL)

typedef __packed struct
{	// MMA 8453 data structure
u8 Status;
u8 x[2];
u8 y[2];
u8 z[2];
}AccelData_t;

/*=============================================================================
	PORTRAIT / LANDSCAPE STATUS REGISTER ( for future use )
=============================================================================*/
#define PL_STATUS_REG_ADDR		0x10

typedef struct
{	// Bit 0 is at the top of the bit struct
u8 Back : 1;		// 0:Front 1:Back
u8 DwnLeft : 1;		// If Landscape		0:Right 1:Left		If Portrait : 0:Up 1:Down		
u8 Landscape : 1;	// 0:Portrait	1:Landscape
u8 Unused : 3;
u8 LockOutDetected;	// 1:Lockout detection has been detected
u8 New : 1;			// 0:no change		1:Portrait/Landscape/Back/Front/Tilt Status has changed
}PL_STATUS_t;

/*=============================================================================
	CTRL REGISTERS DEFINITONS
=============================================================================*/
#define CTRL_REGS_REG_ADDR		0x2A

enum { DR_800HZ=0, DR_400HZ, DR_200HZ, DR_100HZ, DR_50HZ, DR_80MS, DR_160MS, DR_320MS, DR_640MS };

typedef struct
{	// Bit 0 is at the top of the bit struct
u8 RegAddr;
// CTRL REG1
u8 Active            : 1;	// 0:Standby			1:Active
u8 FastRead          : 1;	// 0:Normal mode        1:Skip registers mode
u8 LNoise            : 1;	// 0:Normal             1:Reduced noise
u8 DataRate          : 3;   // 
u8 AslpRate          : 2;	// Unused in normal mode : 0
// CTRL REG2
u8 LowPower          : 1;	// 0:Normal				1:Low power
u8 HighRes           : 1;	// 0:Normal/Low noise	1:High resolution
u8 SleepEn           : 1;	// 1:Auto sleep enabled
u8 SleepModeLowPower : 1;	// 0:Normal				1:Low power
u8 SleepModeHighRes  : 1;	// 0:Normal/Low noise	1:High resolution
u8 Always0           : 1;
u8 Reset             : 1;	// 0:Normal				1:Reset			( Automatically swicthes back to 0 after reset time has elapsed )
u8 SelTest           : 1;	// 0:Disabled			1:Self test
}CTRL_REGS_t;

#define CTRL_REG2_REG_ADDR		0x2B

typedef struct
{	// Bit 0 is at the top of the bit struct
u8 RegAddr;
// CTRL REG2
u8 LowPower          : 1;	// 0:Normal				1:Low power
u8 HighRes           : 1;	// 0:Normal/Low noise	1:High resolution
u8 SleepEn           : 1;	// 1:Auto sleep enabled
u8 SleepModeLowPower : 1;	// 0:Normal				1:Low power
u8 SleepModeHighRes  : 1;	// 0:Normal/Low noise	1:High resolution
u8 Always0           : 1;
u8 Reset             : 1;	// 0:Normal				1:Reset			( Automatically swicthes back to 0 after reset time has elapsed )
u8 SelTest           : 1;	// 0:Disabled			1:Self test
}CTRL_REG2_t;

/*=============================================================================
	PRIVATE VARIABLES
=============================================================================*/
static const CTRL_REGS_t	CtrlRegs = 											// MMA8453 configuration ( all other registers are at reset default values
{
CTRL_REGS_REG_ADDR,	
// CTRL_REG1
1, 	       // Active mode
0,	       // Normal read register increment ( no skip )
0,	       // Normal             No reduced noise
DR_50HZ,   // Data rate
0,         // Unused in normal mode : 0
// CTRL_REG2
0,	// Normal mode ( not low power )
0,	// Normal mode ( not high resolution )
0,	// Auto Sleep disabled
0,	// Normal mode when in sleep mode ( not low power )
0,	// Normal mode when in sleep mode ( not high resolution )
0,
0,	// No reset request
0	// Self test disabled
};
static const CTRL_REG2_t	CtrlReg2 = 											// Used to reset MMA configuration to default values
{
CTRL_REG2_REG_ADDR,	
0,	// Normal mode ( not low power )
0,	// Normal mode ( not high resolution )
0,	// Auto Sleep disabled
0,	// Normal mode when in sleep mode ( not low power )
0,	// Normal mode when in sleep mode ( not high resolution )
0,
1,	// Reset request
0	// Self test disabled
};

static s16 xVal=0;
static s16 yVal=0;
static s16 zVal=0;

/*=============================================================================
	MMA FILTER
	
	Inputs : 
		Val : Previous filtered value
		Data : 2 data bytes read from x,y or z registers
		
	moves bits from Data's original position to an form an s16 variable
	filters the value using previous Val
		
	returns filtered Val
=============================================================================*/
s16 MmaFilter( s16 Val, u8 *Data )
{
s16 NewVal = Data[0] * 0x100 | Data[1];
if ( NewVal < 0 )
	NewVal |= 0x007F;
NewVal /= 64;
if ( NewVal < Val )
	{
	if ( NewVal < Val-FILTER_SIZE )
		Val = ((FILTER_SIZE-1)*Val + NewVal)/FILTER_SIZE;
	else
		Val--;
	}
else
	{
	if ( NewVal > Val+FILTER_SIZE )
		Val = ((FILTER_SIZE-1)*Val + NewVal)/FILTER_SIZE;
	else
		Val++;
	}
return Val;
}

/*=============================================================================
	MMA8453 WR CFG
		
	at first call resets MMA to default configuration
	at second call writed required configuration to MMA registers
	
	returns true when configuration has been written to
=============================================================================*/
bool MMA8453Cfg( void )
{
static enum { RESET_CFG, SETUP_CFG }NextCfgCmd = RESET_CFG;

if ( NextCfgCmd == RESET_CFG )
	{
	I2C2StartWrite( MMA8453_ADDR, (u8 *)&CtrlReg2, sizeof(CtrlReg2) );			// Reset configuration to default values
	NextCfgCmd = SETUP_CFG;
	return false;
	}
else
	{
	I2C2StartWrite( MMA8453_ADDR, (u8 *)&CtrlRegs, sizeof(CtrlRegs) );			// Write required configuration
	NextCfgCmd = RESET_CFG;
	return true;
	}
}

/*=============================================================================
	MMA8453 Task
	
	Swaps between sending a register selection and actuallly reading X,Y,Z
	When reading x,y,z,	uses last read data to detect orientation changes.
	An hysteresis is applied to avoid fast changes at transition angles
	
	returns true each time a read command has been treated
=============================================================================*/
bool MMA8453Task( void )
{
static u8 RxSize;
static AccelData_t Data;
static enum { SELECT_CMD, READ_CMD }NextCmd = SELECT_CMD;

if ( NextCmd == SELECT_CMD )
	{
	u8 RegAddr = STATUS_REG_ADDR;
	I2C2StartWrite( MMA8453_ADDR, &RegAddr, sizeof(RegAddr) );
	NextCmd = READ_CMD;
	return false;
	}
else
	{// get last read value and apply filter
	if ( RxSize == sizeof(Data) )
		{// some valid I2C data size received after last read command was sent
		xVal = MmaFilter( xVal, Data.x );
		yVal = MmaFilter( yVal, Data.y );
		zVal = MmaFilter( zVal, Data.z );
		}
	// Start reading a new value
	RxSize = sizeof(Data);
	I2C2StartRead( MMA8453_ADDR, (u8 *)&Data, &RxSize );
	NextCmd = SELECT_CMD;
	return true;
	}
}

/*=============================================================================
	MMA8453 Orientation
	
	returns the cube orientation
=============================================================================*/
void MMA8453Orientation( float *Gx, float *Gy, float *Gz )
{
*Gx = (float)xVal*ONE_DIV_MMA_8453_G_VAL;
*Gy = (float)yVal*ONE_DIV_MMA_8453_G_VAL;
*Gz = (float)zVal*ONE_DIV_MMA_8453_G_VAL;
}

