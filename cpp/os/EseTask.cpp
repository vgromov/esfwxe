#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <event_groups.h>
#include <task.h>
#include <timers.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/os/EseTask.h>
#include <esfwxe/cpp/os/EseKernel.h>
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

#if defined(USE_TASK_STACK_HIGH_WATERMARK_TRACE) && \
  1 == INCLUDE_pcTaskGetName && \
  1 == INCLUDE_uxTaskGetStackHighWaterMark
  
# define ES_STACK_HIGH_WATERMARK_TRACE() \
  do { \
    ES_DEBUG_TRACE( "Task '%s' SHWM: %u\n", pcTaskGetName(NULL), (unsigned)uxTaskGetStackHighWaterMark(NULL)); \
  } while(0)
  
# define ES_TASK_START_TRACE() \
  do { \
    ES_DEBUG_TRACE("Task: '%s' started\n", pcTaskGetName(NULL)); \
  } while(0)

#else

# define ES_STACK_HIGH_WATERMARK_TRACE()  ((void)0)
# define ES_TASK_START_TRACE()            ((void)0)

#endif
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

EseTask::Tracker::Tracker(EseTask& task) ESE_NOTHROW : 
m_task(task) 
{
  xEventGroupSetBits(
    (EventGroupHandle_t)m_task.m_nf,
    EseTask::flagRunning
  );
}
//----------------------------------------------------------------------------------------------

