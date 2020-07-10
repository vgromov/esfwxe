#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/utils_str.h>

#include <stdarg.h>
#include <cstdio>

#include "EseStringApiIntf.h"
#include "EseStringApi.h"

#ifdef ES_USE_FWID_FORMATTERS
# include <esfwxe/hwIdUtilsBase.h>
#endif
//----------------------------------------------------------------------------------------------

EseStringApiIntf& EseStringApi::instanceGet() ESE_NOTHROW
{
  static EseStringApi s_api;
  return s_api;
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

void EseStringApi::fmtIntN(ESE_STR* buff, size_t buffLen, int val) const ESE_NOTHROW
{
  eseUtilsStrFmtIntN(
    buff,
    buffLen,
    val
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloat(ESE_STR buff, size_t buffLen, float val, int decimals) const ESE_NOTHROW
{
  return eseUtilsStrFmtFloat(
    buff,
    buffLen,
    val,
    decimals
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloatConstRelativeError(ESE_STR buff, size_t buffLen, float val, int decimalsAt1) const ESE_NOTHROW
{
  return eseUtilsStrFmtFloatConstRelativeError(
    buff,
    buffLen,
    val,
    decimalsAt1
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, size_t buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW
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
  return eseUtilsStrVsprintf(
    target,
    fmt,
    arg_ptr
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::vsnprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW
{
  return eseUtilsStrVsnprintf(
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

  int result = eseUtilsStrVsprintf(
    target,
    fmt,
    arg_ptr
  );
  
  va_end(arg_ptr);
  
  return result;
}
//----------------------------------------------------------------------------------------------

int EseStringApi::snprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW
{
  va_list arg_ptr;
  va_start(arg_ptr, fmt);

  int result = eseUtilsStrVsnprintf(
    target,
    maxTargetLen,
    fmt,
    arg_ptr
  );
  
  va_end(arg_ptr);
  
  return result;
}
//----------------------------------------------------------------------------------------------

int EseStringApi::strcmp(ESE_CSTR _1, ESE_CSTR _2) const ESE_NOTHROW
{
  return std::strcmp(
    _1,
    _2
  );
}
//----------------------------------------------------------------------------------------------

int EseStringApi::strncmp(ESE_CSTR _1, ESE_CSTR _2, size_t num ) const ESE_NOTHROW
{
  return std::strncmp(
    _1,
    _2,
    num
  );
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

#ifdef ES_USE_FWID_FORMATTERS

void EseStringApi::fmtEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW
{
  ::fmtEseBasicFirmwareID(
    buff,
    buffLen,
    &id,
    flags
  );
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& id, esU16 flags) const ESE_NOTHROW
{
  ::fmtEseFwInfo(
    buff,
    buffLen,
    &id,
    flags
  );
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtUID(ESE_STR buff, size_t buffLen, const EseUID& key) const ESE_NOTHROW
{
  ::fmtUID(
    buff,
    buffLen,
    &key
  );
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtIdStringFromEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW
{
  ::fmtIdStringFromEseBasicFirmwareID(
    buff, 
    buffLen, 
    &id, 
    flags
  );
}
//----------------------------------------------------------------------------------------------

void EseStringApi::fmtIdStringFromEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& info, esU16 flags) const ESE_NOTHROW
{
  ::fmtIdStringFromEseFwInfo(
    buff, 
    buffLen, 
    &info, 
    flags
  );
}
//----------------------------------------------------------------------------------------------

ESE_STR EseStringApi::uidStrToIdStr(ESE_STR buff, size_t buffLen, ESE_CSTR uid, size_t uidLen) const ESE_NOTHROW
{
  return fmtUIDtoIdString(
    buff,
    buffLen,
    uid,
    uidLen
  );
}
//----------------------------------------------------------------------------------------------

ESE_STR EseStringApi::functionalToIdStr(ESE_STR buff, size_t buffLen, esU32 functional) const ESE_NOTHROW
{
  return fmtFunctionalToIdString(
    buff,
    buffLen,
    functional
  );
}

#endif
