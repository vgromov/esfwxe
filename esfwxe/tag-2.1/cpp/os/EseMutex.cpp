#include "stdafx.h"
#pragma hdrstop

#include "EseMutex.h"

//-------------------------------------------------
// Non-recursive mutex implementation
//
EseMutex::EseMutex() ESE_NOTHROW :
m_h(NULL)
{
}

EseMutex::~EseMutex() ESE_NOTHROW
{
  uninit();
}

void EseMutex::init() ESE_NOTHROW
{
  if( !m_h )
    m_h = xSemaphoreCreateMutex();
}

void EseMutex::uninit() ESE_NOTHROW
{
  if( m_h )
  {
    vSemaphoreDelete(m_h);
    m_h = 0;
  }
}
  
rtosStatus EseMutex::lock(uint32_t tmo /*= rtosMaxDelay*/) ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreTake(
        m_h, 
        (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          pdMS_TO_TICKS(tmo)
      )) ?
        rtosOK : 
        rtosErrorTimeout;
  
  return rtosErrorResource;
}

rtosStatus EseMutex::unlock() ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreGive(m_h)) ?
      rtosOK :
      rtosAlreadyUnlocked;
  
  return rtosErrorResource;
}

rtosStatus EseMutex::lockFromISR(volatile bool& shouldYield) ESE_NOTHROW
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

rtosStatus EseMutex::unlockFromISR(volatile bool& shouldYield) ESE_NOTHROW
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

//-------------------------------------------------
// Recursive mutex implementation
//
#if 1 == configUSE_RECURSIVE_MUTEXES

EseMutexRecursive::EseMutexRecursive() ESE_NOTHROW :
m_h(0)
{
}

EseMutexRecursive::~EseMutexRecursive() ESE_NOTHROW
{
  uninit();
}

void EseMutexRecursive::init() ESE_NOTHROW
{
  if( !m_h )
    m_h = xSemaphoreCreateRecursiveMutex();
}

void EseMutexRecursive::uninit() ESE_NOTHROW
{
  if( m_h )
  {
    vSemaphoreDelete(m_h);
    m_h = 0;
  }
}
  
rtosStatus EseMutexRecursive::lock(uint32_t tmo /*= rtosMaxDelay*/) ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreTakeRecursive(m_h,
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          pdMS_TO_TICKS(tmo)
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  
  return rtosErrorResource;
}

rtosStatus EseMutexRecursive::unlock() ESE_NOTHROW
{
  if( m_h )
    return (pdTRUE == xSemaphoreGiveRecursive(m_h)) ?
      rtosOK :
      rtosAlreadyUnlocked;
  
  return rtosErrorResource;
}

#endif // #if 1 == configUSE_RECURSIVE_MUTEXES
