#include <esfwxe/target.h>
#pragma hdrstop

#include <stdlib.h>
#include <string.h>
#include <esfwxe/utils.h>
#include <esfwxe/commintf.h>
#include "protocolAT.h"
//---------------------------------------------------------------------------

int atCommandSend(EsChannelIo* chnl, const char* cmd, esU32 cmdLen,
  const char* term, esU32 termLen)
{
  // substitute the default command terminator
  if( 0 == term || termLen != 1 )
  {
    term = "\r";
    termLen = 1;
  }

  if( 2 == chnlPutBytes(chnl, (const esU8*)"AT", 2) )
  {
    esBL ok = TRUE;
    if( cmd && 
        cmdLen )
      ok = cmdLen == chnlPutBytes(chnl, (const esU8*)cmd, cmdLen);
    
    if( ok &&
        termLen == chnlPutBytes(chnl, (const esU8*)term, termLen) )
      return atOk;
  }
  
  return atFail;
}
//---------------------------------------------------------------------------

int atCommandRepeat(EsChannelIo* chnl)
{
  if( 2 == chnlPutBytes(chnl, (const esU8*)"A/", 2) )
    return atOk;

  return atFail;
}
//---------------------------------------------------------------------------

int atResponseLineRead(EsChannelIo* chnl, char* out, esU32* outLen,
  const char* sep, esU32 sepLen, esU32 tmo)
{
  if( chnl && out && outLen && *outLen > 0 )
  {
    char* pos = out;
    char* end = out + *outLen;
    esU32 sepPos = 0;
    char c;

    *outLen = 0;

    // substitute default line separator
    if( 0 == sep )
    {
      sep = "\r\n";
      sepLen = 2;
    }

    // read incoming chars one by one
    while( 1 == chnlGetBytes(chnl, (esU8*)&c, 1, tmo) )
    {
      if( sep[sepPos] == c )
        ++sepPos;
      else
      {
        // break with 'not enough buffer' code
        if( pos == end )
        {
          *outLen = pos-out;
          return atNotEnoughBuff;
        }
        // reset separator pos, just in case
        sepPos = 0;
        // copy char to out
        *pos++ = c;
      }

      // we've receive complete separator,
      // check if our buffer is not empty. if not - we're done
      // otherwise - just skip to the next incoming char,
      // not copying anything to out buffer
      if( sepPos >= sepLen )
      {
        sepPos = 0;

        if( pos != out )
        {
          *outLen = pos-out;
          return atOk;
        }
        else
          continue;
      }
    }
  }

  return atFail;
}
//---------------------------------------------------------------------------


