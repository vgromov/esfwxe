#ifndef _ese_string_api_intf_h_
#define _ese_string_api_intf_h_

#include <stdarg.h>

#ifdef ES_USE_FWID_FORMATTERS
# include <esfwxe/hwIdBase.h>
#endif

/// ESFWXE string utils API interface
///
class ESE_ABSTRACT EseStringApiIntf
{
public:
  virtual ESE_CSTR nullStr() const ESE_NOTHROW = 0;
  
  virtual void fmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power) const ESE_NOTHROW = 0;
  virtual void fmtIntN(ESE_STR* buff, size_t buffLen, int val) const ESE_NOTHROW = 0;
  virtual int fmtFloat(ESE_STR buff, size_t buffLen, float val, int decimals) const ESE_NOTHROW = 0;
  virtual int fmtFloatConstRelativeError(ESE_STR buff, size_t buffLen, float val, int decimalsAt1) const ESE_NOTHROW = 0;
  virtual int fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, size_t buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW = 0;

  virtual int strlen(ESE_CSTR str) const ESE_NOTHROW = 0;
  virtual int vsprintf(ESE_STR target, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW = 0;
  virtual int vsnprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW = 0;
  virtual int sprintf(ESE_STR target, ESE_CSTR fmt, ...) const ESE_NOTHROW = 0;
  virtual int snprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW = 0;
  
  virtual int strcmp(ESE_CSTR _1, ESE_CSTR _2) const ESE_NOTHROW = 0;
  virtual int strncmp(ESE_CSTR _1, ESE_CSTR _2, size_t num ) const ESE_NOTHROW = 0;
  
#ifdef ES_USE_FWID_FORMATTERS

  virtual void fmtEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW = 0;
  virtual void fmtEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& id, esU16 flags) const ESE_NOTHROW = 0;
  virtual void fmtUID(ESE_STR buff, size_t buffLen, const EseUID& key) const ESE_NOTHROW = 0;
  virtual void fmtIdStringFromEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW = 0;
  virtual void fmtIdStringFromEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& info, esU16 flags) const ESE_NOTHROW = 0;
  virtual ESE_STR uidStrToIdStr(ESE_STR buff, size_t buffLen, ESE_CSTR uid, size_t uidLen) const ESE_NOTHROW = 0;
  virtual ESE_STR functionalToIdStr(ESE_STR buff, size_t buffLen, esU32 functional) const ESE_NOTHROW = 0;
  
#endif
};

#endif //< _ese_string_api_intf_h_
