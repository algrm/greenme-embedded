#include "Filters.h"

/*=============================================================================
	FILTER
	
	Inputs : 
		Val : previous filtered value
		NewVal : Last sample of Val
		N : Filter size
		
	When NewVal is far from Val
		returns Val = ((N-1)Val + NewVal)/N
	otherwise
		returns Val+1 or Val-1 depending on NewVal ( higher or lower than Val )
=============================================================================*/
s32 Filter( s32 Val, s32 NewVal, u16 N )
{
if ( NewVal < Val )
	{
	if ( NewVal < Val-N )
		Val = ((N-1)*Val + NewVal)/N;
	else
		Val--;
	}
else
	{
	if ( NewVal > Val+N )
		Val = ((N-1)*Val + NewVal)/N;
	else
		Val++;
	}
return Val;
}
