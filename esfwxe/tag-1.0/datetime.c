#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <time.h>
#include <common/datetime.h>

// calendar constants & helpers
//

// return true if year is leap one, false otherwise
BOOL dtIsLeapYear(int year)
{
	return (0 == (year % 4) && 0 != (year % 100)) || 0 == (year % 400);
}

// return number of days in month. valid month range is [1..12]
// 0 is returned otherwise.
int dtGetMonthDayCount(int year, int month)
{
	if( 0 < month && 13 > month )
	{
		if( 2 == month )
			return dtIsLeapYear(year) ? 29 : 28;
		else if( month < 8 )
			return (month % 2) ? 31 : 30;
		else
			return (month % 2) ? 30 : 31;
	}
	else
		return 0;
}

// days in year
int dtGetDaysInYear(int year)
{
	return dtIsLeapYear(year) ? 366 : 365;
}

// day of week. return 0 if input values are invalid. otherwise,
// return value from 1 (monday) to 7 (sunday)
//
// month doomsdays lookup table
static const BYTE c_ddays[][2] = {
	{0, 6}, 	//January      0 (in leap year 6)
	{3, 2}, 	//February     3 (in leap year 2)
	{3, 3}, 	//March        3
	{6, 6},		//April        6
	{1, 1},		//May          1
	{4, 4}, 	//June         4
	{6, 6},		//July         6
	{2, 2},		//August       2
	{5, 5}, 	//September    5
	{0, 0}, 	//October      0
	{3, 3}, 	//November     3
	{5, 5}};	//December     5

int dtGetDayOfWeek(int year, int month, int dayOfMonth)
{
	int result = 0;

	if( 0 < month && 13 > month &&
			0 < dayOfMonth && dtGetMonthDayCount(year, month) >= dayOfMonth )
	{
		// dirty implementation of Doomsday algorythm
		int c = 2 * (3 - (year / 100) % 4);
		int y = year % 100;
		y = y + y/4;
		result = (c + y + c_ddays[month-1][dtIsLeapYear(year)] + dayOfMonth) % 7;
		result = result ? result : 7;
	}

	return result;
}

// check datetime validity
BOOL dtIsValid(int year, int month, int dayOfMonth,
	int hr, int mins, int sec, int ms)
{
	return 0 < month && 13 > month &&
			0 < dayOfMonth && dayOfMonth <= dtGetMonthDayCount(year, month) &&
			0 <= hr && 24 > hr &&
			0 <= mins && 60 > mins &&
			0 <= sec && 60 > sec &&
			0 <= ms && 1000 > ms;
}

// date time manipulation
//

// get leap years count between years
static int dtGetLeapYearsCount(int year0, int year1)
{
	int result = 0;
	// always moving from less to greater
	int year = (year0 < year1) ? year0 : year1;
	year1 = (year0 < year1) ? year1 : year0;
	while( year != year1 )
	{
		if( dtIsLeapYear(year) )
			++result;

		++year;
	}

	return result;
}

// return full year days count between years
static __inline int dtGetDaysUpToYear(int year0, int year1)
{
	int delta = 0;
	if( year1 < year0 )
  	delta = year0-year1;
	else
		delta = year1-year0;

	return dtGetLeapYearsCount(year0, year1) + delta * 365;
}

// calculate current year using day count passed since year 0,
// also, return full days count since year0 up to calculated year
static int dtGetYear(int days, int* fullYearDays)
{
	int inc = days < 0 ? -1 : 1;
	int year = days < 0 ? DT_YEAR0-1 : DT_YEAR0;
	int yearDays = dtGetDaysInYear(year);

	days = days < 0 ? -days : days;
	*fullYearDays = inc < 0 ? yearDays : 0;

	days -= yearDays;
	while( (inc > 0 && days >= 0) || (inc < 0 && days > 0) )
	{
		year += inc;
		if( inc > 0 )
			*fullYearDays += yearDays;
		yearDays = dtGetDaysInYear(year);
		if( inc < 0 )
			*fullYearDays += yearDays;
		days -= yearDays;
	}

	return year;
}

// calculate month having days-in-year count
// also return count of full month days
static int dtGetMonth(int year, int days, int* fullMonthDays)
{
	int month = 1;
	int fullDays = dtGetMonthDayCount(year, month);

	days -= fullDays;
	*fullMonthDays = 0;	
	while( days >= 0 && month <= 12 )
	{
		*fullMonthDays += fullDays;
		++month;
		fullDays = dtGetMonthDayCount(year, month);
		days -= fullDays;
	}

	return month;
}

