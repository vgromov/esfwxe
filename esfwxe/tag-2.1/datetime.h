#ifndef _date_time_h_
#define _date_time_h_

#include <time.h>

#ifdef __cplusplus
	extern "C" {
#endif

// date time utility functions. esDT type granularity is 1ms.
//
#define DT_YEAR0	2000

// calendar constants & helpers
//
// return true if year is leap one, false otherwise
esBL dtIsLeapYear(int year);
// return number of days in month. valid month range is [1..12]
// 0 is returned otherwise.
int dtGetMonthDayCount(int year, int month);
// days in year
int dtGetDaysInYear(int year);
// day of week. return 0 if input values are invalid. otherwise,
// return value from 1 (monday) to 7 (sunday)
int dtGetDayOfWeek(int year, int month, int dayOfMonth);
// check datetime components validity
esBL dtIsValid(int year, int month, int dayOfMonth,
	int hr, int mins, int sec, int ms);

// date time manipulation
//
// extract just time part. sec and ms are optional, if set to 0
void dtExtractTime(const esDT* dt,
	int* hr, int* mins, int* sec,	int* ms);
// decompose datetime. dayOfYear, ms, sec are optional, if set to 0
void dtDecomposeDateTime(const esDT* dt,
	int* year, int* dayOfYear, int* month, int* dayOfMonth,
	int* hr, int* mins, int* sec,
	int* ms);
// compose datetime, perform range checks
esBL dtComposeDateTime(esDT* dt,
	int year, int month, int dayOfMonth,
	int hr, int mins, int sec, int ms);
// compose esDT consisting only of time component
esBL dtComposeTime(esDT* dt, int hr, int mins, int sec, int ms);
// compose esDT consisting only of date component 
esBL dtComposeDate(esDT* dt, int year, int month, int dayOfMonth);

// conversion from|to C standard tm structure, as well as interconvertions
//
//struct tm;
void esDT2tm(const esDT* dt, struct tm *val);
void tm2esDT(const struct tm *val, esDT* dt);

#ifdef __cplusplus
	}
#endif

#endif // _date_time_h_
