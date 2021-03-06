#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/EseException.h>

#include <stdlib.h>

EseException::EseException(const EseException& src) ESE_NOTHROW :
m_facility(src.m_facility),
m_error(src.m_error)
{}

EseException::EseException(int err, EseException::Facility facility) ESE_NOTHROW :
m_facility(facility),
m_error(err)
{}
  
void EseException::Throw(int err, EseException::Facility facility)
{
#ifdef USE_CPP_EXCEPTIONS
  throw EseException(err, facility);
#else
  abort();  
#endif
}

EseException& EseException::operator=(const EseException& src) ESE_NOTHROW
{
  m_facility = src.m_facility;
  m_error = src.m_error;
  
  return *this;
}
