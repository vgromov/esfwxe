#ifndef _ese_semaphore_h_
#define _ese_semaphore_h_

/// Embedded counting semaphore wrapper class
///
class EseSemaphore
{
public:
  EseSemaphore(size_t maxCount, size_t initialCount) ESE_NOTHROW;
  virtual ~EseSemaphore() ESE_NOTHROW;
  
  inline bool isOk() const ESE_NOTHROW { return 0 != m_h; }
  inline size_t maxCountGet() const ESE_NOTHROW { return m_maxCount; }

  rtosStatus lock(uint32_t tmo = rtosMaxDelay) ESE_NOTHROW;
  rtosStatus unlock() ESE_NOTHROW;

  rtosStatus lockFromISR(volatile bool& shouldYield) ESE_NOTHROW;
  rtosStatus unlockFromISR(volatile bool& shouldYield) ESE_NOTHROW;
 
  void init() ESE_NOTHROW;
  void uninit() ESE_NOTHROW;
  

protected:
  void* m_h;
  size_t m_maxCount;
  size_t m_initialCount;
  
private:
  EseSemaphore(const EseSemaphore&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseSemaphore& operator=(const EseSemaphore&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // _ese_semaphore_h_
