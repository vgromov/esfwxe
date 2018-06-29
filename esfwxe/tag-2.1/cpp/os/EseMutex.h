#ifndef _ese_mutex_h_
#define _ese_mutex_h_

/// Embedded mutex wrapper class
///
class EseMutex
{
public:
  EseMutex() ESE_NOTHROW;
  virtual ~EseMutex() ESE_NOTHROW;
  
  bool isOk() const ESE_NOTHROW { return 0 != m_h; }
  rtosStatus lock(esU32 tmo = rtosMaxDelay) ESE_NOTHROW;
  rtosStatus unlock() ESE_NOTHROW;

  rtosStatus lockFromISR(volatile bool& shouldYield) ESE_NOTHROW;
  rtosStatus unlockFromISR(volatile bool& shouldYield) ESE_NOTHROW;
 
  void init() ESE_NOTHROW;
  void uninit() ESE_NOTHROW;
  
protected:
  void* m_h;
  
private:
  EseMutex(const EseMutex&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseMutex& operator=(const EseMutex&) ESE_NOTHROW ESE_REMOVE_DECL;
};

/// Embedded recursive mutex wrapper class
///
#if 1 == configUSE_RECURSIVE_MUTEXES
class EseMutexRecursive
{
public:
  EseMutexRecursive() ESE_NOTHROW;
  virtual ~EseMutexRecursive() ESE_NOTHROW;
  
  bool isOk() const ESE_NOTHROW { return 0 != m_h; }
  rtosStatus lock(esU32 tmo = rtosMaxDelay) ESE_NOTHROW;
  rtosStatus unlock() ESE_NOTHROW;
  
  void init() ESE_NOTHROW;
  void uninit() ESE_NOTHROW;
  
protected:
  void* m_h;
  
private:
  EseMutexRecursive(const EseMutexRecursive&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseMutexRecursive& operator=(const EseMutexRecursive&) ESE_NOTHROW ESE_REMOVE_DECL;
};
#endif

#endif // _ese_mutex_h_
