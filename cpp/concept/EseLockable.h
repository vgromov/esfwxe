#ifndef _ese_lockable_h_
#define _ese_lockable_h_

#include <esfwxe/cpp/os/EseOsDefs.h>

#ifdef USE_CPP_EXCEPTIONS 
# include <esfwxe/cpp/EseException.h>
#endif

/// Locally-scoped locker helper for Locable objects
///
template <typename ObjectT>
class EseLocker
{
public:
  EseLocker(ObjectT& obj) 
#ifdef USE_CPP_EXCEPTIONS
    ESE_NOTHROW
#endif
  :
  m_obj(obj),
  m_locked(false)
  {
    rtosStatus stat = m_obj.lock();
    m_locked = (rtosOK == stat);
#ifdef USE_CPP_EXCEPTIONS    
    if( rtosOK != stat )
      EseException::Throw(stat);
#endif      
  }
  
  ~EseLocker()
#ifdef USE_CPP_EXCEPTIONS
    ESE_NOTHROW
#endif
  {
    if( m_locked )
      m_obj.unlock();
  }

private:  
  ObjectT& m_obj;
  bool m_locked;
  
  ESE_NODEFAULT_CTOR(EseLocker);
  ESE_NONCOPYABLE(EseLocker);
};

#endif // _ese_lockable_h_
