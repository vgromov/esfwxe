#include <common/target.h>
#pragma hdrstop

// events header
#include <common/trace.h>
#include <common/core/app_events.h>
#include <common/core/rtos_intf.h>

// application event basic implementation
//

// debug event trace
//
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
	static ES_ASCII_CSTR c_msgNames[MsgCount] =
		{
			"MSG_SHUTDOWN",
			"MSG_POWER",
			"MSG_KEYDOWN",
			"MSG_KEYUP",
			"MSG_NOTIFY",
			"MSG_DATA",
		};

		static ES_ASCII_CSTR c_pwrStatNames[PwrStatusCount] =
		{
			"PWR_MAINS",
			"PWR_CHARGING",
			"PWR_BATTERY",
			"PWR_LOBATTERY"
		};

void traceEvent(const AppEvent* evt)
{
	TRACE1("event: "TRACE_FMT_STR, c_msgNames[evt->msg])
	switch( evt->msg )
	{
	case MSG_POWER:
		TRACE1("pwr status: "TRACE_FMT_STR, c_pwrStatNames[evt->pwrStatus])
		break;
	case MSG_KEYDOWN:
	case MSG_KEYUP:
		TRACE2("key idx|prev down: %u|"TRACE_FMT_STR, evt->key.idx, evt->key.prevDown ? "down" : "up" )
		break;
	case MSG_NOTIFY:
		TRACE2("notification id|val: %u|%u", evt->nf.id, evt->nf.val)
		break;
	case MSG_DATA:
		TRACE3("data id|owns|size: %u|"TRACE_FMT_STR"|%u", EVENT_GET_DATA_ID(evt->dat.id),
			EVENT_IS_OWNING_DATA(evt->dat.id) ? "yes" : "no", evt->dat.size)
		break;
	}
}

#endif // USE_TRACE

// discard data events (for automatic garbage collection)
void checkUnhandledDataEvent(AppEvent* evt)
{
	if( evt && MSG_DATA == evt->msg )
	{
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
		TRACE0("Data event was not handled. Discarding data event.");
#endif
		freeDataEvent(evt);
	}
}

void freeDataEvent(AppEvent* evt)
{
	if( evt && MSG_DATA == evt->msg && EVENT_IS_OWNING_DATA(evt->dat.id) && 0 != evt->dat.buff )
	{
		rtosFree(evt->dat.buff);
		evt->dat.buff = 0;
		evt->dat.size = 0;
		evt->dat.id = 0;
	}
}

// implementation of RTOS-dependent app_events queue management services
//
#define EVENT_QUEUE_LEN 		64
#define EVENT_POST_TIMEOUT 	10
#define	EVENT_GET_TIMEOUT 	1000

static rtosQUEUE_HANDLE s_events;

BOOL initAppEvents(void)
{
	s_events = rtosQueueCreate(EVENT_QUEUE_LEN, sizeof(AppEvent) );
	return 0 != s_events;
}

// event queue management
BOOL postAppEvent(const AppEvent* evt)
{
	return rtosQueuePushBack(s_events, (void*)evt, EVENT_POST_TIMEOUT);
}

BOOL postUrgentAppEvent(const AppEvent* evt)
{
	return rtosQueuePushFront(s_events, (void*)evt, EVENT_POST_TIMEOUT);
}

#ifndef USE_EMULATOR
BOOL postAppEventFromIsr(const AppEvent* evt, BOOL* higherPriorityTaskWoken)
{
	return rtosQueuePushBackFromIsr(s_events, (void*)evt, higherPriorityTaskWoken);
}
#endif


BOOL getAppEvent(AppEvent* evt)
{
	return rtosQueuePop(s_events, (void*)evt, EVENT_GET_TIMEOUT);
}

// blocking wait for event
void getAppEventBlocking(AppEvent* evt)
{
	rtosQueuePop(s_events, (void*)evt, rtosMAX_DELAY);
}
