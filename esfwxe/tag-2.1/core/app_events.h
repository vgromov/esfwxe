#ifndef _app_events_h_
#define _app_events_h_

// for paainting message support
#include <esgui/include/esgui_types.h>

#ifdef __cplusplus
  extern "C" {
#endif

#define EVENT_OWNS_DATA            0x80000000
#define  EVENT_DATA_ID_MASK        0x7FFFFFFF
#define EVENT_IS_OWNING_DATA(id)  ((esBL)((id) & EVENT_OWNS_DATA))
#define EVENT_GET_DATA_ID(id)      ((esU32)((id) & EVENT_DATA_ID_MASK))

// application event queue interface
//

typedef enum {
  MSG_NONE,                // placeholder for an empty message code
  MSG_SHUTDOWN,            // shutdown confirmation message
  MSG_POWEROFF,            // power off message, system must shut down in response to it
  MSG_POWERSTATS,         // power status update messages 
  MSG_KEYDOWN,            // key press|de-press messages
  MSG_KEYUP,
  MSG_NOTIFY,              // application notification event
  MSG_DATA,                // application data event
  MSG_PAINT,              // widget paint message
  MSG_ERROR,              // error message
  MSG_ENTER_BOOT,          // schedule bootloader
  // special value - count of messages
  MsgCount,
  // special value - user-defined notifications start here
  userNotificationsBase = 10000,

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
#  pragma anon_unions
#endif

// io event struct
typedef struct
{
  Message msg;
  union {
    PwrStatus pwrStatus;  // power status
    
    struct {
      esU32 idx;    // keyboard button index
      esU32 prevDown;

    } key;
    
    struct {
      esU32 id;      // notification event identifier
      esU32 val0;    // notification event values
      esU32 val1;

    } nf;

    // if data event was successfully handled,
    // data pointed to by buff should be de-allocated, size and pointer set to 0,
    // otherwise, main event handler must de-allocate data after event was processed by 
    // the main processing entry point 
    struct {
      esU32 id;      // data identifier, highest bit, if set, specifies, that event record owns data. buffer must be deallocated
      esU32 size;    // received data size
      esU8* buff;    // pointer to data buffer

    } dat;

    // paint messge 
    struct {
      ESGUI_RECT rect;  // repaint rectangle, in screen coords  
      esU32 flags;      // paint flags
      void* wgt;        // pointer to target widget. if 0, top screen widget is selected
      void* sender;      // optional pointer to the sender widget
  
    } paint;

    // poweroff delay value, ms
    esU32 pwroffDelay;
    
    // error message
    struct {
      esU32 code;        // error code
      esU8 domain;      // error domains, platform-dependent

    } err;
  };

} AppEvent;

// interactive event type carachterization helper. interactive events
// may not be processed by disabled widgets, for instance
// interactive events are button and touchscreen events
//
#define ES_IS_INTERACTIVE_EVENT(pevt)  (MSG_KEYUP == (pevt)->msg || MSG_KEYDOWN == (pevt)->msg)

// returns true if event is handled (i.e. MSG_NONE is set as its id)
#define ES_IS_HANDLED_EVENT(pevt) (MSG_NONE == (pevt)->msg)

// for application event functionality to be complete,
// the following services must be implemented in application layer:
//
// event queue management
esBL initAppEvents(esU32 evtQueueLen);
esBL postAppEvent(const AppEvent* evt);
esBL postUrgentAppEvent(const AppEvent* evt);
#ifndef USE_EMULATOR
esBL postAppEventFromIsr(const AppEvent* evt, esBL* higherPriorityTaskWoken);
#endif
// try to get event from the queue
esBL getAppEvent(AppEvent* evt);
// blocking wait for event 
void getAppEventBlocking(AppEvent* evt);

// the followinf basic services are always implemented
//
// discard events (for automatic garbage collection)
void eventCleanup(AppEvent* evt);

// event debug tracing
#if defined(USE_TRACE) && defined(USE_EVENT_TRACE)
  void traceEvent(const AppEvent* evt);
  #define TRACE_EVENT(evt)  traceEvent(evt);
#else
  #define TRACE_EVENT(evt)
#endif  // USE_TRACE

#ifdef __cplusplus
  }
#endif

#endif // _app_events_h_
