#ifndef _ese_breaker_intf_h_
#define _ese_breaker_intf_h_

/// Abstract breaker interface
///

class EseBreakerIntf
{
protected:
  EseBreakerIntf() ESE_NOTHROW {}
  
public:
  virtual ~EseBreakerIntf() ESE_NOTHROW {}
  
  /// Return true if something is breaking execution. If not, wait for breaking execution for tmo ms.
  virtual bool isBreaking(esU32 tmo) const ESE_NOTHROW = 0;
  
private:
  EseBreakerIntf(const EseBreakerIntf&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseBreakerIntf& operator=(const EseBreakerIntf&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // _ese_breaker_intf_h_
