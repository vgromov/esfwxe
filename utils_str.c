#include <esfwxe/target.h>
#pragma hdrstop

#include <stdbool.h>
#include <string.h>
#include <esfwxe/utils_str.h>
#include <esfwxe/utils_float_defs.h>
//----------------------------------------------------------------------------------------------

// special const for empty string
const ESE_CSTR c_nullString = "";
//----------------------------------------------------------------------------------------------

bool eseUtilsStrIsDigitChar(int ch) 
{
  return (0x30 <= (ESE_CHAR)(ch) && 0x39 >= (ESE_CHAR)(ch));
}
//----------------------------------------------------------------------------------------------

bool eseUtilsStrIsHexChar(int ch)
{
  return ((0x30 <= (ESE_CHAR)(ch) && 0x39 >= (ESE_CHAR)(ch)) ||
    ('A' <= (ESE_CHAR)(ch) && 'F' >= (ESE_CHAR)(ch)) ||
    ('a' <= (ESE_CHAR)(ch) && 'f' >= (ESE_CHAR)(ch)));
}
//----------------------------------------------------------------------------------------------

bool eseUtilsStrIsLowerChar(int ch) 
{
  return ((ESE_CHAR)(ch) >= 'a' && (ESE_CHAR)(ch) <= 'z');
}
//----------------------------------------------------------------------------------------------

bool eseUtilsStrIsSpaceChar(int ch)
{
  return (' ' == (ESE_CHAR)(ch)) ||
    ('\t' == (ESE_CHAR)(ch)) ||
    ('\n' == (ESE_CHAR)(ch)) ||
    ('\v' == (ESE_CHAR)(ch)) ||
    ('\f' == (ESE_CHAR)(ch)) ||
    ('\r' == (ESE_CHAR)(ch));
}
//----------------------------------------------------------------------------------------------

int eseUtilsStrToLowerChar(int ch)
{
  if( 
    0x61 <= ch && 
    0x7A >= ch
  )
    return ch-0x20;

  return ch;
}
//----------------------------------------------------------------------------------------------

bool eseUtilsStr2esBCD(const char *str, esBCD* bcd)
{
  if(
    (
      *str == ' ' || 
      eseUtilsStrIsDigitChar(*str)
    ) && 
    eseUtilsStrIsDigitChar(*(str + 1))
  )
  {
    *bcd = (esBCD)((*str == ' ') ? 0 : ((*str - 0x30) << 4)) + (*(str + 1) - 0x30);
  
    return true;
  }

  return false;
}
//----------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------

static const char sc_hexChars[16] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

char eseUtilsStrLoNibbleToHex(esU8 n)
{
  return sc_hexChars[n & 0x0F];
}
//----------------------------------------------------------------------------------------------

esU32 eseUtilsStrBinToHex(const esU8* bin, esU32 binLen, ESE_STR buff, esU32 buffLen, bool doZeroTerminate)
{
  // check buffers && lengths. hex buffer length must be at least double of bin buffer len
  if(
    bin && 
    binLen > 0 && 
    buffLen > 0 && 
    buffLen >= binLen * 2
  )
  {
    const esU8* binEnd = bin + binLen;
    const esU8* binBeg = bin;
    ESE_STR buffEnd = buff + buffLen;
    while(bin < binEnd)
    {
      *buff++ = eseUtilsStrLoNibbleToHex((*bin) >> 4);
      *buff++ = eseUtilsStrLoNibbleToHex(*bin++);
    }
    // zero-terminate char buffer if its length allows it
    if(doZeroTerminate && buff < buffEnd)
      *buff = 0;

    return (esU32)(bin - binBeg);
  }

  return 0;
}
//----------------------------------------------------------------------------------------------

const char* eseUtilsStrUtf32FromUtf8Get(const char* buff, const char* buffEnd, esU32* utf32)
{
  if( !buff || !buffEnd || !utf32 || buffEnd <= buff )
    return NULL;

  const char* pos = buff;

  bool sequence = false;
  esU8 bytecnt = 0;
  esU32 out = 0xFFFFFFFF;

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
          out = 0xFFFFFFFF;
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
            out = 0xFFFFFFFF;
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
          out = 0xFFFFFFFF;
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
    out = 0xFFFFFFFF;

  *utf32 = out;

  return pos;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void eseUtilsStrFmtInt(ESE_STR* buff, ESE_CSTR end, int val, bool neg, int power)
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
//----------------------------------------------------------------------------------------------

void eseUtilsStrFmtIntN(ESE_STR* buff, int buffLen, int val)
{
  eseUtilsStrFmtInt(
    buff, 
    (ESE_CSTR)buff+buffLen, 
    val, 
    (val < 0),
    1
  );
}
//----------------------------------------------------------------------------------------------

int eseUtilsStrFmtFloat(ESE_STR buff, int buffLen, float val, int decimals)
{
  int result = 0;
  if( 
    buff && 
    0 < buffLen && 
    !esIsNanF(val) && 
    esFiniteF(val)
  )
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
    fpart = (int)((esModfF(val, &fi) * (float)fpower) + (neg ? 0.0f : 0.5f));
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
    eseUtilsStrFmtInt(&pos, end, ipart, neg, ipower);
    // put decimals
    if( 0 < decimals )
    {
      *pos++ = '.';
      eseUtilsStrFmtInt(&pos, end, fpart, neg, fpower);
    }
    // terminate with 0 if there is place
    if( pos < end )
      *pos = 0;

    result = pos-buff;
  }

  return result;
}
//----------------------------------------------------------------------------------------------

// format float val with constant relative error
int eseUtilsStrFmtFloatConstRelativeError(ESE_STR buff, int buffLen, float val, int decimalsAt1)
{
  int decimals = decimalsAt1;
  float tmp = val;

  while( tmp >= 10.f && decimals > 0 )
  {
    --decimals;
    tmp /= 10.f;
  }

  return eseUtilsStrFmtFloat(buff, buffLen, val, decimals);
}
//----------------------------------------------------------------------------------------------

// format float val with constant relative error, return resulting decimals
int eseUtilsStrFmtFloatConstRelativeErrorDecimalsGet(ESE_STR buff, int buffLen, float val, int decimalsAt1, int* decimals)
{
  float tmp = val;
  *decimals = decimalsAt1;

  while( tmp >= 10.f && *decimals > 0 )
  {
    --(*decimals);
    tmp /= 10.f;
  }

  return eseUtilsStrFmtFloat(buff, buffLen, val, *decimals);
}
//----------------------------------------------------------------------------------------------

#ifdef ES_USE_STRUTILS_IMPL
# include <esfwxe/utils.h>
# include "utils_str.cc"
#endif
//----------------------------------------------------------------------------------------------
