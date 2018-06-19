#include <esfwxe/target.h>
#pragma hdrstop

// events header
#include <esfwxe/trace.h>
#include <esfwxe/core/app_events.h>
#include <esfwxe/core/rtos_intf.h>
#include <esgui/include/esgui.h>

// application event basic implementation
//

// debug event trace
//
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
	static ES_ASCII_CSTR c_msgNames[MsgCount] =
		{
			"MSG_NONE",
			"MSG_SHUTDOWN",
			"MSG_POWEROFF",
			"MSG_POWERSTATS",
			"MSG_KEYDOWN",
			"MSG_KEYUP",
			"MSG_NOTIFY",
			"MSG_DATA",
			"MSG_PAINT",
			"MSG_ERROR",
			"MSG_ENTER_BOOT",
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
	case MSG_POWERSTATS_UPDATE:
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
void eventCleanup(AppEvent* evt)
{
	if( evt )
	{
		if( MSG_DATA == evt->msg && EVENT_IS_OWNING_DATA(evt->dat.id) && 0 != evt->dat.buff )
		{
			rtosFree(evt->dat.buff);
			evt->dat.buff = 0;
			evt->dat.size = 0;
			evt->dat.id = 0;
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
			TRACE0("Data event was not handled. Discarding data event.");
#endif
		}
	
		evt->msg = MSG_NONE;
	}
}

// implementation of RTOS-dependent app_events queue management services
//
#define EVENT_POST_TIMEOUT 	25
#define	EVENT_GET_TIMEOUT 	1000

static rtosQUEUE_HANDLE s_events;

esBL initAppEvents(esU32 evtQueueLen)
{
	s_events = rtosQueueCreate(evtQueueLen, sizeof(AppEvent) );
	return 0 != s_events;
}

// event queue management
esBL postAppEvent(const AppEvent* evt)
{
	return rtosQueuePushBack(s_events, (void*)evt, EVENT_POST_TIMEOUT);
}

esBL postUrgentAppEvent(const AppEvent* evt)
{
	return rtosQueuePushFront(s_events, (void*)evt, EVENT_POST_TIMEOUT);
}

#ifndef USE_EMULATOR
esBL postAppEventFromIsr(const AppEvent* evt, esBL* higherPriorityTaskWoken)
{
	return rtosQueuePushBackFromIsr(s_events, (void*)evt, higherPriorityTaskWoken);
}
#endif

esBL getAppEvent(AppEvent* evt)
{
	return rtosQueuePop(s_events, (void*)evt, EVENT_GET_TIMEOUT);
}

// blocking wait for event
void getAppEventBlocking(AppEvent* evt)
{
	rtosQueuePop(s_events, (void*)evt, rtosMAX_DELAY);
}
