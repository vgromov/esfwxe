#include <esfwxe/target.h>
#include <cstdlib>
#include <cstring>

#include "EseMemoryIntf.h"
#include "EseMemoryIntfImpl.h"
//-----------------------------------------------------------------------------

EseMemoryIntf& EseMemoryIntfImpl::instanceGet() ESE_NOTHROW
{
  static EseMemoryIntfImpl s_inst;
  return s_inst;
}
//-----------------------------------------------------------------------------

extern "C" void heapStatsGet(uint32_t* curalloc, uint32_t* totfree, uint32_t* maxfree);

void EseMemoryIntfImpl::statsGet(uint32_t& curalloc, uint32_t& totfree, uint32_t& maxfree) const ESE_NOTHROW
{
  heapStatsGet(
    &curalloc,
    &totfree,
    &maxfree
  );
}
//-----------------------------------------------------------------------------

void* EseMemoryIntfImpl::malloc(uint32_t size) const ESE_NOTHROW
{
  return std::malloc(size);
}
//-----------------------------------------------------------------------------

void* EseMemoryIntfImpl::calloc(uint32_t num, uint32_t size) const ESE_NOTHROW
{
  return std::calloc(
    num, 
    size
  );
}
//-----------------------------------------------------------------------------

void* EseMemoryIntfImpl::realloc(void* p, uint32_t size) const ESE_NOTHROW
{
  return std::realloc(
    p,
    size
  );
}
//-----------------------------------------------------------------------------

void EseMemoryIntfImpl::free(void* p) const ESE_NOTHROW
{
  std::free(p);
}
//-----------------------------------------------------------------------------

void* EseMemoryIntfImpl::memset(void* dest, int val, size_t num) const ESE_NOTHROW
{
  return std::memset(
    dest,
    val,
    num
  );
}
//-----------------------------------------------------------------------------

void* EseMemoryIntfImpl::memcpy(void* dest, const void* src, size_t cnt) const ESE_NOTHROW
{
  return std::memcpy(
    dest,
    src,
    cnt
  );
}
//-----------------------------------------------------------------------------

int EseMemoryIntfImpl::memcmp(const void* _1, const void* _2, size_t cnt) const ESE_NOTHROW
{
  return std::memcmp(
    _1,
    _2,
    cnt
  );
}
//-----------------------------------------------------------------------------
