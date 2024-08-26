#ifndef TIM_H
#define TIM_H

#include <StdTypes.h>

u16 OptimalTimPrescaler( float Period, u32 *TimFreq );
u16 ComputeTimPrescaler( TIM_TypeDef* TIMx, u32 *TimFreq );

#endif
