#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <math.h>
#include <float.h>
#include <common/utils.h>

// special const for empty string
ES_ASCII_CSTR c_nullString = "";

#ifndef USE_CUSTOM_DELAY
// MCU tick ns estimate
#define MCU_ns_per_tick 			((DWORD)0xFFFFFFFF / Fmcu)

void usDelay(DWORD useconds)
{
	useconds = (useconds * 1000) / MCU_ns_per_tick;
	while( useconds ) --useconds;
}

void nsDelay(DWORD nseconds)
{
	while( nseconds > MCU_ns_per_tick ) 
		nseconds -= MCU_ns_per_tick; 
}
#endif

// round float to nearest greater (lesser) int
float roundf(float f)
{
	if( f >= 0.f )
		f = (float)(int)(f + 0.5f);
	else
		f = (float)(int)(f - 0.5f);

	return f;
}

// return true if char corresponds to digit [0..9]
BOOL isDigitChar(char ch)
{
	return (0x30 <= ch && 0x39 >= ch);
}

// try to convert 2 chars from str into bcd 
BOOL str2BCDBYTE(const char *str, BCDBYTE* bcd)
{
	if( (*str == ' ' || isDigitChar( *str )) && isDigitChar(*(str+1)) )
	{
		*bcd = (BCDBYTE)((*str == ' ') ? 0 : ((*str - 0x30) << 4)) + (*(str+1) - 0x30);
		return TRUE;
	}

	return FALSE;
}

