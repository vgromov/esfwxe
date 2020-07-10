#ifndef _ese_math_intf_impl_h_
#define _ese_math_intf_impl_h_

class EseMathIntfImpl : public EseMathIntf
{
protected:
  EseMathIntfImpl() ESE_NOTHROW ESE_KEEP {}
  
public:
  static const EseMathIntfImpl& instanceGet() ESE_NOTHROW ESE_KEEP;

  virtual float float_min() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float float_eps() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float float_qnan() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float float_snan() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float pi() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual int abs_i(int x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long abs_l(long x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual int cmp_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long min_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long max_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual int cmp_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long min_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long max_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual unsigned long long mod_ull(unsigned long long val, unsigned long long div) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long mul_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long div_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long add_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long sub_ull(unsigned long long _1, unsigned long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual long long mod_ll(long long val, long long div) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long mul_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long div_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long add_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long sub_ll(long long _1, long long _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long abs_ll(long long x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual int cmp(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float min(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float max(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual bool isNaN(float f) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isInf(float f) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isFinite(float f) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual float abs(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool areEqual(float _1, float _2, uint32_t maxUlps = 1) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float floor(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float ceil(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float round(float x, unsigned fracDigits) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float round(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual float mod(float val, float* intfptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float mul(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float div(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float add(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float sub(float _1, float _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float pow10(int n) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float pow2(int n) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float pow(float x, float p) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float sqrt(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual float log(float val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float log2(float val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float log10(float val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float logN(float base, float val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual float sin(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float cos(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float tan(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual int f2i(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned int f2ui(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual unsigned long long f2ull(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual long long f2ll(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float i2f(int x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float ui2f(unsigned int x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float ull2f(unsigned long long x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float ll2f(long long x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual double f2d(float x) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

#ifdef ESE_USE_MATH_VALSTAT
  virtual EseMathValStatIntf* statisticsCalculatorCreate() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
#endif
#ifdef ESE_USE_MATH_SPLINE
  virtual EseMathSplineIntf* splineCreate(size_t nodesMaxCnt) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
#endif
  
  ESE_NONCOPYABLE(EseMathIntfImpl);
};

#endif //< _ese_math_intf_impl_h_
