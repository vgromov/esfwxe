#ifndef _scheduler_h_
#define	_scheduler_h_

// measurement shceduling support
//

#ifdef __cplusplus 
	extern "C" {
#endif

#if !defined(_BORLANDC_) && !defined(_MSC_VER)
#	pragma anon_unions
#endif

// schedule data, data masks, data access macros
//
#define SCHEDULER_WEEKLY_MON							0x01
#define SCHEDULER_WEEKLY_TUE							0x02
#define SCHEDULER_WEEKLY_WED							0x04
#define SCHEDULER_WEEKLY_THU							0x08
#define SCHEDULER_WEEKLY_FRI							0x10
#define SCHEDULER_WEEKLY_SAT							0x20
#define SCHEDULER_WEEKLY_SUN							0x40
#define SCHEDULER_WEEKLY_DAYS_MASK				(SCHEDULER_WEEKLY_MON|SCHEDULER_WEEKLY_TUE|SCHEDULER_WEEKLY_WED| \
																					SCHEDULER_WEEKLY_THU|SCHEDULER_WEEKLY_FRI|SCHEDULER_WEEKLY_SAT| \
																					SCHEDULER_WEEKLY_SUN)
#define SCHEDULER_WEEKLY_RECURRENT				0x80
#define SCHEDULER_WEEKLY_DAYS_GET(src)		((src) & SCHEDULER_WEEKLY_DAYS_MASK)
#define SCHEDULER_MONTHLY_DAY(d)					((0 < (d) && (d) < 32) ? (1 << (d-1)) : 0)
#define SCHEDULER_MONTHLY_DAYS_MASK				0x7FFFFFFF
#define SCHEDULER_MONTHLY_RECURRENT				0x80000000
#define SCHEDULER_MONTHLY_DAYS_GET(src)		((src) & SCHEDULER_MONTHLY_DAYS_MASK)
#define SCHEDULER_CALENDAR_JAN						0
#define SCHEDULER_CALENDAR_FEB						1
#define SCHEDULER_CALENDAR_MAR						2
#define SCHEDULER_CALENDAR_APR						3
#define SCHEDULER_CALENDAR_MAY						4
#define SCHEDULER_CALENDAR_JUN						5
#define SCHEDULER_CALENDAR_JUL						6
#define SCHEDULER_CALENDAR_AUG						7
#define SCHEDULER_CALENDAR_SEP						8
#define SCHEDULER_CALENDAR_OCT						9
#define SCHEDULER_CALENDAR_NOV						10
#define SCHEDULER_CALENDAR_DEC						11
#define SCHEDULER_CALENDAR_MONTH(m)				((0 <= (m) && (m) < 12) ? (1 << (m)) : 0)
#define SCHEDULER_CALENDAR_MONTH_MASK			(SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_JAN)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_FEB)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_MAR)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_APR)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_MAY)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_JUN)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_JUL)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_AUG)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_SEP)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_OCT)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_NOV)| \
																					SCHEDULER_CALENDAR_MONTH(SCHEDULER_CALENDAR_DEC))
#define SCHEDULER_CALENDAR_RECURRENT			0x8000
#define SCHEDULER_CALENDAR_MONTH_GET(src)	((src) & SCHEDULER_CALENDAR_MONTH_MASK)

typedef struct {
	UINT32 start;										// measurement start time, in ms
	UINT32 stop;										// measurement stop time, in ms
	union {
		// weekly schedule
		UINT8 weekly;
		// monthly schedule
		UINT32 monthly;
		// calendar data, used in calendar schedule
		struct {
			UINT32 calDays[12];					// each 0-30|29 bit corresponds to the day of month
			UINT16 calMonths;						// each 0-11 bit corresponds to the month, 15th bit signals recurrent day event
		};
	};
	UINT16 interval; 								// interval between events in minutes
	UINT8 mode; 										// schedule mode - interval or calendar
	UINT8 dummy;										// explicit alignment + padding to 64 bytes, which is the 4*n next to 61

} CalendarScheduler;

// scheduler constants
enum {
	// schedule config mode
	scheduleInterval = 0,
	scheduleWeekly,
	scheduleMonthly,
	scheduleCalendar,
	// schedule interval range, in minutes
	intervalMin = 1,
	intervalDef = 5,
	intervalMax = 1440,
	// schedule config header size, in bytes
	CalendarScheduler_SZE = sizeof(CalendarScheduler),
};

#define schedulerDistantFuture IMMEDIATE_INT64(0x7FFFFFFFFFFFFFFF)

// initialize schedule to defaul values
void schedulerInitSchedule(CalendarScheduler* schedule);
// update date and time of recently scheduled event end
void schedulerSetRecentEventEnd(DATETIME evt);
// reschedule next measurement start time
void schedulerUpdateNextStart(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* next);
// return true if existing schedule data is valid
BOOL schedulerIsScheduleValid(const CalendarScheduler* schedule);

#ifdef __cplusplus 
	}
#endif

#endif // _scheduler_h_
