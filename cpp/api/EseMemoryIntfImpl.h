#ifndef _ese_memory_intf_impl_h_
#define _ese_memory_intf_impl_h_

/// System memory interface implementation
///
class EseMemoryIntfImpl : public EseMemoryIntf
{
protected:
  inline EseMemoryIntfImpl() ESE_NOTHROW {}

public:
  virtual ~EseMemoryIntfImpl() ESE_NOTHROW {}

  static EseMemoryIntf& instanceGet() ESE_NOTHROW ESE_KEEP;

  /// EseMemoryIntf implementation
  ///
  virtual void statsGet(uint32_t& curalloc, uint32_t& totfree, uint32_t& maxfree) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* malloc(uint32_t size) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* calloc(uint32_t num, uint32_t size) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* realloc(void* p, uint32_t size) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void free(void* p) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* memset(void* dest, int val, size_t num) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* memcpy(void* dest, const void* src, size_t cnt) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int memcmp(const void* _1, const void* _2, size_t cnt) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  ESE_NONCOPYABLE(EseMemoryIntfImpl);
};

#endif //< _ese_memory_intf_impl_h_
