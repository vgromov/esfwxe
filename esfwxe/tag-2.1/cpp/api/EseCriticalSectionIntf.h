#ifndef _ese_critical_section_intf_h_
#define _ese_critical_section_intf_h_

#include <esfwxe/cpp/os/EseOsDefs.h>

class ESE_ABSTRACT EseCriticalSectionIntf
{
public:  
  // Object deallocation service
  virtual void destroy() ESE_NOTHROW = 0;

  virtual bool isOk() const ESE_NOTHROW = 0;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay) ESE_NOTHROW = 0;
  virtual rtosStatus unlock() ESE_NOTHROW = 0;
};

#endif //< _ese_critical_section_intf_h_
