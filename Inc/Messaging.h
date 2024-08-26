#ifndef MESSAGING_H
#define MESSAGING_H

#include "CubeData.h"
#include "Averages.h"

void MakeRadioMessage(u8 msgType, CubeData_t* data, Averages_t* avg);

#endif
