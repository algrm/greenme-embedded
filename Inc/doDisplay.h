#ifndef DODISPLAY_H
#define DODISPLAY_H

#include "CubeData.h"
#include "Time.h"

typedef enum { DISPLAY_TEMP, DISPLAY_HYGR, DISPLAY_LUX, DISPLAY_NOISE, DISPLAY_AIR, DISPLAY_CONFIG, DISPLAY_HAPPY, DISPLAY_UNHAPPY, DISPLAY_MSG } DisplayModes_t;

u8 doDisplay( CubeData_t *Data, Time_t Time);
void DisplaySingleString(char* str, u8 len);
void ShowGreenMe(void);

//DEBUG
void TestDisplay(void);

#endif
