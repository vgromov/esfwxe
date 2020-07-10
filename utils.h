#ifndef _ese_utils_h_
#define _ese_utils_h_

// miscellaneous utilities
//

#ifdef __cplusplus
  extern "C" {
#endif

/// Kinda static (compile time) assersion in C
#define ESE_STATIC_ASSERT(COND, MSG)        typedef char static_assertion_##MSG[(!!(COND))*2-1]

/// Min & max macros
#define ESE_MAX(a,b)                        ((a) < (b) ? (b) : (a))
#define ESE_MIN(a,b)                        ((a) < (b) ? (a) : (b))

/// Const array items count
#define ESE_CONSTARRAY_COUNT( a )           (sizeof(a)/sizeof((a)[0]))

/// Byte juggling macros
#define ESE_LOBYTE( w )                     ((esU16)(w) & 0xFF)
#define ESE_HIBYTE( w )                     (((esU16)(w) >> 8) & 0xFF)
#define ESE_MAKEWORD(a, b)                  ((esU16)(((esU8)(((esU32)(a)) & 0xff)) | ((esU16)((esU8)(((esU32)(b)) & 0xff))) << 8))
#define ESE_LOWORD( dw )                    ((esU16)(((esU32)(dw)) & 0xffff))
#define ESE_HIWORD( dw )                    ((esU16)((((esU32)(dw)) >> 16) & 0xffff))
#define ESE_MAKEDWORD(a, b)                 ((esU32)(((esU16)(((esU32)(a)) & 0xffff)) | ((esU32)((esU16)(((esU32)(b)) & 0xffff))) << 16))
#define ESE_SWAPB_WORD( word )              ((esU16)((LOBYTE(word) << 8) + HIBYTE(word)))
    
/// "Standard" macros re-direction
#ifndef MIN
# define MIN                                ESE_MIN
#endif

#ifndef MAX
# define MAX                                ESE_MAX
#endif

#ifndef CONST_ARRAY_COUNT
# define CONST_ARRAY_COUNT                  ESE_CONSTARRAY_COUNT
#endif

#ifndef LOBYTE
# define LOBYTE                             ESE_LOBYTE
#endif      

#ifndef HIBYTE      
# define HIBYTE                             ESE_HIBYTE
#endif      

#ifndef MAKEWORD      
# define MAKEWORD                           ESE_MAKEWORD
#endif      
      
#ifndef LOWORD      
# define LOWORD                             ESE_LOWORD
#endif                                  

#ifndef HIWORD                          
# define HIWORD                             ESE_HIWORD
#endif      

#ifndef MAKEDWORD     
# define MAKEDWORD                          ESE_MAKEDWORD
#endif      
      
#ifndef SWAPB_WORD
# define SWAPB_WORD                         ESE_SWAPB_WORD
#endif
    
/// Bit manipulation   
#define ES_BIT_SET( lhs, bit )              (lhs) |= (bit)
#define ES_BIT_CLR( lhs, bit )              (lhs) &= ~(bit)
#define ES_BIT_IS_SET(lhs, bit)             ((bit) == ((lhs) & (bit)))
#define ES_BIT_MASK_MATCH(lhs, mask)        (0 != ((lhs) & (mask)))
#define ES_BIT_MASK_MATCH_ALL               ES_BIT_IS_SET

/// Count set bits in arbitrary binary buffer
esU32 countSetBits(const esU8* buff, esU32 buffLen);

/// Reverse bit order in input and return reversed result
esU8 utilsReverseBits(esU8 in);

/// Swap two integer values
void eseUtilsSwapInt(int* a, int* b);

/// Swap odd and even bytes in buffer. !no check is made against 2^n buffer length!
void eseUtilsSwapB(esU8* pb, int count);

/// ns, mks, ms delay 
void msDelay(esU32 ms);
void usDelay(esU32 useconds);
void nsDelay(esU32 nseconds);

/// If using custom delay implementation, the
/// following two functions must be implemented in 
/// target application, along with usDelay & nsDelay
#ifdef USE_CUSTOM_DELAY
  // initialize system ticks
  void initSysTicks(void);
  // retrieve current system ticks
  esU32 getSysTicks(void);
#endif

/// Data unpacking from buffer. works with unaligned data in buffer as well
///
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

/// Data packing to buffer.
///
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

/// Perform EsMemSpaceInfo calculation by selecting appropriate space unit
/// blockCount is space measured in allocation blocks. blockSize is allocation block size in bytes
///
void memSpaceCalc(esU32 blockCnt, esU32 blockSize, EsMemSpaceInfo* space);

#ifdef __cplusplus
  }
#endif

#endif  // _ese_utils_h_

