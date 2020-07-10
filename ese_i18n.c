#ifndef ESE_I18N_STRINGAPI_ACCESS
# define ESE_I18N_STRINGAPI_ACCESS
#endif

#include <string.h>

#if !defined(ESE_I18N_USE_LOCAL_COPY)

# include <esfwxe/ese_i18n.h>
//---------------------------------------------------------------------------
// Available language native names
# define ESE_I18N_LANGS_BEGIN                         static const struct { const ESE_CSTR codeName; const ESE_CSTR nameNative; } sc_langNames[] = {
# define ESE_I18N_LANG_ENTRY(langId, nativeName)      {#langId, nativeName},
# define ESE_I18N_LANGS_END                           {NULL, NULL} };
# include "ese_i18n.strings.cc"
//---------------------------------------------------------------------------

#else

# include <esfwxe/ese_i18n_local.h>
//---------------------------------------------------------------------------
// Available language native names
# define ESE_I18N_LANGS_BEGIN                         static const struct { const ESE_CSTR codeName; const ESE_CSTR nameNative; } sc_langNames[] = {
# define ESE_I18N_LANG_ENTRY(langId, nativeName)      {#langId, nativeName},
# define ESE_I18N_LANGS_END                           {NULL, NULL} };
# include "res/ese_i18n.strings.cc"

#endif //< defined(ESE_I18N_USE_LOCAL_COPY)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// Generate local string constants
#define ESE_I18N_STRING_ENTRY(langId, strId, str)     static const ESE_CSTR sc_stra_##langId##_##strId[] = {(str)};
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) static const ESE_CSTR sc_stra_##langId##_##strId[] = {__VA_ARGS__};
  
#if !defined( ESE_I18N_USE_LOCAL_COPY )
# include "ese_i18n.strings.cc"
#else
# include "res/ese_i18n.strings.cc"
#endif //< defined(ESE_I18N_USE_LOCAL_COPY)
//---------------------------------------------------------------------------

// Generate I18N strings mapping by its stringID
//
typedef struct {
  int strId;
  int straLen;
  const ESE_CSTR* stra;

} EseI18nEntry;

#define ESE_I18N_STRINGS_BEGIN(langId)                static const EseI18nEntry sc_i18nMap_##langId[] = {
#define ESE_I18N_STRING_ENTRY(langId, strId, str)     {strId, sizeof(sc_stra_##langId##_##strId)/sizeof(sc_stra_##langId##_##strId[0]), (const ESE_CSTR*)&sc_stra_##langId##_##strId},
#define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...) {strId, sizeof(sc_stra_##langId##_##strId)/sizeof(sc_stra_##langId##_##strId[0]), (const ESE_CSTR*)&sc_stra_##langId##_##strId},
#define ESE_I18N_STRINGS_END                          {-1, 0, (const ESE_CSTR*)NULL} };

#if !defined(ESE_I18N_USE_LOCAL_COPY)
# include "ese_i18n.strings.cc"
#else
# include "res/ese_i18n.strings.cc"
#endif //< defined(ESE_I18N_USE_LOCAL_COPY)
//---------------------------------------------------------------------------

// Generate I18N strings mapping by language ID
typedef struct {
  const EseI18nEntry* ptr;

} EseI18nEntryPtr;

#define ESE_I18N_LANGS_BEGIN                          static const EseI18nEntryPtr sc_i18nMap[] = {
#define ESE_I18N_LANG_ENTRY(langId, nativeName)       {sc_i18nMap_##langId},
#define ESE_I18N_LANGS_END                            {NULL} };

#if !defined(ESE_I18N_USE_LOCAL_COPY)
# include "ese_i18n.strings.cc"
#else
# include "res/ese_i18n.strings.cc"
#endif //< defined(ESE_I18N_USE_LOCAL_COPY)
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#if !defined(ESE_I18N_USE_LOCAL_COPY)
int eseI18nLangIdGet(ESE_CSTR codeName)
#else
int eseI18nUsrappLangIdGet(ESE_CSTR codeName)
#endif
{
  if( !codeName )
    return 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLangIdInvalid;
#else
      eseI18nUsrappLangIdInvalid;
#endif

  for(
    int code = 0; 
    code < 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLangIdsCount; 
#else
      eseI18nUsrappLangIdsCount; 
#endif
    ++code)
  {
    if( 
      0 == 
      ESE_I18N_STRINGAPI_ACCESS
        strcmp(
          codeName, 
          sc_langNames[code].codeName
        )
    )
      return code;
  }
  
  return -1;
}
//---------------------------------------------------------------------------

