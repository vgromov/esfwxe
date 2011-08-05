#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/datetime.h>
#include <common/scheduler.h>

#define schedulerMsInMinute IMMEDIATE_INT64(60000)
#define schedulerMsInDay 		IMMEDIATE_INT64(86400000)
// recent event holder
static struct {
	int y;
	int mn;
	int d;
	int h;
	int m;
	int s;

} s_recentEvent;

// internal shared data for all scheduler functions below. the following
// static data are used to pursue several targets:
// 1- save data stack
// 2- allow common helper functions to use data transparently with the top-level functions
// 3- I know, it's not safe, but the scheduler code is strictly single-threaded, and, 
// moreover, only one single client would use it
//
static DATETIME s_date;
static DATETIME s_time;
static int s_y, s_mn, s_d, s_h, s_m, s_s, s_daysCnt;
static int s_nextY, s_nextMn, s_nextDay;

// update date and time of recently scheduled event end
void schedulerSetRecentEventEnd(DATETIME evt)
{
	dtDecomposeDateTime(&evt, 
		&s_recentEvent.y, 0, 
		&s_recentEvent.mn, 
		&s_recentEvent.d,
		&s_recentEvent.h, 
		&s_recentEvent.m, 
		&s_recentEvent.s,	0);
}

// initialize schedule to defaul values
void schedulerInitSchedule(CalendarScheduler* schedule)
{
	schedule->mode = scheduleInterval;
	schedule->interval = intervalDef;
	schedule->start = 0;
	schedule->stop = schedulerMsInDay - 1000;
	schedulerSetRecentEventEnd(0);
}

// date scheduler return values
typedef enum {
	schedulerCurDateScheduled,
	schedulerMovedNextDate,
	schedulerMovedNextMonth

} schedulerReturnType;

// increment date. if we happen to move to the
// next month, return TRUE, FALSE otherwise
static schedulerReturnType schedulerMoveToNextDate(void)
{
	if( ++s_nextDay > s_daysCnt )
	{
		if( ++s_nextMn > 12 )
		{
			++s_nextY;
			s_nextMn = 1;
		}
		s_nextDay = 1;
		s_daysCnt = dtGetMonthDayCount(s_nextY, s_nextMn);
		
		return schedulerMovedNextMonth;
	}

	return schedulerMovedNextDate;
}

// try to schedule current date's event. return schedulerCurDate if succeed
// on error, move to the next date and return schedulerMovedNextDate|schedulerMovedNextMonth
// current date|time must be pre-parsed and placed into y, mn, d, h, m, s
static schedulerReturnType schedulerScheduleCurrentDate(const DATETIME* now,
	const CalendarScheduler* schedule, DATETIME* result)
{
	dtComposeDateTime(&s_time, DT_YEAR0, 1, 1, s_h, s_m, s_s, 0);
	dtComposeDateTime(&s_date, s_nextY, s_nextMn, s_nextDay, 0, 0, 0, 0);
	
	if( s_time >= (DATETIME)schedule->start && s_time <= (DATETIME)schedule->stop )
	{
		// if there was recent event for current date - schedule new event in interval minutes,
		// if only it would fall into time window, and event is marked recurrent; otherwise, move to the next date
		if( s_nextY == s_recentEvent.y && s_nextMn == s_recentEvent.mn && s_nextDay == s_recentEvent.d )
		{
			BOOL recurrent = scheduleInterval == schedule->mode ||
				(scheduleWeekly == schedule->mode && ES_BIT_IS_SET(schedule->weekly, SCHEDULER_WEEKLY_RECURRENT)) ||
				(scheduleMonthly == schedule->mode && ES_BIT_IS_SET(schedule->monthly, SCHEDULER_MONTHLY_RECURRENT)) ||
				(scheduleCalendar == schedule->mode && ES_BIT_IS_SET(schedule->calMonths, SCHEDULER_CALENDAR_RECURRENT));

			s_time += schedulerMsInMinute * (DATETIME)schedule->interval; 
			if( s_time <= (DATETIME)schedule->stop && 
					recurrent )
			{
				*result = s_date + s_time;
				return schedulerCurDateScheduled;
			}
			else // schedule start at the next date
				return schedulerMoveToNextDate();
		}
		else // there were no event for current date yet - schedule event right now
		{
			*result = *now;
			return schedulerCurDateScheduled;
		}
	}
	else if( s_time < (DATETIME)schedule->start )
	{
		// use now's date part and start time as a result
		*result = s_date + (DATETIME)schedule->start;
		return schedulerCurDateScheduled;
	}
	else // move to the next date
		return schedulerMoveToNextDate();
}

