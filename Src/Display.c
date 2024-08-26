#include "Stm32f0xx.h"
#include "Tim.h"
#include "Display.h"
#include "GPIODef.h"
#include "Time.h"
#include "string.h"

/*=============================================================================
	DISPLAY TIMER DEFINITIONS
=============================================================================*/
#define DISPLAY_TIMER_FREQ		1000000											//(Hz) us Timer clock frequency
#define FEXT_COMIN_FREQ			60												//(Hz) COM inversion frequency ( spec : 54Hz < fExtComin < 65Hz )
#define FEXT_COMIN_PERIOD_US	(DISPLAY_TIMER_FREQ/FEXT_COMIN_FREQ)			//(us)
#define FEXT_COMIN_ON_TIME_US	100												//(us)
#define SCS_SETUP_TIME_US		6												//(us)
#define SCS_ON_TIME_US			7000											//(us)

#define DISPLAY_TIMER			TIM1
#define DISPLAY_TIMER_DBG		DBGMCU_TIM1_STOP								// Debug timer used to stop timer when core halted ( breakpoint )

#define NB_REFR_LINES_PER_PERIOD	16											// Refresh NB_REFR_LINES_PER_PERIOD for each timer period

#define Y_ALL_LINES_REFRESHED	0xFF											// y value when display has been fully refreshed
/*=============================================================================
	DISPLAY SPI DEFINITIONS
=============================================================================*/
#define SPI 	SPI2

/*=============================================================================
	DISPLAY PIN DEFINITIONS
=============================================================================*/
#define MOSI_PIN		PB15	
#define SCK_PIN			PB13
#define CS_PIN			PA8
#define EXTCOMIN_PIN	PA9
#define DISP_PIN		PA10

/*=============================================================================
	DMA DEFINITIONS
=============================================================================*/
#define SPI_TX_DMA_CHANNEL		DMA1_Channel5

/*=============================================================================
	SHARP DISPLAY DEFINITIONS
	
	Display memory is organised so that it includes all informations needed
	to send from 1 to N lines to the display.
	
	An SPI transfer sends :
	< 8 bit Wite Command>< 8 bit First  line number>< Line's 128 pixels>
	<   8 dummy bits    >< 8 bit second line number>< Line's 128 pixels>
	...
	<   8 dummy bits    >< 8 bit nth    line number>< Line's 128 pixels>< 16 dummy bits >

=============================================================================*/
#define CMD_WR 0x01 //MLCD write line command
#define CMD_CM 0x04 //MLCD clear memory command
#define CMD_NO 0x00 //MLCD NOP command (used to switch VCOM)

/*
typedef __packed struct 
{
u8 CmdWr;																		// Write command for fisrt line sent; dummy bits for other lines
u8 y;																			// Line number
u8 Pixels[DISPLAY_NB_PIXELS_PER_LINE/8];										// One line of pixels
}AffLine_t;
*/
typedef __packed struct 
{
u8 CmdWr;																		// Write command for fisrt line sent; dummy bits for other lines
u8 y;																			// Line number
__packed union
	{
	u8 U8[DISPLAY_NB_PIXELS_PER_LINE/8];										// One line of pixels
	u16 U16[DISPLAY_NB_PIXELS_PER_LINE/16];										// One line of pixels
	u32 U32[DISPLAY_NB_PIXELS_PER_LINE/32];										// One line of pixels
	}Pixels;
}AffLine_t;


typedef __packed struct
{
AffLine_t Line[DISPLAY_NB_LINES];
u16 Dummy;																		// Last 16 dummy bits
}DisplayMem_t;

/*=============================================================================
	PRIVATE DATA
=============================================================================*/
static const GPIODef_t MosiPin 	= { MOSI_PIN, 	TYPE_SPI2, 	OUT_PP, SPEED_MEDIUM, INIT_0 };
static const GPIODef_t SckPin 		= { SCK_PIN, 	TYPE_SPI2, 	OUT_PP, SPEED_MEDIUM, INIT_0 };
static const GPIODef_t CsPin 		= { CS_PIN, 	TYPE_TIM1, 	OUT_PP, SPEED_MEDIUM, INIT_0 };// TIM Output Compare 1 : Chip Select active high
static const GPIODef_t ExtCominPin = { EXTCOMIN_PIN,TYPE_TIM1, OUT_PP, SPEED_MEDIUM, INIT_0 };// TIM OutputCompare 2 : COM inversion on rising edge when SCS=0
static const GPIODef_t DispPin 	= { DISP_PIN, 	TYPE_OUTPUT,OUT_PP, SPEED_MEDIUM, INIT_0 };
static const TIM_OCInitTypeDef FextCominOCSetupData = 
{
TIM_OCMode_PWM1,
TIM_OutputState_Enable,
TIM_OutputNState_Disable,
(FEXT_COMIN_ON_TIME_US)/2,
TIM_OCPolarity_High,
TIM_OCNPolarity_High,
TIM_OCIdleState_Reset,
TIM_OCNIdleState_Reset
};
static const TIM_OCInitTypeDef ScsOCSetupData = 
{
TIM_OCMode_PWM1,
TIM_OutputState_Enable,
TIM_OutputNState_Disable,
(FEXT_COMIN_PERIOD_US-SCS_ON_TIME_US)/2,
TIM_OCPolarity_Low,
TIM_OCNPolarity_Low,
TIM_OCIdleState_Reset,
TIM_OCNIdleState_Reset
 };
 