// valid hex chars
static const char c_hexChars[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

// convert lo byte nibble to hex byte representation (single char)
char loNibbleToHex(BYTE n)
{
	return c_hexChars[n & 0x0F];
}

// return how many bin bytes were converted into hex representation
DWORD binToHex( const BYTE* bin, DWORD binLen, ES_ASCII_STR buff, DWORD buffLen, BOOL doZeroTerminate )
{
	// check buffers && lengths. hex buffer length must be at least double of bin buffer len
	if( bin && binLen > 0 && buffLen > 0 && buffLen >= binLen*2 )
	{
		const BYTE* binEnd = bin+binLen;
		const BYTE* binBeg = bin;
		ES_ASCII_STR buffEnd = buff+buffLen;
		while(bin < binEnd)
		{
			*buff++ = loNibbleToHex((*bin) >> 4);
			*buff++ = loNibbleToHex(*bin++);
		}
		// zero-terminate char buffer if its length allows it
		if( doZeroTerminate && buff < buffEnd )
			*buff = 0;

		return bin-binBeg;
	}

	return 0;	
}

// count set bits in arbitrary binary buffer
DWORD countSetBits(const BYTE* buff, DWORD buffLen)
{
	DWORD result = 0;
	if( buff && buffLen )
	{
		const BYTE* end = buff+buffLen;
	
		while( buff < end )
		{
			BYTE tmp = *buff++;
			while( tmp )
			{
				++result;
				tmp &= (tmp - 1); // this sets rightmost tmp bit in 0, especially useful for sparse ones
			}
		}
	}

	return result;
}

// data unpacking from buffer, should be ok with unaligned data in buffer
//
static __inline BOOL getData(BYTE** start, const BYTE* end, BYTE* data, DWORD size)
{
	if( end >= (*start + size) )
	{
		while(size--)
		{
			*data = *(*start);
			++data; ++(*start);
		}

		return TRUE;
	}
	
	return FALSE;	
}

BOOL getUINT8(BYTE** start, const BYTE* end, UINT8* u8)
{
	if( *start < end  )
	{
		*u8 = *(*start);
		++(*start);		

		return TRUE;
	}
	
	return FALSE;	
}

BOOL getBOOL(BYTE** start, const BYTE* end, BOOL* b)
{
	BYTE dummy;
	BOOL result = getData(start, end, &dummy, 1);
	*b = dummy;

	return result;
}

BOOL getUINT16(BYTE** start, const BYTE* end, UINT16* u16)
{
	return getData(start, end, (BYTE*)u16, 2); 
}

BOOL getUINT32(BYTE** start, const BYTE* end, UINT32* u32)
{
	return getData(start, end, (BYTE*)u32, 4);	
}

BOOL getUINT64(BYTE** start, const BYTE* end, UINT64* u64)
{
	return getData(start, end, (BYTE*)u64, 8);
}

BOOL getFLOAT(BYTE** start, const BYTE* end, FLOAT* f)
{
	return getData(start, end, (BYTE*)f, sizeof(FLOAT));
}

BOOL getDOUBLE(BYTE** start, const BYTE* end, DOUBLE* d)
{
	return getData(start, end, (BYTE*)d, sizeof(DOUBLE));
}

BOOL getBYTEARRAY(BYTE** start, const BYTE* end, BYTEARRAY* ba)
{
	BOOL result = getData(start, end, (BYTE*)&ba->size, 4) &&
								((*start) + ba->size) <= end;
	
	if( result )
	{ 
		ba->data = (BYTE*)(*start);	 // the byte array is supposed to be part of data bytestream
		*start += ba->size;
	}

	return result;
}

// data packing to buffer.
//
static __inline BOOL putData(BYTE** start, const BYTE* end, const BYTE* data, DWORD size)
{
	if( end >= (*start + size) )
	{
		while(size--)
		{
			*(*start) = *data;
			++data; ++(*start);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL putUINT8(BYTE** start, const BYTE* end, UINT8 u8)
{
	if( *start < end )
	{
		**start = u8;
		++(*start);

		return TRUE;
	}

	return FALSE;	
}

BOOL putUINT16(BYTE** start, const BYTE* end, UINT16 u16)
{
	return putData(start, end, (const BYTE*)&u16, 2);
}

BOOL putUINT32(BYTE** start, const BYTE* end, UINT32 u32)
{
	return putData(start, end, (const BYTE*)&u32, 4);
}

BOOL putUINT64(BYTE** start, const BYTE* end, UINT64 u64)
{
	return putData(start, end, (const BYTE*)&u64, 8);
}

BOOL putFLOAT(BYTE** start, const BYTE* end, FLOAT f)
{
	return putData(start, end, (const BYTE*)&f, sizeof(FLOAT));
}

BOOL putDOUBLE(BYTE** start, const BYTE* end, DOUBLE d)
{
	return putData(start, end, (const BYTE*)&d, sizeof(DOUBLE));
}

BOOL putBYTEARRAY(BYTE** start, const BYTE* end, BYTEARRAY ba)
{
	return putDWORD(start, end, ba.size) &&
				 putData(start, end, ba.data, ba.size);
}

// convert float to formatted string representation
//
// formatting helper
static void int2buff(ES_ASCII_STR* buff, ES_ASCII_CSTR end, int val, BOOL neg, int power)
{
	ES_ASCII_STR pos = *buff;
	while(pos < end && power)
	{
		int ival = val / power;
		*pos++ = (char)(0x30 + (neg ? -ival : ival));
		val -= ival * power;
		power /= 10;
	}
  *buff = pos;
}

int fmtFloat(ES_ASCII_STR buff, int buffLen, float val, int decimals)
{
	int result = 0;
	if( buff && 0 < buffLen && !isnan(val) && !isinf(val))
	{
		ES_ASCII_CSTR end = buff+buffLen;
    ES_ASCII_STR pos = buff;
		int ipower = 1; 	// integer part power
		int fpower = 1; // fractional part power
		double fi;
		int fpart;
		int ipart = decimals; // temporarily save decimals
		BOOL neg = val < .0;
		// find fractional part as integer
		while( 0 < decimals-- )
			fpower *= 10;
		decimals = ipart; // restore decimals
		fpart = (int)((modf(val, &fi) * (float)fpower) + (neg ? 0. : 0.5));
		ipart = (int)fi;
		if( fpart == fpower )
		{
			fpart = 0;
			++ipart;
		}
		fpower /= 10; // correct fpower after rounding of extra digit in prev statement
		if( fpart == fpower*10 )
		{
			++ipart;
			fpart = 0;
		}
		// find power of ten in integer part
		while( ipart / (10*ipower) )
			ipower *= 10;
		// put sign
		if( neg )
			*pos++ = '-';
		// put integer part
		int2buff(&pos, end, ipart, neg, ipower);
		// put decimals
		if( 0 < decimals )
		{
			*pos++ = '.';
			int2buff(&pos, end, fpart, neg, fpower);
		}
		// terminate with 0 it there is place
		if( pos < end )
			*pos = 0;

		result = pos-buff;
	}

	return result;
}

// format float val with constant relative error
int fmtFloatConstRelativeError(ES_ASCII_STR buff, int buffLen, float val, int decimalsAt1)
{
	int decimals = decimalsAt1;
	float tmp = val;

	while( tmp >= 10.f && decimals > 0 )
	{
		--decimals;
		tmp /= 10.f;
	}

	return fmtFloat(buff, buffLen, val, decimals);
}

// perform EsMemSpace calculation by selecting appropriate space unit 
// blockCount is space measured in allocation blocks. blockSize is allocation block size in bytes
void memSpaceCalc(DWORD blockCnt, DWORD blockSize, EsMemSpace* space)
{
	// max allocation blocks count which may be expressed in units
	DWORD maxBlocks = 0xFFFFFFFF / blockSize;
	DWORD newCount;
	DWORD frac = 0;
	space->frac = 0;
	if( blockCnt > maxBlocks )
	{
		space->unit = msuKbyte;
		if(blockSize < 1024)
		{ 
			space->frac = (WORD)(blockCnt % (1024/blockSize));
			blockCnt /= (1024/blockSize);
		}
		else
			blockCnt *= (blockSize/1024);
	}
	else
	{
		space->unit = msuByte;
		blockCnt *= blockSize;
	}
	frac = blockCnt % 1024;
	newCount = blockCnt / 1024;
	// try to normalize value to the biggest unit scale
	while( newCount &&
				 space->unit < msuGbyte )
	{
		++space->unit;
		if( newCount < 1024 )
		{
			space->frac = (WORD)frac;
			blockCnt = newCount;
			break;
		}
		frac = newCount % 1024;
		newCount /= 1024;
	}
	space->count = (WORD)blockCnt;
}
