#include <type.h>
#pragma hdrstop

// simple fixed point math implementation
//
#include "fxp.h"

// internal typedef - multiplication|division accumulator
typedef long long fxp_accum;

// division & multiplication
fxp fxpDiv(fxp val1, fxp val2)
{
	if( val2 == 0 ) 
		return FXP_MAX;
	else
		return (fxp)((((fxp_accum)val1 << FXP_FRAC)/val2));
}

fxp fxpMul(fxp val1, fxp val2)
{
	return (fxp)(((fxp_accum)val1*(fxp_accum)val2) >> FXP_FRAC);
}
