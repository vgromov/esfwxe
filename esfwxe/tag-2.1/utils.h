#ifndef _utils_h_
#define _utils_h_

// miscellaneous utilities
//

#ifdef __cplusplus
  extern "C" {
#endif

// Kinda static (compile time) assersion in C
#define ESE_STATIC_ASSERT(COND, MSG) \
typedef char static_assertion_##MSG[(!!(COND))*2-1]

// min & max macros
#ifndef MAX
# define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef MIN
# define MIN(a,b)  ((a) < (b) ? (a) : (b))
#endif

// null string
extern ES_ASCII_CSTR c_nullString;

// const array items count
#ifndef CONST_ARRAY_COUNT
#  define CONST_ARRAY_COUNT( array )  (sizeof(array)/sizeof(array[0]))
#endif

// BCD byte to ASCII char conversion
#define LOBCD2CHAR(nBCD)    ((((esBCD)(nBCD)) & 0x0F) + 0x30)
#define HIBCD2CHAR(nBCD)    ((((esBCD)(nBCD)) >> 4) + 0x30)

// BCD byte to decimal esU8
#define esBCD2BYTE(nBCD)  ((((esBCD)(nBCD)) >> 4)*10 + (((esBCD)(nBCD)) & 0x0F))
// decimal esU8 to esBCD conversion. !! no byte value validation is made, values greater than 99 will be lost !!
#define BYTE2esBCD(n)      (((((esU8)(n) / 10) % 10) << 4) + ((esU8)(n) % 10))

// return true if float value is not valid
esBL isInvalidFloatValue(float val);

// round float to nearest greater (lesser) int
float utilsRoundf(float f);

// bin to hex conversion
//
// convert lo byte nibble to hex byte representation (single char)
char loNibbleToHex(esU8 n);
// return how many bin bytes were converted into hex representation
esU32 binToHex( const esU8* bin, esU32 binLen, ES_ASCII_STR buff, esU32 buffLen, esBL doZeroTerminate );

// access bytes in word
//
#ifndef LOBYTE
#  define LOBYTE( w )            ((esU16)(w) & 0xFF)
#endif
#ifndef HIBYTE
#  define HIBYTE( w )            (((esU16)(w) >> 8) & 0xFF)
#endif
// make word macro
//
#ifndef MAKEWORD
#  define MAKEWORD(a, b)          ((esU16)(((esU8)(((esU32)(a)) & 0xff)) | ((esU16)((esU8)(((esU32)(b)) & 0xff))) << 8))
#endif

// access words in dword
//
#ifndef LOWORD
#  define LOWORD( dw )            ((esU16)(((esU32)(dw)) & 0xffff))
#endif 
#ifndef HIWORD
#  define HIWORD( dw )            ((esU16)((((esU32)(dw)) >> 16) & 0xffff))
#endif
// make dword macro
//
#ifndef MAKEDWORD
#  define MAKEDWORD(a, b)        ((esU32)(((esU16)(((esU32)(a)) & 0xffff)) | ((esU32)((esU16)(((esU32)(b)) & 0xffff))) << 16))
#endif

// swap word's bytes
#define  SWAPB_WORD( word )      ((esU16)((LOBYTE(word) << 8) + HIBYTE(word)))

// bit manipulation
#define ES_BIT_SET( lhs, bit )  (lhs) |= (bit)
#define ES_BIT_CLR( lhs, bit )  (lhs) &= ~(bit)
#define ES_BIT_IS_SET(lhs, bit)  ((bit) == ((lhs) & (bit)))
#define ES_BIT_MASK_MATCH(lhs, mask) (0 != ((lhs) & (mask)))
#define ES_BIT_MASK_MATCH_ALL   ES_BIT_IS_SET

// count set bits in arbitrary binary buffer
esU32 countSetBits(const esU8* buff, esU32 buffLen);
// reverse bit order in input and return reversed result
esU8 utilsReverseBits(esU8 in);

// swap two integer values
void eseUtilsSwapInt(int* a, int* b);

// swap odd and even bytes in buffer. !no check is made against 2^n buffer length!
void eseUtilsSwapB(esU8* pb, int count);

// ns & mks delay 
void usDelay(esU32 useconds);
void nsDelay(esU32 nseconds);

// if using custom delay implementation, the
// following two functions must be implemented in 
// target application, along with usDelay & nsDelay
#ifdef USE_CUSTOM_DELAY
  // initialize system ticks
  void initSysTicks(void);
  // retrieve current system ticks
  esU32 getSysTicks(void);
#endif

// return true if char corresponds to digit [0..9]
#define utilsIsDigitChar(ch) (0x30 <= (ES_ASCII_CHAR)(ch) && 0x39 >= (ES_ASCII_CHAR)(ch))
#define utilsIsLowerCh(ch) ((ES_ASCII_CHAR)(ch) >= 'a' && (ES_ASCII_CHAR)(ch) <= 'z')
// try to convert 2 chars from str into bcd 
esBL str2esBCD(const char *str, esBCD* bcd);

// data unpacking from buffer. works with unaligned data in buffer as well
//
esBL get_esU8(esU8** start, const esU8* end, esU8* u8);
#define get_esI8(start, end, i8)  get_esU8((start), (end), (esU8*)(i8))
esBL get_esU16(esU8** start, const esU8* end, esU16* u16);
#define get_esI16(start, end, i16) get_esU16((start), (end), (esU16*)(i16))
esBL get_esU32(esU8** start, const esU8* end, esU32* u32);
#define get_esI32(start, end, i32) get_esU32((start), (end), (esU32*)(i32))
#define get_DWORD get_esU32
esBL get_esBL(esU8** start, const esU8* end, esBL* b);
esBL get_esU64(esU8** start, const esU8* end, esU64* u64);
#define get_esI64(start, end, i64) get_esU64((start), (end), (esU64*)(i64))
#define get_esDT get_esI64
#define get_esTS get_esI64
esBL get_esF(esU8** start, const esU8* end, esF* f);
esBL get_esD(esU8** start, const esU8* end, esD* d);
esBL get_esBA(esU8** start, const esU8* end, esBA* ba);

// data packing to buffer.
//
esBL put_esU8(esU8** start, const esU8* end, esU8 u8);
#define put_esI8(start, end, i8)  put_esU8((start), (end), (esU8)(i8))
esBL put_esU16(esU8** start, const esU8* end, esU16 u16);
#define put_esI16(start, end, i16) put_esU16((start), (end), (esU16)(i16))
esBL put_esU32(esU8** start, const esU8* end, esU32 u32);
#define put_esI32(start, end, i32) put_esU32((start), (end), (esU32)(i32))
#define put_DWORD put_esU32
#define put_esBL put_esU8
esBL put_esU64(esU8** start, const esU8* end, esU64 u64);
#define put_esI64(start, end, i64) put_esU64((start), (end), (esU64)(i64))
#define put_esDT put_esI64
#define put_esTS put_esI64
esBL put_esF(esU8** start, const esU8* end, esF f);
esBL put_esD(esU8** start, const esU8* end, esD d);
esBL put_esBA(esU8** start, const esU8* end, esBA ba);

// custom string formatters
//
// convert float to formatted string representation.
// return number of chars put into buffer
int fmtFloat(ES_ASCII_STR buff, int buffLen, float val, int decimals);
// format float val with constant relative error
int fmtFloatConstRelativeError(ES_ASCII_STR buff, int buffLen, float val, int decimalsAt1);
// format float val with constant relative error, return resulting decimals
int fmtFloatConstRelativeErrorDecimalsGet(ES_ASCII_STR buff, int buffLen, float val, int decimalsAt1, int* decimals);

// perform EsMemSpaceInfo calculation by selecting appropriate space unit 
// blockCount is space measured in allocation blocks. blockSize is allocation block size in bytes
void memSpaceCalc(esU32 blockCnt, esU32 blockSize, EsMemSpaceInfo* space);

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
int utilsVstrFmtStream(  utilsPfnChStreamFn pfn,  void* target,  ES_ASCII_CSTR fmt, va_list lst);

// custom implementations of string formatting routines
int utilsSprintf(ES_ASCII_STR target, ES_ASCII_CSTR fmt, ...);
int utilsSnprintf(ES_ASCII_STR target, esU32 maxTargetLen, ES_ASCII_CSTR fmt, ...);

// 0-terminated string length
int utilsStrLenGet(ES_ASCII_CSTR str);

#endif // USE_ES_STRFMT_IMPL

#ifndef ES_USE_STRUTILS_IMPL
// Re-route to standard implementation
# define utilsStrLenGet  strlen
# define utilsSprintf    sprintf
# define utilsSnprintf   snprintf
#endif

#ifdef __cplusplus
  }
#endif

#endif  // _utils_h_

