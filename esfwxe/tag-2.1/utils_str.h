#ifndef _utils_str_h_
#define _utils_str_h_

#include <stdbool.h>

/// String utilities
///

#ifdef __cplusplus
  extern "C" {
#endif

// null string
extern ESE_CSTR c_nullString;

// UTF-8 string traversal support
//

/// Try to extract one utf32 character from utf8 buffer.
/// @param [IN] buff      - input UTF-8 buffer
/// @param [IN] buffEnd   - an end of input UTF-8 buffer
/// @param [OUT] utf32    - an extracted utf32 symbol goes there. If either 0 termination occurred,
///                         or something went wrong, a 0 is returned
/// @return               - a pointer to the position in input buffer after
///                         character extraction attempt
///
const char* eseUtilsStrUtf32FromUtf8Get(const char* buff, const char* buffEnd, esU32* utf32);

// Custom string formatters
//
void eseUtilsStrFmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power);
void eseUtilsStrFmtIntN(ESE_STR* buff, int buffLen, int val);

// convert float to formatted string representation.
// return number of chars put into buffer
int eseUtilsStrFmtFloat(ESE_STR buff, int buffLen, float val, int decimals);
// format float val with constant relative error
int eseUtilsStrFmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1);
// format float val with constant relative error, return resulting decimals
int eseUtilsStrFmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals);

#ifdef ES_USE_STRUTILS_IMPL
#  include <stdarg.h>

// streaming pfn return codes
enum {
  utilsStreamOk    = 0,
  utilsStreamEnd  = -1,
};

// abstract byte streaming function
typedef int (*utilsPfnChStreamFn)(void* target, esU8 c);

// abstract string formatter and streamer
int eseUtilsStrVstrFmtStream(utilsPfnChStreamFn pfn,  void* target,  ESE_CSTR fmt, va_list lst);

// custom implementations of string formatting routines
int eseUtilsStrSprintf(ESE_STR target, ESE_CSTR fmt, ...);
int eseUtilsStrSnprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, ...);

// 0-terminated string length
int eseUtilsStrLenGet(ESE_CSTR str);

#else

# include <string.h>

// Re-route to standard implementation
# define eseUtilsStrLenGet     strlen
# define eseUtilsStrSprintf    sprintf
# define eseUtilsStrSnprintf   snprintf

#endif

#ifdef __cplusplus
  }
#endif

#endif  // _utils_str_h_
