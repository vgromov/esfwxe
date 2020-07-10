#ifndef _ese_breaker_intf_h_
#define _ese_breaker_intf_h_

/// Abstract breaker interface
///
class ESE_ABSTRACT EseBreakerIntf
{
public:
  /// Return true if something is breaking execution. If not, wait for breaking execution for tmo ms.
  virtual bool isBreaking(esU32 tmo) const ESE_NOTHROW = 0;
};

#endif // _ese_breaker_intf_h_
