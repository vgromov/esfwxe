#ifndef _ese_exception_h_
#define _ese_exception_h_

class EseException
{
public:
  enum Facility 
  {
    facilityInvalid,
    facilityUnknown,
    facilityOS,
    facilityFS,
    facilityApplication
  };
  
public:
  EseException(int err = 0, EseException::Facility facility = facilityInvalid) ESE_NOTHROW;
  EseException(const EseException& src) ESE_NOTHROW;

  static void Throw(int err = rtosError, EseException::Facility facility = EseException::facilityOS);
  
  bool isOk() const ESE_NOTHROW { return m_facility != facilityInvalid; }

  Facility facilityGet() const ESE_NOTHROW { return m_facility; }
  int errorGet() const ESE_NOTHROW { return m_error; }

  EseException& operator=(const EseException& src) ESE_NOTHROW;

protected:  
  Facility m_facility;
  int m_error;
};

#endif // _ese_exception_h_
