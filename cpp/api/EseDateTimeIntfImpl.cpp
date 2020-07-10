#include <esfwxe/target.h>
#include <esfwxe/utils.h>
#include <time.h>
#include <esfwxe/datetime.h>
#pragma hdrstop

#include "EseDateTimeIntf.h"
#include "EseDateTimeIntfImpl.h"
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::isLeapYear(int year) const ESE_NOTHROW
{
  return TRUE == dtIsLeapYear(year);
}
//----------------------------------------------------------------------------------------------

int EseDateTimeIntfImpl::monthDayCountGet(int year, int month) const ESE_NOTHROW
{
  return dtGetMonthDayCount(
    year,
    month
  );
}
//----------------------------------------------------------------------------------------------

int EseDateTimeIntfImpl::daysInYearGet(int year) const ESE_NOTHROW
{
  return dtGetDaysInYear(year);
}
//----------------------------------------------------------------------------------------------

int EseDateTimeIntfImpl::dayOfWeekGet(int year, int month, int dayOfMonth) const ESE_NOTHROW
{
  return dtGetDayOfWeek(
    year,
    month,
    dayOfMonth
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::isValid() const ESE_NOTHROW
{
  int year;
  int month; 
  int dayOfMonth; 
  int hr; 
  int mins;
  int sec;
  int ms;
  
  decompose(
    year, 
    month, 
    dayOfMonth, 
    hr, 
    mins, 
    &sec, 
    &ms
  );
  
  return isValid(
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    sec,
    ms  
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::isValid(esDT dt) const ESE_NOTHROW
{
  int year;
  int month; 
  int dayOfMonth; 
  int hr; 
  int mins;
  int sec;
  int ms;
  
  dtDecomposeDateTime(
    &dt,
    &year, 
    nullptr,
    &month, 
    &dayOfMonth, 
    &hr, 
    &mins, 
    &sec, 
    &ms
  );
  
  return dtIsValid(
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::isValid(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms /*= 0*/) const ESE_NOTHROW
{
  return dtIsValid(
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

void EseDateTimeIntfImpl::timeExtract(int& hr, int& mins, int* sec /*= nullptr*/, int* ms /*= nullptr*/) const ESE_NOTHROW
{
  dtExtractTime(
    &m_val,
    &hr,
    &mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

void EseDateTimeIntfImpl::dateExtract(int& year, int& month, int& dayOfMonth, int* dayOfYear /*= nullptr*/) const ESE_NOTHROW
{
  int hr;
  int mins;
  
  decompose(
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    nullptr,
    nullptr,
    dayOfYear
  );
}
//----------------------------------------------------------------------------------------------

void EseDateTimeIntfImpl::decompose(int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec /*= nullptr*/, int* ms /*= nullptr*/, int* dayOfYear /*= nullptr*/) const ESE_NOTHROW
{
  dtDecomposeDateTime(
    &m_val,
    &year,
    dayOfYear,
    &month,
    &dayOfMonth,
    &hr,
    &mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

void EseDateTimeIntfImpl::decompose(esDT in, int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec /*= nullptr*/, int* ms /*= nullptr*/, int* dayOfYear /*= nullptr*/) const ESE_NOTHROW
{
  dtDecomposeDateTime(
    &in,
    &year,
    dayOfYear,
    &month,
    &dayOfMonth,
    &hr,
    &mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::compose(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms /*= 0*/) ESE_NOTHROW
{
  return TRUE == dtComposeDateTime(
    &m_val,
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::compose(esDT& out, int year, int month, int dayOfMonth, int hr /*= 0*/, int mins /*= 0*/, int sec /*= 0*/, int ms /*= 0*/) const ESE_NOTHROW
{
  return TRUE == dtComposeDateTime(
    &out,
    year,
    month,
    dayOfMonth,
    hr,
    mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::timeCompose(int hr, int mins, int sec, int ms /*= 0*/) ESE_NOTHROW
{
  return TRUE == dtComposeTime(
    &m_val,
    hr,
    mins,
    sec,
    ms
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::dateCompose(int year, int month, int dayOfMonth) ESE_NOTHROW
{
  return TRUE == dtComposeDate(
    &m_val,
    year,
    month,
    dayOfMonth
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::sameDate(int year, int month, int dayOfMonth) const ESE_NOTHROW
{
  if( !isValid() )
    return false;

  int thisYear;
  int thisMonth;
  int thisDayOfMonth;
  
  dateExtract(
    thisYear, 
    thisMonth, 
    thisDayOfMonth
  );
  
  return year == thisYear && 
    month == thisMonth &&
    dayOfMonth == thisDayOfMonth;
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::sameDate(esDT dt) const ESE_NOTHROW
{
  int otherYear;
  int otherMonth;
  int otherDayOfMonth;
  int otherHr;
  int otherMin;

  decompose(
    dt, 
    otherYear, 
    otherMonth, 
    otherDayOfMonth, 
    otherHr, 
    otherMin
  );
  
  if( 
    !isValid(
      otherYear,
      otherMonth,
      otherDayOfMonth,
      otherHr,
      otherMin,
      0
    ) 
  )
    return false;
  
  return sameDate(
    otherYear,
    otherMonth,
    otherDayOfMonth
  );
}
//----------------------------------------------------------------------------------------------

bool EseDateTimeIntfImpl::sameDate(const EseDateTimeIntf& other) const ESE_NOTHROW
{
  if( !other.isValid() )
    return false;

  int otherYear;
  int otherMonth;
  int otherDayOfMonth;
  
  other.dateExtract(
    otherYear,
    otherMonth,
    otherDayOfMonth
  );

  return sameDate(
    otherYear,
    otherMonth,
    otherDayOfMonth
  );
}
//----------------------------------------------------------------------------------------------
