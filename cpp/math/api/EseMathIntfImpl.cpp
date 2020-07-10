#include <esfwxe/target.h>
#include <esfwxe/ese_assert.h>

#include <cmath>

#include <esfwxe/cpp/math/EseMathConsts.h>
#include <esfwxe/cpp/math/EseMathUtils.h>

#include "EseMathIntf.h"
#include "EseMathIntfImpl.h"

#ifdef ESE_USE_MATH_VALSTAT
# include <esfwxe/math/EseMathValStat.h>
# include "EseMathValStatIntf.h"
# include "EseMathValStatIntfImpl.h"
#endif

#ifdef ESE_USE_MATH_SPLINE
# include <esfwxe/cpp/math/EseMathSpline.h>
# include "EseMathSplineIntf.h"
# include "EseMathSplineIntfImpl.h"
#endif
//--------------------------------------------------------------------------------------

const EseMathIntfImpl& EseMathIntfImpl::instanceGet() ESE_NOTHROW
{
  static EseMathIntfImpl s_impl;
  
  return s_impl;  
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::float_min() const ESE_NOTHROW
{
  return EseMath::float_min;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::float_eps() const ESE_NOTHROW
{
  return EseMath::float_eps;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::float_qnan() const ESE_NOTHROW
{
  return EseMath::float_qnan;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::float_snan() const ESE_NOTHROW
{
  return EseMath::float_snan;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::pi() const ESE_NOTHROW
{
  return EseMath::pi;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

int EseMathIntfImpl::abs_i(int x) const ESE_NOTHROW
{
  return (x < 0) ? (-x) : x;
}
//--------------------------------------------------------------------------------------

long EseMathIntfImpl::abs_l(long x) const ESE_NOTHROW
{
  return (x < 0) ? (-x) : x;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

int EseMathIntfImpl::cmp_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return -1;
  else if( _1 > _2 )
    return 1;
  
  return 0;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::min_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return _1;
    
  return _2;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::max_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  if( _1 > _2 )
    return _1;
    
  return _2;
}
//--------------------------------------------------------------------------------------

int EseMathIntfImpl::cmp_ll(long long _1, long long _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return -1;
  else if( _1 > _2 )
    return 1;
  
  return 0;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::min_ll(long long _1, long long _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return _1;
    
  return _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::max_ll(long long _1, long long _2) const ESE_NOTHROW
{
  if( _1 > _2 )
    return _1;
    
  return _2;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::mod_ull(unsigned long long val, unsigned long long div) const ESE_NOTHROW
{
  return val % div;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::mul_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  return _1 * _2;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::div_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  return _1 / _2;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::add_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  return _1 + _2;
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::sub_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW
{
  return _1 - _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::mod_ll(long long val, long long div) const ESE_NOTHROW
{
  return val % div;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::mul_ll(long long _1, long long _2) const ESE_NOTHROW
{
  return _1 * _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::div_ll(long long _1, long long _2) const ESE_NOTHROW
{
  return _1 / _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::add_ll(long long _1, long long _2) const ESE_NOTHROW
{
  return _1 + _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::sub_ll(long long _1, long long _2) const ESE_NOTHROW
{
  return _1 - _2;
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::abs_ll(long long x) const ESE_NOTHROW
{
  return (x < 0) ? (-x) : x;
}
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

int EseMathIntfImpl::cmp(float _1, float _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return -1;
  else if( _1 > _2 )
    return 1;
  
  return 0;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::min(float _1, float _2) const ESE_NOTHROW
{
  if( _1 < _2 )
    return _1;

  return _2;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::max(float _1, float _2) const ESE_NOTHROW
{
  if( _1 > _2 )
    return _1;
    
  return _2;
}
//--------------------------------------------------------------------------------------

bool EseMathIntfImpl::isNaN(float f) const ESE_NOTHROW
{
  return EseMath::isNaN(f);
}
//--------------------------------------------------------------------------------------

bool EseMathIntfImpl::isInf(float f) const ESE_NOTHROW
{
  return EseMath::isInf(f);
}
//--------------------------------------------------------------------------------------

bool EseMathIntfImpl::isFinite(float f) const ESE_NOTHROW
{
  return EseMath::isFinite(f);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::abs(float x) const ESE_NOTHROW
{
  return EseMath::abs(x);
}
//--------------------------------------------------------------------------------------

bool EseMathIntfImpl::areEqual(float _1, float _2, uint32_t maxUlps) const ESE_NOTHROW
{
  return EseMath::areEqual(
    _1, 
    _2,
    maxUlps
  );
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::floor(float x) const ESE_NOTHROW
{
  return std::floorf(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::ceil(float x) const ESE_NOTHROW
{
  return std::ceilf(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::round(float x, unsigned fracDigits) const ESE_NOTHROW
{
  return EseMath::round(
    x,
    fracDigits
  );
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::round(float x) const ESE_NOTHROW
{
  return EseMath::round(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::mod(float val, float* intfptr) const ESE_NOTHROW
{
  return std::modff(
    val,
    intfptr
  );
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::mul(float _1, float _2) const ESE_NOTHROW
{
  return _1*_2;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::div(float _1, float _2) const ESE_NOTHROW
{
  return _1/_2;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::add(float _1, float _2) const ESE_NOTHROW
{
  return _1+_2;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::sub(float _1, float _2) const ESE_NOTHROW
{
  return _1-_2;
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::pow10(int n) const ESE_NOTHROW
{
  return EseMath::pow10(n);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::pow2(int n) const ESE_NOTHROW
{
  return EseMath::pow2(n);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::pow(float x, float p) const ESE_NOTHROW
{
  return EseMath::pow(
    x, 
    p
  );
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::sqrt(float x) const ESE_NOTHROW
{
  return EseMath::sqrt(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::log(float val) ESE_NOTHROW
{
  return EseMath::log(val);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::log2(float val) ESE_NOTHROW
{
  return EseMath::log2(val);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::log10(float val) ESE_NOTHROW
{
  return EseMath::log10(val);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::logN(float base, float val) ESE_NOTHROW
{
  return EseMath::logN(
    base,
    val
  );
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::sin(float x) const ESE_NOTHROW
{
  return std::sinf(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::cos(float x) const ESE_NOTHROW
{
  return std::cosf(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::tan(float x) const ESE_NOTHROW
{
  return std::tanf(x);
}
//--------------------------------------------------------------------------------------

int EseMathIntfImpl::f2i(float x) const ESE_NOTHROW
{
  return static_cast<int>(x);
}
//--------------------------------------------------------------------------------------

unsigned int EseMathIntfImpl::f2ui(float x) const ESE_NOTHROW
{
  return static_cast<unsigned int>(x);
}
//--------------------------------------------------------------------------------------

unsigned long long EseMathIntfImpl::f2ull(float x) const ESE_NOTHROW
{
  return static_cast<unsigned long long>(x);
}
//--------------------------------------------------------------------------------------

long long EseMathIntfImpl::f2ll(float x) const ESE_NOTHROW
{
  return static_cast<long long>(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::i2f(int x) const ESE_NOTHROW
{
  return static_cast<float>(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::ui2f(unsigned int x) const ESE_NOTHROW
{
  return static_cast<float>(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::ull2f(unsigned long long x) const ESE_NOTHROW
{
  return static_cast<float>(x);
}
//--------------------------------------------------------------------------------------

float EseMathIntfImpl::ll2f(long long x) const ESE_NOTHROW
{
  return static_cast<float>(x);
}
//--------------------------------------------------------------------------------------

double EseMathIntfImpl::f2d(float x) const ESE_NOTHROW
{
  return static_cast<double>(x);
}
//--------------------------------------------------------------------------------------

#ifdef ESE_USE_MATH_VALSTAT
EseMathValStatIntf* EseMathIntfImpl::statisticsCalculatorCreate() const ESE_NOTHROW
{
  return new EseMathValStatIntfImpl;
}
//--------------------------------------------------------------------------------------
#endif

#ifdef ESE_USE_MATH_SPLINE
EseMathSplineIntf* EseMathIntfImpl::splineCreate(size_t nodesMaxCnt) const ESE_NOTHROW
{
  ESE_ASSERT( nodesMaxCnt > 2 );

  return new EseMathSplineIntfImpl(nodesMaxCnt);
}
//--------------------------------------------------------------------------------------
#endif
