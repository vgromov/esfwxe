#ifndef _ese_os_intf_h_
#define _ese_os_intf_h_

#include "EseOsTimerIntf.h"
#include "EseOsTaskIntf.h"

// Forward decls
class EseOsCriticalSectionIntf;
class EseOsQueueIntf;

/// OS interface
///
class ESE_ABSTRACT EseOsIntf
{
public:
  /// Kernel services
  ///
  virtual void suspend() const ESE_NOTHROW = 0;
  virtual void resume() const ESE_NOTHROW = 0;

  /// SysTick access
  ///
  virtual uint32_t sysTickGet() const ESE_NOTHROW = 0;
  virtual void sysTickInc(uint32_t ms) const ESE_NOTHROW = 0;

  /// OS Idle task control
  virtual void idleSuspend() const ESE_NOTHROW = 0;
  virtual void idleResume() const ESE_NOTHROW = 0;
  
  /// OS Timer daemon control
  virtual void timerDaemonSuspend() const ESE_NOTHROW = 0;
  virtual void timerDaemonResume() const ESE_NOTHROW = 0;
  
  /// Task ticks access
  virtual uint32_t taskTicksGet() const ESE_NOTHROW = 0;
  
  /// Sleep delay services
  virtual void sleep(uint32_t tmo) const ESE_NOTHROW = 0;
  virtual void sleepUntil(uint32_t& prevTicks, uint32_t tmo) const ESE_NOTHROW = 0;

  /// Critical section factory
  virtual EseOsCriticalSectionIntf* criticalSectionCreate() const ESE_NOTHROW = 0;
  
  /// Queue factory
  virtual EseOsQueueIntf* queueCreate(size_t length, size_t eltSize) const ESE_NOTHROW = 0;
  
  /// OS Timer factory
  virtual EseOsTimerIntf* timerCreate(int id, EseOsTimerIntf::WorkerT worker = nullptr, void* data = nullptr) const ESE_NOTHROW = 0;

  /// OS Task factory
  virtual EseOsTaskIntf* taskCreate(int id, EseOsTaskIntf::WorkerT worker = nullptr, EseOsTaskIntf::StopCheckT stopCheck = nullptr, void* data = nullptr) const ESE_NOTHROW = 0;
};

#endif //< _ese_os_intf_h_
