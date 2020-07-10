#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>

// FreeRTOS config dependencies
#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/concept/EseLockable.h>
#include <esfwxe/cpp/os/EseQueue.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseTask.h>
#include <esfwxe/cpp/os/EseTimer.h>
#include <esfwxe/cpp/os/EseKernel.h>

#include "EseOsCriticalSectionIntf.h"
#include "EseOsCriticalSectionIntfImpl.h"

#include "EseOsQueueIntf.h"
#include "EseOsQueueIntfImpl.h"

#include "EseOsTimerIntf.h"
#include "EseOsTimerIntfImpl.h"

#include "EseOsTaskIntf.h"
#include "EseOsTaskIntfImpl.h"

#include "EseOsIntf.h"
#include "EseOsIntfImpl.h"
//----------------------------------------------------------------------------------------------

EseOsIntf& EseOsIntfImpl::instanceGet() ESE_NOTHROW
{
  static EseOsIntfImpl s_inst;
  return s_inst;
}
//----------------------------------------------------------------------------------------------

uint32_t EseOsIntfImpl::sysTickGet() const ESE_NOTHROW
{
  return EseKernel::sysTickGet();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::sysTickInc(uint32_t ms) const ESE_NOTHROW
{
  EseKernel::sysTickInc(ms);
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::suspend() const ESE_NOTHROW
{
  EseKernel::suspend();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::resume() const ESE_NOTHROW
{
  EseKernel::resume();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::idleSuspend() const ESE_NOTHROW
{
  EseKernel::idleSuspend();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::idleResume() const ESE_NOTHROW
{
  EseKernel::idleResume();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::timerDaemonSuspend() const ESE_NOTHROW
{
  EseTimer::daemonSuspend();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::timerDaemonResume() const ESE_NOTHROW
{
  EseTimer::daemonResume();
}
//----------------------------------------------------------------------------------------------

uint32_t EseOsIntfImpl::taskTicksGet() const ESE_NOTHROW
{
  return EseTask::tickCountGet();
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::sleep(uint32_t tmo) const ESE_NOTHROW
{
  EseTask::sleep(tmo);
}
//----------------------------------------------------------------------------------------------

void EseOsIntfImpl::sleepUntil(uint32_t& prevTicks, uint32_t tmo) const ESE_NOTHROW
{
  EseTask::sleepUntil(
    prevTicks,
    tmo
  );
}
//----------------------------------------------------------------------------------------------

EseOsCriticalSectionIntf* EseOsIntfImpl::criticalSectionCreate() const ESE_NOTHROW
{
  return new EseOsCriticalSectionIntfImpl;
}
//----------------------------------------------------------------------------------------------

EseOsQueueIntf* EseOsIntfImpl::queueCreate(size_t length, size_t eltSize) const ESE_NOTHROW
{
  ESE_ASSERT(length);
  ESE_ASSERT(eltSize);
  
  return new EseOsQueueIntfImpl(
    length, 
    eltSize
  );
}
//----------------------------------------------------------------------------------------------

EseOsTimerIntf* EseOsIntfImpl::timerCreate(int id, EseOsTimerIntf::WorkerT worker /*= NULL*/, void* data /*= NULL*/) const ESE_NOTHROW
{
  return new EseOsTimerIntfImpl(
    id,
    worker,
    data
  );
}
//----------------------------------------------------------------------------------------------

EseOsTaskIntf* EseOsIntfImpl::taskCreate(int id, EseOsTaskIntf::WorkerT worker /*= NULL*/, EseOsTaskIntf::StopCheckT stopCheck /*= NULL*/, void* data /*= NULL*/) const ESE_NOTHROW
{
  return new EseOsTaskIntfImpl(
    id,
    worker,
    stopCheck,
    data
  );
}
//----------------------------------------------------------------------------------------------