static const TIM_OCInitTypeDef IRQOCSetupData = 
{
TIM_OCMode_PWM1,
TIM_OutputState_Enable,
TIM_OutputNState_Disable,
(FEXT_COMIN_PERIOD_US-(SCS_ON_TIME_US-2*SCS_SETUP_TIME_US))/2,
TIM_OCPolarity_Low,
TIM_OCNPolarity_Low,
TIM_OCIdleState_Reset,
TIM_OCNIdleState_Reset
 };
 
static DisplayMem_t Display;													// Display memory with Write commands and dummy bits included
static u8 y=0;
static u8 angle = ANGLE_0;

/*=============================================================================
	Display IRQ
	
	Starts an SPI transmission of NB_REFR_LINES_PER_PERIOD
	< 8 bit Wite Command>< 8 bit First  line number>< Line's 128 pixels>
	<   8 dummy bits    >< 8 bit second line number>< Line's 128 pixels>
	...
	<   8 dummy bits    >< 8 bit nth    line number>< Line's 128 pixels>< 16 dummy bits >
	
	Nb Cycles @24 MHZ < 16 + 66 + 11
	% occupation(IRQ) < 0,03% = 60Hz*93c/24MHz
	% occupatio(DMA)  < 0,08%  = 60Hz*(16*(16+2)+2)/24MHz
	% occupation < 0,11%
=============================================================================*/
void TIM1_CC_IRQHandler( void )
{// SCS has just set : ready to send SPI Data

DISPLAY_TIMER->SR &= ~TIM_IT_CC3;
if ( y <= DISPLAY_NB_LINES-NB_REFR_LINES_PER_PERIOD )
	{
	SPI_TX_DMA_CHANNEL->CCR &= ~DMA_CCR_EN;											// Disable Transmit DMA
	SPI_TX_DMA_CHANNEL->CMAR = (u32)&Display.Line[y].CmdWr;
	SPI_TX_DMA_CHANNEL->CNDTR = NB_REFR_LINES_PER_PERIOD*sizeof(Display.Line[0])+
														sizeof(Display.Dummy);	// Load DMA transfer size N Lines + Write Command + Last added 16 bit dummy
	SPI_TX_DMA_CHANNEL->CCR |= DMA_CCR_EN;											// Start DMA transmission as soon as DMA is re enabled
	y += NB_REFR_LINES_PER_PERIOD;
	}
else
	{
	y = Y_ALL_LINES_REFRESHED;
	}
}

/*=============================================================================
	DISPLAY SETUP
	
	Initializes display : must be called soon after power up
=============================================================================*/
void DisplaySetup( void )
{
const SPI_InitTypeDef SPI_InitStructure = 
{
SPI_Direction_1Line_Tx,
SPI_Mode_Master,
SPI_DataSize_8b,
SPI_CPOL_Low,																	// Idle SCLK state is 0
SPI_CPHA_1Edge,																	// SCLK rising edge validates MOSI
SPI_NSS_Soft,
SPI_BaudRatePrescaler_32,		// SPI Clk = 750kHz = PCLK/256
SPI_FirstBit_LSB,
7	
};
const DMA_InitTypeDef DMACfg = 
{
SPI2_BASE + 0x0C,			// Peripheral Address
0,							// Memory base address ( undefined at this stage )
DMA_DIR_PeripheralDST,		// Memory to perpheral transfer
0,							// Buffer size empty at this stage
DMA_PeripheralInc_Disable,
DMA_MemoryInc_Enable,
DMA_PeripheralDataSize_Byte,
DMA_MemoryDataSize_Byte,
DMA_Mode_Normal,
DMA_Priority_Low,
DMA_M2M_Disable
};
const NVIC_InitTypeDef DispTmrIRQCfg = { TIM1_CC_IRQn, 3, ENABLE };				// Enable display timer IRQ at lowest priority
u32 y;
TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
u32 TimFreq = DISPLAY_TIMER_FREQ;
u16 TimPrescaler = 1;

// GPIO CONFIGURATION
GpioSetup( &MosiPin );
GpioSetup( &SckPin );
GpioSetup( &CsPin );
GpioSetup( &ExtCominPin );
GpioSetup( &DispPin );

// SPI DMA Configuration
RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);								// Enable DMA clock
DMA_DeInit( SPI_TX_DMA_CHANNEL );  
DMA_Init( SPI_TX_DMA_CHANNEL, (DMA_InitTypeDef *)&DMACfg);

