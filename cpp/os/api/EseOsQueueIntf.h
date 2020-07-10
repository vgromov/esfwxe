#ifndef _ese_os_queue_intf_h_
#define _ese_os_queue_intf_h_

/// OS queue interface
///
class ESE_ABSTRACT EseOsQueueIntf
{
public:
  /// Dynamic interface object deallocation support
  virtual void destroy() ESE_NOTHROW = 0;

  /// Return true if queue is created OK
  virtual bool isOk() const ESE_NOTHROW = 0;
  
  /// Return queue length
  virtual size_t lengthGet() const ESE_NOTHROW = 0;
  
  /// Return size of queue element
  virtual size_t elementSizeGet() const ESE_NOTHROW = 0;
  
  /// Return count of elements in queue
  virtual size_t countGet() const ESE_NOTHROW = 0;
  
  /// Return free elements available in queue
  virtual size_t freeCountGet() const ESE_NOTHROW = 0;
  
  /// Return true if queue is empty
  virtual bool isEmpty() const ESE_NOTHROW = 0;
  
  /// Return true if queue is full
  virtual bool isFull() const ESE_NOTHROW = 0;
  
  /// Reset queue
  virtual void reset() ESE_NOTHROW = 0;
  
  /// Push an element into queue's back
  virtual rtosStatus pushBack(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW = 0;
  
  /// Push an element into queue's front
  virtual rtosStatus pushFront(const void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW = 0;

  /// Pop an element from queue's front
  virtual rtosStatus popFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW = 0;
  
  /// Peek an element at the fron of queue
  virtual rtosStatus peekFront(void* eltData, size_t eltSize, uint32_t tmo) ESE_NOTHROW = 0;

  /// Push an element into queue's back from ISR
  virtual rtosStatus pushBackFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW = 0;
  
  /// Push an element into queue's front from ISR
  virtual rtosStatus pushFrontFromISR(const void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW = 0;
  
  /// Pop an element from queue's front from ISR
  virtual rtosStatus popFrontFromISR(void* eltData, size_t eltSize, volatile bool& shouldYield) ESE_NOTHROW = 0;
  
  /// Peek an element at the fron of queue from ISR
  virtual rtosStatus peekFrontFromISR(void* eltData, size_t eltSize) ESE_NOTHROW = 0;
  
  /// Templated wrappers around untyped interface services
  ///
  
  /// Push an element into queue's back
  template <typename ItemT>
  inline rtosStatus pushBack(const ItemT& item, uint32_t tmo) ESE_NOTHROW
  {
    return pushBack(reinterpret_cast<const void*>(&item), sizeof(ItemT), tmo);
  }
  
  /// Push an element into queue's front
  template <typename ItemT>
  inline rtosStatus pushFront(const ItemT& item, uint32_t tmo) ESE_NOTHROW
  {
    return pushFront(reinterpret_cast<const void*>(&item), sizeof(ItemT), tmo);
  }

  /// Pop an element from queue's front
  template <typename ItemT>
  inline rtosStatus popFront(ItemT& item, uint32_t tmo) ESE_NOTHROW
  {
    return popFront(reinterpret_cast<void*>(&item), sizeof(ItemT), tmo);
  }
  
  /// Peek an element at the fron of queue
  template <typename ItemT>
  inline rtosStatus peekFront(ItemT& item, uint32_t tmo) ESE_NOTHROW
  {
    return peekFront(reinterpret_cast<void*>(&item), sizeof(ItemT), tmo);
  }

  /// Push an element into queue's back from ISR
  template <typename ItemT>
  inline rtosStatus pushBackFromISR(const ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return pushBackFromISR(reinterpret_cast<const void*>(&item), sizeof(ItemT), shouldYield);
  }
  
  /// Push an element into queue's front from ISR
  template <typename ItemT>
  inline rtosStatus pushFrontFromISR(const ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return pushFrontFromISR(reinterpret_cast<const void*>(&item), sizeof(ItemT), shouldYield);
  }
  
  /// Pop an element from queue's front from ISR
  template <typename ItemT>
  inline rtosStatus popFrontFromISR(ItemT& item, volatile bool& shouldYield) ESE_NOTHROW
  {
    return popFrontFromISR(reinterpret_cast<void*>(&item), sizeof(ItemT), shouldYield);
  }
  
  /// Peek an element at the fron of queue from ISR
  template <typename ItemT>
  inline rtosStatus peekFrontFromISR(ItemT& item) ESE_NOTHROW
  {
    return peekFrontFromISR(reinterpret_cast<void*>(&item), sizeof(ItemT));
  }
};

#endif //< _ese_os_queue_intf_h_
