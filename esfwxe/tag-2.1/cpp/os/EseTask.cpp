#include "stdafx.h"
#pragma hdrstop

#include "EseTask.h"

#if defined(USE_TASK_STACK_HIGH_WATERMARK_TRACE) && \
  1 == INCLUDE_pcTaskGetName && \
  1 == INCLUDE_uxTaskGetStackHighWaterMark
  
# define ES_STACK_HIGH_WATERMARK_TRACE \
  ES_DEBUG_TRACE2( "Task '%s' SHWM: %u\n", pcTaskGetName(NULL), uxTaskGetStackHighWaterMark(NULL))
  
# define ES_TASK_START_TRACE \
  ES_DEBUG_TRACE1("Task: '%s' started\n", pcTaskGetName(NULL))
  
#else

# define ES_STACK_HIGH_WATERMARK_TRACE
# define ES_TASK_START_TRACE

#endif

EseTask::EseTask() ESE_NOTHROW :
m_h(NULL)
{}

EseTask::~EseTask()
{
  stop();
}

void EseTask::start(esU32 priority, esU32 stackDepth /*= 0*/)
{
  if( m_h )
    EseException::Throw(rtosTaskAlreadyRunning);

  // initialize mutexes, just in case
  m_mx.init();
  m_mxStopping.init();

  if( !stackDepth )
    stackDepth = configMINIMAL_STACK_SIZE;

  if( pdPASS != xTaskCreate( 
      &worker, 
#ifdef DEBUG
      dbgNameGet(),
#else 
      0,
#endif
      stackDepth, 
      this, 
      priority, 
      (TaskHandle_t*)&m_h
    ) 
  )
  {
    EseException::Throw(rtosTaskNotCreated);
  }
}

// Should be called periodically from task code, to allow graceful task shutdown
#pragma Otime
bool EseTask::checkForStopping(esU32 tmo /*= 0*/) ESE_NOTHROW
{
  if( rtosOK == m_mxStopping.lock(tmo) )
  {
    m_mxStopping.unlock();
    return false;
  }
  else
    return true;
}

void EseTask::stop() ESE_NOTHROW
{
  // Signal we want to stop task
  m_mxStopping.lock();

  // Resume task, just in case 
  resume();

  // Perform optional activity in onStopping handler
  onStopping();

  // Wait for task worker to unlock mutex
  if(rtosOK == m_mx.lock())
    m_mx.unlock();
  
  // Cleanup task resources
  if( m_h )
  {
    vTaskDelete((TaskHandle_t)m_h);
    m_h = 0;
  }

  // unlock stopping flag just in case 
  m_mxStopping.unlock();
}

#pragma Otime
esU32 EseTask::tickCountGet() ESE_NOTHROW
{
  if( 0 != __get_IPSR() ) 
  {
    return xTaskGetTickCountFromISR();
  }
  else
  {
    return xTaskGetTickCount();
  }
}

#pragma Otime
esU32 EseTask::priorityGet() const ESE_NOTHROW
{
  if( m_h )
    return uxTaskPriorityGet( (TaskHandle_t)m_h );

  return tskIDLE_PRIORITY;
}

#pragma Otime
void EseTask::prioritySet( esU32 prio ) ESE_NOTHROW
{
  if( m_h )
    vTaskPrioritySet( (TaskHandle_t)m_h, prio );
}

#pragma Otime
void EseTask::prioritySetThis( esU32 prio ) ESE_NOTHROW
{
  vTaskPrioritySet(0, prio);
}

#pragma Otime
esU32 EseTask::priorityGetThis() ESE_NOTHROW
{
  return uxTaskPriorityGet(0);
}

#pragma Otime
void EseTask::suspend() ESE_NOTHROW
{
  if( m_h )
  {
    vTaskSuspend( (TaskHandle_t)m_h );
    onAfterSuspend();
  }
}

#pragma Otime
void EseTask::resume() ESE_NOTHROW
{
  if( 0 != __get_IPSR() )
  {
    if( m_h && 
        pdTRUE == xTaskResumeFromISR( (TaskHandle_t)m_h ) 
    )
    {
      onBeforeResumeFromIsr();
      portYIELD_FROM_ISR(1);
    }
  }
  else if( m_h )
  {
    onBeforeResume();
    vTaskResume( (TaskHandle_t)m_h );
  }
}

#pragma Otime
void EseTask::suspendThis() ESE_NOTHROW
{
  vTaskSuspend(0);
}

#pragma Otime
void EseTask::sleep(esU32 ms) ESE_NOTHROW
{
  vTaskDelay(
    (ms == rtosMaxDelay) ? 
      portMAX_DELAY : 
      pdMS_TO_TICKS(ms)
  );
}

#pragma Otime
void EseTask::sleepUntil(esU32& prevTicks, esU32 ms) ESE_NOTHROW
{
  ES_ASSERT(ms < rtosMaxDelay);
  TickType_t previousWakeTime = prevTicks;

  vTaskDelayUntil( 
    &previousWakeTime,
    pdMS_TO_TICKS(ms) 
  );

  prevTicks = previousWakeTime;
}

#pragma Otime
void EseTask::yield() ESE_NOTHROW
{
  if( 0 == __get_IPSR() )
  {
    portYIELD();
  }
}

#pragma Otime
void EseTask::worker( void* params ) ESE_NOTHROW
{
  EseTask* task = reinterpret_cast<EseTask*>(params);
  ES_ASSERT(task);

  {
    // Scoped task execution lock ASAP
    EseLocker<EseMutex> lock(task->m_mx);
    
    ES_TASK_START_TRACE

#ifdef USE_CPP_EXCEPTIONS
    try
    {
      task->onEnter();
    }
    catch(const EseException& ex)
    {
      task->m_ex = ex;
    }
    catch(...)
    {
      task->m_ex = EseException(0, EseException::facilityUnknown);
    }
#else
    task->onEnter();
#endif    
  
    if( !task->m_ex.isOk() ) ///< No errors in onEnter handler, proceed to execution block
    {
#ifdef USE_CPP_EXCEPTIONS
      try
      {
        task->execute();
      }
      catch(const EseException& ex)
      {
        task->m_ex = ex;
      }
      catch(...)
      {
        task->m_ex = EseException(0, EseException::facilityUnknown);
      }
#else
      task->execute();
#endif      
    }
    
    // onExit handler is executed unconditionally, even if onEnter was not fully succeeded
#ifdef USE_CPP_EXCEPTIONS
    try
    {
      task->onExit();
    }
    catch(const EseException& ex)
    {
      task->m_ex = ex;
    }
    catch(...)
    {
      task->m_ex = EseException(0, EseException::facilityUnknown);
    }
#else
    task->onExit();
#endif

    // Leverage priority to the maximum just before task exit,
    // to perform handle nullification and task deletion without|with as little context switch as possible 
    //
    vTaskPrioritySet(0, taskMaxPriority);

    task->m_h = 0;
    // Execution lock is unlocked here automatically
  }

  // Optionally trace stack high watermark
  ES_STACK_HIGH_WATERMARK_TRACE

  // Delete self. Nothing would be executed after call to vTaskDelete
  // so nullify task handle prior to the call
  vTaskDelete(NULL);
}
