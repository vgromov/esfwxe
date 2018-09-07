#ifndef _ese_memory_intf_h_
#define _ese_memory_intf_h_

/// Memory management API
///
class ESE_ABSTRACT EseMemoryIntf
{
public:
  virtual void statsGet(uint32_t& curalloc, uint32_t& totfree, uint32_t& maxfree) const ESE_NOTHROW = 0;
  virtual void* malloc(uint32_t size) ESE_NOTHROW = 0;
  virtual void* calloc(uint32_t num, uint32_t size) ESE_NOTHROW = 0;
  virtual void* realloc(void* p, uint32_t size) ESE_NOTHROW = 0;
  virtual void free(void* p) ESE_NOTHROW = 0;
};


#endif // _ese_memory_intf_h_
