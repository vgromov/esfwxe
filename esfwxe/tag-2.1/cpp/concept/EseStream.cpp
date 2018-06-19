#include "stdafx.h"
#pragma hdrstop

#include "EseStream.h"

EseStream::EseStream() ESE_NOTHROW :
m_open(false)
{}

bool EseStream::open() ESE_NOTHROW
{
  if( !m_open )
    m_open = doOpen();
    
  return m_open;
}

void EseStream::close() ESE_NOTHROW
{
  if( isOpen() )
  {
    doClose();
    m_open = false;
  }
}
