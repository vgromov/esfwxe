#include "stdafx.h"
#pragma hdrstop

#include "EseQueue.h"

EseQueueBase::EseQueueBase() ESE_NOTHROW :
m_h(NULL)
{}

EseQueueBase::~EseQueueBase() ESE_NOTHROW
{
  uninit();
}
  
size_t EseQueueBase::countGet() const ESE_NOTHROW
{
  if( m_h )
  {
    if(0 != __get_IPSR())
      return uxQueueMessagesWaitingFromISR(m_h);
    else
      return uxQueueMessagesWaiting(m_h);
  }
  
  return 0;
}
  
size_t EseQueueBase::freeCountGet() const ESE_NOTHROW
{
  if( m_h )
  {
    if(0 == __get_IPSR())
      return uxQueueSpacesAvailable(m_h);
  }
  
  return 0;
}
  
void EseQueueBase::reset() ESE_NOTHROW
{
  if( m_h )
    xQueueReset(m_h);
}

void EseQueueBase::uninit() ESE_NOTHROW
{
  if( m_h )
  {
    vQueueDelete(m_h);
    m_h = 0;
  }
}

rtosStatus EseQueueBase::pushBack(const void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW
{
  if( m_h && item )
  {
    return (pdTRUE == xQueueSendToBack(m_h,
          item,
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          (useTicks ? 
            tmo : 
            pdMS_TO_TICKS(tmo)
          )
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  }
  
  return rtosErrorResource;
}
  
rtosStatus EseQueueBase::pushFront(const void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW
{
  if( m_h && item )
  {
    return (pdTRUE == xQueueSendToFront(m_h,
          item,
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          (useTicks ?
            tmo :
            pdMS_TO_TICKS(tmo)
          )
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  }
  
  return rtosErrorResource;
}

rtosStatus EseQueueBase::popFront(void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW
{
  if( m_h && item )
  {
    return (pdTRUE == xQueueReceive(m_h,
          item,
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          (useTicks ? 
            tmo :
            pdMS_TO_TICKS(tmo)
          )
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  }
  
  return rtosErrorResource;
}
  
rtosStatus EseQueueBase::peekFront(void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW
{
  if( m_h && item )
  {
    return (pdTRUE == xQueuePeek(m_h,
          item,
          (tmo == rtosMaxDelay) ? 
          portMAX_DELAY : 
          (useTicks ?
            tmo :
            pdMS_TO_TICKS(tmo)
          )
        )
      ) ?
      rtosOK : 
      rtosErrorTimeout;
  }
  
  return rtosErrorResource;
}

rtosStatus EseQueueBase::pushBackFromISR(const void* item, volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h && item )
  {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    rtosStatus result = rtosOK;
    if( errQUEUE_FULL == xQueueSendToBackFromISR(m_h, item, &higherPriorityTaskWoken) )
      result = rtosErrorQueueFull;
    shouldYield = shouldYield || (pdTRUE == higherPriorityTaskWoken);
    
    return result;
  }
  
  return rtosErrorResource;
}
  

rtosStatus EseQueueBase::pushFrontFromISR(const void* item, volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h && item )
  {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    rtosStatus result = rtosOK;
    if( errQUEUE_FULL == xQueueSendToFrontFromISR(m_h, item, &higherPriorityTaskWoken) )
      result = rtosErrorQueueFull;
    shouldYield = shouldYield || (pdTRUE == higherPriorityTaskWoken);
    
    return result;
  }
  
  return rtosErrorResource;
}
  
rtosStatus EseQueueBase::popFrontFromISR(void* item, volatile bool& shouldYield) ESE_NOTHROW
{
  if( m_h && item )
  {
    BaseType_t higherPriorityTaskWoken = pdFALSE;
    rtosStatus result = rtosOK;
    if( pdFALSE == xQueueReceiveFromISR(m_h, item, &higherPriorityTaskWoken) )
      result = rtosErrorQueueEmpty;
    shouldYield = shouldYield || (pdTRUE == higherPriorityTaskWoken);
    
    return result;
  }
  
  return rtosErrorResource;
}
  
rtosStatus EseQueueBase::peekFrontFromISR(void* item) ESE_NOTHROW
{
  if( m_h && item )
  {
    return (pdTRUE == xQueuePeekFromISR(m_h, item)) ?
      rtosOK :
      rtosErrorQueueEmpty;
  }
  
  return rtosErrorResource;
}
  
/// Initialize queue container
void EseQueueBase::init(size_t len, size_t itemSize) ESE_NOTHROW
{
  if( !m_h )
    m_h = xQueueCreate(len, itemSize);
}
