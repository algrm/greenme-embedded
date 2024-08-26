#ifndef HDC1080_H
#define HDC1080_H

#include "StdTypes.h"

void HumTempCfg( void );														// On the go sensor reconfiguration
float HDC1080Temp( void );														//(°C) returns temperature
float HDC1080Hygr( void );														//(%) returns hygrometry
bool HumTempTask( void );														// returns true when humidity scan task returns control
void PurgeHumidityStart( void );												// starts heating humidity sensor to purge humidity
void PurgeHumidityStop( void );													// Stops humidity sensor forced heating

#endif // HDC1080_H

