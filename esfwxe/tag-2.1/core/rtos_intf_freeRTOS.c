#include <esfwxe/target.h>
#include <esfwxe/core/rtos_intf.h>
#pragma hdrstop

// rtos_intf implementation for freertos
//

// free rtos stuff
#include <FreeRTOS.h>
#include <semphr.h>
#include <timers.h>

esU32 rtosMsToTimestamp( esU32 ms )
{
    if( ms )
    {
        ms = ((TickType_t)(ms) == rtosMAX_DELAY ? portMAX_DELAY : (TickType_t)ms / portTICK_PERIOD_MS );

        if(!ms)
            return 1;

        return ms;
    }
    
    return 0;
}

esU32 rtosTimestampToMs( esU32 ts )
{
    return (TickType_t)(ts) * portTICK_PERIOD_MS;
}

// freertos stack checking
#if 0 < configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( TaskHandle_t *pxTask, signed portCHAR *pcTaskName )
{
    
}
#endif

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

rtosTASK_HANDLE rtosTaskCreate(rtosTASK_FUNCTION worker, ESE_CSTR name, esU32 stackDepth, void* params, esU32 priority)
{
    rtosTASK_HANDLE result = 0;
    xTaskCreate(worker, name, stackDepth, params, priority, &result);
    return result;
}

#if 1 == INCLUDE_vTaskSuspend
rtosTASK_HANDLE rtosTaskCreateSuspended(rtosTASK_FUNCTION worker, ESE_CSTR name, esU32 stackDepth, void* params, esU32 priority)
{
    rtosTASK_HANDLE result = 0;
    xTaskCreate(worker, name, stackDepth, params, priority, &result);
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
    TickType_t_SZE = sizeof(TickType_t),
    DWORD_SZE = sizeof(esU32)
};

void rtosTaskDelayUntil( esU32* previousCallTimeStamp, esU32 ms)
{
#if TickType_t_SZE > DWORD_SZE
#    error "FREERTOS tick type exceeds esU32"
#endif
    
    vTaskDelayUntil((TickType_t*)previousCallTimeStamp, rtosMsToTimestamp(ms));
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

esBL rtosSchedulerIsRunning(void)
{
    return taskSCHEDULER_RUNNING == xTaskGetSchedulerState();
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
#    pragma message("Counting semaphore concept is not supported, using binary ones instead")
    rtosSEMAPHORE_HANDLE result = 0;
    vSemaphoreCreateBinary(result);
    return result;
#endif
}

void rtosSemaphoreDelete(rtosSEMAPHORE_HANDLE sem)
{
    vQueueDelete( (QueueHandle_t)sem );
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
    vQueueDelete( (QueueHandle_t)mx );
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
    return pdPASS == xQueueSendToBack(queue, elem, rtosMsToTimestamp(tmo));
}

esBL rtosQueuePushFront(rtosQUEUE_HANDLE queue, const void* elem, esU32 tmo)
{
    return pdPASS == xQueueSendToFront(queue, elem, rtosMsToTimestamp(tmo));
}

esBL rtosQueuePop(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo)
{
    return pdPASS == xQueueReceive(queue, elem, rtosMsToTimestamp(tmo));
}

void rtosQueueReset(rtosQUEUE_HANDLE queue)
{
  xQueueReset(queue);
}

esBL rtosQueuePeek(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo)
{
    return pdPASS == xQueuePeek(queue, elem, rtosMsToTimestamp(tmo));
}

esU32 rtosQueueCountGet(rtosQUEUE_HANDLE queue)
{
    return uxQueueMessagesWaiting(queue);
}

esBL rtosQueuePushBackFromIsr(rtosQUEUE_HANDLE queue, const void* elem, esBL* higherPriorityTaskWoken)
{
    portBASE_TYPE woken;
    esBL result = pdPASS == xQueueSendFromISR(queue, elem, &woken);
    *higherPriorityTaskWoken = pdTRUE == woken;
    return result;    
}

esBL rtosQueuePopFromIsr(rtosQUEUE_HANDLE queue, void* elem, esBL* higherPriorityTaskWoken)
{
    portBASE_TYPE woken;
    esBL result = pdPASS == xQueueReceiveFromISR(queue, elem, &woken);
    *higherPriorityTaskWoken = pdTRUE == woken;
    return result;
}

// timers
//
#if 1 == configUSE_TIMERS
rtosTIMER_HANDLE rtosTimerCreate(ESE_CSTR name, esU32 period, esBL autoRestart, void* timerData, rtosTIMER_CALLBACK callback)
{
    return xTimerCreate(name, rtosMsToTimestamp(period), autoRestart, timerData, callback);
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
