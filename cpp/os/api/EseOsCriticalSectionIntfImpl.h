#ifndef _ese_critical_section_intf_imple_h_
#define _ese_critical_section_intf_imple_h_

class EseOsCriticalSectionIntfImpl : public EseOsCriticalSectionIntf
{
protected:
  EseOsCriticalSectionIntfImpl() ESE_NOTHROW ESE_KEEP;
  
public:
  virtual ~EseOsCriticalSectionIntfImpl() ESE_NOTHROW ESE_KEEP;
  
  /// EseOsCriticalSectionIntf interface
  ///
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus lock(esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus unlock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
private:
  EseMutexRecursive m_mx;
  
  ESE_NONCOPYABLE(EseOsCriticalSectionIntfImpl);
  
  friend class EseOsIntfImpl;
};

#endif //< _ese_critical_section_intf_imple_h_
