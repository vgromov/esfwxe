#ifndef _ese_utils_str_h_
#define _ese_utils_str_h_

#include <stdbool.h>

/// String utilities
///

#ifdef __cplusplus
  extern "C" {
#endif

/// Null string
extern const ESE_CSTR c_nullString;

// BCD byte to ASCII char conversion
#define LOBCD2CHAR(nBCD)              ((((esBCD)(nBCD)) & 0x0F) + 0x30)
#define HIBCD2CHAR(nBCD)              ((((esBCD)(nBCD)) >> 4) + 0x30)

/// BCD byte to decimal esU8
#define esBCD2BYTE(nBCD)              ((((esBCD)(nBCD)) >> 4)*10 + (((esBCD)(nBCD)) & 0x0F))

/// Decimal esU8 to esBCD conversion. !! no byte value validation is made, values greater than 99 will be lost !!
#define BYTE2esBCD(n)                 (((((esU8)(n) / 10) % 10) << 4) + ((esU8)(n) % 10))

/// ASCII character qualifiers
///

bool eseUtilsStrIsDigitChar(int ch);
bool eseUtilsStrIsHexChar(int ch);
bool eseUtilsStrIsLowerChar(int ch);
bool eseUtilsStrIsSpaceChar(int ch);

/// Try to convert printable char to lower form
int eseUtilsStrToLowerChar(int ch);

/// Try to convert 2 chars from str into bcd 
bool eseUtilsStr2esBCD(const char *str, esBCD* bcd);

/// Bin to hex conversion
///
/// Convert lo byte nibble to hex byte representation (single char)
char eseUtilsStrLoNibbleToHex(esU8 n);

/// Return how many bin bytes were converted into hex representation
esU32 eseUtilsStrBinToHex(const esU8* bin, esU32 binLen, ESE_STR buff, esU32 buffLen, bool doZeroTerminate);

/// UTF-8 string traversal support
///

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

#ifdef __cplusplus
  }
#endif

#ifdef ES_USE_STRUTILS_IMPL

/// NB! Additionally, tune-up custom formatting function by
/// defining the following macros:
///#define ESE_STRUTILS_USE_NTOA_BUFFER_SIZE          32U
///#define ESE_STRUTILS_USE_FTOA_BUFFER_SIZE          32U
///#define ESE_STRUTILS_DISABLE_SUPPORT_FLOAT
///#define ESE_STRUTILS_DISABLE_SUPPORT_EXPONENTIAL
///#define ESE_STRUTILS_USE_DEFAULT_FLOAT_PRECISION   6U
///#define ESE_STRUTILS_USE_MAX_FLOAT                 1e9
///#define ESE_STRUTILS_DISABLE_SUPPORT_LONG_LONG
///#define ESE_STRUTILS_DISABLE_SUPPORT_PTRDIFF_T
///#define ESE_STRUTILS_USE_PRINTF

# ifdef __cplusplus
  extern "C" {
# endif

#include <stdarg.h>

// abstract string formatter and streamer
#ifdef ESE_STRUTILS_USE_PRINTF
int eseUtilsStrPrintf(ESE_CSTR format, ...);
int eseUtilsStrVprintf(ESE_CSTR format, va_list va);
#endif

// custom implementations of string formatting routines
int eseUtilsStrVsprintf(ESE_STR target, ESE_CSTR fmt, va_list lst);

int eseUtilsStrSprintf(ESE_STR target, ESE_CSTR fmt, ...);
int eseUtilsStrSnprintf(ESE_STR target, esU32 maxTargetLen, ESE_CSTR fmt, ...);

int eseUtilsStrVsnprintf(ESE_STR buffer, size_t count, ESE_CSTR format, va_list va);
int eseUtilsStrFctprintf(void(*out)(char character, void* arg), void* arg, ESE_CSTR format, ...);

// 0-terminated string length
unsigned int eseUtilsStrLenGet(ESE_CSTR str);
unsigned int eseUtilsStrStrnlenGet(ESE_CSTR str, size_t maxsize);

# ifdef __cplusplus
  }
# endif

#else

# ifdef __cplusplus
#   include <cstring>
// Re-route to standard implementation
#   define eseUtilsStrLenGet          std::strlen
#   define eseUtilsStrSprintf         std::sprintf
#   define eseUtilsStrSnprintf        std::snprintf
#   define eseUtilsStrVsprintf        std::vsprintf
#   define eseUtilsStrVsnprintf       std::vsnprintf
# else
#   include <string.h>
// Re-route to standard implementation
#   define eseUtilsStrLenGet          strlen
#   define eseUtilsStrSprintf         sprintf
#   define eseUtilsStrSnprintf        snprintf
#   define eseUtilsStrVsprintf        vsprintf
#   define eseUtilsStrVsnprintf       vsnprintf
# endif

#endif

#endif  // _ese_utils_str_h_
