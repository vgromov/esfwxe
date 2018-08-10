#include <esfwxe/target.h>
#pragma hdrstop

#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <esfwxe/utils.h>

#ifdef __arm__

#  ifndef esIsInfF
#    define esIsInfF      isinf
#  endif

#  ifndef esFiniteF
#    define esFiniteF    isfinite
#  endif

#  ifndef esIsNanF
#    define esIsNanF      isnan
#  endif

#elif defined(_MSC_VER)

#  ifndef esIsInfF
#    define esIsInfF(x)  (0==_finite(x))
#  endif

#  ifndef esFiniteF
#    define esFiniteF    _finite
#  endif

#  ifndef esIsNanF
#    define esIsNanF    _isnan
#  endif

#elif defined(__BORLANDC__)

// if we're building escomm package - import implementations from escore
//# if defined(ESCOMM_EXPORTS)
extern int es_finitef(float f);
extern int es_isnanf(float f);
extern float es_modff(float x, float* intpart);

//# else
//
//#    ifndef esFiniteF
//#      define esFiniteF      _finite
//#    endif
//
//#    ifndef esIsInfF
//#      define esIsInfF(x)    (0==_finite(x))
//#    endif
//
//#    ifndef esIsNanF
//#      define esIsNanF        _isnan
//#    endif
//
//# endif
#endif

// special const for empty string
ESE_CSTR c_nullString = "";

#ifndef USE_CUSTOM_DELAY
// MCU tick ns estimate
#define MCU_ns_per_tick       ((esU32)0xFFFFFFFF / Fmcu)

void usDelay(esU32 useconds)
{
  useconds = (useconds * 1000) / MCU_ns_per_tick;
  while( useconds ) --useconds;
}

void nsDelay(esU32 nseconds)
{
  while( nseconds > MCU_ns_per_tick ) 
    nseconds -= MCU_ns_per_tick; 
}
#endif

// return true if float value is not valid
esBL isInvalidFloatValue(float val)
{
  return (esIsInfF(val) || esIsNanF(val));
}

// round float to nearest greater (lesser) int
float utilsRoundf(float f)
{
  if( f >= 0.f )
    f = (float)(int)(f + 0.5f);
  else
    f = (float)(int)(f - 0.5f);

  return f;
}

// try to convert 2 chars from str into bcd 
esBL str2esBCD(const char *str, esBCD* bcd)
{
  if( (*str == ' ' || utilsIsDigitChar( *str )) && utilsIsDigitChar(*(str+1)) )
  {
    *bcd = (esBCD)((*str == ' ') ? 0 : ((*str - 0x30) << 4)) + (*(str+1) - 0x30);
    return TRUE;
  }

  return FALSE;
}

