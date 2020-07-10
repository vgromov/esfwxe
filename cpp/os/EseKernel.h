#ifndef _ese_kernel_h_
#define _ese_kernel_h_

namespace EseKernel
{
  const esU32 c_msInTick = 1000/configTICK_RATE_HZ;
  void sysTickInc(esU32 ms) ESE_NOTHROW;

  void doSysTick() ESE_NOTHROW;

  esU32 sysTickGet() ESE_NOTHROW;

  void start() ESE_NOTHROW;
  void stop() ESE_NOTHROW;
  void suspend() ESE_NOTHROW;
  void resume(bool doYield = true) ESE_NOTHROW;

  void idleSuspend() ESE_NOTHROW;
  void idleResume() ESE_NOTHROW;
  
  bool isInISR() ESE_NOTHROW;
  
  /// Temporarily stop execution of the task, by @ms milliseconds
  /// If kernel is not running, use an alternative delay code.
  /// Otherwise, fallback to current task-aware code
  ///
  void sleep(esU32 ms) ESE_NOTHROW;
  
  /// Temporarily stop execution of the task, if any, until @ms milliseconds expire since @prevTicks stamp
  /// If kernel is not running, fallback to an alternative delay until code.
  ///
  void sleepUntil(esU32& prevTicks, esU32 ms) ESE_NOTHROW;
}

/// Locally-scoped ISR masking helper
///
class EseIsrCriticalSection
{
public:
  EseIsrCriticalSection() ESE_NOTHROW;
  ~EseIsrCriticalSection() ESE_NOTHROW;

private:  
  esU32 m_primask;
  
  ESE_NONCOPYABLE(EseIsrCriticalSection);
};

#endif // _ese_kernel_h_
