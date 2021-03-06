#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseStreamIntf.h>

#include "EseStream.h"
//----------------------------------------------------------------------------------------------

EseStream::EseStream() ESE_NOTHROW :
m_open(false)
{}
//----------------------------------------------------------------------------------------------

EseStream::~EseStream() ESE_NOTHROW
{}
//----------------------------------------------------------------------------------------------

size_t EseStream::invalidPos() const ESE_NOTHROW
{
  return npos;
}
//----------------------------------------------------------------------------------------------

bool EseStream::isOpen() const ESE_NOTHROW
{ 
  return m_open; 
}
//----------------------------------------------------------------------------------------------

bool EseStream::isEos() const ESE_NOTHROW
{ 
  size_t pos = posGet(); 
  return npos != pos && 
    pos >= sizeGet(); 
}
//----------------------------------------------------------------------------------------------

bool EseStream::open() ESE_NOTHROW
{
  if( !m_open )
    m_open = doOpen();
    
  return m_open;
}
//----------------------------------------------------------------------------------------------

void EseStream::close() ESE_NOTHROW
{
  if( isOpen() )
  {
    doClose();
    m_open = false;
  }
}
//----------------------------------------------------------------------------------------------
