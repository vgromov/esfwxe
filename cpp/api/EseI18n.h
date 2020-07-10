#ifndef _ese_i18n_impl_h_
#define _ese_i18n_impl_h_

/// Implementation of I18N strings database abstraction
///

class EseI18n : public EseI18nIntf
{
private:
  EseI18n() ESE_NOTHROW ESE_KEEP;

public:
  static EseI18nIntf& instanceGet() ESE_NOTHROW ESE_KEEP;

  virtual bool haveLangId(int langId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool haveStringId(int strId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual int defaultLangIdGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int langIdsCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int currentLangIdGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void currentLangIdSet(int langId) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  virtual int langIdGetByCode(const char* code) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* langCodeGet(int langId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* langNativeNameGet(int langId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* stringGet(int strId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* stringGet(int langId, int strId) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* stringArrayItemGet(int strId, int idx) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual const char* stringArrayItemGet(int langId, int strId, int idx) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
protected:
  int m_defaultLangId;
  int m_currentLangId;
  
  ESE_NONCOPYABLE(EseI18n);
};

#endif // _ese_i18n_impl_h_
