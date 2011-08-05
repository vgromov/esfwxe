#ifndef _rtos_defs_h_
#define _rtos_defs_h_

#include <stddef.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define rtosMAX_DELAY	((DWORD)0xFFFFFFFF)

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

// task worker function type
#ifdef USE_EMULATOR
	// pointer to thread worker object (EsThread::Worker) is passed as second parameter
	typedef void (*rtosTASK_FUNCTION)(void*,void*);
#else
	typedef void (*rtosTASK_FUNCTION)(void*);
#endif

// ticks|ms conversions
//
RTOS_FUNC DWORD rtosMsToTimestamp(DWORD ms);
RTOS_FUNC DWORD rtosTimestampToMs(DWORD ts);

// task-safe memory management
//
RTOS_FUNC void* rtosMalloc(size_t size);
RTOS_FUNC void* rtosCalloc(size_t elemCnt, size_t elemSize);
RTOS_FUNC void rtosFree(void* ptr);
RTOS_FUNC void rtosMemStats(size_t* curalloc, size_t* totfree, size_t* maxfree);

// task services
//
RTOS_FUNC DWORD rtosMAX_PRIORITY(void);
RTOS_FUNC rtosTASK_HANDLE rtosCurrentTaskGet(void);
RTOS_FUNC rtosTASK_HANDLE rtosTaskCreate(rtosTASK_FUNCTION worker, ES_ASCII_CSTR name, DWORD stackDepth, void* params, DWORD priority);
RTOS_FUNC rtosTASK_HANDLE rtosTaskCreateSuspended(rtosTASK_FUNCTION worker, ES_ASCII_CSTR name, DWORD stackDepth, void* params, DWORD priority);
// calling the following services with task = 0, is equivalent to calling them on current task ID 
RTOS_FUNC void rtosTaskDelete(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskSuspend(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskResume(rtosTASK_HANDLE task);
// task priority control
RTOS_FUNC DWORD rtosTaskPriorityGet(rtosTASK_HANDLE task);
RTOS_FUNC void rtosTaskPrioritySet(rtosTASK_HANDLE task, DWORD priority);
// current task blocking delay (may be called from within task worker)
RTOS_FUNC void rtosTaskDelay(DWORD ms);
RTOS_FUNC DWORD rtosTaskGetTimestamp(void);
RTOS_FUNC void rtosTaskDelayUntil( DWORD* previousCallTimeStamp, DWORD ms);

// task scheduler services
//
RTOS_FUNC void rtosStartScheduler(void);
RTOS_FUNC void rtosEndScheduler(void);
RTOS_FUNC void rtosSuspendAllTasks(void);
RTOS_FUNC void rtosResumeAllTasks(void);
RTOS_FUNC void rtosEnterCritical(void);
RTOS_FUNC void rtosExitCritical(void);
RTOS_FUNC void rtosTaskYield(void);
RTOS_FUNC void rtosExitSwitchingIsr(BOOL doSwitch);

// sync objects services
//
// semaphore
RTOS_FUNC rtosSEMAPHORE_HANDLE rtosSemaphoreCreate(DWORD initialCnt, DWORD maxCnt);
RTOS_FUNC void rtosSemaphoreDelete(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC BOOL rtosSemaphoreTakeBlocking(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC BOOL rtosSemaphoreTake(rtosSEMAPHORE_HANDLE sem, DWORD tmo);
RTOS_FUNC void rtosSemaphoreGive(rtosSEMAPHORE_HANDLE sem);
RTOS_FUNC void rtosSemaphoreGiveFromIsr(rtosSEMAPHORE_HANDLE sem, BOOL* higherPriorityTaskWoken);

// mutex
RTOS_FUNC rtosMUTEX_HANDLE rtosMutexCreate(void);
RTOS_FUNC void rtosMutexDelete(rtosMUTEX_HANDLE mx);
RTOS_FUNC BOOL rtosMutexLock(rtosMUTEX_HANDLE mx, DWORD tmo);
RTOS_FUNC void rtosMutexUnlock(rtosMUTEX_HANDLE mx);

// queue concept
//
RTOS_FUNC rtosQUEUE_HANDLE rtosQueueCreate(DWORD len, DWORD elemSize);
RTOS_FUNC void rtosQueueDelete(rtosQUEUE_HANDLE queue);
RTOS_FUNC BOOL rtosQueuePushBack(rtosQUEUE_HANDLE queue, const void* elem, DWORD tmo);
RTOS_FUNC BOOL rtosQueuePushFront(rtosQUEUE_HANDLE queue, const void* elem, DWORD tmo);
RTOS_FUNC BOOL rtosQueuePop(rtosQUEUE_HANDLE queue, void* elem, DWORD tmo);
RTOS_FUNC BOOL rtosQueuePeek(rtosQUEUE_HANDLE queue, void* elem, DWORD tmo);
RTOS_FUNC DWORD rtosQueueCountGet(rtosQUEUE_HANDLE queue);
RTOS_FUNC BOOL rtosQueuePushBackFromIsr(rtosQUEUE_HANDLE queue, const void* elem, BOOL* higherPriorityTaskWoken);
RTOS_FUNC BOOL rtosQueuePopFromIsr(rtosQUEUE_HANDLE queue, void* elem, BOOL* higherPriorityTaskWoken);

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
