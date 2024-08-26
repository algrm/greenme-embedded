#ifndef FILTERS_H
#define FILTERS_H

#include <StdTypes.h>

s32 Filter( s32 Val, s32 NewVal, u16 N );										// returns a filtered value Ret=(Val*(N-1) + NewVal)/N

#endif
