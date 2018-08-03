#include <esfwxe/ese_i18n.h>

//---------------------------------------------------------------------------

// Available language native names
#define ESE_I18N_LANGS_BEGIN \
  static const ESE_CSTR sc_langNativeNames[] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) \
  nativeName,
#define ESE_I18N_LANGS_END \
  NULL };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

// Generate local string constants
#define ESE_I18N_STRING_ENTRY(langId, strId, str) \
  static const ESE_CSTR sc_stra_##langId##_##strId[] = {(str)};
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) \
  static const ESE_CSTR sc_stra_##langId##_##strId[] = {__VA_ARGS__};
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

// Generate I18N strings mapping by its stringID
//
typedef struct {
  int strId;
  int straLen;
  const ESE_CSTR* stra;

} EseI18nEntry;

#define ESE_I18N_STRINGS_BEGIN(langId) static const EseI18nEntry sc_i18nMap_##langId[] = {
#define ESE_I18N_STRING_ENTRY(langId, strId, str) {strId, sizeof(sc_stra_##langId##_##strId)/sizeof(sc_stra_##langId##_##strId[0]), (const ESE_CSTR*)&sc_stra_##langId##_##strId},
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) {strId, sizeof(sc_stra_##langId##_##strId)/sizeof(sc_stra_##langId##_##strId[0]), (const ESE_CSTR*)&sc_stra_##langId##_##strId},
#define ESE_I18N_STRINGS_END {-1, 0, (const ESE_CSTR*)NULL} };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

// Generate I18N strings mapping by language ID
#define ESE_I18N_LANGS_BEGIN static const EseI18nEntry* sc_i18nMap[] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) sc_i18nMap_##langId,
#define ESE_I18N_LANGS_END (const EseI18nEntry*)NULL };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ESE_CSTR eseI18nLangNativeNameGet(eseI18nLangId langId)
{
  if( langId >= eseI18nLangIdsCount )
    return NULL;

  return sc_langNativeNames[langId];
}
//---------------------------------------------------------------------------

static const ESE_CSTR* eseI18nStraGet(eseI18nLangId idLang, eseI18nStrId idStr, int* straLen)
{
  if( idStr >= eseI18nStrIdsCount || idLang >= eseI18nLangIdsCount )
    return NULL;

  while(1)
  {
    const EseI18nEntry* nodes = sc_i18nMap[idLang];
    const EseI18nEntry* node;

    int idx = 0;
    do
    {
      node = &nodes[idx++];

      if( idStr == node->strId )
      {
        if(straLen)
          *straLen = node->straLen;
        return node->stra;
      }

    } while( -1 != node->strId );

    // Retry en, if not already
    if( eseI18nLang_en != idLang )
      idLang = eseI18nLang_en;
    else
      break;
  }

  if(straLen)
    *straLen = 0;
  return NULL;
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrGet(eseI18nLangId idLang, eseI18nStrId idStr)
{
  int len = 0;
  const ESE_CSTR* stra = eseI18nStraGet(
    idLang,
    idStr,
    &len
  );
  if( stra && len > 0 )
    return stra[0];

  return NULL;
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrArrayGet(eseI18nLangId idLang, eseI18nStrId idStr, int idx)
{
  int len = 0;
  const ESE_CSTR* stra = eseI18nStraGet(
    idLang,
    idStr,
    &len
  );
  if( stra && idx < len )
    return stra[idx];

  return NULL;
}
//---------------------------------------------------------------------------

