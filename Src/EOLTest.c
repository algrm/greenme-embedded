#include "EOLTest.h"
#include "Uart1.h"
#include "HDC1080.h"
#include "Time.h"
#include "dBAMeter.h"
#include <string.h>
#include "Cfg.h"
#include "Radio.h"

/*=============================================================================
	Definitions
=============================================================================*/
typedef struct
{
u8 Cmd;
u8 Unused;
s16 CalVal;
}TstCmd_t;

/*=============================================================================
	Private variables
=============================================================================*/
//u8 TestBuf[128];

/*=============================================================================
	End Of Line Test Setup
=============================================================================*/
void EndOfLineTestSetup( void )
{// No longer useful : no USB link
}

/*=============================================================================
	End Of Line Test Request
=============================================================================*/
void EOLTestRequest( u8 *Buf )
{
TstCmd_t *Tst = (TstCmd_t *)Buf;
Cfg_t *CfgPtr = CfgPointer();
if ( Tst->Cmd == 'C' )
	{// Calibration
	CfgPtr->AudioCalibOffsetx100 = Tst->CalVal;
	CfgUpdate();
//	RadioSendFrame(RADIO_TEST_REQ, 0, 0);
	}
else if ( Tst->Cmd == 'E' )
	{// End of test
//	RadioSetPermanentNetConfig( 2, 0xFEFD, 0xFEFC);							// Test was OK : change to default radio ID
	}
}
