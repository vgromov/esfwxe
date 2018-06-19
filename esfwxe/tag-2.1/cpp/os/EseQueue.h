#ifndef _ese_queue_h_
#define _ese_queue_h_

/// RTOS queue concept implementation.
///
class EseQueueBase
{
protected:
  EseQueueBase() ESE_NOTHROW;
  
public:
  ~EseQueueBase() ESE_NOTHROW;
  
  /// Return true if queue is created OK
  bool isOk() const ESE_NOTHROW { return 0 != m_h; }
  
  /// Return count of elements in queue
  size_t countGet() const ESE_NOTHROW;
  
  /// Return free elements available in queue
  size_t freeCountGet() const ESE_NOTHROW;
  
  /// Return true if queue is empty
  bool isEmpty() const ESE_NOTHROW { return 0 == countGet(); }
  
  /// Return true if queue is full
  bool isFull() const ESE_NOTHROW { return 0 == freeCountGet(); }
  
  /// Reset queue
  void reset() ESE_NOTHROW;

  /// Uninitialize queue container
  void uninit() ESE_NOTHROW;

protected:
  /// Push an element into queue's back
  rtosStatus pushBack(const void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW;
  
  /// Push an element into queue's front
  rtosStatus pushFront(const void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW;

  /// Pop an element from queue's front
  rtosStatus popFront(void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW;
  
  /// Peek an element at the fron of queue
  rtosStatus peekFront(void* item, uint32_t tmo, bool useTicks) ESE_NOTHROW;

  /// Push an element into queue's back from ISR
  rtosStatus pushBackFromISR(const void* item, volatile bool& shouldYield) ESE_NOTHROW;
  
  /// Push an element into queue's front from ISR
  rtosStatus pushFrontFromISR(const void* item, volatile bool& shouldYield) ESE_NOTHROW;
  
  /// Pop an element from queue's front from ISR
  rtosStatus popFrontFromISR(void* item, volatile bool& shouldYield) ESE_NOTHROW;
  
  /// Peek an element at the fron of queue from ISR
  rtosStatus peekFrontFromISR(void* item) ESE_NOTHROW;
  
  /// Initialize queue container
  void init(size_t len, size_t itemSize) ESE_NOTHROW;
  
public:
  void* m_h;
  
private:
  EseQueueBase(const EseQueueBase&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseQueueBase& operator=(const EseQueueBase&) ESE_NOTHROW ESE_REMOVE_DECL;
};

/// Template specialization for RTOS queue
///
template <typename ItemT>
class EseQueue : public EseQueueBase
{
public:
  /// Push an element into queue's back
  inline rtosStatus pushBack(const ItemT& item, uint32_t tmo, bool useTicks = false) ESE_NOTHROW
  {
    return EseQueueBase::pushBack(reinterpret_cast<const void*>(&item), tmo, useTicks);
  }
  
  /// Push an element into queue's front
  inline rtosStatus pushFront(const ItemT& item, uint32_t tmo, bool useTicks = false) ESE_NOTHROW
  {
    return EseQueueBase::pushFront(reinterpret_cast<const void*>(&item), tmo, useTicks);
  }

  /// Pop an element from queue's front
  inline rtosStatus popFront(ItemT& item, uint32_t tmo, bool useTicks = false) ESE_NOTHROW
  {
    return EseQueueBase::popFront(reinterpret_cast<void*>(&item), tmo, useTicks);
  }
  
  /// Peek an element at the fron of queue
  inline rtosStatus peekFront(ItemT& item, uint32_t tmo, bool useTicks = false) ESE_NOTHROW
  {
    return EseQueueBase::peekFront(reinterpret_cast<void*>(&item), tmo, useTicks);
  }

  /// Push an element into queue's back from ISR
  inline rtosStatus pushBackFromISR(const ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return EseQueueBase::pushBackFromISR(reinterpret_cast<const void*>(&item), shouldYield);
  }
  
  /// Push an element into queue's front from ISR
  inline rtosStatus pushFrontFromISR(const ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return EseQueueBase::pushFrontFromISR(reinterpret_cast<const void*>(&item), shouldYield);
  }
  
  /// Pop an element from queue's front from ISR
  inline rtosStatus popFrontFromISR(ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return EseQueueBase::popFrontFromISR(reinterpret_cast<void*>(&item), shouldYield);
  }
  
  /// Peek an element at the fron of queue from ISR
  inline rtosStatus peekFrontFromISR(ItemT& item) ESE_NOTHROW
  {
    return EseQueueBase::peekFrontFromISR(reinterpret_cast<void*>(&item));
  }
  
  /// Initialize queue container
  inline void init(size_t len) ESE_NOTHROW
  {
    EseQueueBase::init(len, sizeof(ItemT));
  }
};

#endif // _ese_queue_h_
