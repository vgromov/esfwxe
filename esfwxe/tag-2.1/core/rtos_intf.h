#ifndef _rtos_defs_h_
#define _rtos_defs_h_

#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define rtosMAX_DELAY	((esU32)0xFFFFFFFF)

// stub definition for rtos functions modifier. allow to link them from external dll
#ifndef RTOS_FUNC
#	define RTOS_FUNC
#endif

// MT object handles
//
typedef void* rtosTASK_HANDLE;
typedef void* rtosSEMAPHORE_HANDLE;
typedef void* rtosMUTEX_HANDLE;
typedef void* rtosQUEUE_HANDLE;
typedef void* rtosTIMER_HANDLE;

// task worker function type
#ifdef USE_EMULATOR
	// pointer to thread worker object (EsThread::Worker) is passed as second parameter
	typedef void (*rtosTASK_FUNCTION)(void*,void*);
#else
	typedef void (*rtosTASK_FUNCTION)(void*);
#endif

// ticks|ms conversions
//
RTOS_FUNC esU32 rtosMsToTimestamp(esU32 ms);
RTOS_FUNC esU32 rtosTimestampToMs(esU32 ts);

// task-safe memory management
//
RTOS_FUNC void* rtosMalloc(size_t size);
RTOS_FUNC void* rtosCalloc(size_t elemCnt, size_t elemSize);
RTOS_FUNC void rtosFree(void* ptr);
RTOS_FUNC void rtosMemStats(size_t* curalloc, size_t* totfree, size_t* maxfree);

