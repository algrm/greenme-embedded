#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include "StdTypes.h"


//don't forget to change compiler optimization level for production
//#define DEBUG_ON
//#define FWVERSION		0x0001
#define FWVERSION		0x023B	//2.59

#define REQUIRED_CCS811_MIN_FW 0x2000						//minimum firmware requirements for modules
#define REQUIRED_ARMN8_MIN_FW  0x03020300

#define TX_INTERVAL_MS			 600000	//transmit short message every 10 mn				
#define FULL_TX_INTERVAL_MS	3600000	//transmit full message every 1 hour	

#define LUX_GAIN_ADJUST			1.25												// LUX gain adjustment mostly due to lens = 1.25;
#define TEMPERATURE_OFFSET	-1.60												//(°C) board temperature offset due to components heat dissipation

#define SOUND_EVENT_NSAMPLES_DEFAULT		 15							//units, number of sample to take to compute avg for sound events
#define SOUND_EVENT_THRESHOLD_DEFAULT	    5							//dBA, threshold value to generate a message


static enum u8 {FR_FR, EN_US, EN_GB, DE_DE, NL_NL, PT_PT, ES_ES, NB_LOCALES} locales;
//							0			1				2			 3			4			 5			6

#endif

