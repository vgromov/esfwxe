#ifndef _ese_os_queue_intf_impl_h_
#define _ese_os_queue_intf_impl_h_

/// OS queue interface implementation
///
class EseOsQueueIntfImpl : public EseOsQueueIntf
{
protected:
  EseOsQueueIntfImpl(size_t length, size_t eltSize) ESE_NOTHROW ESE_KEEP;

public:
  virtual ~EseOsQueueIntfImpl() ESE_NOTHROW ESE_KEEP;
  
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t lengthGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_len; }
  virtual size_t elementSizeGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_eltSize; }
  virtual size_t countGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t freeCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isEmpty() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isFull() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void reset() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus pushBack(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus pushFront(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus popFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus peekFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus pushBackFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus pushFrontFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus popFrontFromISR(void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus peekFrontFromISR(void* eltData, size_t eltSize) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

protected:
  EseQueueBase m_queue;
  size_t m_len;
  size_t m_eltSize;

  ESE_NODEFAULT_CTOR(EseOsQueueIntfImpl);
  ESE_NONCOPYABLE(EseOsQueueIntfImpl);
  
  friend class EseOsIntfImpl;
};

#endif //< _ese_os_queue_intf_impl_h_