EseTask::Tracker::~Tracker() ESE_NOTHROW
{
  EseKernel::suspend();
  
  xEventGroupSetBits(
    (EventGroupHandle_t)m_task.m_nf,
    EseTask::flagStopped
  );

  ES_STACK_HIGH_WATERMARK_TRACE();
  
  m_task.m_h = NULL;
  EseKernel::resume(false);
  // Drop dead here
  vTaskDelete(NULL);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

EseTask::EseTask() ESE_NOTHROW :
m_h(NULL),
m_nf(NULL)
{}
//----------------------------------------------------------------------------------------------

EseTask::~EseTask()
{
  stop();
  
  if( m_nf )
  {
    vEventGroupDelete( (EventGroupHandle_t)m_nf );
    m_nf = NULL;
  }
}
//----------------------------------------------------------------------------------------------

void EseTask::start(esU32 priority, esU32 stackDepth /*= 0*/)
{
  bool alreadyRunning = false;
  
  taskENTER_CRITICAL();
  alreadyRunning = NULL != m_h;
  taskEXIT_CRITICAL();

  if( alreadyRunning )
    EseException::Throw(rtosTaskAlreadyRunning);

  // Lazy-initialize state and mutexes, just in case
  if( !m_nf )
    m_nf = xEventGroupCreate();

  ESE_ASSERT(m_nf);
  xEventGroupClearBits(
    (EventGroupHandle_t)m_nf,
    flagsAll
  );

  if( !stackDepth )
    stackDepth = configMINIMAL_STACK_SIZE;

  if( pdPASS != xTaskCreate( 
      &worker, 
#ifdef DEBUG
      dbgNameGet(),
#else 
      NULL,
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
  
  // Wait until task really starts itself, if scheduler already running
  if( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
    xEventGroupWaitBits(
      (EventGroupHandle_t)m_nf,
      flagRunning,
      pdFALSE,
      pdTRUE,   //< Wait for all bits (in this case - it does not matter)
      portMAX_DELAY
    );
}
//----------------------------------------------------------------------------------------------

// Should be called periodically from task code, to allow graceful task shutdown
bool EseTask::checkForStopping(esU32 tmo /*= 0*/) ESE_NOTHROW
{
  if( !m_nf )
    return true;
  
  EventBits_t bits = xEventGroupWaitBits(
    (EventGroupHandle_t)m_nf,
    flagStopping,
    pdFALSE, //< Do not clear bits on wait success
    pdTRUE,  //< Wait for all bits (in this case - it does not matter)
    (tmo == rtosMaxDelay) ? 
      portMAX_DELAY : 
      pdMS_TO_TICKS(tmo) 
  );
  
  if( ES_BIT_IS_SET(bits, flagStopping) )
    return true;

  return false;
}
//----------------------------------------------------------------------------------------------

bool EseTask::completionWait(uint32_t tmo /*= rtosMaxDelay*/) ESE_NOTHROW
{
  if( !m_nf )
    return true;

  EventBits_t bits = xEventGroupGetBits((EventGroupHandle_t)m_nf);
  if( ES_BIT_IS_SET(bits, flagStopped) )
    return true;

  bits = xEventGroupWaitBits(
    (EventGroupHandle_t)m_nf,
    flagStopped,
    pdFALSE,  //< Do not clear bits on wait success
    pdTRUE,   //< Wait for all bits (in this case - it does not matter)
    (tmo == rtosMaxDelay) ? 
      portMAX_DELAY : 
      pdMS_TO_TICKS(tmo) 
  );
  
  if( ES_BIT_IS_SET(bits, flagStopped) )
    return true;
  
  return false;
}
//----------------------------------------------------------------------------------------------

void EseTask::stop() ESE_NOTHROW
{
  // Signal we want to stop task
  if( m_h )
  {
    ESE_ASSERT( m_nf );
    xEventGroupSetBits( 
      (EventGroupHandle_t)m_nf,
      flagStopping
    );

    // Resume task, just in case 
    resume();

    // Perform optional activity in onStopping handler
    onStopping();
  }

  completionWait();
  
  // Tracker task resources just in case
  if( m_h )
  {
    vTaskDelete((TaskHandle_t)m_h);
    m_h = NULL;
  }
}
//----------------------------------------------------------------------------------------------

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
//----------------------------------------------------------------------------------------------

esU32 EseTask::priorityGet() const ESE_NOTHROW
{
  if( m_h )
    return uxTaskPriorityGet( (TaskHandle_t)m_h );

  return tskIDLE_PRIORITY;
}
//----------------------------------------------------------------------------------------------

void EseTask::prioritySet( esU32 prio ) ESE_NOTHROW
{
  if( m_h )
    vTaskPrioritySet( (TaskHandle_t)m_h, prio );
}
//----------------------------------------------------------------------------------------------

void EseTask::prioritySetThis( esU32 prio ) ESE_NOTHROW
{
  vTaskPrioritySet(NULL, prio);
}
//----------------------------------------------------------------------------------------------

esU32 EseTask::priorityGetThis() ESE_NOTHROW
{
  return uxTaskPriorityGet(NULL);
}
//----------------------------------------------------------------------------------------------

void EseTask::suspend() ESE_NOTHROW
{
  if( m_h )
  {
    onBeforeSuspend();
    vTaskSuspend( (TaskHandle_t)m_h );
    onAfterSuspend();
  }
}
//----------------------------------------------------------------------------------------------

void EseTask::resume() ESE_NOTHROW
{
  if( 0 != __get_IPSR() )
  {
    if( 
      m_h && 
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
//----------------------------------------------------------------------------------------------

void EseTask::suspendThis() ESE_NOTHROW
{
  vTaskSuspend(NULL);
}
//----------------------------------------------------------------------------------------------

extern "C" void esNonOsIneffectiveWaitFor(esU32 ms); //< Platform-specific code to be implemented elsewhere, maybe in retarget
extern "C" void esNonOsEffectiveWaitFor(esU32 ms); //< Platform-specific code to be implemented elsewhere, maybe in retarget

void EseTask::sleep(esU32 ms) ESE_NOTHROW
{
  if( EseKernel::isInISR() )
    esNonOsIneffectiveWaitFor(ms);
  else if( taskSCHEDULER_RUNNING != xTaskGetSchedulerState() )
    esNonOsEffectiveWaitFor(ms);
  else
    vTaskDelay(
      (ms == rtosMaxDelay) ? 
        portMAX_DELAY : 
        pdMS_TO_TICKS(ms)
    );
}
//----------------------------------------------------------------------------------------------

void EseTask::sleepUntil(esU32& prevTicks, esU32 ms) ESE_NOTHROW
{
  ES_ASSERT(ms < rtosMaxDelay);
  TickType_t previousWakeTime = prevTicks;

  if( EseKernel::isInISR() )
  {
    esNonOsIneffectiveWaitFor(ms);
    previousWakeTime += pdMS_TO_TICKS(ms);
  }
  else if( taskSCHEDULER_RUNNING != xTaskGetSchedulerState() )
  {
    esNonOsEffectiveWaitFor(ms);
    previousWakeTime += pdMS_TO_TICKS(ms);
  }
  else
    vTaskDelayUntil( 
      &previousWakeTime,
      pdMS_TO_TICKS(ms) 
    );

  prevTicks = previousWakeTime;
}
//----------------------------------------------------------------------------------------------

void EseTask::yield() ESE_NOTHROW
{
  if( !EseKernel::isInISR() )
  {
    portYIELD();
  }
}
//----------------------------------------------------------------------------------------------

void EseTask::worker( void* params ) ESE_NOTHROW
{
  EseTask* task = reinterpret_cast<EseTask*>(params);
  ES_ASSERT(task);

  {
  Tracker track(*task);

  ES_TASK_START_TRACE();

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
    task->m_ex = EseException(
      0, 
      EseException::facilityUnknown
    );
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
      task->m_ex = EseException(
        0, 
        EseException::facilityUnknown
      );
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
    task->m_ex = EseException(
      0, 
      EseException::facilityUnknown
    );
  }
#else
  task->onExit();
#endif

  }//< Scoped Tracker dtor will be fired here
}
//----------------------------------------------------------------------------------------------
