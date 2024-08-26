#include "I2CExtSensors.h"
#include "I2C1.h"
#include "Time.h"
#include "GpioDef.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/
#define T6703_I2C_ADDR	0x15
#define SN_GCJA5_ADDR		0x33
#define T6703_RESPONSE_TIMEOUT_US 10000000		//max allowed response time: 10s
#define SN_GCJA5_RESPONSE_TIMEOUT_US 10000000		//max allowed response time: 10s



static enum {UNKNOWN, MOUNTED, UNMOUNTED} BoardStatus;
static u16 CO2ppm = 0;
static pmReadResult_t pmValues;

/*=============================================================================
	CO2 Task
	reads CO2 sensor value continuously
=============================================================================*/
void CO2Task( void )
{
	static const u8 PpmRequestFrame[5] = { 0x04, 0x13, 0x8B, 0x00, 0x01 };
	static enum { I2C_RESET, I2C_READY, I2C_TX_PENDING , I2C_RX_PENDING }Status;
	static struct
	{
		u8 Func;
		u8 NbBytes;
		u8 MSB_ppm;
		u8 LSB_ppm;
	}RxData;
	
	static u32 StartTimeus = 0;
	static u32 lastResponseTimeus = 0;

	switch ( Status )
	{
		case I2C_RESET : 
			I2C1Setup( I2C_B8B9, I2C_BAUDRATE_100K ); 
		  StartTimeus = GetusTick();
			lastResponseTimeus = StartTimeus;
			Status = I2C_READY;
			break;
		case I2C_READY :
			if ( Elapsed_us(StartTimeus) > 1000000 )	//wait at least 1s after startup
			{
				I2C1StartWrite( T6703_I2C_ADDR, (u8 *)PpmRequestFrame , sizeof(PpmRequestFrame) );
				Status = I2C_TX_PENDING;
			}
			break;
		case I2C_TX_PENDING :
			if ( IsI2C1WriteDone() )
			{
				RxData.Func = 0;
				RxData.NbBytes = 0;
				I2C1StartRead( T6703_I2C_ADDR, &RxData.Func, sizeof(RxData) );
				Status = I2C_RX_PENDING;
			}
			break;
		case I2C_RX_PENDING :
		default :
			if ( I2C1RxDataReady() )
			{
				if ( (RxData.Func == 0x04) && (RxData.NbBytes == 2) )
				{
					CO2ppm = RxData.MSB_ppm*0x100 + RxData.LSB_ppm;
					lastResponseTimeus = GetusTick();
				}
				Status = I2C_READY;
			}
		break;	
	}
	
	//reset I2C if not answer after T6703_RESPONSE_TIMEOUT_US
	if (Elapsed_us(lastResponseTimeus) > T6703_RESPONSE_TIMEOUT_US)
		Status = I2C_RESET;
}

/*=============================================================================
	returns CO2 value
=============================================================================*/
u16 getCO2ppm()
{
	return CO2ppm;
}


/*=============================================================================
	check if an external sensor is mounted
=============================================================================*/
bool isExtSensorMounted()
{
	if (BoardStatus == UNKNOWN)
	{
		static const GPIODef_t ExtSClkPin = { PB8, TYPE_LOGIC_INPUT, PULL_DOWN, SPEED_MEDIUM, INIT_OPEN };

		GpioSetup( &ExtSClkPin );
		if ( IsInputOn( &ExtSClkPin ) )
			{
				BoardStatus = MOUNTED;
			}
			else 
			{
				BoardStatus = UNMOUNTED;
			
			}
	}
	
	if (BoardStatus == MOUNTED)
		return true;
	else 
		return false;
}


/*=============================================================================
	Particle Task
	reads particle matters values
=============================================================================*/
void ParticleTask( void )
{

	static enum { I2C_RESET, I2C_READY, I2C_TX_PENDING , I2C_RX_PENDING }Status;
	static u8 rxData[12];
	u32 val1, val2_5, val10;

	static u8 REG_PMV = 0x00;

	static u32 StartTimeus = 0;
	static u32 lastResponseTimeus = 0;


	switch ( Status )
	{
		case I2C_RESET : 
			I2C1Setup( I2C_B8B9, I2C_BAUDRATE_100K );
			StartTimeus = GetusTick();	
			Status = I2C_READY;
			break;
		case I2C_READY :
			if ( Elapsed_us(StartTimeus) > 8000000 )
			{
				StartTimeus = GetusTick();
				lastResponseTimeus = StartTimeus;
				I2C1StartWriteNoStop( SN_GCJA5_ADDR, &REG_PMV , sizeof(REG_PMV) );
				Status = I2C_TX_PENDING;
			}
			break;
		case I2C_TX_PENDING :
			if ( IsI2C1WriteDone() || ( Elapsed_us(StartTimeus) > 500 ) )
			{
				I2C1StartRead( SN_GCJA5_ADDR, rxData, sizeof(rxData) );
				Status = I2C_RX_PENDING;
			}
		break;
		case I2C_RX_PENDING :
			
			if ( I2C1RxDataReady() )
			{
				lastResponseTimeus = GetusTick();
				val1 = rxData[0] + (rxData[1] << 8) + (rxData[2] << 16) + (rxData[3] << 24);
        val2_5 = rxData[4] + (rxData[5] << 8) + (rxData[6] << 16) + (rxData[7] << 24);
        val10 = rxData[8] + (rxData[9] << 8) + (rxData[10] << 16) + (rxData[11] << 24);
				if ((val1 > 0xfffff) || (val2_5 > 0xfffff) || (val10 > 0xfffff))
				{
					pmValues.readError = true;
					StartTimeus = GetusTick();
					Status = I2C_RESET;
				}
				else
				{
					pmValues.readError = false;
					if (val1 > UINT16_MAX*10)
						val1 = UINT16_MAX;
					else
							pmValues.pm1x100 = val1/10.0;
					
					if (val1 > UINT16_MAX*10)
						val1 = UINT16_MAX;
					else
						pmValues.pm1x100 = val1/10.0;
					
					if (val2_5 > UINT16_MAX*10)
						val2_5 = UINT16_MAX;
					else
						pmValues.pm2_5x100 = val2_5/10.0;
					
					if (val10 > UINT16_MAX*10)
						val10 = UINT16_MAX;
					else
						pmValues.pm10x100 = val10/10.0;
					
					StartTimeus = GetusTick();
					Status = I2C_READY;
				}
				
			}
			break;	
		default :
			break;	
	}
	
		//reset I2C if not answer after T6703_RESPONSE_TIMEOUT_US
	if (Elapsed_us(lastResponseTimeus) > SN_GCJA5_RESPONSE_TIMEOUT_US)
		Status = I2C_RESET;
}

/*=============================================================================
	returns PM values
=============================================================================*/
pmReadResult_t* GetPmValues()
{
	return &pmValues;
}


