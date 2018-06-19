#ifndef _ese_kernel_h_
#define _ese_kernel_h_

namespace EseKernel
{
  const esU32 c_msInTick = 1000/configTICK_RATE_HZ;
  void sysTickInc(esU32 ms) ESE_NOTHROW;

  void start() ESE_NOTHROW;
  void stop() ESE_NOTHROW;
  void suspend() ESE_NOTHROW;
  void resume() ESE_NOTHROW;

  void idleSuspend() ESE_NOTHROW;
  void idleResume() ESE_NOTHROW;
}


/// Locally-scoped ISR masking helper
///
class EseIsrCriticalSection
{
public:
  inline EseIsrCriticalSection() ESE_NOTHROW :
  m_primask(__get_PRIMASK())
  {
    __disable_irq();
  }
  
  inline ~EseIsrCriticalSection() ESE_NOTHROW
  {
    if( !m_primask )
      __enable_irq();
  }

private:  
  esU32 m_primask;
  
  EseIsrCriticalSection(const EseIsrCriticalSection&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseIsrCriticalSection& operator=(const EseIsrCriticalSection&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // _ese_kernel_h_
