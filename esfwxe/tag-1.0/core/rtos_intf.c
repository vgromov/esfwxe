#include <common/target.h>
#include <common/core/rtos_intf.h>
#pragma hdrstop

// rtos_intf implementation for freertos
//

// free rtos stuff
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

DWORD rtosMsToTimestamp( DWORD ms )
{
	return ((portTickType)(ms) == rtosMAX_DELAY ? portMAX_DELAY : (portTickType)(ms) / portTICK_RATE_MS);
}

DWORD rtosTimestampToMs( DWORD ts )
{
	return (((portTickType)(ts) * configTICK_RATE_HZ) / 1000);
}

// freertos stack checking
#if defined(USE_TRACE) && defined(USE_STACK_TRACE)
#include <common/trace.h>
void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed portCHAR *pcTaskName )
{
	TRACE1("Stack overflow detected for %s\r\n", pcTaskName)
}
#endif

DWORD rtosMAX_PRIORITY(void)	
{ 
	return (configMAX_PRIORITIES-1); 
}

// task services
//
rtosTASK_HANDLE rtosCurrentTaskGet(void)
{
	return xTaskGetCurrentTaskHandle();
}

rtosTASK_HANDLE rtosTaskCreate(rtosTASK_FUNCTION worker, const char* name, DWORD stackDepth, void* params, DWORD priority)
{
	rtosTASK_HANDLE result = 0;
	xTaskCreate(worker, (const portCHAR*)name, stackDepth, params, priority, &result);
	return result;
}

#if 1 == INCLUDE_vTaskSuspend
rtosTASK_HANDLE rtosTaskCreateSuspended(rtosTASK_FUNCTION worker, const char* name, DWORD stackDepth, void* params, DWORD priority)
{
	rtosTASK_HANDLE result = 0;
	xTaskCreate(worker, (const portCHAR*)name, stackDepth, params, priority, &result);
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
DWORD rtosTaskPriorityGet(rtosTASK_HANDLE task)
{
	return uxTaskPriorityGet(task);
}
#endif

#if 1 == INCLUDE_vTaskPrioritySet
void rtosTaskPrioritySet(rtosTASK_HANDLE task, DWORD priority)
{
	vTaskPrioritySet(task, priority);
}
#endif

// current task blocking delay (may be called from within task worker)
void rtosTaskDelay(DWORD ms)
{
	vTaskDelay( rtosMsToTimestamp(ms) );
}

DWORD rtosTaskGetTimestamp(void)
{
	return xTaskGetTickCount();
}

enum {
	portTickType_SZE = sizeof(portTickType),
	DWORD_SZE = sizeof(DWORD)
};

void rtosTaskDelayUntil( DWORD* previousCallTimeStamp, DWORD ms)
{
#if portTickType_SZE > DWORD_SZE
#	error "FREERTOS tick type exceeds DWORD"
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

void rtosExitSwitchingIsr(BOOL doSwitch)
{
	portBASE_TYPE _switch = doSwitch ? pdTRUE : pdFALSE;
	portEXIT_SWITCHING_ISR(_switch);
}

// sync objects services
//
#if 1 == configUSE_COUNTING_SEMAPHORES
// semaphore
rtosSEMAPHORE_HANDLE rtosSemaphoreCreate(DWORD initialCnt, DWORD maxCnt)
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

BOOL rtosSemaphoreTakeBlocking(rtosSEMAPHORE_HANDLE sem)
{
	return pdTRUE == xSemaphoreTake(sem, portMAX_DELAY);
}

BOOL rtosSemaphoreTake(rtosSEMAPHORE_HANDLE sem, DWORD tmo)
{
	return pdTRUE == xSemaphoreTake(sem, rtosMsToTimestamp(tmo));
}

void rtosSemaphoreGive(rtosSEMAPHORE_HANDLE sem)
{
	xSemaphoreGive(sem);
}

void rtosSemaphoreGiveFromIsr(rtosSEMAPHORE_HANDLE sem, BOOL* higherPriorityTaskWoken)
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

BOOL rtosMutexLock(rtosMUTEX_HANDLE mx, DWORD tmo)
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
rtosQUEUE_HANDLE rtosQueueCreate(DWORD len, DWORD elemSize)
{
	return xQueueCreate(len, elemSize);
}

void rtosQueueDelete(rtosQUEUE_HANDLE queue)
{
	vQueueDelete(queue);
}

BOOL rtosQueuePushBack(rtosQUEUE_HANDLE queue, const void* elem, DWORD tmo)
{
	return pdTRUE == xQueueSendToBack(queue, elem, rtosMsToTimestamp(tmo));
}

BOOL rtosQueuePushFront(rtosQUEUE_HANDLE queue, const void* elem, DWORD tmo)
{
	return pdTRUE == xQueueSendToFront(queue, elem, rtosMsToTimestamp(tmo));
}

BOOL rtosQueuePop(rtosQUEUE_HANDLE queue, void* elem, DWORD tmo)
{
	return pdTRUE == xQueueReceive(queue, elem, rtosMsToTimestamp(tmo));
}

BOOL rtosQueuePeek(rtosQUEUE_HANDLE queue, void* elem, DWORD tmo)
{
	return pdTRUE == xQueuePeek(queue, elem, rtosMsToTimestamp(tmo));
}

DWORD rtosQueueCountGet(rtosQUEUE_HANDLE queue)
{
	return uxQueueMessagesWaiting(queue);
}

BOOL rtosQueuePushBackFromIsr(rtosQUEUE_HANDLE queue, const void* elem, BOOL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	BOOL result = pdTRUE == xQueueSendFromISR(queue, elem, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;	
}

BOOL rtosQueuePopFromIsr(rtosQUEUE_HANDLE queue, void* elem, BOOL* higherPriorityTaskWoken)
{
	portBASE_TYPE woken;
	BOOL result = pdTRUE == xQueueReceiveFromISR(queue, elem, &woken);
	*higherPriorityTaskWoken = pdTRUE == woken;
	return result;
}
