#ifndef _esfwxe_i18n_h_
#define _esfwxe_i18n_h_

/// @file: ese_i18n.h
/// Embedded I18N support services.
/// Should an application use an embedded I18N-d strings,
/// it should declare esfwxe_i18n.strings.<LocaleID>.cc X-Macro file, see sample esfwxe_i18n.strings.template.cc
///

#include <esfwxe/target.h>
#include <esfwxe/type.h>

#ifdef __cplusplus
  extern "C" {
#endif

/// Declare available locale IDs.
///
typedef enum {
#define ESE_I18N_LANG_ENTRY(langId, nativeName) eseI18nLang_## langId,
#include "ese_i18n.strings.cc"
  eseI18nLangIdsCount //< Special ID, must be the last in enum

} eseI18nLangId;

/// Declare I18N string IDs
///
#define ESE_I18N_GEN_STRING_IDS
typedef enum {
#define ESE_I18N_STRING_ENTRY(langId, strId, str) strId,
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) strId,
#include "ese_i18n.strings.cc"
  eseI18nStrIdsCount  //< Special string ID, must be the last in enum

} eseI18nStrId;

/// Return native name of the languageID, or NULL, if not found
ESE_CSTR eseI18nLangNativeNameGet(eseI18nLangId langId);

/// Return I18N string using eseI18nId (idLang) and string ID (idStr).
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nStrGet(eseI18nLangId idLang, eseI18nStrId idStr);

/// Return I18N string array member using eseI18nId (idLang), array string ID (idStr),
/// and array member index, id.
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nStrArrayGet(eseI18nLangId idLang, eseI18nStrId idStr, int idx);

#define eseI(idLang, idStr) eseI18nStrGet((eseI18nLangId)(idLang),(eseI18nStrId)(idStr))
#define eseIA(idLang, idStr, idx) eseI18nStrArrayGet((eseI18nLangId)(idLang),(eseI18nStrId)(idStr),(idx))

#ifdef __cplusplus
  }
#endif

#endif // _esfwxe_i18n_h_
