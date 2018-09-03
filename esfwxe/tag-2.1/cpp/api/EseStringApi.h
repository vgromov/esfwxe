#ifndef _ese_string_api_h_
#define _ese_string_api_h_

/// String API implementation
///
class EseStringApi : public EseStringApiIntf
{
private:
  EseStringApi() ESE_NOTHROW;
  
public:
  static EseStringApiIntf* instanceGet() ESE_NOTHROW;

  virtual ESE_CSTR nullStr() const ESE_NOTHROW ESE_OVERRIDE;
  virtual void fmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power) const ESE_NOTHROW ESE_OVERRIDE;
  virtual void fmtIntN(ESE_STR* buff, int buffLen, int val) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int fmtFloat(ESE_STR buff, int buffLen, float val, int decimals) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int fmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int strlen(ESE_CSTR str) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int vsprintf(ESE_STR target, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int vsnprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, va_list arg_ptr) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int sprintf(ESE_STR target, ESE_CSTR fmt, ...) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int snprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, ...) const ESE_NOTHROW ESE_OVERRIDE;
  
private:
  EseStringApi(const EseStringApi&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseStringApi& operator=(const EseStringApi&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif //< _ese_string_api_h_
