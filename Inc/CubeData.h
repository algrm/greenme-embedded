#ifndef CUBEDATA_H
#define CUBEDATA_H
#include "StdTypes.h"
#include "I2CStdSensors.h"

typedef struct 
{
	u8 Red;																			// Red ratio of white light 255:99.6%
	u8 Green;																		// Green ratio of white light 255:99.6%
	u8 Blue;																		// Blue ratio of white light 255:99.6%
	u8 Flicker;																		// Flicker as a percentage of total light
	u16 WhiteTus;																	// Unused ( for previous version compatibility )
}ColorAndFlicker_t;


typedef struct
{
	// IRQ Updated data : only read access outside of IRQ : read each in one cycle
	u16 VOCppb;
	s16 Lux;
	u16 Tempx100; 							//on peut gagner 3 bits ici (valeur max : 5000)	
	u16 Hygrx100;								//on peut gagner 2 bits ici (valeur max : 10000)
	u16 dBAx100;
	u8 noiseAveragex2;			//average noise in half dBA
	u8 noiseMaxx2; 					//max noise in half dBA
	ColorAndFlicker_t colors;
	u8 Flicker;

	u16 CO2ppm;
	u16 pm10;
	u16 pm2_5;
	u16 pm1;

	u8 Orientation;
	u8 IsCCS811Active 	:1;
	u8 IsCO2Mounted   	:1;
	u8 IsPMMounted   	  :1;
	u8 happy						:5;
	
	u16 LastOrientation 		:4;	//Up, left, right, upside-down
	u16 tiltSecCounter  		:4;	
	u16 displayMode     		:4;		//measure, config, question, smiley
	u16 isRadioJoined   		:2;		//armn8 has join network : 0=unknown, 1=yes, 3=no
	u16 isBoardInitialized	:1;
	u16 radioDataMode   		:1;		//armn8 is in idle mode (1) or config mode (0) 
	
}CubeData_t;



#endif