// valid hex chars
static const char c_hexChars[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

// convert lo byte nibble to hex byte representation (single char)
char loNibbleToHex(esU8 n)
{
  return c_hexChars[n & 0x0F];
}

// return how many bin bytes were converted into hex representation
esU32 binToHex( const esU8* bin, esU32 binLen, ESE_STR buff, esU32 buffLen, esBL doZeroTerminate )
{
  // check buffers && lengths. hex buffer length must be at least double of bin buffer len
  if( bin && binLen > 0 && buffLen > 0 && buffLen >= binLen*2 )
  {
    const esU8* binEnd = bin+binLen;
    const esU8* binBeg = bin;
    ESE_STR buffEnd = buff+buffLen;
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
esU32 countSetBits(const esU8* buff, esU32 buffLen)
{
  esU32 result = 0;
  if( buff && buffLen )
  {
    const esU8* end = buff+buffLen;
  
    while( buff < end )
    {
      esU8 tmp = *buff++;
      while( tmp )
      {
        ++result;
        tmp &= (tmp - 1); // this sets rightmost tmp bit in 0, especially useful for sparse ones
      }
    }
  }

  return result;
}

// reverse bit order in input and return reversed result
esU8 utilsReverseBits(esU8 in)
{
  return (esU8)(((((((esU32)in) * 0x0802LU) & 0x22110LU) | ((((esU32)in) * 0x8020LU) & 0x88440LU))*0x10101LU) >> 16);
}
//----------------------------------------------------------------------------------------------

void eseUtilsSwapInt(int* a, int* b)
{
  int tmp = *b;
  *b = *a;
  *a = tmp;
}
//----------------------------------------------------------------------------------------------

void eseUtilsSwapB(esU8* pb, int count)
{
  esU8* pEnd = pb+count;
  esU8 tmp;

  while(pb < pEnd)
  {
    tmp = pb[0];
    pb[0] = pb[1];
    pb[1] = tmp;
    pb += 2;
  }
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

const char* eseUtilsUtf32FromUtf8Get(const char* buff, const char* buffEnd, esU32* utf32)
{
  if( !buff || !buffEnd || !utf32 || buffEnd <= buff )
    return NULL;

  const char* pos = buff;

  bool sequence = false;
  esU8 bytecnt = 0;
  esU32 out = 0;

  while( pos < buffEnd )
  {
    // Check if symbol at pos is stand-alone, or part of combined utf-8 sequence
    esU8 b = *((const esU8*)pos++);

    if( b & 0x80 ) //< Either start of the sequence, or continuation
    {
      if( 0xC0 == (b & 0xC0) ) //< Starting
      {
        if( sequence ) //< Already started - reset and skip
        {
          sequence = false;
          bytecnt = 0;
          out = 0;
          continue;
        }
        else
        {
          sequence = true; //< Start sequence, calculate amount of trailing bytes
          bytecnt = 0;
          while( b & 0x80 )
          {
            ++bytecnt;
            b <<= 1;
          }

          // Check the resulting byte count. If invalid - reset and skip
          if( bytecnt > 4 )
          {
            sequence = false;
            bytecnt = 0;
            out = 0;
            continue;
          }

          // Assign the rest of byte, taking into account that is is already shifted left by bytecnt
          out = ((esU32)b) << (5*bytecnt-6);
          --bytecnt;
        }
      }
      else //< Continuation
      {
        if( !sequence || 0 == bytecnt ) //< Continuation without a start, or bytecount is exceeded, reset and skip
        {
          bytecnt = 0;
          out = 0;
          continue;
        }
        else
        {
          --bytecnt;
          out |= (((esU32)(b & 0x3F)) << bytecnt*6);

          if( 0 == bytecnt ) //< Finished, exiting
            break;
        }
      }
    }
    else //< Stand-alone char, exit immediately
    {
      out = b;
      break;
    }
  }

  if( 0 != bytecnt ) //< Unfinished multibyte sequence, reset out to 0
    out = 0;

  *utf32 = out;

  return pos;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

// data unpacking from buffer, should be ok with unaligned data in buffer
//
static __inline esBL getData(esU8** start, const esU8* end, esU8* data, esU32 size)
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
//----------------------------------------------------------------------------------------------

esBL get_esU8(esU8** start, const esU8* end, esU8* u8)
{
  if( *start < end  )
  {
    *u8 = *(*start);
    ++(*start);    

    return TRUE;
  }
  
  return FALSE;  
}
//----------------------------------------------------------------------------------------------

esBL get_esBL(esU8** start, const esU8* end, esBL* b)
{
  esU8 dummy;
  esBL result = getData(start, end, &dummy, 1);
  *b = dummy;

  return result;
}
//----------------------------------------------------------------------------------------------

esBL get_esU16(esU8** start, const esU8* end, esU16* u16)
{
  return getData(start, end, (esU8*)u16, 2);
}
//----------------------------------------------------------------------------------------------

esBL get_esU32(esU8** start, const esU8* end, esU32* u32)
{
  return getData(start, end, (esU8*)u32, 4);
}
//----------------------------------------------------------------------------------------------

esBL get_esU64(esU8** start, const esU8* end, esU64* u64)
{
  return getData(start, end, (esU8*)u64, 8);
}
//----------------------------------------------------------------------------------------------

esBL get_esF(esU8** start, const esU8* end, esF* f)
{
  return getData(start, end, (esU8*)f, sizeof(esF));
}
//----------------------------------------------------------------------------------------------

esBL get_esD(esU8** start, const esU8* end, esD* d)
{
  return getData(start, end, (esU8*)d, sizeof(esD));
}
//----------------------------------------------------------------------------------------------

esBL get_esBA(esU8** start, const esU8* end, esBA* ba)
{
  esBL result = getData(start, end, (esU8*)&ba->size, 4) &&
                ((*start) + ba->size) <= end;
  
  if( result )
  { 
    ba->data = (esU8*)(*start);   // the byte array is supposed to be part of data bytestream
    *start += ba->size;
  }

  return result;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

// data packing to buffer.
//
static __inline esBL putData(esU8** start, const esU8* end, const esU8* data, esU32 size)
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
//----------------------------------------------------------------------------------------------

esBL put_esU8(esU8** start, const esU8* end, esU8 u8)
{
  if( *start < end )
  {
    **start = u8;
    ++(*start);

    return TRUE;
  }

  return FALSE;  
}
//----------------------------------------------------------------------------------------------

esBL put_esU16(esU8** start, const esU8* end, esU16 u16)
{
  return putData(start, end, (const esU8*)&u16, 2);
}
//----------------------------------------------------------------------------------------------

esBL put_esU32(esU8** start, const esU8* end, esU32 u32)
{
  return putData(start, end, (const esU8*)&u32, 4);
}
//----------------------------------------------------------------------------------------------

esBL put_esU64(esU8** start, const esU8* end, esU64 u64)
{
  return putData(start, end, (const esU8*)&u64, 8);
}
//----------------------------------------------------------------------------------------------

esBL put_esF(esU8** start, const esU8* end, esF f)
{
  return putData(start, end, (const esU8*)&f, sizeof(esF));
}
//----------------------------------------------------------------------------------------------

esBL put_esD(esU8** start, const esU8* end, esD d)
{
  return putData(start, end, (const esU8*)&d, sizeof(esD));
}
//----------------------------------------------------------------------------------------------

esBL put_esBA(esU8** start, const esU8* end, esBA ba)
{
  return put_esU32(start, end, ba.size) &&
         putData(start, end, ba.data, ba.size);
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

// convert float to formatted string representation
//
// formatting helper
static void int2buff(ESE_STR* buff, ESE_CSTR end, int val, esBL neg, int power)
{
  ESE_STR pos = *buff;
  while(pos < end && power)
  {
    int ival = val / power;
    *pos++ = (char)(0x30 + (neg ? -ival : ival));
    val -= ival * power;
    power /= 10;
  }
  *buff = pos;
}

int fmtFloat(ESE_STR buff, int buffLen, float val, int decimals)
{
  int result = 0;
  if( buff && 0 < buffLen && !esIsNanF(val) && esFiniteF(val))
  {
    ESE_CSTR end = buff+buffLen;
    ESE_STR pos = buff;
    int ipower = 1;   // integer part power
    int fpower = 1; // fractional part power
    float fi;
    int fpart;
    int ipart = decimals; // temporarily save decimals
    esBL neg = val < .0f;
    // find fractional part as integer
    while( 0 < decimals-- )
      fpower *= 10;
    decimals = ipart; // restore decimals
    fpart = (int)((es_modff(val, &fi) * (float)fpower) + (neg ? 0.0f : 0.5f));
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
    // terminate with 0 if there is place
    if( pos < end )
      *pos = 0;

    result = pos-buff;
  }

  return result;
}

// format float val with constant relative error
int fmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1)
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

// format float val with constant relative error, return resulting decimals
int fmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals)
{
  float tmp = val;
  *decimals = decimalsAt1;

  while( tmp >= 10.f && *decimals > 0 )
  {
    --(*decimals);
    tmp /= 10.f;
  }

  return fmtFloat(buff, buffLen, val, *decimals);
}

// perform EsMemSpaceInfo calculation by selecting appropriate space unit 
// blockCount is space measured in allocation blocks. blockSize is allocation block size in bytes
void memSpaceCalc(esU32 blockCnt, esU32 blockSize, EsMemSpaceInfo* space)
{
  // max allocation blocks count which may be expressed in units
  esU32 maxBlocks = 0xFFFFFFFF / blockSize;
  esU32 newCount;
  esU32 frac;

  space->frac = 0;
  if( blockCnt > maxBlocks )
  {
    space->unit = msuKbyte;
    if(blockSize < 1024)
    { 
      space->frac = (esU16)(blockCnt % (1024/blockSize));
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
      space->frac = (esU16)frac;
      blockCnt = newCount;
      break;
    }
    frac = newCount % 1024;
    newCount /= 1024;
  }
  space->count = (esU16)blockCnt;
}

#ifdef ES_USE_STRUTILS_IMPL
#  include "esStrUtils.cc"
#endif

