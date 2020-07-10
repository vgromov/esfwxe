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
#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/concept/EseLockable.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseTask.h>
#include <esfwxe/cpp/os/EseKernel.h>
//----------------------------------------------------------------------------------------------

#ifndef ESE_SYSTICK_TRACE_CFG
# define ESE_SYSTICK_TRACE_CFG()    ((void)0)
# define ESE_SYSTICK_TRACE_ON()     ((void)0)
# define ESE_SYSTICK_TRACE_OFF()    ((void)0)
#endif
//----------------------------------------------------------------------------------------------

extern "C" {

extern void xPortSysTickHandler(void);

static volatile uint32_t s_tick = 0;

uint32_t HAL_GetTick(void)
{
  return s_tick;
}
//----------------------------------------------------------------------------------------------

// HAL expects tick is expressed in ms

void HAL_IncTick(void)
{
  s_tick += EseKernel::c_msInTick;
}
//----------------------------------------------------------------------------------------------

void HAL_Delay(uint32_t Delay)
{
  EseTask::sleep(Delay);
}
//----------------------------------------------------------------------------------------------

} // extern "C"
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

/// Kernel services implementation
///

// Standard system tick handler implementation
void EseKernel::doSysTick() ESE_NOTHROW
{
  ESE_SYSTICK_TRACE_ON();

  if( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
    xPortSysTickHandler();

  HAL_IncTick();
  
  ESE_SYSTICK_TRACE_OFF();
}
//----------------------------------------------------------------------------------------------

void EseKernel::sysTickInc(esU32 ms) ESE_NOTHROW
{
  s_tick += ms;
#if (0 != configUSE_TICKLESS_IDLE) || \
    (0 != INCLUDE_vTaskStepTick )
  vTaskStepTick( rtosMS_TO_TICKS(ms) );
#endif
}
//----------------------------------------------------------------------------------------------

esU32 EseKernel::sysTickGet() ESE_NOTHROW
{
  return s_tick;
}
//----------------------------------------------------------------------------------------------

void EseKernel::start() ESE_NOTHROW
{
  ESE_SYSTICK_TRACE_CFG();
  
  vTaskStartScheduler();
}
//----------------------------------------------------------------------------------------------

void EseKernel::stop() ESE_NOTHROW
{
  vTaskEndScheduler();
}
//----------------------------------------------------------------------------------------------

void EseKernel::suspend() ESE_NOTHROW
{
  vTaskSuspendAll();
}
//----------------------------------------------------------------------------------------------

void EseKernel::resume(bool doYield /*= true*/) ESE_NOTHROW
{
  if( 
    pdTRUE == xTaskResumeAll() &&
    doYield
  )
  {
    EseTask::yield();
  }
}
//----------------------------------------------------------------------------------------------

void EseKernel::idleSuspend() ESE_NOTHROW
{
  vTaskSuspend(
    xTaskGetIdleTaskHandle()
  );
}
//----------------------------------------------------------------------------------------------

void EseKernel::idleResume() ESE_NOTHROW
{
  vTaskResume(
    xTaskGetIdleTaskHandle()
  );
}
//----------------------------------------------------------------------------------------------

bool EseKernel::isInISR() ESE_NOTHROW
{
  return 0 != __get_IPSR();
}
//----------------------------------------------------------------------------------------------

void EseKernel::sleep(esU32 ms) ESE_NOTHROW
{
  // Fallback to task-sleep
  EseTask::sleep(ms);
}
//----------------------------------------------------------------------------------------------

void EseKernel::sleepUntil(esU32& prevTicks, esU32 ms) ESE_NOTHROW
{
  // Fallback to task-sleep-until
  EseTask::sleepUntil(
    prevTicks,
    ms
  );
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

EseIsrCriticalSection::EseIsrCriticalSection() ESE_NOTHROW :
m_primask(0)
{
  if( EseKernel::isInISR() )
  {
    m_primask = __get_PRIMASK();
    taskENTER_CRITICAL_FROM_ISR();
  }
  else
    taskENTER_CRITICAL();
}
//----------------------------------------------------------------------------------------------

EseIsrCriticalSection::~EseIsrCriticalSection() ESE_NOTHROW
{
  if( EseKernel::isInISR() )
    taskEXIT_CRITICAL_FROM_ISR(m_primask);
  else
    taskEXIT_CRITICAL();
}
//----------------------------------------------------------------------------------------------