// extract just time part
void dtExtractTime(const DATETIME* dt,
	int* hr, int* mins, int* sec,	int* ms)
{
	DATETIME tmp = (*dt < 0) ? -(*dt) : *dt; // work with positive values
	if( ms )
		*ms = tmp % IMMEDIATE_INT64(1000);
	tmp /= IMMEDIATE_INT64(1000);    			// now tmp is datetime in seconds

	if( sec )
		*sec = tmp % IMMEDIATE_INT64(60);
	tmp /= IMMEDIATE_INT64(60);  					// now tmp is datetime in minutes

	*mins = tmp % IMMEDIATE_INT64(60);
	tmp /= IMMEDIATE_INT64(60);  					// now tmp is datetime in hours

	*hr = tmp % IMMEDIATE_INT64(24);
}

// decompose datetime
void dtDecomposeDateTime(const DATETIME* dt,
	int* year, int* dayOfYear, int* month, int* dayOfMonth,
	int* hr, int* mins, int* sec,
	int* ms)
{
	DATETIME tmp = *dt;
	int dummy = (int)(tmp / IMMEDIATE_INT64(86400000));   	// now dummy contains day count
	*year = dtGetYear(dummy, &dummy); // now dummy contains full year days count
	if( tmp < 0 )
		tmp += ((INT64)dummy) * IMMEDIATE_INT64(86400000); // make the rest of calculations on positive values
	else
		tmp -= ((INT64)dummy) * IMMEDIATE_INT64(86400000);

	if( ms )
		*ms = tmp % IMMEDIATE_INT64(1000);
	tmp /= IMMEDIATE_INT64(1000);    			// now tmp is datetime in seconds

	if( sec )
		*sec = tmp % IMMEDIATE_INT64(60);
	tmp /= IMMEDIATE_INT64(60);  					// now tmp is datetime in minutes

	*mins = tmp % IMMEDIATE_INT64(60);
	tmp /= IMMEDIATE_INT64(60);  					// now tmp is datetime in hours

	*hr = tmp % IMMEDIATE_INT64(24);
	tmp /= IMMEDIATE_INT64(24);						// now tmp is full days within year
	if( dayOfYear )
		*dayOfYear = (int)(tmp);

	// find month
	*month = dtGetMonth(*year, (int)(tmp), &dummy);
	tmp -= dummy;    				// now tmp is day of month
	*dayOfMonth = (int)(tmp+1);
}

// calculate full month days before the specified month
static int dtGetDaysUpToMonth(int year, int month)
{
	int result = 0;
	while(--month > 0)
		result += dtGetMonthDayCount(year, month);

	return result;
}

// compose datetime. perform range checks
BOOL dtComposeDateTime(DATETIME* dt,
	int year, int month, int dayOfMonth,
	int hr, int mins, int sec, int ms)
{
	DATETIME result = 0;
	if( dtIsValid(year, month, dayOfMonth, hr, mins, sec, ms) )
	{
		// first, set year & month in days
		int fullYearDays = dtGetDaysUpToYear(DT_YEAR0, year);
		if( year < DT_YEAR0 )
			fullYearDays = -fullYearDays;
	
		result += (INT64)dtGetDaysUpToMonth(year, month) + (INT64)(dayOfMonth-1);
		result *= IMMEDIATE_INT64(24);
		result += (INT64)hr;
		result *= IMMEDIATE_INT64(60);
		result += (INT64)mins;
		result *= IMMEDIATE_INT64(60);
		result += (INT64)sec;
		result *= IMMEDIATE_INT64(1000);
		result += (INT64)ms;
	
		result += ((INT64)fullYearDays) * IMMEDIATE_INT64(86400000);
	
		*dt = result;
		return TRUE;
	}

	return FALSE;
}

// compose DATETIME consisting only of time component
BOOL dtComposeTime(DATETIME* dt, int hr, int mins, int sec, int ms)
{
	return dtComposeDateTime(dt, DT_YEAR0, 1, 1, hr, mins, sec, ms);
}

// compose DATETIME consisting only of date component 
BOOL dtComposeDate(DATETIME* dt, int year, int month, int dayOfMonth)
{
	return dtComposeDateTime(dt, year, month, dayOfMonth, 0, 0, 0, 0);
}

// conversion from|to C standard tm structure
void DATETIME2tm(const DATETIME* dt, struct tm* val)
{
	if(dt && val)
	{
		dtDecomposeDateTime(dt, &val->tm_year, &val->tm_yday,
			&val->tm_mon, &val->tm_mday,
			&val->tm_hour, &val->tm_min, &val->tm_sec, 0);
		val->tm_isdst = -1;
		val->tm_wday = dtGetDayOfWeek(val->tm_year, val->tm_mon,
			val->tm_mday);
		// adjust to standard C values
		//
		// years since 1900
		val->tm_year -= 1900;
		// months 0-11
		--val->tm_mon;
		 // 0-6, Sunday is 0
		if( val->tm_wday == 7 )
      val->tm_wday = 0;
	}
}

void tm2DATETIME(const struct tm* val, DATETIME* dt)
{
	if( val && dt )
		dtComposeDateTime(dt, val->tm_year + 1900, val->tm_mon+1, val->tm_mday,
			val->tm_hour, val->tm_min, val->tm_sec, 0);
}

