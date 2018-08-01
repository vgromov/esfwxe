#include <esfwxe/ese_i18n.h>

//---------------------------------------------------------------------------

// Available language native names
static const ESE_CSTR sc_langNativeNames[eseI18nLangIdsCount] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) nativeName,
#include "ese_i18n.strings.cc"
};
//---------------------------------------------------------------------------

// I18N strings mapping
//
static const struct { int strId; const ESE_CSTR stra[]; } sc_i18nMap[eseI18nLangIdsCount] = {
#define ESE_I18N_STRINGS_BEGIN(lanId) {
#define ESE_I18N_STRING_ENTRY(strId, str)
#define ESE_I18N_STRING_ARR_ENTRY(strId, ...)
#define ESE_I18N_STRINGS_END },
{NULL} //< Terminating element
};

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ESE_CSTR eseI18nLangNativeNameGet(eseI18nId langId)
{
  if( langId >= eseI18nLangIdsCount )
    return NULL;

  return sc_langNativeNames[langId];
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrGet(eseI18nId idLang, eseI18nStrId idStr)
{
  if( idStr >= eseI18nStrIdsCount )
    return NULL;
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrArrayGet(eseI18nId idLang, eseI18nStrId idStr, int idx)
{
  if( idStr >= eseI18nStrIdsCount )
    return NULL;


}
//---------------------------------------------------------------------------

