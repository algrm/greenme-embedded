#ifndef DISPLAY_H
#define DISPLAY_H

#include "StdTypes.h"

/*=============================================================================
	DEFINITIONS
=============================================================================*/
#define DISPLAY_NB_LINES			128
#define DISPLAY_NB_PIXELS_PER_LINE	128

typedef enum {BLACK, WHITE} PixelCol_t;

typedef enum {ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270} DisplayOrientations_t; 

/*=============================================================================
	FUNCTIONS	
=============================================================================*/
void DisplaySetup( void );														// Initializes display : must be called soon after power up
void SetPixel( u8 x, u8 y, u8 color, u8 angle );									// x : 0 to DISPLAY_NB_PIXELS_PER_LINE-1; y : 0 to DISPLAY_NB_LINES-1
void DisplayOn( void );															// Powers the display ON
void DisplayOff( void );														// Shuts down the display
void DisplayRefresh( void );													// Refreshes the whole display
bool IsDisplayRefreshed( void );												// returns true when display has been fully refreshed ( false while refreshing )
void DisplayBlack (void);
void DisplayWhite (void);
void DisplayWhiteBorders (void);

void SetPixel180(u8 x, u8 y, u8 color);
void Set16PixelsAngle0( u8 x, u8 y, u16 Data );
#endif //DISPLAY_H
