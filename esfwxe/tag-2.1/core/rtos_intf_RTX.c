#include <esfwxe/target.h>
#include <esfwxe/core/rtos_intf.h>
#pragma hdrstop

// rtos_intf implementation for freertos
//

// rtx rtos stuff
#include <RTL.h>
#include <rtxConfig.h>
#define RTX_MAX_DELAY ((esU16)0xFFFF)

/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

/*--------------------------- os_idle_demon ---------------------------------*/

__task void os_idle_demon (void) {
  /* The idle demon is a system task, running when no other task is ready */
  /* to run. The 'os_xxx' function calls are not allowed from this task.  */

  for (;;) {
  /* HERE: include optional user code to be executed when no task runs.*/
  }
}


/*--------------------------- os_tmr_call -----------------------------------*/

void os_tmr_call (U16 info) {
  /* This function is called when the user timer has expired. Parameter  */
  /* 'info' holds the value, defined when the timer was created.         */

  /* HERE: include optional user code to be executed on timeout. */
}


/*--------------------------- os_error --------------------------------------*/

void os_error (U32 err_code) {
  /* This function is called when a runtime error is detected. Parameter */
  /* 'err_code' holds the runtime error code (defined in RTL.H).         */

  /* HERE: include optional code to be executed on runtime error. */
  for (;;);
}


/*----------------------------------------------------------------------------
 *      RTX Configuration Functions
 *---------------------------------------------------------------------------*/

static void os_def_interrupt (void) __irq  {
  /* Default Interrupt Function: may be called when timer ISR is disabled */
  OS_IACK();
}


#include <RTX_lib.c>

esU32 rtosMsToTimestamp( esU32 ms )
{
	if( ms )
	{
		ms = (ms == rtosMAX_DELAY) ? RTX_MAX_DELAY : (portTickType)ms / portTICK_RATE_MS );

		if(!ms)
			return 1;

		return ms;
	}
	
	return 0;
}

esU32 rtosTimestampToMs( esU32 ts )
{
	return (portTickType)(ts) * portTICK_RATE_MS;
}

esU32 rtosMAX_PRIORITY(void)	
{ 
	return (configMAX_PRIORITIES-1); 
}

// task services
//
rtosTASK_HANDLE rtosCurrentTaskGet(void)
{
	return xTaskGetCurrentTaskHandle();
}

rtosTASK_HANDLE rtosTaskCreate(rtosTASK_FUNCTION worker, ES_ASCII_CSTR name, esU32 stackDepth, void* params, esU32 priority)
{
	rtosTASK_HANDLE result = 0;
	xTaskCreate(worker, (const signed char*)name, stackDepth, params, priority, &result);
	return result;
}

#if 1 == INCLUDE_vTaskSuspend
rtosTASK_HANDLE rtosTaskCreateSuspended(rtosTASK_FUNCTION worker, ES_ASCII_CSTR name, esU32 stackDepth, void* params, esU32 priority)
{
	rtosTASK_HANDLE result = 0;
	xTaskCreate(worker, (const signed char*)name, stackDepth, params, priority, &result);
	vTaskSuspend(result);	
	return result;
}

void rtosTaskSuspend(rtosTASK_HANDLE task)
{
	vTaskSuspend(task);
}

void rtosTaskResume(rtosTASK_HANDLE task)
{
	vTaskResume(task);
}
#endif

#if 1 == INCLUDE_vTaskDelete
void rtosTaskDelete(rtosTASK_HANDLE task)
{
	vTaskDelete(task);
}
#endif

#if 1 == INCLUDE_uxTaskPriorityGet
// task priority control
esU32 rtosTaskPriorityGet(rtosTASK_HANDLE task)
{
	return uxTaskPriorityGet(task);
}
#endif

#if 1 == INCLUDE_vTaskPrioritySet
void rtosTaskPrioritySet(rtosTASK_HANDLE task, esU32 priority)
{
	vTaskPrioritySet(task, priority);
}
#endif

// current task blocking delay (may be called from within task worker)
void rtosTaskDelay(esU32 ms)
{
	vTaskDelay( rtosMsToTimestamp(ms) );
}

esU32 rtosTaskGetTimestamp(void)
{
	return xTaskGetTickCount();
}

enum {
	portTickType_SZE = sizeof(portTickType),
	DWORD_SZE = sizeof(esU32)
};

void rtosTaskDelayUntil( esU32* previousCallTimeStamp, esU32 ms)
{
#if portTickType_SZE > DWORD_SZE
#	error "FREERTOS tick type exceeds esU32"
#endif
	
	vTaskDelayUntil((portTickType*)previousCallTimeStamp, rtosMsToTimestamp(ms));
}

// task scheduler services
//
void rtosStartScheduler(void)
{
	vTaskStartScheduler();
}

#ifdef vTaskEndScheduler
void rtosEndScheduler(void)
{
	vTaskEndScheduler();
}
#endif

void rtosSuspendAllTasks(void)
{
	vTaskSuspendAll();
}

void rtosResumeAllTasks(void)
{
	xTaskResumeAll();
}

void rtosEnterCritical(void)
{
	portENTER_CRITICAL();
}

void rtosExitCritical(void)
{
	portEXIT_CRITICAL();
}

void rtosTaskYield(void)
{
	taskYIELD();
}

void rtosExitSwitchingIsr(esBL doSwitch)
{
	portBASE_TYPE _switch = doSwitch ? pdTRUE : pdFALSE;
	portEXIT_SWITCHING_ISR(_switch);
}

