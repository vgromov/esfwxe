#ifndef _ese_i18n_intf_h_
#define _ese_i18n_intf_h_

class ESE_ABSTRACT EseI18nIntf
{
public:
  virtual bool haveLangId(int langId) const ESE_NOTHROW = 0;
  virtual bool haveStringId(int strId) const ESE_NOTHROW = 0;
  
  virtual int defaultLangIdGet() const ESE_NOTHROW = 0;
  virtual int langIdsCountGet() const ESE_NOTHROW = 0;
  
  virtual const char* langNativeNameGet(int langId) const ESE_NOTHROW = 0;
  virtual const char* stringGet(int strId) const ESE_NOTHROW = 0;
  virtual const char* stringGet(int langId, int strId) const ESE_NOTHROW = 0;
  virtual const char* stringArrayItemGet(int strId, int idx) const ESE_NOTHROW = 0;
  virtual const char* stringArrayItemGet(int langId, int strId, int idx) const ESE_NOTHROW = 0;
};

#endif //< _ese_i18n_intf_h_
