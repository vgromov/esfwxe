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
#include <esfwxe/cpp/os/EseSemaphore.h>

EseSemaphore::EseSemaphore(size_t maxCount, size_t initialCount) :
m_h(NULL),
m_maxCount(maxCount < 1 ? 1 : maxCount),
m_initialCount(initialCount > maxCount ? maxCount : initialCount)
{
}

EseSemaphore::~EseSemaphore() ESE_NOTHROW
{
  uninit();
}

void EseSemaphore::init() ESE_NOTHROW
{
  if( !m_h )
  {
    if( 1 == m_maxCount )
    {
      m_h = xSemaphoreCreateBinary();
      if( m_initialCount )
        xSemaphoreGive( m_h );
    }
    else
      m_h = xSemaphoreCreateCounting(
        m_maxCount, 
        m_initialCount
      );
  }
}

void EseSemaphore::uninit() ESE_NOTHROW
{
  if( m_h )
  {
    vSemaphoreDelete(m_h);
    m_h = 0;
  }
}
  
rtosStatus EseSemaphore::lock(esU32 tmo /*= rtosMaxDelay*/) ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreTake(m_h, 
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          pdMS_TO_TICKS(tmo)
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  
  return rtosErrorResource;
}

rtosStatus EseSemaphore::unlock() ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreGive(m_h)) ?
      rtosOK :
      rtosAlreadyUnlocked;
  
  return rtosErrorResource;
}

rtosStatus EseSemaphore::lockFromISR(volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h )
  {
    rtosStatus result = rtosOK;
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if(pdTRUE != xSemaphoreTakeFromISR(m_h, &higherPriorityTaskWoken) )
      result = rtosAlreadyLocked;
  
    shouldYield = shouldYield || (pdTRUE == higherPriorityTaskWoken);
    return result;
  }
  
  return rtosErrorResource;
}

rtosStatus EseSemaphore::unlockFromISR(volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h )
  {
    rtosStatus result = rtosOK;
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    if(pdTRUE != xSemaphoreGiveFromISR(m_h, &higherPriorityTaskWoken) )
      result = rtosAlreadyUnlocked;
  
    shouldYield = shouldYield || (pdTRUE == higherPriorityTaskWoken);
    return result;
  }
  
  return rtosErrorResource;
}

