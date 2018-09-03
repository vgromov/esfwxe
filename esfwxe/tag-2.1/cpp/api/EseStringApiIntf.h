#ifndef _ese_string_api_intf_h_
#define _ese_string_api_intf_h_

#include <stdarg.h>

/// ESFWXE string utils API interface
///
class ESE_ABSTRACT EseStringApiIntf
{
public:
  virtual ESE_CSTR nullStr() const ESE_NOTHROW = 0;
  virtual void fmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power) const ESE_NOTHROW = 0;
  virtual void fmtIntN(ESE_STR* buff, int buffLen, int val) const ESE_NOTHROW = 0;
  virtual int fmtFloat(ESE_STR buff, int buffLen, float val, int decimals) const ESE_NOTHROW = 0;
  virtual int fmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1) const ESE_NOTHROW = 0;
  virtual int fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW = 0;
  virtual int strlen(ESE_CSTR str) const ESE_NOTHROW = 0;
  virtual int vsprintf(ESE_STR target, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW = 0;
  virtual int vsnprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW = 0;
  virtual int sprintf(ESE_STR target, ESE_CSTR fmt, ...) const ESE_NOTHROW = 0;
  virtual int snprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW = 0;
};

#endif //< _ese_string_api_intf_h_
