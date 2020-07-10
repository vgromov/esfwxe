#ifndef _ese_date_time_intf_impl_h_
#define _ese_date_time_intf_impl_h_

/// Date Time interface implementation
///
class EseDateTimeIntfImpl : public EseDateTimeIntf
{
public:
  EseDateTimeIntfImpl(esDT dt = 0) ESE_NOTHROW ESE_KEEP : m_val(dt) {}
  virtual ~EseDateTimeIntfImpl() ESE_NOTHROW ESE_KEEP {}
  
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { delete this; }
  virtual esDT valueGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_val; }
  virtual void valueSet(esDT dt) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { m_val = dt; }
  virtual bool isLeapYear(int year) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int monthDayCountGet(int year, int month) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int daysInYearGet(int year) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int dayOfWeekGet(int year, int month, int dayOfMonth) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isValid() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isValid(esDT dt) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isValid(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms = 0) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void timeExtract(int& hr, int& mins, int* sec = nullptr, int* ms = nullptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void dateExtract(int& year, int& month, int& dayOfMonth, int* dayOfYear = nullptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void decompose(int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec = nullptr, int* ms = nullptr, int* dayOfYear = nullptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void decompose(esDT in, int& year, int& month, int& dayOfMonth, int& hr, int& mins, int* sec = nullptr, int* ms = nullptr, int* dayOfYear = nullptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool compose(int year, int month, int dayOfMonth, int hr, int mins, int sec, int ms = 0) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool compose(esDT& out, int year, int month, int dayOfMonth, int hr = 0, int mins = 0, int sec = 0, int ms = 0) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool timeCompose(int hr, int mins, int sec, int ms = 0) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool dateCompose(int year, int month, int dayOfMonth) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool sameDate(int year, int month, int dayOfMonth) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool sameDate(esDT dt) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool sameDate(const EseDateTimeIntf& other) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

protected:
  esDT m_val;

  ESE_NONCOPYABLE(EseDateTimeIntfImpl);
};

#endif //< _ese_date_time_intf_impl_h_
