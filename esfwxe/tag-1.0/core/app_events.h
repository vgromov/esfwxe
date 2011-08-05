#ifndef _app_events_h_
#define _app_events_h_

// for paainting message support
#include <esgui/include/esgui_types.h>

#ifdef __cplusplus
	extern "C" {
#endif

#define EVENT_OWNS_DATA						0x80000000
#define	EVENT_DATA_ID_MASK				0x7FFFFFFF
#define EVENT_IS_OWNING_DATA(id)	((BOOL)((id) & EVENT_OWNS_DATA))
#define EVENT_GET_DATA_ID(id)			((DWORD)((id) & EVENT_DATA_ID_MASK))

// application event queue interface
//

typedef enum {
	MSG_SHUTDOWN,			// shutdown confirmation message
	MSG_POWER, 				// power status messages 
	MSG_KEYDOWN,			// key press|de-press messages
	MSG_KEYUP,
	MSG_NOTIFY,				// application notification event
	MSG_DATA,					// application data event
	MSG_PAINT,				// widget paint message
	MSG_ERROR,				// error message
	// special value - count of messages
	MsgCount

} Message;

// power status ids
typedef enum {
	PWR_MAINS,
	PWR_CHARGING,
	PWR_BATTERY,
	// special value - count of power statuses
	PwrStatusCount

} PwrStatus;

#if !defined(_BORLANDC_) && !defined(_MSC_VER)
#	pragma anon_unions
#endif

// io event struct
typedef struct
{
	Message msg;
	union {
		PwrStatus pwrStatus;	// power status
		
		struct {
			DWORD idx;		// keyboard button index
			BYTE	prevDown : 1;
		} key;
		
		struct {
			DWORD id;			// notification event identifier
			DWORD val0;	  // notification event values
			DWORD val1;
		} nf;

		// if data event was successfully handled,
		// data pointed to by buff should be de-allocated, size and pointer set to 0,
		// otherwise, main event handler must de-allocate data after event was processed by 
		// the main processing entry point 
		struct {
			DWORD id;			// data identifier, highest bit, if set, specifies, that event record owns data. buffer must be deallocated
			DWORD size;		// received data size
			BYTE* buff;	  // pointer to data buffer
		} dat;

		// paint messge 
		struct {
			ESGUI_RECT rect;	// repaint rectangle, in screen coords			
			void* wgt;				// pointer to target widget. if 0, top screen widget is selected
			void* sender;			// optional pointer to the sender widget
			BOOL	bgndErase : 1; // erase background
			BOOL	force : 1;	// force widget repaint
		} paint;
		
		// error message
		struct {
			DWORD code;				// error code
			BYTE domain;			// error domains, platform-dependent
		} err;
	};

} AppEvent;

// for application event functionality to be complete,
// the following services must be implemented in application layer:
//
// event queue management
BOOL initAppEvents(void);
BOOL postAppEvent(const AppEvent* evt);
BOOL postUrgentAppEvent(const AppEvent* evt);
#ifndef USE_EMULATOR
BOOL postAppEventFromIsr(const AppEvent* evt, BOOL* higherPriorityTaskWoken);
#endif
// try to get event from the queue
BOOL getAppEvent(AppEvent* evt);
// blocking wait for event 
void getAppEventBlocking(AppEvent* evt);

// the followinf basic services are always implemented
//
// discard events (for automatic garbage collection)
void checkUnhandledDataEvent(AppEvent* evt);
void freeDataEvent(AppEvent* evt);

// event debug tracing
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
	void traceEvent(const AppEvent* evt);
	#define TRACE_EVENT(evt)	traceEvent(evt);
#else
	#define TRACE_EVENT(evt)
#endif	// USE_TRACE

#ifdef __cplusplus
	}
#endif

#endif // _app_events_h_
