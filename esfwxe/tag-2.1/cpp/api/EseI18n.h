#ifndef _ese_i18n_impl_h_
#define _ese_i18n_impl_h_

/// Implementation of I18N strings database abstraction
///

class EseI18n : public EseI18nIntf
{
private:
  EseI18n() ESE_NOTHROW;

public:
  static EseI18nIntf* instanceGet() ESE_NOTHROW;

  virtual bool haveLangId(int langId) const ESE_NOTHROW ESE_OVERRIDE;
  virtual bool haveStringId(int strId) const ESE_NOTHROW ESE_OVERRIDE;
  
  virtual int defaultLangIdGet() const ESE_NOTHROW ESE_OVERRIDE;
  virtual int langIdsCountGet() const ESE_NOTHROW ESE_OVERRIDE;
  
  virtual const char* langNativeNameGet(int langId) const ESE_NOTHROW ESE_OVERRIDE;
  virtual const char* stringGet(int strId) const ESE_NOTHROW ESE_OVERRIDE;
  virtual const char* stringGet(int langId, int strId) const ESE_NOTHROW ESE_OVERRIDE;
  virtual const char* stringArrayItemGet(int strId, int idx) const ESE_NOTHROW ESE_OVERRIDE;
  virtual const char* stringArrayItemGet(int langId, int strId, int idx) const ESE_NOTHROW ESE_OVERRIDE;
  
protected:
  int m_defaultLangId;
  
private:
  EseI18n(const EseI18n&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseI18n& operator=(const EseI18n&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif // _ese_i18n_impl_h_
