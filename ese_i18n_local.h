#ifndef _esfwxe_i18n_local_h_
#define _esfwxe_i18n_local_h_

/// @file: ese_i18n_local.h
/// Embedded I18N support services for UserApplication local strings storage.

#include <esfwxe/target.h>
#include <esfwxe/type.h>

#ifdef __cplusplus
  extern "C" {
#endif

/// Declare available locale IDs.
///
typedef enum {
  eseI18nUsrappLangIdInvalid                          = -1,
#define ESE_I18N_LANG_ENTRY(langId, nativeName)       eseI18nUsrappLang_## langId,
#include "res/ese_i18n.strings.cc"
  eseI18nUsrappLangIdsCount //< Special ID, must be the last in enum

} eseI18nUsrappLangId;

/// Declare I18N string IDs
///
#define ESE_I18N_GEN_STRING_IDS
typedef enum {
  eseI18nUsrappStrIdInvalid                           = -1,
  __eseI18nUsrappStrIdBase                            = 0xFFFF, //< Reserve 65535 strings to the core space
#define ESE_I18N_STRING_ENTRY(langId, strId, str)     strId,
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) strId,
#include "res/ese_i18n.strings.cc"
  __eseI18nUsrappStrIdsEnd,                                     //< Special string IDs, must be the last in enum
  eseI18nUsrappStrIdFirst                             = __eseI18nUsrappStrIdBase+1,
  eseI18nUsrappStrIdLast                              = __eseI18nUsrappStrIdsEnd-1,
  eseI18nUsrappStrIdsCount                            = __eseI18nUsrappStrIdsEnd-__eseI18nUsrappStrIdBase-1

} eseI18nUsrappStrId;

/// Return languageID by ISO code name, or -1, if not found
int eseI18nUsrappLangIdGet(ESE_CSTR codeName);

/// Return the default language id
/// Implementation should be created elsewhere
///
int eseI18nUsrappLangIdDefaultGet();

/// Return the currently active language id.
/// Implementation should be created elsewhere
///
int eseI18nUsrappLangIdCurrentGet();

/// Return ISO code name of the languageID, or NULL, if not found
ESE_CSTR eseI18nUsrappLangCodeNameGet(eseI18nUsrappLangId langId);

/// Return native name of the languageID, or NULL, if not found
ESE_CSTR eseI18nUsrappLangNativeNameGet(eseI18nUsrappLangId langId);

/// Return I18N string using eseI18nUsrappLangId (idLang) and string ID (idStr).
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nUsrappStrGet(eseI18nUsrappLangId idLang, eseI18nUsrappStrId idStr);

/// Return I18N string array member using eseI18nUsrappLangId (idLang), array string ID (idStr),
/// and array member index, id.
/// If string is not found by non-english idLang, try to find its English counterpart.
/// If, by any means, string is not found, return NULL.
///
ESE_CSTR eseI18nUsrappStrArrayGet(eseI18nUsrappLangId idLang, eseI18nUsrappStrId idStr, int idx);

#ifdef __cplusplus
  }
#endif

#define eseI(idLang, idStr)       eseI18nUsrappStrGet((eseI18nUsrappLangId)(idLang),(eseI18nUsrappStrId)(idStr))
#define eseIA(idLang, idStr, idx) eseI18nUsrappStrArrayGet((eseI18nUsrappLangId)(idLang),(eseI18nUsrappStrId)(idStr),(idx))

#endif // _esfwxe_i18n_local_h_
