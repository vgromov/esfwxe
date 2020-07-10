#ifndef _ese_os_intf_impl_h_
#define _ese_os_intf_impl_h_

/// EseOsIntf implementation
///
class EseOsIntfImpl : public EseOsIntf
{
private:
  EseOsIntfImpl() ESE_NOTHROW ESE_KEEP {}
  
public:
  static EseOsIntf& instanceGet() ESE_NOTHROW ESE_KEEP;

  /// EseOsIntf
  ///
  virtual uint32_t sysTickGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void sysTickInc(uint32_t ms) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void suspend() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void resume() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void idleSuspend() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void idleResume() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void timerDaemonSuspend() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void timerDaemonResume() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual uint32_t taskTicksGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void sleep(uint32_t tmo) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void sleepUntil(uint32_t& prevTicks, uint32_t tmo) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  // Factories
  virtual EseOsCriticalSectionIntf* criticalSectionCreate() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual EseOsQueueIntf* queueCreate(size_t length, size_t eltSize) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual EseOsTimerIntf* timerCreate(int id, EseOsTimerIntf::WorkerT worker, void* data) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual EseOsTaskIntf* taskCreate(int id, EseOsTaskIntf::WorkerT worker, EseOsTaskIntf::StopCheckT stopCheck, void* data) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  ESE_NONCOPYABLE(EseOsIntfImpl);
};

#endif //< _ese_os_intf_impl_h_
