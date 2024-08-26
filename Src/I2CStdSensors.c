#include "Stm32f0xx.h"
#include "Tim.h"
#include "I2CStdSensors.h"
#include "CCS811.h"
#include "ISL29035.h"
#include "MMA8453.h"
#include "HDC1080.h"
#include "ColorSensor.h"
#include "I2C2.h"
#include "Debug.h"

/*==============================================================================
	I2C STANDARD SENSORS DEFINITIONS
===============================================================================*/
#define HYST	0.1																//(G) Accelerometer hysteresis for orientation change detection

#define I2C_SCAN_TIMER_FREQ				3000000									//(Hz) I2C Standard peripherals Timer clock frequency
#define SCAN_FREQ						50										//(Hz) I2C peripherals scan frequency

#define I2C_SCAN_TIMER					TIM6
#define I2C_SCAN_TIMER_CLK_ENABLE()		RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM6, ENABLE)

#define ABS(x)	((x >= 0) ? x : -x)												// Absolute value of x

/*==============================================================================
	PRIVATE DATA
===============================================================================*/
const NVIC_InitTypeDef I2CTmrIRQCfg = { TIM6_DAC_IRQn, 3, ENABLE };				// Enable timer IRQ at lowest priority

/*=============================================================================
	I2C STANDARD SENSORS SCAN IRQ
	
	Scans the accelerometer every ~80ms = 4/50Hz
	Scans all other peripherals every 400ms = 5*80ms
	Reconfigures every peripherals every 1600ms = 4*400ms ( temp : may take twice this time )
	
	Accelerometer scan cycle :
		Accel Request -> Accel Read -> Other periph Action -> Other periph action -> Accel Request ...
	Other peripherals AND accelerometer reconfiguration :
		- Peripheral request, 20ms later Peripheral read
=============================================================================*/
void TIM6_DAC_IRQHandler( void )
{
static bool AccelerometerScan = false;
static enum { SCAN_BUS_CFG, GAZ_RD, LUX_RD, RGB_RD, HUM_TEMP, NB_SLOW_SCAN_STAGES }SlowScanStage = SCAN_BUS_CFG;
static enum { SCAN_BUS_RESET, LUX_CFG, RGB_CFG, HUM_TEMP_CFG }CfgStage = SCAN_BUS_RESET;
bool SlowTaskFinished=false;

I2C_SCAN_TIMER->SR = 0;															// Clear all IRQ flags
if ( AccelerometerScan )
	{
	if ( MMA8453Task() )
		AccelerometerScan = false;												// Accelerometer scan finished
	}
else
	{
	switch( SlowScanStage )
		{
		case SCAN_BUS_CFG : 
			if ( CfgStage == SCAN_BUS_RESET )
				{
				I2C2Setup( I2C_B10B11, I2C_BAUDRATE_100K ); 					// Reset I2C communication before each communication to avoid locks and error treatment
				SlowTaskFinished = MMA8453Cfg();								// Reconfigure Accelerometer in case it was deconfigured
				if ( SlowTaskFinished )
					CfgStage = LUX_CFG;
				}
			else if ( CfgStage == LUX_CFG )
				{
				LuxCfg();
				CfgStage = RGB_CFG;
				SlowTaskFinished = false;
				}
			else if ( CfgStage == RGB_CFG )
				{
				RgbCfg();
				CfgStage = HUM_TEMP_CFG;
				SlowTaskFinished = false;
				}
			else
				{
				HumTempCfg();
				CfgStage = SCAN_BUS_RESET;
				SlowTaskFinished = true;
				}
			break;	
		case GAZ_RD : 
			SlowTaskFinished = CCS811Task();									// Read gaz indication 
			break;
		case LUX_RD :
			SlowTaskFinished = LuxTask();										// Read Luxmeter
			break;
		case RGB_RD :
			SlowTaskFinished = RgbTask();										// Read colormeter
			break;
		default :
			SlowTaskFinished = HumTempTask();									// Alternately read humidity and temperature
			break;
		}
	if ( SlowTaskFinished )
		{
		if ( ++SlowScanStage >= NB_SLOW_SCAN_STAGES )
			SlowScanStage = SCAN_BUS_CFG;
		AccelerometerScan = true;
		}
	}
}

