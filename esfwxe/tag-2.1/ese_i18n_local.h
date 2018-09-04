#ifndef _esfwxe_i18n_local_h_
#define _esfwxe_i18n_local_h_

/// @file: ese_i18n_local.h
/// Embedded I18N support services for UerApplication local strings storage.

#include <esfwxe/target.h>
#include <esfwxe/type.h>

/// Declare I18N string IDs
///
#define ESE_I18N_GEN_STRING_IDS
typedef enum {
#define ESE_I18N_STRING_ENTRY(langId, strId, str) strId,
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) strId,
#include "res/ese_i18n.strings.cc"
  eseI18nUsrappStrIdsCount  //< Special string ID, must be the last in enum

} eseI18nUsrappStrId;

/// Return I18N string using eseI18nId (idLang) and string ID (idStr).
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nUsrappStrGet(eseI18nLangId idLang, eseI18nUsrappStrId idStr);

/// Return I18N string array member using eseI18nId (idLang), array string ID (idStr),
/// and array member index, id.
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nUsrappStrArrayGet(eseI18nLangId idLang, eseI18nUsrappStrId idStr, int idx);

#endif // _esfwxe_i18n_local_h_
