#ifndef _utils_h_
#define _utils_h_

// miscellaneous utilities
//

#ifdef __cplusplus
	extern "C" {
#endif

// min & max macros
#ifndef MAX
	#define MAX(a,b)	((a) < (b) ? (b) : (a))
#endif

#ifndef MIN
	#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifndef isnan
#	define isnan _isnan
#endif
#ifndef isinf
#	define isinf(x)	(0 == _finite(x))
#endif

// null string 
extern ES_ASCII_CSTR c_nullString;

// const array items count
#ifndef CONST_ARRAY_COUNT
#	define CONST_ARRAY_COUNT( array )	(sizeof(array)/sizeof(array[0]))
#endif

// BCD byte to ASCII char conversion
#define LOBCD2CHAR(nBCD)		((((BCDBYTE)(nBCD)) & 0x0F) + 0x30)
#define HIBCD2CHAR(nBCD)		((((BCDBYTE)(nBCD)) >> 4) + 0x30)

// BCD byte to decimal BYTE
#define BCDBYTE2BYTE(nBCD)	((((BCDBYTE)(nBCD)) >> 4)*10 + (((BCDBYTE)(nBCD)) & 0x0F))
// decimal BYTE to BCDBYTE conversion. !! no byte value validation is made, values greater than 99 will be lost !!
#define BYTE2BCDBYTE(n)			(((((BYTE)(n) / 10) % 10) << 4) + ((BYTE)(n) % 10))

// round float to nearest greater (lesser) int
float roundf(float f);

// bin to hex conversion
//
// convert lo byte nibble to hex byte representation (single char)
char loNibbleToHex(BYTE n);
// return how many bin bytes were converted into hex representation
DWORD binToHex( const BYTE* bin, DWORD binLen, ES_ASCII_STR buff, DWORD buffLen, BOOL doZeroTerminate );

// access bytes in word
//
#ifndef LOBYTE
#	define LOBYTE( w )						((WORD)(w) & 0xFF)
#endif
#ifndef HIBYTE
#	define HIBYTE( w )						(((WORD)(w) >> 8) & 0xFF)
#endif
// make word macro
//
#ifndef MAKEWORD
#	define MAKEWORD(a, b)					((WORD)(((BYTE)(((DWORD)(a)) & 0xff)) | ((WORD)((BYTE)(((DWORD)(b)) & 0xff))) << 8))
#endif

// access words in dword
//
#ifndef LOWORD
#	define LOWORD( dw )						((WORD)(((DWORD)(dw)) & 0xffff))
#endif 
#ifndef HIWORD
#	define HIWORD( dw )						((WORD)((((DWORD)(dw)) >> 16) & 0xffff))
#endif
// make dword macro
//
#ifndef MAKEDWORD
#	define MAKEDWORD(a, b)				((DWORD)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD)(b)) & 0xffff))) << 16))
#endif

// swap word's bytes
#define	SWAPB_WORD( word )			((WORD)((LOBYTE(word) << 8) + HIBYTE(word)))

// bit manipulation
#define ES_BIT_SET( lhs, bit )	(lhs) |= (bit)
#define ES_BIT_CLR( lhs, bit )	(lhs) &= ~(bit)
#define ES_BIT_IS_SET(lhs, bit)	((bit) == ((lhs) & (bit)))
#define ES_BIT_MASK_MATCH(lhs, mask) (0 != ((lhs) & (mask)))

// count set bits in arbitrary binary buffer
DWORD countSetBits(const BYTE* buff, DWORD buffLen);

// ns & mks delay 
void usDelay(DWORD useconds);
void nsDelay(DWORD nseconds);

// if using custom delay implementation, the
// following two functions must be implemented in 
// target application, along with usDelay & nsDelay
#ifdef USE_CUSTOM_DELAY
	// initialize system ticks
	void initSysTicks(void);
	// retrieve current system ticks
	DWORD getSysTicks(void);
#endif

// return true if char corresponds to digit [0..9]
BOOL isDigitChar(char ch);
// try to convert 2 chars from str into bcd 
BOOL str2BCDBYTE(const char *str, BCDBYTE* bcd);

// data unpacking from buffer. works with unaligned data in buffer as well
//
BOOL getUINT8(BYTE** start, const BYTE* end, UINT8* u8);
#define getINT8(start, end, i8)	getUINT8((start), (end), (UINT8*)(i8))
BOOL getUINT16(BYTE** start, const BYTE* end, UINT16* u16);
#define getINT16(start, end, i16) getUINT16((start), (end), (UINT16*)(i16))
BOOL getUINT32(BYTE** start, const BYTE* end, UINT32* u32);
#define getINT32(start, end, i32) getUINT32((start), (end), (UINT32*)(i32))
#define getDWORD getUINT32
BOOL getBOOL(BYTE** start, const BYTE* end, BOOL* b);
BOOL getUINT64(BYTE** start, const BYTE* end, UINT64* u64);
#define getINT64(start, end, i64) getUINT64((start), (end), (UINT64*)(i64))
#define getDATETIME getINT64
#define getTIMESPAN getINT64
BOOL getFLOAT(BYTE** start, const BYTE* end, FLOAT* f);
BOOL getDOUBLE(BYTE** start, const BYTE* end, DOUBLE* d);
BOOL getBYTEARRAY(BYTE** start, const BYTE* end, BYTEARRAY* ba);

// data packing to buffer.
//
BOOL putUINT8(BYTE** start, const BYTE* end, UINT8 u8);
#define putINT8(start, end, i8)	putUINT8((start), (end), (UINT8)(i8))
BOOL putUINT16(BYTE** start, const BYTE* end, UINT16 u16);
#define putINT16(start, end, i16) putUINT16((start), (end), (UINT16)(i16))
BOOL putUINT32(BYTE** start, const BYTE* end, UINT32 u32);
#define putINT32(start, end, i32) putUINT32((start), (end), (UINT32)(i32))
#define putDWORD putUINT32
#define putBOOL putUINT8
BOOL putUINT64(BYTE** start, const BYTE* end, UINT64 u64);
#define putINT64(start, end, i64) putUINT64((start), (end), (UINT64)(i64))
#define putDATETIME putINT64
#define putTIMESPAN putINT64
BOOL putFLOAT(BYTE** start, const BYTE* end, FLOAT f);
BOOL putDOUBLE(BYTE** start, const BYTE* end, DOUBLE d);
BOOL putBYTEARRAY(BYTE** start, const BYTE* end, BYTEARRAY ba);

// custom string formatters
//
// convert float to formatted string representation.
// return number of chars put into buffer
int fmtFloat(ES_ASCII_STR buff, int buffLen, float val, int decimals);
// format float val with constant relative error
int fmtFloatConstRelativeError(ES_ASCII_STR buff, int buffLen, float val, int decimalsAt1);

// perform EsMemSpace calculation by selecting appropriate space unit 
// blockCount is space measured in allocation blocks. blockSize is allocation block size in bytes
void memSpaceCalc(DWORD blockCnt, DWORD blockSize, EsMemSpace* space);

#ifdef __cplusplus
	}
#endif

#endif	// _utils_h_

