#ifndef _ese_timer_h_
#define _ese_timer_h_

#ifdef configUSE_TIMERS

/// OS software timer base class
///
class EseTimer
{
public:
  EseTimer(const char* name = NULL) ESE_NOTHROW;
  virtual ~EseTimer() ESE_NOTHROW;
  
  /// Return true, if timer object is created OK
  bool isOk() const ESE_NOTHROW { return NULL != m_h; }

  /// Retrieve current timer timeout
  inline esU32 tmoGet() const ESE_NOTHROW { return m_tmo; }
  
  /// Return true if timer is configured as continuous
  bool isContinuous() const ESE_NOTHROW { return m_continuous; }

  /// Return true, if timer is currently running
  bool isRunning() const ESE_NOTHROW;

  /// Start timer with specified timeout in ms, and single|continuous mode
  /// Timer instance is created internally, if needed
  ///
  bool start(esU32 tmo, bool continuous) ESE_NOTHROW;

  /// Reset timer counter to 0
  void reset(esU32 tmo = portMAX_DELAY) ESE_NOTHROW;

  /// Stop timer execution.
  void stop() ESE_NOTHROW;

  /// ISR services
  ///
  /// Start timer with specified timeout in ms, and single|continuous mode
  /// Timer instance is created internally, if needed
  ///
  bool startFromISR(esU32 tmo, volatile bool& shouldYield) ESE_NOTHROW;

  /// Reset timer counter to 0
  void resetFromISR(volatile bool& shouldYield) ESE_NOTHROW;

  /// Stop timer execution.
  void stopFromISR(volatile bool& shouldYield) ESE_NOTHROW;
  
  /// Timer daemon control
  static void daemonSuspend() ESE_NOTHROW;
  static void daemonResume() ESE_NOTHROW;

private:
  void create() ESE_NOTHROW;
  void cleanup() ESE_NOTHROW;

  static void timerCallback( void* htimer ) ESE_NOTHROW;

protected:
  /// Timer expiration handler. To be implemented in derived classes
  virtual void onExpire() ESE_NOTHROW = 0;
  virtual void onStart() ESE_NOTHROW {}

protected:
  void* m_h;
  const char* m_name;
  esU32 m_tmo;
  bool m_continuous;

private:
  EseTimer(const EseTimer&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseTimer& operator=(const EseTimer&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // configUSE_TIMERS

#endif // _ese_timer_h_
