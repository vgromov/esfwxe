#ifndef _ese_lockable_h_
#define _ese_lockable_h_

#include "esfwxe/cpp/os/EseOsDefs.h"
#include "esfwxe/cpp/EseException.h"

/// Locally-scoped locker helper for Locable objects
///
template <typename ObjectT>
class EseLocker
{
public:
  EseLocker(ObjectT& obj) :
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
  {
    if( m_locked )
      m_obj.unlock();
  }

private:  
  ObjectT& m_obj;
  bool m_locked;
  
  EseLocker() ESE_NOTHROW ESE_REMOVE_DECL;
  EseLocker(const EseLocker&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseLocker& operator=(const EseLocker&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // _ese_lockable_h_