// task services
//
RTOS_FUNC esU32 rtosMAX_PRIORITY(void);
RTOS_FUNC rtosTASK_HANDLE rtosCurrentTaskGet(void);
RTOS_FUNC rtosTASK_HANDLE rtosTaskCreate(rtosTASK_FUNCTION worker, ESE_CSTR name, esU32 stackDepth, void* params, esU32 priority);
RTOS_FUNC rtosTASK_HANDLE rtosTaskCreateSuspended(rtosTASK_FUNCTION worker, ESE_CSTR name, esU32 stackDepth, void* params, esU32 priority);
// calling the following services with task = 0, is equivalent to calling them on current task ID 
RTOS_FUNC void rtosTaskDelete(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskSuspend(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskResume(rtosTASK_HANDLE task);
// task priority control
RTOS_FUNC esU32 rtosTaskPriorityGet(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskPrioritySet(rtosTASK_HANDLE task, esU32 priority);
// current task blocking delay (may be called from within task worker)
RTOS_FUNC void rtosTaskDelay(esU32 ms);
RTOS_FUNC esU32 rtosTaskGetTimestamp(void);
RTOS_FUNC void rtosTaskDelayUntil( esU32* previousCallTimeStamp, esU32 ms);

// task scheduler services
//
void rtosStartScheduler(void);
RTOS_FUNC void rtosEndScheduler(void);
RTOS_FUNC esBL rtosSchedulerIsRunning(void);
RTOS_FUNC void rtosSuspendAllTasks(void);
RTOS_FUNC void rtosResumeAllTasks(void);
RTOS_FUNC void rtosEnterCritical(void);
RTOS_FUNC void rtosExitCritical(void);
RTOS_FUNC void rtosTaskYield(void);
RTOS_FUNC void rtosExitSwitchingIsr(esBL doSwitch);

// sync objects services
//
// semaphore
RTOS_FUNC rtosSEMAPHORE_HANDLE rtosSemaphoreCreate(esU32 initialCnt, esU32 maxCnt);
RTOS_FUNC void rtosSemaphoreDelete(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC esBL rtosSemaphoreTakeBlocking(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC esBL rtosSemaphoreTake(rtosSEMAPHORE_HANDLE sem, esU32 tmo);
RTOS_FUNC void rtosSemaphoreGive(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC void rtosSemaphoreGiveFromIsr(rtosSEMAPHORE_HANDLE sem, esBL* higherPriorityTaskWoken);

// mutex
RTOS_FUNC rtosMUTEX_HANDLE rtosMutexCreate(void);
RTOS_FUNC void rtosMutexDelete(rtosMUTEX_HANDLE mx);
RTOS_FUNC esBL rtosMutexLock(rtosMUTEX_HANDLE mx, esU32 tmo);
RTOS_FUNC void rtosMutexUnlock(rtosMUTEX_HANDLE mx);

// queue concept
//
RTOS_FUNC rtosQUEUE_HANDLE rtosQueueCreate(esU32 len, esU32 elemSize);
RTOS_FUNC void rtosQueueDelete(rtosQUEUE_HANDLE queue);
RTOS_FUNC esBL rtosQueuePushBack(rtosQUEUE_HANDLE queue, const void* elem, esU32 tmo);
RTOS_FUNC esBL rtosQueuePushFront(rtosQUEUE_HANDLE queue, const void* elem, esU32 tmo);
RTOS_FUNC esBL rtosQueuePop(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo);
RTOS_FUNC esBL rtosQueuePeek(rtosQUEUE_HANDLE queue, void* elem, esU32 tmo);
RTOS_FUNC void rtosQueueReset(rtosQUEUE_HANDLE queue);
RTOS_FUNC esU32 rtosQueueCountGet(rtosQUEUE_HANDLE queue);
RTOS_FUNC esBL rtosQueuePushBackFromIsr(rtosQUEUE_HANDLE queue, const void* elem, esBL* higherPriorityTaskWoken);
RTOS_FUNC esBL rtosQueuePopFromIsr(rtosQUEUE_HANDLE queue, void* elem, esBL* higherPriorityTaskWoken);

// timers concept
//
typedef void (*rtosTIMER_CALLBACK)( rtosTIMER_HANDLE );
RTOS_FUNC rtosTIMER_HANDLE rtosTimerCreate(ESE_CSTR name, esU32 period, esBL autoRestart, void* timerData, rtosTIMER_CALLBACK callback);
RTOS_FUNC esBL rtosTimerStart(rtosTIMER_HANDLE timer, esU32 tmo);
RTOS_FUNC esBL rtosTimerReset(rtosTIMER_HANDLE timer, esU32 tmo);
RTOS_FUNC esBL rtosTimerStartFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken);
RTOS_FUNC esBL rtosTimerResetFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken);
RTOS_FUNC esBL rtosTimerChangePeriod(rtosTIMER_HANDLE timer, esU32 newPeriod, esU32 tmo);
RTOS_FUNC esBL rtosTimerChangePeriodFromISR(rtosTIMER_HANDLE timer, esU32 newPeriod, esBL* higherPriorityTaskWoken);
RTOS_FUNC esBL rtosTimerStop(rtosTIMER_HANDLE timer, esU32 tmo);
RTOS_FUNC esBL rtosTimerStopFromISR(rtosTIMER_HANDLE timer, esBL* higherPriorityTaskWoken);
RTOS_FUNC esBL rtosTimerIsActive(rtosTIMER_HANDLE timer);
RTOS_FUNC void* rtosTimerDataGet(rtosTIMER_HANDLE timer);

#ifdef __cplusplus
	}
#endif

// task definition and implementation helper defines
#define DEF_TASK( Name ) \
rtosTASK_HANDLE task ## Name ## Create( void* pParams )

// task worker function implementation
#ifdef USE_EMULATOR
#	define IMPL_TASK_BEGIN( Name, StackDepth, Priority ) \
	static void task ## Name( void* params, void* threadWorkerObj ); \
	rtosTASK_HANDLE task ## Name ## Create( void* params ) \
	{ \
		return rtosTaskCreate(task ## Name, #Name, (StackDepth), params, (Priority)); \
	} \
	static void task ## Name( void* params, void* threadWorkerObj ) \
	{ 

#	define IMPL_TASK_BEGIN_SUSPENDED( Name, StackDepth, Priority ) \
	static void task ## Name( void* params, void* threadWorkerObj ); \
	rtosTASK_HANDLE task ## Name ## Create( void* params ) \
	{ \
		return rtosTaskCreateSuspended(task ## Name, #Name, (StackDepth), params, (Priority)); \
	} \
	static void task ## Name( void* params, void* threadWorkerObj ) \
	{ 
	
#else	// #ifdef USE_EMULATOR

#	define IMPL_TASK_BEGIN( Name, StackDepth, Priority ) \
	static void task ## Name( void* params ); \
	rtosTASK_HANDLE task ## Name ## Create( void* params ) \
	{ \
		return rtosTaskCreate(task ## Name, #Name, (StackDepth), params, (Priority)); \
	} \
	static void task ## Name( void* params ) \
	{
	 
#	define IMPL_TASK_BEGIN_SUSPENDED( Name, StackDepth, Priority ) \
	static void task ## Name( void* params ); \
	rtosTASK_HANDLE task ## Name ## Create( void* params ) \
	{ \
		return rtosTaskCreateSuspended(task ## Name, #Name, (StackDepth), params, (Priority)); \
	} \
	static void task ## Name( void* params ) \
	{	
	
#endif // #ifdef USE_EMULATOR

#define IMPL_TASK_END }

#define RTOS_FREE_AND_ZERO(ptr) if(0 != (ptr)){ rtosFree((void*)(ptr)); (ptr) = 0; }

#endif // _rtos_defs_h_