static void schedulerSearchInit(const DATETIME* now, DATETIME* result)
{
	*result = schedulerDistantFuture;
	// split time and date components here
	dtDecomposeDateTime(now, &s_y, 0, &s_mn, &s_d, &s_h, &s_m, &s_s, 0);
	s_nextY = s_y; s_nextMn = s_mn; s_nextDay = s_d;
	s_daysCnt = dtGetMonthDayCount(s_y, s_mn);
}

static void	schedulerScheduleInterval(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* result)
{		
	schedulerSearchInit(now, result);
	// try to schedule current date
	if( schedulerCurDateScheduled != 
			schedulerScheduleCurrentDate(now, schedule, result) )
	{
		// next date was scheduled, set date time from nextY nextMn nextDay + start
		dtComposeDate(result, s_nextY, s_nextMn, s_nextDay);
		*result += (DATETIME)schedule->start;
	}	
}

static void schedulerScheduleWeekly(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* result)
{
	int dow;
	BYTE days;
	schedulerSearchInit(now, result);
	if( 0 == SCHEDULER_WEEKLY_DAYS_GET(schedule->weekly) )
		return;
			
__wrap:
	// calculate day of week for current date
	dow = dtGetDayOfWeek(s_nextY, s_nextMn, s_nextDay);
	days = SCHEDULER_WEEKLY_DAYS_GET(schedule->weekly);

	// find appropriate dow
	days >>= (dow-1);
	while( !ES_BIT_IS_SET(days, 1) )
	{
		schedulerMoveToNextDate();
		if( ++dow > 7 )
			// reset search
			goto __wrap;
		days >>= 1;
	}

	if( s_y == s_nextY && s_mn == s_nextMn && s_d == s_nextDay ) 
	{
		if( schedulerCurDateScheduled != 
				schedulerScheduleCurrentDate(now, schedule, result) )
			// we failed to schedule at the current date, so continue further search
			goto __wrap;
	}
	else
	{
		// just schedule the found date|time
		dtComposeDate(result, s_nextY, s_nextMn, s_nextDay);
		*result += (DATETIME)schedule->start;
	}	
}

static void schedulerScheduleMonthly(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* result)
{
	DWORD days;
	schedulerSearchInit(now, result);
	if( 0 == SCHEDULER_MONTHLY_DAYS_GET(schedule->monthly) )
		return;
	
__wrap:
	// find next scheduled day, current date is included in the search
	days = SCHEDULER_MONTHLY_DAYS_GET(schedule->monthly) & (SCHEDULER_MONTHLY_DAYS_MASK >> (31-s_daysCnt));
	days >>= (s_nextDay-1);
	while( !ES_BIT_IS_SET(days, 1) )
	{
		if( schedulerMovedNextMonth == schedulerMoveToNextDate() )
			// we crossed month border - reset search
			goto __wrap;

		days >>= 1;
	}			

	if( s_y == s_nextY && s_mn == s_nextMn && s_d == s_nextDay ) 
	{
		if( schedulerCurDateScheduled != 
				schedulerScheduleCurrentDate(now, schedule, result) )
			// we failed to schedule at the current date, so continue further search
			goto __wrap;
	}
	else
	{
		// just schedule the found date|time
		dtComposeDate(result, s_nextY, s_nextMn, s_nextDay);
		*result += (DATETIME)schedule->start;
	}	
}

