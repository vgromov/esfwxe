#ifndef _ese_memory_intf_h_
#define _ese_memory_intf_h_

/// Memory management API
///
class ESE_ABSTRACT EseMemoryIntf
{
public:
  virtual void statsGet(uint32_t& curalloc, uint32_t& totfree, uint32_t& maxfree) const ESE_NOTHROW = 0;
  virtual void* malloc(uint32_t size) const ESE_NOTHROW = 0;
  virtual void* calloc(uint32_t num, uint32_t size) const ESE_NOTHROW = 0;
  virtual void* realloc(void* p, uint32_t size) const ESE_NOTHROW = 0;
  virtual void free(void* p) const ESE_NOTHROW = 0;
  virtual void* memset(void* dest, int val, size_t num) const ESE_NOTHROW = 0;
  virtual void* memcpy(void* dest, const void* src, size_t cnt) const ESE_NOTHROW = 0;
  virtual int memcmp(const void* _1, const void* _2, size_t cnt) const ESE_NOTHROW = 0;
  
  template <typename POD_T>
  POD_T* malloc() const ESE_NOTHROW { return reinterpret_cast<POD_T*>( this->malloc( sizeof(POD_T) ) ); }

  template <typename POD_T>
  POD_T* malloc(size_t cnt) const ESE_NOTHROW { return reinterpret_cast<POD_T*>( this->malloc( sizeof(POD_T)*cnt ) ); }

  template <typename POD_T>
  POD_T* memcpy(POD_T* dest, const POD_T* src) const ESE_NOTHROW { return reinterpret_cast<POD_T*>( this->memcpy( reinterpret_cast<void*>(dest), reinterpret_cast<const void*>(src), sizeof(POD_T) ) ); }
};


#endif // _ese_memory_intf_h_
