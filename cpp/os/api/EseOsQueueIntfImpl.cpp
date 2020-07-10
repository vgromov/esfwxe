#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/os/EseQueue.h>

#include "EseOsQueueIntf.h"
#include "EseOsQueueIntfImpl.h"
//----------------------------------------------------------------------------------------------

EseOsQueueIntfImpl::EseOsQueueIntfImpl(size_t length, size_t eltSize) ESE_NOTHROW :
m_len(length),
m_eltSize(eltSize)
{
  ESE_ASSERT(m_len);
  ESE_ASSERT(m_eltSize);
  
  m_queue.init(
    m_len,
    m_eltSize
  );
}
//----------------------------------------------------------------------------------------------

EseOsQueueIntfImpl::~EseOsQueueIntfImpl() ESE_NOTHROW
{
  m_queue.uninit();
}
//----------------------------------------------------------------------------------------------

void EseOsQueueIntfImpl::destroy() ESE_NOTHROW
{
  delete this;
}
//----------------------------------------------------------------------------------------------

bool EseOsQueueIntfImpl::isOk() const ESE_NOTHROW
{
  return m_queue.isOk();
}
//----------------------------------------------------------------------------------------------

size_t EseOsQueueIntfImpl::countGet() const ESE_NOTHROW
{
  return m_queue.countGet();
}
//----------------------------------------------------------------------------------------------

size_t EseOsQueueIntfImpl::freeCountGet() const ESE_NOTHROW
{
  return m_queue.freeCountGet();
}
//----------------------------------------------------------------------------------------------

bool EseOsQueueIntfImpl::isEmpty() const ESE_NOTHROW
{
  return m_queue.isEmpty();
}
//----------------------------------------------------------------------------------------------

bool EseOsQueueIntfImpl::isFull() const ESE_NOTHROW
{
  return m_queue.isFull();
}
//----------------------------------------------------------------------------------------------

void EseOsQueueIntfImpl::reset() ESE_NOTHROW
{
  m_queue.reset();
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::pushBack(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.pushBack(
    eltData,
    tmo,
    false
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::pushFront(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.pushFront(
    eltData,
    tmo,
    false
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::popFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.popFront(
    eltData,
    tmo,
    false
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::peekFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.peekFront(
    eltData,
    tmo,
    false
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::pushBackFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.pushBackFromISR(
    eltData,
    shouldYield
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::pushFrontFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.pushFrontFromISR(
    eltData,
    shouldYield
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::popFrontFromISR(void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.popFrontFromISR(
    eltData,
    shouldYield
  );
}
//----------------------------------------------------------------------------------------------

rtosStatus EseOsQueueIntfImpl::peekFrontFromISR(void* eltData, size_t eltSize) ESE_NOTHROW
{
  ESE_ASSERT(eltSize == m_eltSize);
  return m_queue.peekFrontFromISR(
    eltData
  );
}
//----------------------------------------------------------------------------------------------
