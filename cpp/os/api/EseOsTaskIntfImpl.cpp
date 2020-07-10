#include <esfwxe/target.h>
#include <esfwxe/type.h>

#include <cstring>

// FreeRTOS config dependencies
#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseTask.h>

#include "EseOsTaskIntf.h"
#include "EseOsTaskIntfImpl.h"
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

EseOsTaskIntfImpl::Impl::Impl(EseOsTaskIntfImpl& owner) ESE_NOTHROW :
m_owner(owner)
{}
//--------------------------------------------------------------------------------------

bool EseOsTaskIntfImpl::Impl::checkForStopping(uint32_t tmo) ESE_NOTHROW
{
  if(m_owner.m_stopCheck)
    return m_owner.m_stopCheck(
        m_owner,
        m_owner.m_data
      ) || 
      EseTask::checkForStopping(tmo);
  
  return EseTask::checkForStopping(tmo);
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onStopping() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageStopping
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onBeforeSuspend() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageBeforeSuspend
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onAfterSuspend() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageAfterSuspend
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onBeforeResume() ESE_NOTHROW
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageBeforeResume
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onEnter()
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageEntering
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::execute()
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageWorking
    );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::Impl::onExit()
{
  if( m_owner.m_worker )
    m_owner.m_worker(
      m_owner,
      m_owner.m_data,
      stageExiting
    );
}
//--------------------------------------------------------------------------------------

#ifdef DEBUG
const char* EseOsTaskIntfImpl::Impl::dbgNameGet() const ESE_NOTHROW
{
  return m_owner.m_name;
}
#endif
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

EseOsTaskIntfImpl::EseOsTaskIntfImpl(
  int id /*= 0*/, 
  EseOsTaskIntf::WorkerT worker /*= NULL*/, 
  EseOsTaskIntf::StopCheckT stopCheck /*= NULL*/, 
  void* data /*= NULL*/
) ESE_NOTHROW :
m_impl(*this),
m_worker(worker),
m_stopCheck(stopCheck),
m_data(data),
m_id(id)
{
#ifdef DEBUG
  std::memset(
    m_name,
    0,
    sizeof(m_name)
  );
  
  snprintf(
    m_name,
    sizeof(m_name),
    "i%d",
    id
  );
#endif
}
//--------------------------------------------------------------------------------------

EseOsTaskIntfImpl::~EseOsTaskIntfImpl() ESE_NOTHROW
{
  m_impl.stop();
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//--------------------------------------------------------------------------------------

bool EseOsTaskIntfImpl::isOk() const ESE_NOTHROW
{
  return m_impl.isOk();
}
//--------------------------------------------------------------------------------------

uint32_t EseOsTaskIntfImpl::tickCountGet() const ESE_NOTHROW
{
  return EseTask::tickCountGet();
}
//--------------------------------------------------------------------------------------

uint32_t EseOsTaskIntfImpl::priorityGet() const ESE_NOTHROW
{
  return m_impl.priorityGet();
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::prioritySet( uint32_t prio ) ESE_NOTHROW
{
  m_impl.prioritySet(prio);
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::start(uint32_t priority, uint32_t stackDepth, void* data, EseOsTaskIntf::WorkerT worker, EseOsTaskIntf::StopCheckT stopCheck) ESE_NOTHROW
{
  stop();
  
  if( data )
    m_data = data;
  
  if(worker)
    m_worker = worker;
  
  if(stopCheck)
    m_stopCheck = stopCheck;
    
  m_impl.start(
    priority,
    stackDepth
  );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::stop() ESE_NOTHROW
{
  m_impl.stop();
}
//--------------------------------------------------------------------------------------

bool EseOsTaskIntfImpl::completionWait(uint32_t tmo) ESE_NOTHROW
{
  return m_impl.completionWait(tmo);
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::suspend() ESE_NOTHROW
{
  m_impl.suspend();
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::resume() ESE_NOTHROW
{
  m_impl.resume();
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::sleep(uint32_t ms) const ESE_NOTHROW
{
  EseTask::sleep(ms);
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::sleepUntil(uint32_t& prevTicks, uint32_t ms) const ESE_NOTHROW
{
  EseTask::sleepUntil(
    prevTicks,
    ms
  );
}
//--------------------------------------------------------------------------------------

void EseOsTaskIntfImpl::yield() const ESE_NOTHROW
{
  EseTask::yield();
}
//--------------------------------------------------------------------------------------

bool EseOsTaskIntfImpl::checkForStopping(uint32_t tmo /*= 0*/) ESE_NOTHROW
{
  return m_impl.checkForStopping(tmo);
}
//--------------------------------------------------------------------------------------
