#include <esfwxe/target.h>
#include <esfwxe/type.h>

#include "EseI18nIntf.h"
#include "EseI18n.h"

#include <esfwxe/ese_i18n.c>
//---------------------------------------------------------------------------

#ifndef ESE_I18N_LANG_ID_DEFAULT
# error "ESE_I18N_LANG_ID_DEFAULT must be defined to the default firmware languageId"
#endif

EseI18n::EseI18n() ESE_NOTHROW :
m_defaultLangId( ESE_I18N_LANG_ID_DEFAULT )
{}
//---------------------------------------------------------------------------

EseI18nIntf* EseI18n::instanceGet() ESE_NOTHROW
{
  static EseI18n s_api;
  return &s_api;
}
//---------------------------------------------------------------------------

bool EseI18n::haveLangId(int langId) const ESE_NOTHROW
{
  return -1 < langId && 
    langId < eseI18nLangIdsCount;
}
//---------------------------------------------------------------------------

bool EseI18n::haveStringId(int strId) const ESE_NOTHROW
{
  return -1 < strId &&
    strId < eseI18nStrIdsCount;
}
//---------------------------------------------------------------------------
  
int EseI18n::defaultLangIdGet() const ESE_NOTHROW
{
  return m_defaultLangId;
}
//---------------------------------------------------------------------------

int EseI18n::langIdsCountGet() const ESE_NOTHROW
{
  return eseI18nLangIdsCount;
}
//---------------------------------------------------------------------------

const char* EseI18n::langNativeNameGet(int langId) const ESE_NOTHROW
{
  ESE_ASSERT(-1 < langId);
  ESE_ASSERT(langId < eseI18nLangIdsCount);
  
  return eseI18nLangNativeNameGet(
    static_cast<eseI18nLangId>(langId)
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringGet(int strId) const ESE_NOTHROW
{
  ESE_ASSERT(-1 < strId);
  ESE_ASSERT(strId < eseI18nStrIdsCount);
  
  return eseI(
    m_defaultLangId,
    strId
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringGet(int langId, int strId) const ESE_NOTHROW
{
  ESE_ASSERT(-1 < langId);
  ESE_ASSERT(langId < eseI18nLangIdsCount);

  ESE_ASSERT(-1 < strId);
  ESE_ASSERT(strId < eseI18nStrIdsCount);
  
  return eseI(
    langId,
    strId
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringArrayItemGet(int strId, int idx) const ESE_NOTHROW
{
  ESE_ASSERT(-1 < strId);
  ESE_ASSERT(strId < eseI18nStrIdsCount);
  
  return eseIA(
    m_defaultLangId,
    strId,
    idx
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringArrayItemGet(int langId, int strId, int idx) const ESE_NOTHROW
{
  ESE_ASSERT(-1 < langId);
  ESE_ASSERT(langId < eseI18nLangIdsCount);

  ESE_ASSERT(-1 < strId);
  ESE_ASSERT(strId < eseI18nStrIdsCount);
  
  return eseIA(
    langId,
    strId,
    idx
  );
}
//---------------------------------------------------------------------------
