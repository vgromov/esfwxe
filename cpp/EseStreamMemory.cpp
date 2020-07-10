#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/utils.h>
#include <esfwxe/ese_assert.h>

#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/concept/EseStreamIntf.h>
#include <esfwxe/cpp/concept/EseStream.h>

#include <algorithm>
#include "EseStreamMemory.h"
//----------------------------------------------------------------------------------------------

EseStreamMemory::EseStreamMemory(esU8* mem, size_t cnt, bool lockable) ESE_NOTHROW :
m_mx(NULL),
m_start(mem),
m_end(mem+cnt),
m_pos(mem)
{
  if( lockable )
  {
    m_mx = new EseMutex;
    ESE_ASSERT(m_mx);
    
    m_mx->init();
  }
}
//----------------------------------------------------------------------------------------------

EseStreamMemory::~EseStreamMemory() ESE_NOTHROW
{
  close();
  if( m_mx )
  {
    m_mx->uninit();

    delete m_mx;
    m_mx = NULL;
  }
}
//----------------------------------------------------------------------------------------------

void EseStreamMemory::destroy() ESE_NOTHROW
{
  delete this;
}
//----------------------------------------------------------------------------------------------

rtosStatus EseStreamMemory::lock(esU32 tmo /*= rtosMaxDelay*/) ESE_NOTHROW
{
  if( m_mx )
    return m_mx->lock(tmo);
  
  return rtosOK;
}
//----------------------------------------------------------------------------------------------

void EseStreamMemory::unlock() ESE_NOTHROW
{
  if( m_mx )
    m_mx->unlock();
}
//----------------------------------------------------------------------------------------------

size_t EseStreamMemory::posGet() const ESE_NOTHROW
{
  if( isOpen() )
    return m_pos-m_start; 

  return EseStream::npos;
}
//----------------------------------------------------------------------------------------------

size_t EseStreamMemory::posSet(ptrdiff_t offs, EseStream::PosMode mode /*= EseStream::posFromStart*/ ) ESE_NOTHROW
{
  if( isOpen() )
  {
    switch(mode)
    {
    case EseStream::posFromStart:  // offset is counting from start
      if( offs >= 0 && offs < sizeGet() )
      {
        m_pos = m_start+offs;
        return m_pos-m_start;
      }
      break;
    case EseStream::posFromEnd:
      if( offs >= 0 && offs < sizeGet() )
      {
        m_pos = m_end-1-offs;
        return m_pos-m_start;
      }
      break;
    case EseStream::posRelative:
      if( !isEos() && 
          (
            (offs < 0 && std::abs(offs) <= (m_pos-m_start)) ||
            (offs >= 0 && offs+(m_pos-m_start) < sizeGet()) 
          ) 
        )
      {
        m_pos += offs;
        return m_pos-m_start;
      }
      break;
    }
  }

  return EseStream::npos;
}
//----------------------------------------------------------------------------------------------

size_t EseStreamMemory::read(esU8* data, size_t len) ESE_NOTHROW
{
  if( isOpen() && data && len && !isEos() )
  {
    esU32 r = std::min(len, sizeGet());
    std::memcpy(data, m_pos, r);
    m_pos += r;
    
    return r;
  }

  return 0;
}
//----------------------------------------------------------------------------------------------

size_t EseStreamMemory::write(const esU8* data, size_t len) ESE_NOTHROW
{
  if( isOpen() && data && len && !isEos() )
  {
    esU32 w = std::min(len, sizeGet());
    std::memcpy(m_pos, data, w);
    m_pos += w;
    
    return w;
  }
  
  return 0;
}
//----------------------------------------------------------------------------------------------
