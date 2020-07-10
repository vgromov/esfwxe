#include <esfwxe/target.h>
#include <esfwxe/type.h>

// FreeRTOS config dependencies
#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/os/EseTimer.h>

#include "EseOsTimerIntf.h"
#include "EseOsTimerIntfImpl.h"
//----------------------------------------------------------------------------------------------

EseOsTimerIntfImpl::Impl::Impl(EseOsTimerIntfImpl& owner, int id) ESE_NOTHROW :
m_owner(owner),
m_id(id)
{}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::Impl::onStart() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageStarted
    );
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::Impl::onStop() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageStopped
    );
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::Impl::onExpire() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageExpired
    );
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

EseOsTimerIntfImpl::EseOsTimerIntfImpl(int id, EseOsTimerIntf::WorkerT worker, void* data) ESE_NOTHROW :
m_impl(
  *this, 
  id
),
m_worker(worker),
m_data(data)
{}
//----------------------------------------------------------------------------------------------

EseOsTimerIntfImpl::~EseOsTimerIntfImpl() ESE_NOTHROW
{
  m_impl.stop();
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//----------------------------------------------------------------------------------------------

bool EseOsTimerIntfImpl::isOk() const ESE_NOTHROW
{
  return m_impl.isOk();
}
//----------------------------------------------------------------------------------------------

uint32_t EseOsTimerIntfImpl::tmoGet() const ESE_NOTHROW
{
  return m_impl.tmoGet();
}
//----------------------------------------------------------------------------------------------

bool EseOsTimerIntfImpl::isContinuous() const ESE_NOTHROW
{
  return m_impl.isContinuous();
}
//----------------------------------------------------------------------------------------------

bool EseOsTimerIntfImpl::isRunning() const ESE_NOTHROW
{
  return m_impl.isRunning();
}
//----------------------------------------------------------------------------------------------

bool EseOsTimerIntfImpl::start(uint32_t tmo, bool continuous, void* data, EseOsTimerIntf::WorkerT worker) ESE_NOTHROW
{
  stop();
  
  if( worker )
    m_worker = worker;
  
  if( data )
    m_data = data;

  return m_impl.start(
    tmo,
    continuous
  );
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::reset(uint32_t tmo) ESE_NOTHROW
{
  m_impl.reset(tmo);
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::resetFromIsr(volatile bool& shouldYield) ESE_NOTHROW
{
  m_impl.resetFromISR(shouldYield);
}
//----------------------------------------------------------------------------------------------

void EseOsTimerIntfImpl::stop() ESE_NOTHROW
{
  if( m_impl.isRunning() )
    m_impl.stop();
}
//----------------------------------------------------------------------------------------------
