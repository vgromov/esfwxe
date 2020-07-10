#ifndef _ese_date_time_intf_h_
#define _ese_date_time_intf_h_

/// Date Time services interface
///
class ESE_ABSTRACT EseDateTimeIntf
{
public:
  /// Dynamic object deallocation
  virtual void destroy() ESE_NOTHROW = 0;
  
  /// Contained date time value access
  virtual esDT valueGet() const ESE_NOTHROW = 0;
  virtual void valueSet(esDT dt) ESE_NOTHROW = 0;
  
  /// Return true if year is leap one, false otherwise
  virtual bool isLeapYear(int year) const ESE_NOTHROW = 0;
  
  /// Return number of days in month. valid month range is [1..12]
  /// 0 is returned otherwise.
  ///
  virtual int monthDayCountGet(int year, int month) const ESE_NOTHROW = 0;
  
  /// Return Days in year
  virtual int daysInYearGet(int year) const ESE_NOTHROW = 0;
  
  /// Day of week calculator. 
  /// Return 0 if input values are invalid. otherwise,
  /// return value from 1 (monday) to 7 (sunday)
  ///
  virtual int dayOfWeekGet(int year, int month, int dayOfMonth) const ESE_NOTHROW = 0;
  
  /// Check Date Time components validity
  virtual bool isValid() const ESE_NOTHROW = 0;
  virtual bool isValid(esDT dt) const ESE_NOTHROW = 0;
  virtual bool isValid(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms = 0) const ESE_NOTHROW = 0;

  /// Date Time manipulation
  ///
  
  /// Extract just time part. sec and ms are optional, if set to NULL
  virtual void timeExtract(int& hr, int& mins, int* sec = nullptr, int* ms = nullptr) const ESE_NOTHROW = 0;
  
  /// Extract just date part
  virtual void dateExtract(int& year, int& month, int& dayOfMonth, int* dayOfYear = nullptr) const ESE_NOTHROW = 0;
  
  /// Decompose date time. dayOfYear, ms, sec are optional, if set to NULL
  virtual void decompose(int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec = nullptr, int* ms = nullptr, int* dayOfYear = nullptr) const ESE_NOTHROW = 0;
  virtual void decompose(esDT in, int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec = nullptr, int* ms = nullptr, int* dayOfYear = nullptr) const ESE_NOTHROW = 0;
  
  /// Compose datetime, perform range checks
  virtual bool compose(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms = 0) ESE_NOTHROW = 0;
  virtual bool compose(esDT& out, int year, int month, int dayOfMonth, int hr = 0, int mins = 0, int sec = 0, int ms = 0) const ESE_NOTHROW = 0;
  
  /// Compose esDT consisting only of time component
  virtual bool timeCompose(int hr, int mins, int sec, int ms = 0) ESE_NOTHROW = 0;
  
  /// Compose esDT consisting only of date component 
  virtual bool dateCompose(int year, int month, int dayOfMonth) ESE_NOTHROW = 0;
  
  /// Comparison
  ///
  
  /// Return true if date is the same
  virtual bool sameDate(int year, int month, int dayOfMonth) const ESE_NOTHROW = 0;

  /// Return true if date is the same
  virtual bool sameDate(esDT dt) const ESE_NOTHROW = 0;
  
  /// Return true if date is the same
  virtual bool sameDate(const EseDateTimeIntf& other) const ESE_NOTHROW = 0;
};

#endif //< _ese_date_time_intf_h_