#if !defined(ESE_I18N_USE_LOCAL_COPY)
ESE_CSTR eseI18nLangCodeNameGet(eseI18nLangId langId)
#else
ESE_CSTR eseI18nUsrappLangCodeNameGet(eseI18nUsrappLangId langId)
#endif
{
  if( 
    langId >= 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLangIdsCount 
#else
      eseI18nUsrappLangIdsCount 
#endif
  )
    return NULL;

  return sc_langNames[langId].codeName;
}
//---------------------------------------------------------------------------

#if !defined(ESE_I18N_USE_LOCAL_COPY)
ESE_CSTR eseI18nLangNativeNameGet(eseI18nLangId langId)
#else
ESE_CSTR eseI18nUsrappLangNativeNameGet(eseI18nUsrappLangId langId)
#endif
{
  if( 
    langId >= 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLangIdsCount 
#else
      eseI18nUsrappLangIdsCount
#endif
  )
    return NULL;

  return sc_langNames[langId].nameNative;
}
//---------------------------------------------------------------------------

static const ESE_CSTR* eseI18nStraGet(
#if !defined(ESE_I18N_USE_LOCAL_COPY)
    eseI18nLangId idLang, eseI18nStrId idStr,
#else
    eseI18nUsrappLangId idLang, eseI18nUsrappStrId idStr,
#endif 
  int* straLen
)
{
  if( 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      (idStr >= eseI18nStrIdsCount)
#else
      (
        (idStr < eseI18nUsrappStrIdFirst) ||
        (idStr > eseI18nUsrappStrIdLast)
      )
#endif
    || 
    idLang >= 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLangIdsCount
#else
      eseI18nUsrappLangIdsCount
#endif
  )
    return NULL;

  while(1)
  {
    const EseI18nEntry* nodes = sc_i18nMap[idLang].ptr;
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
    if( 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
      eseI18nLang_en 
#else
      eseI18nUsrappLang_en
#endif
      != idLang 
    )
      idLang = 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
        eseI18nLang_en;
#else
        eseI18nUsrappLang_en;
#endif
    else
      break;
  }

  if(straLen)
    *straLen = 0;
  return NULL;
}
//---------------------------------------------------------------------------

#if !defined(ESE_I18N_USE_LOCAL_COPY)
ESE_CSTR eseI18nStrGet(eseI18nLangId idLang, eseI18nStrId idStr)
#else
ESE_CSTR eseI18nUsrappStrGet(eseI18nUsrappLangId idLang, eseI18nUsrappStrId idStr)
#endif
{
  return 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
    eseI18nStrArrayGet
#else
    eseI18nUsrappStrArrayGet
#endif
    (
      idLang, 
      idStr, 
      0
    );
}
//---------------------------------------------------------------------------

#if !defined(ESE_I18N_USE_LOCAL_COPY)
ESE_CSTR eseI18nStrArrayGet(eseI18nLangId idLang, eseI18nStrId idStr, int idx)
#else
ESE_CSTR eseI18nUsrappStrArrayGet(eseI18nUsrappLangId idLang, eseI18nUsrappStrId idStr, int idx)
#endif
{
  int len;
__retryWithDefaultLangId:

  len = 0;
  const ESE_CSTR* stra = eseI18nStraGet(
    idLang,
    idStr,
    &len
  );
  
  if(
    NULL == stra &&
#if !defined(ESE_I18N_USE_LOCAL_COPY)
    eseI18nLangIdDefaultGet() 
#else
    eseI18nUsrappLangIdDefaultGet() 
#endif
      != idLang
  )
  {
    idLang = 
#if !defined(ESE_I18N_USE_LOCAL_COPY)
    (eseI18nLangId)eseI18nLangIdDefaultGet();
#else
    (eseI18nUsrappLangId)eseI18nUsrappLangIdDefaultGet();
#endif
    
    goto __retryWithDefaultLangId;
  }
  
  if( 
    stra && 
    idx < len 
  )
    return stra[idx];

  return NULL;
}
//---------------------------------------------------------------------------
