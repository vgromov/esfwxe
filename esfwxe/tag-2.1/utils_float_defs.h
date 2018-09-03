#ifndef _ese_utils_float_defs_h_
#define _ese_utils_float_defs_h_

#include <math.h>
#include <float.h>

#ifdef __arm__

#  ifndef esIsInfF
#    define esIsInfF      isinf
#  endif

#  ifndef esFiniteF
#    define esFiniteF    isfinite
#  endif

# ifndef esIsNanF
#   define esIsNanF      isnan
# endif

# ifndef esModfF
#   define esModfF       modff
# endif

#elif defined(_MSC_VER)

#  ifndef esIsInfF
#    define esIsInfF(x)  (0==_finite(x))
#  endif

#  ifndef esFiniteF
#    define esFiniteF    _finite
#  endif

#  ifndef esIsNanF
#    define esIsNanF    _isnan
#  endif

#elif defined(__BORLANDC__)

// if we're building escomm package - import implementations from escore
//# if defined(ESCOMM_EXPORTS)
extern "C" int es_finitef(float f);
extern "C" int es_isnanf(float f);
extern "C" float es_modff(float x, float* intpart);

# ifndef esModfF
#   define esModfF      es_modff
# endif

# ifndef esFiniteF
#   define esFiniteF    es_finitef
# endif

# ifndef esIsInfF
#   define esIsInfF(x)  (0 == es_finitef(x))
# endif

# ifndef esIsNanF
#   define esIsNanF     es_isnanf
# endif
#endif

#endif //< _ese_utils_float_defs_h_
