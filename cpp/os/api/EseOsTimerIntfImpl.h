#ifndef _ese_os_timer_intf_impl_h_
#define _ese_os_timer_intf_impl_h_

/// OS timer interface implementor
///
class EseOsTimerIntfImpl : public EseOsTimerIntf
{
private:
  class Impl : public EseTimer
  {
  public:
    Impl(EseOsTimerIntfImpl& owner, int id) ESE_NOTHROW;
    
    inline int idGet() const ESE_NOTHROW { return m_id; }
    
  protected:
    virtual void onExpire() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onStart() ESE_NOTHROW ESE_OVERRIDE;
    virtual void onStop() ESE_NOTHROW ESE_OVERRIDE;
    
  protected:
    EseOsTimerIntfImpl& m_owner;
    int m_id;
    
    ESE_NODEFAULT_CTOR(Impl);
    ESE_NONCOPYABLE(Impl);
  };
  friend class Impl;

protected:
  EseOsTimerIntfImpl(int id, EseOsTimerIntf::WorkerT worker, void* data) ESE_NOTHROW ESE_KEEP;
  virtual ~EseOsTimerIntfImpl() ESE_NOTHROW ESE_KEEP;

public:
  /// EseOsTimerIntf
  ///
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int idGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_impl.idGet(); }
  virtual void* dataGet() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_data; }
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t tmoGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isContinuous() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isRunning() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool start(uint32_t tmo, bool continuous, void* data, EseOsTimerIntf::WorkerT worker) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void reset(uint32_t tmo = rtosMaxDelay) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void resetFromIsr(volatile bool& shouldYield) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void stop() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
protected:
  Impl m_impl;
  WorkerT m_worker;
  void* m_data;

  ESE_NODEFAULT_CTOR(EseOsTimerIntfImpl);
  ESE_NONCOPYABLE(EseOsTimerIntfImpl);
  
  friend class EseOsIntfImpl;
};

#endif //<_ese_os_timer_intf_impl_h_