// sync objects services
//
#if 1 == configUSE_COUNTING_SEMAPHORES
// semaphore
rtosSEMAPHORE_HANDLE rtosSemaphoreCreate(esU32 initialCnt, esU32 maxCnt)
{
#ifdef xSemaphoreCreateCounting
	return xSemaphoreCreateCounting(maxCnt, initialCnt);
#else
#	pragma message("Counting semaphore concept is not supported, using binary ones instead")
	rtosSEMAPHORE_HANDLE result = 0;
	vSemaphoreCreateBinary(result);
	return result;
#endif
}

void rtosSemaphoreDelete(rtosSEMAPHORE_HANDLE sem)
{
	vQueueDelete( (xQueueHandle)sem );
}

esBL rtosSemaphoreTakeBlocking(rtosSEMAPHORE_HANDLE sem)
{
	return pdTRUE == xSemaphoreTake(sem, portMAX_DELAY);
}

esBL rtosSemaphoreTake(rtosSEMAPHORE_HANDLE sem, esU32 tmo)
{
	return pdTRUE == xSemaphoreTake(sem, rtosMsToTimestamp(tmo));
}

void rtosSemaphoreGive(rtosSEMAPHORE_HANDLE sem)
{
	xSemaphoreGive(sem);
}

void rtosSemaphoreGiveFromIsr(rtosSEMAPHORE_HANDLE sem, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	xSemaphoreGiveFromISR(sem, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
}
#endif

#if 1 == configUSE_MUTEXES
// mutex
rtosMUTEX_HANDLE rtosMutexCreate(void)
{
	return xSemaphoreCreateMutex();
}

void rtosMutexDelete(rtosMUTEX_HANDLE mx)
{
	vQueueDelete( (xQueueHandle)mx );
}

esBL rtosMutexLock(rtosMUTEX_HANDLE mx, esU32 tmo)
{
	return pdTRUE == xSemaphoreTake(mx, tmo);
}

void rtosMutexUnlock(rtosMUTEX_HANDLE mx)
{
	xSemaphoreGive( mx );
}
#endif

// queue concept
//
rtosQUEUE_HANDLE rtosQueueCreate(esU32 len, esU32 elemSize)
{
	return xQueueCreate(len, elemSize);
}

void rtosQueueDelete(rtosQUEUE_HANDLE queue)
{
	vQueueDelete(queue);
}

esBL rtosQueuePushBack(rtosQUEUE_HANDLE queue, const void* elem, esU32 tmo)
{
	return pdTRUE == xQueueSendToBack(queue, elem, rtosMsToTimestamp(tmo));
}

esBL rtosQueuePushFront(rtosQUEUE_HANDLE queue, const void* elem, esU32 tmo)
{
	return pdTRUE == xQueueSendToFront(queue, elem, rtosMsToTimestamp(tmo));
}

esBL rtosQueuePop(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo)
{
	return pdTRUE == xQueueReceive(queue, elem, rtosMsToTimestamp(tmo));
}

esBL rtosQueuePeek(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo)
{
	return pdTRUE == xQueuePeek(queue, elem, rtosMsToTimestamp(tmo));
}

esU32 rtosQueueCountGet(rtosQUEUE_HANDLE queue)
{
	return uxQueueMessagesWaiting(queue);
}

esBL rtosQueuePushBackFromIsr(rtosQUEUE_HANDLE queue, const void* elem, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xQueueSendFromISR(queue, elem, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;	
}

esBL rtosQueuePopFromIsr(rtosQUEUE_HANDLE queue, void* elem, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xQueueReceiveFromISR(queue, elem, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}

// timers
//
#if 1 == configUSE_TIMERS
rtosTIMER_HANDLE rtosTimerCreate(ES_ASCII_CSTR name, esU32 period, esBL autoRestart, void* timerData, rtosTIMER_CALLBACK callback)
{
	return xTimerCreate((const signed char*)name, rtosMsToTimestamp(period), autoRestart, timerData, callback);
}

esBL rtosTimerDelete(rtosTIMER_HANDLE timer, esU32 tmo)
{
	return pdTRUE == xTimerDelete(timer, rtosMsToTimestamp(tmo));
}

esBL rtosTimerStart(rtosTIMER_HANDLE timer, esU32 tmo)
{
	return pdTRUE == xTimerStart(timer, rtosMsToTimestamp(tmo));
}

esBL rtosTimerReset(rtosTIMER_HANDLE timer, esU32 tmo)
{
	return pdTRUE == xTimerReset(timer, rtosMsToTimestamp(tmo));
}

esBL rtosTimerStartFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xTimerStartFromISR(timer, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}

esBL rtosTimerResetFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xTimerResetFromISR(timer, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}

esBL rtosTimerChangePeriod(rtosTIMER_HANDLE timer, esU32 newPeriod, esU32 tmo)
{
	return pdTRUE == xTimerChangePeriod(timer, rtosMsToTimestamp(newPeriod), rtosMsToTimestamp(tmo));
}

esBL rtosTimerChangePeriodFromISR(rtosTIMER_HANDLE timer, esU32 newPeriod, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xTimerChangePeriodFromISR(timer, rtosMsToTimestamp(newPeriod), &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}

esBL rtosTimerStop(rtosTIMER_HANDLE timer, esU32 tmo)
{
	return pdTRUE == xTimerStop(timer, rtosMsToTimestamp(tmo));
}

esBL rtosTimerStopFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	esBL result = pdTRUE == xTimerStopFromISR(timer, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}

esBL rtosTimerIsActive(rtosTIMER_HANDLE timer)
{
	return pdTRUE == xTimerIsTimerActive(timer);
}

void* rtosTimerDataGet(rtosTIMER_HANDLE timer)
{
	return pvTimerGetTimerID(timer);
}
#endif // #if 1 == configUSE_TIMERS
