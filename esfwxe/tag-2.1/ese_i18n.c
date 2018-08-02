#include <esfwxe/ese_i18n.h>

//---------------------------------------------------------------------------

// Available language native names
#define ESE_I18N_LANGS_BEGIN \
  static const ESE_CSTR sc_langNativeNames[eseI18nLangIdsCount] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) \
  nativeName,
#define ESE_I18N_LANGS_END \
  NULL };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

// I18N strings mapping
//
typedef struct {
  int strId;
  const ESE_CSTR* stra;

} EseI18nEntry;

#define ESE_I18N_STRINGS_BEGIN(langId) \
  static const EseI18nEntry sc_i18nMap_##langId[] {
#define ESE_I18N_STRING_ENTRY(strId, str) {strId, {str}},
#define ESE_I18N_STRING_ARR_ENTRY(strId, ...) {strId, {__VA_ARGS__}},
#define ESE_I18N_STRINGS_END {-1, {NULL}} };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

#define ESE_I18N_LANGS_BEGIN \
  static const EseI18nEntry* sc_i18nMap[eseI18nLangIdsCount] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) \
  sc_i18nMap_##langId,
#define ESE_I18N_LANGS_END \
  NULL };
#include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

ESE_CSTR eseI18nLangNativeNameGet(eseI18nId langId)
{
  if( langId >= eseI18nLangIdsCount )
    return NULL;

  return sc_langNativeNames[langId];
}
//---------------------------------------------------------------------------

static ESE_CSTR* eseI18nStraGet(eseI18nId idLang, eseI18nStrId idStr)
{
  if( idStr >= eseI18nStrIdsCount )
    return NULL;

retry_en:
  const EseI18nEntry* nodes = sc_i18nMap[idLang];
  const EseI18nEntry* node;

  int idx = 0;
  do
  {
    node = &nodes[idx++];

    if( idStr == node->strId )
      return node->stra;

  } while( -1 != node->strId );

  // Retry en (0), if not already
  if( 0 != idLang )
  {
    idLang = 0;
    goto retry_en;
  }

  return NULL;
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrGet(eseI18nId idLang, eseI18nStrId idStr)
{
  ESE_CSTR* stra = eseI18nStraGet(idLang, idStr);
  if( stra )
    return stra[0];

  return stra;
}
//---------------------------------------------------------------------------

ESE_CSTR eseI18nStrArrayGet(eseI18nId idLang, eseI18nStrId idStr, int idx)
{
  ESE_CSTR* stra = eseI18nStraGet(idLang, idStr);
  if( stra )
    return stra[idx];

  return stra;
}
//---------------------------------------------------------------------------

