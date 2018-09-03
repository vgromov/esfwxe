#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/utils_str.h>

#include <stdarg.h>

#include "EseStringApiIntf.h"
#include "EseStringApi.h"

#include <esfwxe/utils_str.c>
//----------------------------------------------------------------------------------------------

EseStringApi::EseStringApi() ESE_NOTHROW
{}
//----------------------------------------------------------------------------------------------

EseStringApiIntf* EseStringApi::instanceGet() ESE_NOTHROW
{
  static EseStringApi s_api;
  return &s_api;
}
//----------------------------------------------------------------------------------------------

ESE_CSTR EseStringApi::nullStr() const ESE_NOTHROW
{
  return c_nullString;
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power) const ESE_NOTHROW
{
  eseUtilsStrFmtInt(
    buff,
    end,
    val,
    neg,
    power
  );
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtIntN(ESE_STR* buff, int buffLen, int val) const ESE_NOTHROW
{
  eseUtilsStrFmtIntN(
    buff,
    buffLen,
    val
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloat(ESE_STR buff, int buffLen, float val, int decimals) const ESE_NOTHROW
{
  return eseUtilsStrFmtFloat(
    buff,
    buffLen,
    val,
    decimals
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1) const ESE_NOTHROW
{
  return eseUtilsStrFmtFloatConstRelativeError(
    buff,
    buffLen,
    val,
    decimalsAt1
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW
{
  return eseUtilsStrFmtFloatConstRelativeErrorDecimalsGet(
    buff,
    buffLen,
    val,
    decimalsAt1,
    decimals
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::strlen(ESE_CSTR str) const ESE_NOTHROW
{
  return eseUtilsStrLenGet(str);
}
//----------------------------------------------------------------------------------------------

int EseStringApi::vsprintf(ESE_STR target, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW
{
  return vsprintf(
    target,
    fmt,
    arg_ptr
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::vsnprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW
{
  return vsnprintf(
    target,
    maxTargetLen,
    fmt,
    arg_ptr
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::sprintf(ESE_STR target, ESE_CSTR fmt, ...) const ESE_NOTHROW
{
  va_list arg_ptr;
  va_start(arg_ptr, fmt);

  int result = vsprintf(
    target,
    fmt,
    arg_ptr
  );
  
  va_end(arg_ptr);
  
  return result;
}
//----------------------------------------------------------------------------------------------

int EseStringApi::snprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW
{
  va_list arg_ptr;
  va_start(arg_ptr, fmt);

  int result = vsnprintf(
    target,
    maxTargetLen,
    fmt,
    arg_ptr
  );
  
  va_end(arg_ptr);
  
  return result;
}
//----------------------------------------------------------------------------------------------
