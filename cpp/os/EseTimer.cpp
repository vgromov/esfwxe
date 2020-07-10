#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/os/EseTimer.h>

#ifdef configUSE_TIMERS

#if defined(USE_TIMER_TASK_STACK_HIGH_WATERMARK_TRACE) && \
  defined(USE_TASK_STACK_HIGH_WATERMARK_TRACE) && \
  1 == INCLUDE_pcTaskGetName && \
  1 == INCLUDE_uxTaskGetStackHighWaterMark

# define ES_TIMER_TASK_SHWM ES_DEBUG_TRACE1( "Timer task SHWM: %u\n", uxTaskGetStackHighWaterMark(0))
#else
# define ES_TIMER_TASK_SHWM
#endif

EseTimer::EseTimer(const char* name /*= 0*/) ESE_NOTHROW :
m_h(NULL),
m_name(name),
m_tmo(0),
m_continuous(false)
{
}

EseTimer::~EseTimer() ESE_NOTHROW
{
  stop();
  cleanup();
}

void EseTimer::cleanup() ESE_NOTHROW
{
  if( m_h )
  {
    xTimerDelete(m_h, portMAX_DELAY);
    m_h = NULL;
  }
}

void EseTimer::create() ESE_NOTHROW
{
  if( !m_h )
  {
    m_h = xTimerCreate( 
            m_name,
            pdMS_TO_TICKS(m_tmo),
            m_continuous ? pdTRUE : pdFALSE,
            reinterpret_cast<void*>(this),
            &EseTimer::timerCallback
          );
  }
}

bool EseTimer::isRunning() const ESE_NOTHROW
{
  if( m_h )
  {
    return pdTRUE == xTimerIsTimerActive( m_h );
  }

  return false;
}

bool EseTimer::start(esU32 tmo, bool continuous) ESE_NOTHROW
{
  stop();

  if( continuous != m_continuous )
  {
    cleanup();
    m_continuous = continuous;
  }

  if( tmo != m_tmo )
  {
    m_tmo = tmo;
    if( m_h )
      xTimerChangePeriod( m_h, pdMS_TO_TICKS(tmo), portMAX_DELAY );
  }

  if( !m_h )
    create();
  
  if( m_h )
  {
    onStart();
    return pdPASS == xTimerStart(m_h, portMAX_DELAY);
  }

  return false;
}

void EseTimer::reset(esU32 tmo) ESE_NOTHROW
{
  if( m_h )
  {
    xTimerReset(m_h, (rtosMaxDelay == tmo) ? portMAX_DELAY : tmo);
  }
}

void EseTimer::stop() ESE_NOTHROW
{
  if( m_h )
  {
    xTimerStop(m_h, portMAX_DELAY);
    
    onStop();
  }
}

bool EseTimer::startFromISR(esU32 tmo, volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h )
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t result = xTimerStartFromISR( m_h, &xHigherPriorityTaskWoken );
    shouldYield = (shouldYield || pdTRUE == xHigherPriorityTaskWoken);
    return pdPASS == result;
  }

  return false;
}

void EseTimer::resetFromISR(volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h )
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTimerResetFromISR( m_h, &xHigherPriorityTaskWoken );
    shouldYield = (shouldYield || pdTRUE == xHigherPriorityTaskWoken);
  }
}

void EseTimer::stopFromISR(volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h )
  {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTimerStopFromISR( m_h, &xHigherPriorityTaskWoken );
    shouldYield = (shouldYield || pdTRUE == xHigherPriorityTaskWoken);
  }
}

void EseTimer::timerCallback( void* htimer ) ESE_NOTHROW
{
  ES_ASSERT(htimer);
  void* id = pvTimerGetTimerID(htimer);
  ES_ASSERT(id);
  
  ES_TIMER_TASK_SHWM
  
  EseTimer* This = reinterpret_cast<EseTimer*>(id);
  This->onExpire();  
}

void EseTimer::daemonSuspend() ESE_NOTHROW
{
  vTaskSuspend(
    xTimerGetTimerDaemonTaskHandle()
  );
}

void EseTimer::daemonResume() ESE_NOTHROW
{
  vTaskResume(
    xTimerGetTimerDaemonTaskHandle()
  );
}

#endif // configUSE_TIMERS
