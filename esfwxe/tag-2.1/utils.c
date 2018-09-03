#include <esfwxe/target.h>
#pragma hdrstop

#include <stdbool.h>
#include <esfwxe/utils.h>
#include <esfwxe/utils_float_defs.h>

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
//----------------------------------------------------------------------------------------------