// SPI Configuration
SPI_I2S_DeInit(SPI);
SPI_Init(SPI, (SPI_InitTypeDef *)&SPI_InitStructure);
SPI_Cmd( SPI, ENABLE );															// Enable SPI
SPI_I2S_DMACmd(SPI, SPI_I2S_DMAReq_Tx, ENABLE);

// Timer setup ( This is the refresh display timer )
DBGMCU_Config( DISPLAY_TIMER_DBG, DISABLE );									// DO NOT STOP TMR counter when core halted ( to avoid LCD damage )

TimPrescaler = ComputeTimPrescaler( DISPLAY_TIMER, &TimFreq );
if ( TimPrescaler == 0 )
	TIM_TimeBaseStructure.TIM_Prescaler = 0;									// Prescaler too low : set prescaler to maximum speed
else
	TIM_TimeBaseStructure.TIM_Prescaler = TimPrescaler-1;						// Set prescaler for timer resolution

TIM_TimeBaseStructure.TIM_Period = DISPLAY_TIMER_FREQ/FEXT_COMIN_FREQ/2-1;		// Set LCD COM inversion frequency
TIM_TimeBaseStructure.TIM_ClockDivision = 0;									// Use Timer clock / 1
TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_CenterAligned2;			// Center aligned mode with OC IRQ only when Up counting 
TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
TIM_TimeBaseInit( DISPLAY_TIMER, &TIM_TimeBaseStructure);

TIM_OC1Init(DISPLAY_TIMER, (TIM_OCInitTypeDef* )&ScsOCSetupData);				// Setup automatic SCS signal
TIM_OC2Init(DISPLAY_TIMER, (TIM_OCInitTypeDef* )&FextCominOCSetupData);			// Setup automatic FEXT COMIN signal
TIM_OC3Init(DISPLAY_TIMER, (TIM_OCInitTypeDef* )&IRQOCSetupData);				// Setup IRQ
TIM_ITConfig(DISPLAY_TIMER, TIM_IT_CC3, ENABLE);								// Enable IRQ when SCS sets
TIM_Cmd( DISPLAY_TIMER, ENABLE );												// Start timer clock

TIM_CtrlPWMOutputs( DISPLAY_TIMER, ENABLE);										// Enables OC outputs

// IRQ Setup
NVIC_Init( (NVIC_InitTypeDef *)&DispTmrIRQCfg );								// Enable Display timer IRQ

// Prepare Display memory and set all pixels white
memset( &Display, 0xFF, sizeof(Display) );
for ( y=0; y<DISPLAY_NB_LINES; y++ )
	{
	Display.Line[y].CmdWr = CMD_WR;
	Display.Line[y].y = y+1;
	}
WaitMs((NB_REFR_LINES_PER_PERIOD*1000)/FEXT_COMIN_FREQ);						// Wait untill Display RAM is fully downloaded
OutputSet(&DispPin);															// Switch display ON now that display memory is OK
}

/*=============================================================================
	Set Pixel 
	
	Inputs :
		x : 0 to DISPLAY_NB_PIXELS_PER_LINE-1
		y : 0 to DISPLAY_NB_LINES-1
		PixCol : BLACK or WHITE
	
	Sets required pixel to required color
	
	17 Cycles @24MHz
=============================================================================*/
void SetPixel( u8 x, u8 y, u8 color, u8 angle )
{
	u8 *Ptr;
	u8 tmp;
	///
	angle = ANGLE_0;
	///
	if ((x < 128) && (y < 128))
	{
		switch(angle)
		{
			case ANGLE_0:
				break;
			case ANGLE_180:
				x = 128 - x;
				y = 128 - y;
				break;
			case ANGLE_90:
				tmp = y;
				y = 128 - x;
				x = tmp;
				break;
			case ANGLE_270:
				tmp = y;
				y = x;
				x = 128 - tmp;
				break;
				
		}

		Ptr = &Display.Line[y].Pixels.U8[(x&0x7F)>>3];

		if ( color == 0 )
			{	*Ptr &= ~(1<<(x&0x07));	}
		else
			{	*Ptr |= (1<<(x&0x07));	}	
	}		

}

void DisplaySetAngle(u8 newAngle)
{
	angle = newAngle;
}


