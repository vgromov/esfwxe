// Compound I18N string definition file for BOI-01 embedded application.
// Do not include directly into the project, it's being implicitly included from 
// ese_i18n.h/.c files, as appropriate
//
#ifndef ESE_I18N_LANGS_BEGIN
# define ESE_I18N_LANGS_BEGIN
#endif
#ifndef ESE_I18N_LANG_ENTRY
# define ESE_I18N_LANG_ENTRY(langId, nativeName)
#endif
#ifndef ESE_I18N_LANGS_END
# define ESE_I18N_LANGS_END
#endif
#ifndef ESE_I18N_STRINGS_BEGIN
# define ESE_I18N_STRINGS_BEGIN(langId)
#endif
#ifndef ESE_I18N_STRING_ENTRY
# define ESE_I18N_STRING_ENTRY(langId, strId, str)
#endif
#ifndef ESE_I18N_STRING_ARR_ENTRY
# define ESE_I18N_STRING_ARR_ENTRY(langId, strId, ...)
#endif
#ifndef ESE_I18N_STRINGS_END
# define ESE_I18N_STRINGS_END
#endif

#ifndef ESE_I18N_GEN_STRING_IDS //< Exclude all non-basic strings from IDs compilation

// Must always be defined, even if it's an empty one
ESE_I18N_STRINGS_BEGIN(en)
// TODO: Add strings mapping
ESE_I18N_STRINGS_END

#endif // !ESE_I18N_GEN_STRING_IDS

ESE_I18N_STRINGS_BEGIN(ru)
// TODO: Add strings mapping
ESE_I18N_STRINGS_END

// Language IDs map
ESE_I18N_LANGS_BEGIN
ESE_I18N_LANG_ENTRY(en, "English") //< English _must_ always be defined, and be the first one
// TODO: Add other language names mapping
ESE_I18N_LANGS_END

#undef ESE_I18N_LANGS_BEGIN
#undef ESE_I18N_LANG_ENTRY
#undef ESE_I18N_LANGS_END

#undef ESE_I18N_STRINGS_BEGIN
#undef ESE_I18N_STRING_ENTRY
#undef ESE_I18N_STRING_ARR_ENTRY
#undef ESE_I18N_STRINGS_END

#undef ESE_I18N_GEN_STRING_IDS
