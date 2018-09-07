#include "stdafx.h"
#pragma hdrstop

#include <algorithm>
#include "EseStreamMemory.h"
//----------------------------------------------------------------------------------------------

EseStreamMemory::EseStreamMemory(esU8* mem, size_t cnt) ESE_NOTHROW :
m_start(mem),
m_end(mem+cnt),
m_pos(mem)
{}
//----------------------------------------------------------------------------------------------

EseStreamMemory::~EseStreamMemory() ESE_NOTHROW
{
  close();
}
//----------------------------------------------------------------------------------------------

void EseStreamMemory::destroy() ESE_NOTHROW
{
  delete this;
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