static void schedulerScheduleCalendar(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* result)
{
	WORD month;
	DWORD days;
	schedulerSearchInit(now, result);
	if( !schedule->calMonths )
		return;

__monthSearch:
	month = SCHEDULER_CALENDAR_MONTH_GET(schedule->calMonths);
	month >>= (s_nextMn-1);
	while( !ES_BIT_IS_SET(month, 1) )
	{
		if(++s_nextMn > 12)
		{
			// reset month search
			++s_nextY;
			s_nextMn = 1;
			month = SCHEDULER_CALENDAR_MONTH_GET(schedule->calMonths);
			s_nextDay = 1;
			continue;
		}
		s_nextDay = 1;
		month >>= 1;
	}	

__daySearch:
	s_daysCnt = dtGetMonthDayCount(s_nextY, s_nextMn);
	// find next scheduled day, current date is included in the search
	days = schedule->calDays[s_nextMn-1] & (SCHEDULER_MONTHLY_DAYS_MASK >> (31-s_daysCnt));
	days >>= (s_nextDay-1);
	while( !ES_BIT_IS_SET(days, 1) )
	{
		if( schedulerMovedNextMonth == schedulerMoveToNextDate() )
			// we crossed month border - reset month search
			goto __monthSearch;

		days >>= 1;
	}			

	if( s_y == s_nextY && s_mn == s_nextMn && s_d == s_nextDay ) 
	{
		schedulerReturnType sch = schedulerScheduleCurrentDate(now, schedule, result);
		if( schedulerMovedNextDate == sch )
			// we failed to schedule at the current date, so continue further search
			goto __daySearch;
		else if( schedulerMovedNextMonth == sch )
			goto __monthSearch;
	}
	else
	{
		// just schedule the found date|time
		dtComposeDate(result, s_nextY, s_nextMn, s_nextDay);
		*result += (DATETIME)schedule->start;
	}		
}

// reschedule next measurement start time
void schedulerUpdateNextStart(const CalendarScheduler* schedule, const DATETIME* now, DATETIME* next)
{
	if( schedule )
	{
		// calculate next wakeup time
		switch( schedule->mode )
		{
		case scheduleInterval:
			schedulerScheduleInterval(schedule, now, next);
			break;
		case scheduleWeekly:
			schedulerScheduleWeekly(schedule, now, next);
			break;
		case scheduleMonthly:
			schedulerScheduleMonthly(schedule, now, next);
			break;
		default:
			schedulerScheduleCalendar(schedule, now, next);
			break;
		}
	}
	else
		*next = schedulerDistantFuture;
}

// check calendar shcedule months
static BOOL schedulerCheckCalendarMonths(const CalendarScheduler* schedule)
{
	WORD months = SCHEDULER_CALENDAR_MONTH_GET(schedule->calMonths);
	BOOL result = months != 0;
	const DWORD* days = schedule->calDays; 
	
	while( result && months != 0 )
	{
		if( ES_BIT_IS_SET(months, 1) )
			result = (*days & SCHEDULER_MONTHLY_DAYS_MASK) != 0;			

		months >>= 1;
		++days;
	}

	return result;
}

// return true if existing schedule data is valid
BOOL schedulerIsScheduleValid(const CalendarScheduler* schedule)
{
	if(schedule && schedule->mode <= scheduleCalendar)
	{
		BOOL intOk = 			
			schedule->interval >= intervalMin && 
			schedule->interval <= intervalMax;
		BOOL wndOk = 
			schedule->start < schedule->stop &&
			schedule->stop < schedulerMsInDay;
		
		switch( schedule->mode )
		{
		case scheduleInterval:
			return 	intOk && 
							wndOk;
		case scheduleWeekly:
			return 0 != SCHEDULER_WEEKLY_DAYS_GET(schedule->weekly) &&
				((ES_BIT_IS_SET(schedule->weekly, SCHEDULER_WEEKLY_RECURRENT) && intOk) ||
					!ES_BIT_IS_SET(schedule->weekly, SCHEDULER_WEEKLY_RECURRENT)) && 
				wndOk;
		case scheduleMonthly:
			return 0 != SCHEDULER_MONTHLY_DAYS_GET(schedule->monthly) &&
				((ES_BIT_IS_SET(schedule->monthly, SCHEDULER_MONTHLY_RECURRENT) && intOk) ||
					!ES_BIT_IS_SET(schedule->monthly, SCHEDULER_MONTHLY_RECURRENT)) && 
				wndOk;
		case scheduleCalendar:
			return schedulerCheckCalendarMonths(schedule) &&
				((ES_BIT_IS_SET(schedule->calMonths, SCHEDULER_CALENDAR_RECURRENT) && intOk) ||
					!ES_BIT_IS_SET(schedule->calMonths, SCHEDULER_CALENDAR_RECURRENT)) && 
				wndOk;			
		default:
			return FALSE;
		}
	}

	return FALSE;
}
