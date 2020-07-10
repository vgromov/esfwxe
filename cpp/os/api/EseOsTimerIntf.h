#ifndef _ese_os_timer_intf_h_
#define _ese_os_timer_intf_h_

/// OS timer interface
///
class ESE_ABSTRACT EseOsTimerIntf
{
public:
  enum Stage
  {
    stageStarted,
    stageExpired,
    stageStopped
  };

  // Staged timer worker
  typedef void (*WorkerT)(EseOsTimerIntf& intf, void* data, Stage stage);

public:
  // Timer deallocator
  virtual void destroy() ESE_NOTHROW = 0;
  
  /// Return timer ID, assigned to it upon creation, see EseOsIntf factory
  virtual int idGet() const ESE_NOTHROW = 0;
  
  /// Return currently assigned interface instance data
  virtual void* dataGet() ESE_NOTHROW = 0;
  
  /// Return true, if timer object is created OK
  virtual bool isOk() const ESE_NOTHROW = 0;

  /// Retrieve current timer timeout
  virtual uint32_t tmoGet() const ESE_NOTHROW = 0;
  
  /// Return true if timer is configured as continuous
  virtual bool isContinuous() const ESE_NOTHROW = 0;

  /// Return true, if timer is currently running
  virtual bool isRunning() const ESE_NOTHROW = 0;

  /// Start timer with specified timeout in ms, and single|continuous mode
  /// worker callback will be fired upon timer start, stop, and expiration stages
  ///
  virtual bool start(uint32_t tmo, bool continuous, void* data = NULL, EseOsTimerIntf::WorkerT worker = NULL) ESE_NOTHROW = 0;
  
  /// (Try to) reset timer counter to 0
  virtual void reset(uint32_t tmo = rtosMaxDelay) ESE_NOTHROW = 0;
  
  /// Reset timer counter to 0 from ISR
  virtual void resetFromIsr(volatile bool& shouldYield) ESE_NOTHROW = 0;

  /// Stop timer execution.
  virtual void stop() ESE_NOTHROW = 0;
};

#endif //< _ese_os_timer_intf_h_
