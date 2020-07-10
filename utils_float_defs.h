#ifndef _ese_utils_float_defs_h_
#define _ese_utils_float_defs_h_

#ifdef __cplusplus
# include <cmath>
# include <cfloat>
#else
# include <math.h>
# include <float.h>
#endif

#if defined(__GNUC__) || defined(__arm__)

# ifndef esIsInfF
#   ifdef __cplusplus
#     define esIsInfF(x)      std::isinf(static_cast<float>(x))
#   else
#     define esIsInfF         isinff
#   endif 
# endif 
  
# ifndef esFiniteF 
#   ifdef __cplusplus
#     define esFiniteF(x)     std::isfinite(static_cast<float>(x))
#   else
#     define esFiniteF        isfinite
#   endif
# endif
  
# ifndef esIsNanF 
#   ifdef __cplusplus
#     define esIsNanF(x)      std::isnan(static_cast<float>(x))
#   else
#     if defined(__GNUC__)
#       define esIsNanF       isnan
#     else
#       define esIsNanF       isnanf
#     endif
#   endif
# endif
  
# ifndef esModfF
#   ifdef __cplusplus
#     define esModfF(x, iptr) std::modf(static_cast<float>(x), reinterpret_cast<float*>(iptr))
#   else
#     define esModfF          modff
#   endif
# endif

#elif defined(_MSC_VER)

#  ifndef esIsInfF
#    define esIsInfF(x)       (0==_finite(x))
#  endif    
    
#  ifndef esFiniteF   
#    define esFiniteF         _finite
#  endif    
    
#  ifndef esIsNanF    
#    define esIsNanF          _isnan
#  endif

#elif defined(__BORLANDC__)

// if we're building escomm package - import implementations from escore
//# if defined(ESCOMM_EXPORTS)
#ifdef __cplusplus
extern "C" {
#endif

int es_finitef(float f);
int es_isnanf(float f);
float es_modff(float x, float* intpart);

#ifdef __cplusplus
}
#endif

# ifndef esModfF
#   define esModfF          es_modff
# endif   
    
# ifndef esFiniteF    
#   define esFiniteF        es_finitef
# endif   
    
# ifndef esIsInfF   
#   define esIsInfF(x)      (0 == es_finitef(x))
# endif

# ifndef esIsNanF
#   define esIsNanF     es_isnanf
# endif
#endif

#endif //< _ese_utils_float_defs_h_
