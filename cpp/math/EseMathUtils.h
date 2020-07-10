#ifndef _ese_math_utils_h_
#define _ese_math_utils_h_

namespace EseMath {

bool isNaN(float f) ESE_NOTHROW;
bool isInf(float f) ESE_NOTHROW;
bool isFinite(float f) ESE_NOTHROW;

float pow10(int n) ESE_NOTHROW;
float pow2(int n) ESE_NOTHROW;
float pow(float x, float p) ESE_NOTHROW;
float sqrt(float x) ESE_NOTHROW;

float log(float val) ESE_NOTHROW;
float log2(float val) ESE_NOTHROW;
float log10(float val) ESE_NOTHROW;
float logN(float base, float val) ESE_NOTHROW;

float abs(float x) ESE_NOTHROW;
float round0(float val) ESE_NOTHROW;
float round(float x, unsigned fracDigits) ESE_NOTHROW;
float round(float x) ESE_NOTHROW;

bool areEqual(float _1, float _2, uint32_t maxUlps = 1) ESE_NOTHROW;

} // namespace EsMath

#endif // _ese_math_utils_h_
