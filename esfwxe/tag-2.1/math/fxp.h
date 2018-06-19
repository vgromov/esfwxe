#ifndef _fxp_h_
#define _fxp_h_

#include <fxpConfig.h>

#ifndef FXP_FRAC
#	error "FXP_FRAC must be #defined"
#endif

// simple fixed point math header file
//

#ifdef __cplusplus
	extern "C" {
#endif

// fxp s17.14 typedef 
typedef long fxp;
// helper consts
#define FXP_MAX_FRAC 	(1 << FXP_FRAC)
#define	FXP_FRAC_MASK (FXP_MAX_FRAC-1)
#define FXP_MAX 			0x7FFFFFFF
#define	FXP_MIN				0x80000001

// division & multiplication
fxp fxpDiv(fxp val1, fxp val2);
fxp fxpMul(fxp val1, fxp val2);

// addition and subtraction
#define fxpAdd(val1, val2) 	((fxp)(val1)+(fxp)(val2))
#define fxpSub(val1, val2) 	((fxp)(val1)-(fxp)(val2))

// fractional part extraction
#define fxpFrac(val) 				((fxp)(val) & FXP_FRAC_MASK)

// typecasts
#define fxpToInt(fxp) 			((fxp) >> FXP_FRAC)
#define intToFxp(intVal)		((fxp)(intVal) << FXP_FRAC)
#define fxpToFloat(fxp) 		(((float)(fxp)) / (float)FXP_MAX_FRAC)
#define floatToFxp(val) 		((fxp)((val) * FXP_MAX_FRAC))

#ifdef __cplusplus
	}
#endif 

#endif // _fxp_h_
