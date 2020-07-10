#ifndef _ese_math_spline_intf_impl_h_
#define _ese_math_spline_intf_impl_h_

class EseMathSplineIntfImpl : public EseMathSplineIntf
{
protected:
  EseMathSplineIntfImpl(size_t cntMax) ESE_NOTHROW ESE_KEEP;

public:
  virtual ~EseMathSplineIntfImpl() ESE_NOTHROW ESE_KEEP;

  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isEqualTo(const EseMathSplineIntf& other) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isEmpty() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t maxNodesCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t nodesCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void nodesCountSet(size_t cnt) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void assign(const EseMathSplineIntf& other) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void assign(const EseMathSpline& other) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void assign(const uint8_t* data, size_t dataLen, bool owning) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void nodeSet(size_t idx, float x, float a, float b, float c, float d) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool nodeGet(size_t idx, float& x, float& a, float& b, float& c, float& d) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void cleanup() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual float valueGet(float x, bool& valid) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const EseMathSpline& splineRawGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void crcUpdate(EseCrcIntf& icrc) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

#ifdef ESE_USE_SPLINE_SERIALIZATION
# if defined(ESE_USE_SPLINE_SERIALIZATION_READ) && 0 != ESE_USE_SPLINE_SERIALIZATION_READ
  virtual bool readFrom(EseStreamIntf& in) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
# endif
# if defined(ESE_USE_SPLINE_SERIALIZATION_WRITE) && 0 != ESE_USE_SPLINE_SERIALIZATION_WRITE
  virtual bool writeTo(EseStreamIntf& out) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool writeNodesTo(EseStreamIntf& out) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
# endif
#endif

protected:
  EseMathSpline m_impl;
  size_t m_cntMax;
  
  friend class EseMathIntfImpl;
};

#endif //< _ese_math_spline_intf_impl_h_
