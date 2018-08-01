// Template I18N string definition file. Copy it to the actual project folder, remove .template from name
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
# define ESE_I18N_STRING_ENTRY(strId, str)
#endif
#ifndef ESE_I18N_STRING_ARR_ENTRY
# define ESE_I18N_STRING_ARR_ENTRY(strId, ...)
#endif
#ifndef ESE_I18N_STRINGS_END
# define ESE_I18N_STRINGS_END
#endif

// Must always be defined, even if it's an empty one
ESE_I18N_STRINGS_BEGIN(en) 
// Entries to be declared ..
ESE_I18N_STRINGS_END

// Add other language section here as appropriate
//ESE_I18N_STRINGS_BEGIN(otherLangId) 
// Entries to be declared ..
//ESE_I18N_STRINGS_END

// Language IDs map
ESE_I18N_LANGS_BEGIN
ESE_I18N_LANG_ENTRY(en, "English") //< English _must_ always be defined, and be the first one
// Declare other IDs as appropriate for target firmware
//ESE_I18N_LANG_ENTRY(otherLangId, "Other languare native name")
ESE_I18N_LANGS_END

#undef ESE_I18N_LANGS_BEGIN
#undef ESE_I18N_LANG_ENTRY
#undef ESE_I18N_LANGS_END

#undef ESE_I18N_STRINGS_BEGIN
#undef ESE_I18N_STRING_ENTRY
#undef ESE_I18N_STRING_ARR_ENTRY
#undef ESE_I18N_STRINGS_END
