#ifndef AVERAGES_H
#define AVERAGES_H
#include "StdTypes.h"
#include "CubeData.h"



typedef struct
{
	u32 sum_noisex2;
	u32 sum_flicker;
	u32 sum_colorR;
	u32 sum_colorG;
	u32 sum_colorB;
	u32 sum_colorW;
	u32 sum_lux;
	u16 nb;
} Averages_t;


void AvgReset(Averages_t* avg, CubeData_t* data);
void AvgZero(Averages_t* avg);
void AddPointToAverage(CubeData_t* data, Averages_t* avg);


#endif
