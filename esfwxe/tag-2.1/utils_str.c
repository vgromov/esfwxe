#include <esfwxe/target.h>
#pragma hdrstop

#include <stdbool.h>
#include <string.h>
#include <esfwxe/utils_str.h>
#include <esfwxe/utils_float_defs.h>
//----------------------------------------------------------------------------------------------

// special const for empty string
ESE_CSTR c_nullString = "";
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
#  include "utils_str.cc"
#endif
//----------------------------------------------------------------------------------------------
