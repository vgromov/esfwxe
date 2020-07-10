#ifndef _ese_math_intf_h_
#define _ese_math_intf_h_

// Forward decls
class EseMathValStatIntf;
class EseMathSplineIntf;

/// Math utilities and constants interface
///
class ESE_ABSTRACT EseMathIntf
{
public:
  /// Special math constants
  ///
  virtual float float_min() const ESE_NOTHROW = 0;
  virtual float float_eps() const ESE_NOTHROW = 0;
  virtual float float_qnan() const ESE_NOTHROW = 0;
  virtual float float_snan() const ESE_NOTHROW = 0;
  virtual float pi() const ESE_NOTHROW = 0;

  /// Math int and long utilities 
  ///
  virtual int abs_i(int x) const ESE_NOTHROW = 0;
  virtual long abs_l(long x) const ESE_NOTHROW = 0;

  /// Math long long utilities
  ///
  virtual int cmp_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;
  virtual unsigned long long min_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;
  virtual unsigned long long max_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;

  virtual int cmp_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long min_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long max_ll(long long _1, long long _2) const ESE_NOTHROW = 0;

  virtual unsigned long long mod_ull(unsigned long long val, unsigned long long div) const ESE_NOTHROW = 0;
  virtual unsigned long long mul_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;
  virtual unsigned long long div_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;
  virtual unsigned long long add_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;
  virtual unsigned long long sub_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW = 0;

  virtual long long mod_ll(long long val, long long div) const ESE_NOTHROW = 0;
  virtual long long mul_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long div_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long add_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long sub_ll(long long _1, long long _2) const ESE_NOTHROW = 0;
  virtual long long abs_ll(long long x) const ESE_NOTHROW = 0;

  /// Math float utilities
  ///
  virtual int cmp(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float min(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float max(float _1, float _2) const ESE_NOTHROW = 0;
  
  virtual bool isNaN(float f) const ESE_NOTHROW = 0;
  virtual bool isInf(float f) const ESE_NOTHROW = 0;
  virtual bool isFinite(float f) const ESE_NOTHROW = 0;
  
  virtual float abs(float x) const ESE_NOTHROW = 0;
  virtual bool areEqual(float _1, float _2, uint32_t maxUlps = 1) const ESE_NOTHROW = 0;
  virtual float floor(float x) const ESE_NOTHROW = 0;
  virtual float ceil(float x) const ESE_NOTHROW = 0;
  virtual float round(float x, unsigned fracDigits) const ESE_NOTHROW = 0;
  virtual float round(float x) const ESE_NOTHROW = 0;

  virtual float mod(float val, float* intfptr = nullptr) const ESE_NOTHROW = 0;
  virtual float mul(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float div(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float add(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float sub(float _1, float _2) const ESE_NOTHROW = 0;
  virtual float pow10(int n) const ESE_NOTHROW = 0;
  virtual float pow2(int n) const ESE_NOTHROW = 0;
  virtual float pow(float x, float p) const ESE_NOTHROW = 0;
  virtual float sqrt(float x) const ESE_NOTHROW = 0;

  virtual float log(float val) ESE_NOTHROW = 0;
  virtual float log2(float val) ESE_NOTHROW = 0;
  virtual float log10(float val) ESE_NOTHROW = 0;
  virtual float logN(float base, float val) ESE_NOTHROW = 0;

  virtual float sin(float x) const ESE_NOTHROW = 0;
  virtual float cos(float x) const ESE_NOTHROW = 0;
  virtual float tan(float x) const ESE_NOTHROW = 0;
  
  /// Type conversions
  ///
  virtual int f2i(float x) const ESE_NOTHROW = 0;
  virtual unsigned int f2ui(float x) const ESE_NOTHROW = 0;
  virtual unsigned long long f2ull(float x) const ESE_NOTHROW = 0;
  virtual long long f2ll(float x) const ESE_NOTHROW = 0;
  virtual float i2f(int x) const ESE_NOTHROW = 0;
  virtual float ui2f(unsigned int x) const ESE_NOTHROW = 0;
  virtual float ull2f(unsigned long long x) const ESE_NOTHROW = 0;
  virtual float ll2f(long long x) const ESE_NOTHROW = 0;
  virtual double f2d(float x) const ESE_NOTHROW = 0;

  /// Optional math functionality
  ///
#ifdef ESE_USE_MATH_VALSTAT
  virtual EseMathValStatIntf* statisticsCalculatorCreate() const ESE_NOTHROW = 0;
#endif
#ifdef ESE_USE_MATH_SPLINE
  virtual EseMathSplineIntf* splineCreate(size_t nodesMaxCnt) const ESE_NOTHROW = 0;
#endif
};

#endif //< _ese_math_intf_h_
