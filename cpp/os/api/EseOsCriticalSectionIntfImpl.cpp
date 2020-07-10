#include <esfwxe/target.h>
#include <esfwxe/type.h>

// FreeRTOS config dependencies
#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/os/EseMutex.h>

#include "EseOsCriticalSectionIntf.h"
#include "EseOsCriticalSectionIntfImpl.h"
//-----------------------------------------------------------------------------

EseOsCriticalSectionIntfImpl::EseOsCriticalSectionIntfImpl() ESE_NOTHROW
{
  m_mx.init();
}
//-----------------------------------------------------------------------------

EseOsCriticalSectionIntfImpl::~EseOsCriticalSectionIntfImpl() ESE_NOTHROW
{
  m_mx.uninit();
}
//-----------------------------------------------------------------------------

void EseOsCriticalSectionIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//-----------------------------------------------------------------------------

bool EseOsCriticalSectionIntfImpl::isOk() const ESE_NOTHROW
{
  return m_mx.isOk();
}
//-----------------------------------------------------------------------------

rtosStatus EseOsCriticalSectionIntfImpl::lock(esU32 tmo) ESE_NOTHROW
{
  return m_mx.lock(tmo);
}
//-----------------------------------------------------------------------------

rtosStatus EseOsCriticalSectionIntfImpl::unlock() ESE_NOTHROW
{
  return m_mx.unlock();
}
//-----------------------------------------------------------------------------
