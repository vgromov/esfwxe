#ifndef _ese_string_api_h_
#define _ese_string_api_h_

/// String API implementation
///
class EseStringApi : public EseStringApiIntf
{
private:
  EseStringApi() ESE_NOTHROW ESE_KEEP {}
  
public:
  static EseStringApiIntf& instanceGet() ESE_NOTHROW ESE_KEEP;

  virtual ESE_CSTR nullStr() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtIntN(ESE_STR* buff, size_t buffLen, int val) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int fmtFloat(ESE_STR buff, size_t buffLen, float val, int decimals) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int fmtFloatConstRelativeError(ESE_STR buff, size_t buffLen, float val, int decimalsAt1) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, size_t buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int strlen(ESE_CSTR str) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int vsprintf(ESE_STR target, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int vsnprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int sprintf(ESE_STR target, ESE_CSTR fmt, ...) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int snprintf(ESE_STR target, size_t maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int strcmp(ESE_CSTR _1, ESE_CSTR _2) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int strncmp(ESE_CSTR _1, ESE_CSTR _2, size_t num ) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
#ifdef ES_USE_FWID_FORMATTERS

  virtual void fmtEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& id, esU16 flags) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtUID(ESE_STR buff, size_t buffLen, const EseUID& key) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtIdStringFromEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID& id, esU16 flags) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void fmtIdStringFromEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo& info, esU16 flags) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual ESE_STR uidStrToIdStr(ESE_STR buff, size_t buffLen, ESE_CSTR uid, size_t uidLen) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual ESE_STR functionalToIdStr(ESE_STR buff, size_t buffLen, esU32 functional) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
#endif
  
  ESE_NONCOPYABLE(EseStringApi);
};

#endif //< _ese_string_api_h_
