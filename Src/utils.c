#include "utils.h"
#include "time.h"


/*************************************************************
* GetElapsedus
* donne le temps �coul� depuis lastTicks (microsecondes)
* Attention : g�re au maximum 1 reset compteur, soit 1,19h
**************************************************************/
u32 GetElapsedus(u32 lastTicks)
{
		return GetusTick() - lastTicks;
}
