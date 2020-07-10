#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>

#include <string.h>

#include "EseI18nIntf.h"
#include "EseI18n.h"

#define eseI(idLang, idStr)       eseI18nStrGet((eseI18nLangId)(idLang),(eseI18nStrId)(idStr))
#define eseIA(idLang, idStr, idx) eseI18nStrArrayGet((eseI18nLangId)(idLang),(eseI18nStrId)(idStr),(idx))
#include <esfwxe/ese_i18n.c>
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

extern "C" {

int eseI18nLangIdCurrentGet() ESE_NOTHROW
{
  return EseI18n::instanceGet().currentLangIdGet();
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

EseI18n::EseI18n() ESE_NOTHROW :
m_defaultLangId(eseI18nLangIdInvalid),
m_currentLangId(eseI18nLangIdInvalid)
{
  m_defaultLangId = eseI18nLangIdDefaultGet();
  m_currentLangId = m_defaultLangId;
}
//---------------------------------------------------------------------------

EseI18nIntf& EseI18n::instanceGet() ESE_NOTHROW
{
  static EseI18n s_api;
  return s_api;
}
//---------------------------------------------------------------------------

bool EseI18n::haveLangId(int langId) const ESE_NOTHROW
{
  return eseI18nLangIdInvalid < langId && 
    langId < eseI18nLangIdsCount;
}
//---------------------------------------------------------------------------

bool EseI18n::haveStringId(int strId) const ESE_NOTHROW
{
  return eseI18nStrIdInvalid < strId &&
    strId < eseI18nStrIdsCount;
}
//---------------------------------------------------------------------------
  
int EseI18n::defaultLangIdGet() const ESE_NOTHROW
{
  return m_defaultLangId;
}
//---------------------------------------------------------------------------

int EseI18n::currentLangIdGet() const ESE_NOTHROW
{
  return m_currentLangId;
}
//---------------------------------------------------------------------------

void EseI18n::currentLangIdSet(int langId) ESE_NOTHROW
{
  if( m_currentLangId != langId )
  {
    if( haveLangId(m_currentLangId) )
      m_currentLangId = langId;
    else
      m_currentLangId = defaultLangIdGet();
  }
}
//---------------------------------------------------------------------------

int EseI18n::langIdsCountGet() const ESE_NOTHROW
{
  return eseI18nLangIdsCount;
}
//---------------------------------------------------------------------------

int EseI18n::langIdGetByCode(const char* code) const ESE_NOTHROW
{
  return eseI18nLangIdGet(code);
}
//---------------------------------------------------------------------------

const char* EseI18n::langCodeGet(int langId) const ESE_NOTHROW
{
  if( 
    eseI18nLangIdInvalid < langId &&
    langId < eseI18nLangIdsCount
  )
    return eseI18nLangCodeNameGet(
      static_cast<eseI18nLangId>(langId)
    );
  
  return NULL;
}
//---------------------------------------------------------------------------

const char* EseI18n::langNativeNameGet(int langId) const ESE_NOTHROW
{
  if( 
    eseI18nLangIdInvalid < langId &&
    langId < eseI18nLangIdsCount
  )
    return eseI18nLangNativeNameGet(
      static_cast<eseI18nLangId>(langId)
    );

  return NULL;
}
//---------------------------------------------------------------------------

const char* EseI18n::stringGet(int strId) const ESE_NOTHROW
{
  return stringGet(
    currentLangIdGet(),
    strId
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringGet(int langId, int strId) const ESE_NOTHROW
{
  if(
    eseI18nLangIdInvalid < langId &&
    langId < eseI18nLangIdsCount &&
    eseI18nStrIdInvalid < strId &&
    strId < eseI18nStrIdsCount
  )
    return eseI(
      langId,
      strId
    );

  return NULL;
}
//---------------------------------------------------------------------------

const char* EseI18n::stringArrayItemGet(int strId, int idx) const ESE_NOTHROW
{
  return stringArrayItemGet(
    currentLangIdGet(),
    strId,
    idx
  );
}
//---------------------------------------------------------------------------

const char* EseI18n::stringArrayItemGet(int langId, int strId, int idx) const ESE_NOTHROW
{
  if(
    eseI18nLangIdInvalid < langId &&
    langId < eseI18nLangIdsCount &&
    eseI18nStrIdInvalid < strId &&
    strId < eseI18nStrIdsCount
  )
    return eseIA(
      langId,
      strId,
      idx
    );

  return NULL;
}
//---------------------------------------------------------------------------