/*=============================================================================
	Set DisplayBlack

	Sets display black
	
	17 Cycles @24MHz
=============================================================================*/
void DisplayBlack ()
{
	u8 y;
	//memset( &Display, 0xFF, sizeof(Display) );
for ( y=0; y<DISPLAY_NB_LINES; y++ )
	{
	//Display.Line[y].CmdWr = CMD_WR;
	Display.Line[y].Pixels.U32[0] = 0;
	Display.Line[y].Pixels.U32[1] = 0;
	Display.Line[y].Pixels.U32[2] = 0;
	Display.Line[y].Pixels.U32[3] = 0;
		
	}

	/*
	
	u8 x = 0;
	u8 y = 0;
	
	while (x < 128) {
		y = 0;
		while (y <128) {
			u8 *Ptr = &Display.Line[y].Pixels.U8[(x&0x7F)>>3];
			*Ptr &= ~(1<<(x&0x07));
			y++;
		}
		x++;
	}
	*/
	
}


void DisplayWhite ()
{
	u8 y;
	for ( y=0; y<DISPLAY_NB_LINES; y++ )
	{
		Display.Line[y].Pixels.U32[0] = 0xffffffff;
		Display.Line[y].Pixels.U32[1] = 0xffffffff;
		Display.Line[y].Pixels.U32[2] = 0xffffffff;
		Display.Line[y].Pixels.U32[3] = 0xffffffff;
	}
}

void DisplayWhiteBorders ()
{
	u8 y;
	DisplayBlack();
	for ( y=0; y<16; y++ )
	{
		Display.Line[y].Pixels.U32[0] = 0xffffffff;
		Display.Line[y].Pixels.U32[1] = 0xffffffff;
		Display.Line[y].Pixels.U32[2] = 0xffffffff;
		Display.Line[y].Pixels.U32[3] = 0xffffffff;
	}
	
	for ( y=16; y<112; y++ )
	{
		Display.Line[y].Pixels.U32[0] = 0x0000ffff;
		Display.Line[y].Pixels.U32[1] = 0;
		Display.Line[y].Pixels.U32[2] = 0;
		Display.Line[y].Pixels.U32[3] = 0xffff0000;
	}
	
	for ( y=112; y<128; y++ )
	{
		Display.Line[y].Pixels.U32[0] = 0xffffffff;
		Display.Line[y].Pixels.U32[1] = 0xffffffff;
		Display.Line[y].Pixels.U32[2] = 0xffffffff;
		Display.Line[y].Pixels.U32[3] = 0xffffffff;
	}
	
}
/*=============================================================================
	Display On
	
	Inputs : None
	
	Sets display's DISP pin
=============================================================================*/
void DisplayOn( void )
{
OutputSet( &DispPin );
}

/*=============================================================================
	Display Off

	Inputs : None
	
	Resets display's DISP pin
	While Display is OFF, SetPixel functions still work
=============================================================================*/
void DisplayOff( void )
{
OutputReset( &DispPin );
}

/*=============================================================================
	Display Refresh

	Inputs : None
	
=============================================================================*/
void DisplayRefresh( void )
{
y = 0;
}

/*=============================================================================
=============================================================================*/
bool IsDisplayRefreshed( void )
{
if ( y == Y_ALL_LINES_REFRESHED )
	return true;
else
	return false;
}

//=============================================================================
//	Set 16 Pixels 
//	
//	Inputs :
//		x : 0 to DISPLAY_NB_PIXELS_PER_LINE-1
//		y : 0 to DISPLAY_NB_LINES-1
//	
//	Sets required 16 pixels white without changing already white pixels
//	
//	NB Cycles @HCLK < 24MHz TBD
//=============================================================================//
void Set16PixelsAngle0( u8 x, u8 y, u16 Data )		//x y dans le repère 
{
	while (!IsDisplayRefreshed());
	if ((x < 128) && (y < 128))
	{
		u8 BitOffset;
		u8 WordPos;
		u8 X, Y;		//dans le repère de l'écran
		
		Y = x;				
		X = 127 - 16 - y;
		
		BitOffset = X%16;
		WordPos = X/16;
		if ( BitOffset == 0 )
		{
			Display.Line[Y].Pixels.U16[WordPos] |= Data;
		}
		else
		{
			union
			{
				u32 U32;
				u16 U16[2];
			} Tmp = {0};
			Tmp.U16[0] = Data;
			Tmp.U32 <<= BitOffset;
			Display.Line[Y].Pixels.U16[WordPos] |= Tmp.U16[0];
			if ( WordPos*16 < 128 )
			{
				// If second 16 bits is outside display side, do not display
				Display.Line[Y].Pixels.U16[WordPos+1] |= Tmp.U16[1];
			}
		}
	}
}
