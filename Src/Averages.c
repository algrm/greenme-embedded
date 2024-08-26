#include "Averages.h"
#include "CubeData.h"

/**********
Set all averages initial values
***********/
void AvgReset(Averages_t *avg, CubeData_t* data)
{
	avg->nb = 1;
	avg->sum_colorB = data->colors.Blue;
	avg->sum_colorG = data->colors.Green;
	avg->sum_colorR = data->colors.Red;
	avg->sum_colorW = data->colors.WhiteTus;
	avg->sum_flicker = data->Flicker;
	avg->sum_lux = data->Lux;
	avg->sum_noisex2 = data->dBAx100/50;
}

/**********
Set all averages to zero
***********/
void AvgZero(Averages_t *avg)
{
	avg->nb = 0;
	avg->sum_colorB = 0;
	avg->sum_colorG = 0;
	avg->sum_colorR = 0;
	avg->sum_colorW = 0;
	avg->sum_flicker = 0;
	avg->sum_lux = 0;
	avg->sum_noisex2 = 0;
}

/**********
Add point to averages. We assume that the sum will not exceed UINT_32 limit
max samples : 65536 <=> 18.2 hours at 1 sample/s
***********/
void AddPointToAverage(CubeData_t* data, Averages_t* avg)
{
	avg->sum_noisex2 += data->dBAx100/100.0*2;
	avg->sum_flicker += data->Flicker;
	avg->sum_lux 		 += data->Lux;
	avg->sum_colorR  += data->colors.Red;
	avg->sum_colorG  += data->colors.Green;
	avg->sum_colorB  += data->colors.Blue;
	avg->sum_colorW  += data->colors.WhiteTus;
	avg->nb++;
}

