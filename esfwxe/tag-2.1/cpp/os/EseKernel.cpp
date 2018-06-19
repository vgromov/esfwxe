#include "stdafx.h"
#pragma hdrstop

#include "EseKernel.h"

#if defined(USE_SYSTICK_TRACE)
# define ESE_SYSTICK_TRACE_CFG  DEBUG_TRACE_PIN_CONFIG(C,11)
# define ESE_SYSTICK_TRACE_ON   DEBUG_TRACE_PIN_ON(C,11)
# define ESE_SYSTICK_TRACE_OFF  DEBUG_TRACE_PIN_OFF(C,11)
#else
# define ESE_SYSTICK_TRACE_CFG
# define ESE_SYSTICK_TRACE_ON
# define ESE_SYSTICK_TRACE_OFF
#endif

extern "C" {

extern void xPortSysTickHandler(void);

static volatile uint32_t s_tick = 0;

#pragma Otime
uint32_t HAL_GetTick(void)
{
  return s_tick;
}

// HAL expects tick is expressed in ms
#pragma Otime
void HAL_IncTick(void)
{
  s_tick += EseKernel::c_msInTick;
}

// Standar system tick handler implementation
#pragma Otime
void xPortSysTickHandler(void)
{
  ESE_SYSTICK_TRACE_ON

  if( taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
  {
    vPortRaiseBASEPRI();
    {
      /* Increment the RTOS tick. */
      if( xTaskIncrementTick() != pdFALSE )
      {
        /* A context switch is required.  Context switching is performed in
        the PendSV interrupt.  Pend the PendSV interrupt. */
        portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
      }
    }
    vPortClearBASEPRIFromISR();  
  }
  HAL_IncTick();
  
  ESE_SYSTICK_TRACE_OFF
}

#pragma Otime
void HAL_Delay(uint32_t Delay)
{
  if( 0 == __get_IPSR() && taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
  {
    EseTask::sleep(Delay);
  }
  else
  {
    uint32_t tickstart = s_tick;
    while((s_tick - tickstart) < Delay)
    {
#ifndef JTAG_DEBUG
      HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
#endif
    }
  }
}

} // extern "C"

/// Kernel services implementation
///
#pragma Otime
void EseKernel::sysTickInc(esU32 ms) ESE_NOTHROW
{
  s_tick += ms;
#if (0 != configUSE_TICKLESS_IDLE) || \
    (0 != INCLUDE_vTaskStepTick )
  vTaskStepTick( rtosMS_TO_TICKS(ms) );
#endif
}

#pragma Otime
void EseKernel::start() ESE_NOTHROW
{
  ESE_SYSTICK_TRACE_CFG
  
  vTaskStartScheduler();
}

#pragma Otime
void EseKernel::stop() ESE_NOTHROW
{
  vTaskEndScheduler();
}

#pragma Otime
void EseKernel::suspend() ESE_NOTHROW
{
  vTaskSuspendAll();
}

#pragma Otime
void EseKernel::resume() ESE_NOTHROW
{
  if( pdTRUE == xTaskResumeAll() )
  {
    EseTask::yield();
  }
}

#pragma Otime
void EseKernel::idleSuspend() ESE_NOTHROW
{
  vTaskSuspend(
    xTaskGetIdleTaskHandle()
  );
}

#pragma Otime
void EseKernel::idleResume() ESE_NOTHROW
{
  vTaskResume(
    xTaskGetIdleTaskHandle()
  );
}