/*=============================================================================
	I2C STANDARD SENSORS SETUP
	
	Configure I2C scan task timer and I2C GPIO
=============================================================================*/
void I2CStdSensorsSetup( void )
{
u32 TimFreq = I2C_SCAN_TIMER_FREQ	;
u16 TimPrescaler = 1;

// Enable peripheral clocks
I2C_SCAN_TIMER_CLK_ENABLE();
DBGMCU_Config( DBGMCU_TIM6_STOP, ENABLE );										// Stop TMR counter when core halted ( breakpoint )

I2C_SCAN_TIMER->CR1 = 0;														// stop timer and deinit
TimPrescaler = ComputeTimPrescaler( I2C_SCAN_TIMER, &TimFreq );

if ( TimPrescaler == 0 )
	I2C_SCAN_TIMER->PSC = 0;													// Prescaler too low : set prescaler to maximum speed
else
	I2C_SCAN_TIMER->PSC = TimPrescaler-1;										// Set prescaler for timer resolution
I2C_SCAN_TIMER->ARR = I2C_SCAN_TIMER_FREQ/SCAN_FREQ-1;							// Set scan timer period
I2C_SCAN_TIMER->RCR = 0;														// Repetition counter unused
I2C_SCAN_TIMER->EGR = TIM_PSCReloadMode_Immediate;								// Reset prescaler and repetition counter internal values
I2C_SCAN_TIMER->DIER = TIM_IT_Update;											// enable update IRQ
I2C_SCAN_TIMER->CR1 = TIM_CR1_CEN;												// Configure timer and start counting
NVIC_Init(( NVIC_InitTypeDef  *)&I2CTmrIRQCfg);
}

/*=============================================================================
	CO2ppm
	
	returns the Gaz sensor ppm value
=============================================================================*/
u16 eCO2_ppm( void )
{
return CCS811_eCO2_ppm();
}

/*=============================================================================
	TVoc ppb
	
	returns the Gaz sensor estimated VOC ppb value
=============================================================================*/
u16 TVoc_ppb( void )
{
return CCS811_TVoc_ppb();
}

/*=============================================================================
	Temperature
	
	returns the temperature in deg C
=============================================================================*/
u16 Temperaturex100( void )
{
float T = HDC1080Temp();

if ( T < 0 )
	return 0;
else
	return T*100;
}

/*=============================================================================
	Hygrometry
	
	returns the hygrometrie (%)
=============================================================================*/
u16 Hygrometryx100( void )
{
return (100.0*HDC1080Hygr());
}

/*=============================================================================
	Lux
	
	returns Last Lux Value (Lux unit)
=============================================================================*/
u16 Lux( void )
{
return ISL29035Lux();
}

/*=============================================================================
	Cube Orientation
	
	returns the cube orientation and avoids fast changes by adding an 
		hysteresis
=============================================================================*/
CubeOrientation_t CubeOrientation( void )
{
//                                         UP  TILTED TILTED TILTED TILTED UPSIDE
//                                             FRONT   LEFT   BACK  RIGHT   DOWN
static const s8 HystPosx[NB_CUBE_POS] = {   0 , HYST,   0  , -HYST,   0  ,   0   };// Hysteresis on x Axis depending on previous orientation
static const s8 HystPosy[NB_CUBE_POS] = { HYST,  0  ,   0  ,   0  ,   0  , -HYST };// Hysteresis on y axis depending on previous orientation
static const s8 HystPosz[NB_CUBE_POS] = {   0 ,  0  , HYST ,   0  , -HYST,   0   };// Hysteresis on z axis depending on previous orientation
float x,y,z, Absx, Absy, Absz;
static CubeOrientation_t Orientation = UP;

MMA8453Orientation( &x, &y, &z );

x += HystPosx[Orientation];
y += HystPosy[Orientation];
z += HystPosz[Orientation];

Absx = ABS(x);
Absy = ABS(y);
Absz = ABS(z);

if ( Absx > Absy )
	{
	if ( Absx > Absz )
		{
		if ( x > 0 )
			Orientation = TILTED_FRONT;
		else
			Orientation = TILTED_BACK;
		}
	else
		{
		if ( z > 0 )
			Orientation = TILTED_LEFT;
		else
			Orientation = TILTED_RIGHT;
		}
	}
else
	{
	if ( Absy > Absz )
		{
		if ( y > 0 )
			Orientation = UP;
		else
			Orientation = UPSIDE_DOWN;
		}
	else
		{
		if ( z > 0 )
			Orientation = TILTED_LEFT;
		else
			Orientation = TILTED_RIGHT;
		}
	}

return Orientation;
}

