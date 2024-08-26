#ifndef DISPLAYMANAGEMENT_H
#define DISPLAYMANAGEMENT_H

#include "StdTypes.h"
#include "Main.h"

typedef enum { DISPLAY_MEASURES, DISPLAY_CONFIG, DISPLAY_HAPPY, DISPLAY_UNHAPPY, DISPLAY_QUESTION, DISPLAY_MESSAGE } DisplayModes_t;

void doDisplay(CubeData_t* Data, u8 lastOrientation, u8 warmingUp);
void ShowHappy(bool happy, u8 lastOrientation, CubeData_t* Data);
void DisplayConfig(void );
void DisplayMeasures(u8 warmingUp, CubeData_t* Data );
u8   getDisplayMode(void);
void setDisplayMode(u8 mode);
u8   getLastDisplayRefreshTick(void);
void setLastDisplayRefreshTick(u32 ticks);
void DisplayQuestion(void);
void initLocale();
void DisplayMessage(CubeData_t*);


#endif
