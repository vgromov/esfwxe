#include <esfwxe/target.h>
#pragma hdrstop

#include <string.h>
#include <esfwxe/utils.h>
#include "mc24xx256.h"

// page read timeout
#define MC24XX256_PGREAD_TMO  50
// chip page count
#define MC24XX256_PAGE_COUNT  0x00000200
// page size in bytes
#define MC24XX256_PAGE_SIZE    0x00000040
// local page write buffer - address word + page 
static esU8 s_wrBuff[MC24XX256_PAGE_SIZE+2];

// write bytes to the chip. only 3 lower bits are used from the chipAddr
// internally, paged operations are invoked to perform continuous writes
esU32 mc24xx256PutBytes(i2cHANDLE handle, esU8 chipAddr, esU16 memAddr, const esU8* data, esU32 dataLen)
{
  const esU8* pos = data;
  const esU8* end = data+dataLen;
  // make chip address
  esU8 addr = MC24XX256_MAKE_ADDR(chipAddr);
  esBL writtenOk = TRUE;
  while( memAddr < MC24XX256_SIZE && 
         writtenOk && pos < end )
  {
    // find initial address within current page
    esU16 writeLen;
    esU16 pageOffs = memAddr % MC24XX256_PAGE_SIZE;
    s_wrBuff[0] = HIBYTE(memAddr);
    s_wrBuff[1] = LOBYTE(memAddr);
    // how many bytes to write this time
    writeLen = MIN(MC24XX256_PAGE_SIZE - pageOffs, end-pos);
    memcpy(s_wrBuff+2, pos, writeLen);
    
    // perform (possibly partial) page write
    writtenOk =  i2cPutBytes(handle, addr, s_wrBuff, writeLen + 2, true) == (writeLen + 2);
    
    if( writtenOk )
    {
      // wait some time until internal write completes
      usDelay(5000);
      // increment page, memAddr, data position  & result
      memAddr += writeLen;
      pos += writeLen;
    }
  }

  return pos-data;
}

// sequentially read bytes from memory chip
esU32 mc24xx256GetBytes(i2cHANDLE handle, esU8 chipAddr, esU16 memAddr, esU8* data, esU32 dataLen)
{
  if( dataLen && 
      memAddr < MC24XX256_SIZE && 
      (memAddr + dataLen) <= MC24XX256_SIZE 
  )
  {
    // make chip address
    esU8 addr = MC24XX256_MAKE_ADDR(chipAddr);
    // prepare mem address 
    s_wrBuff[0] = HIBYTE(memAddr);
    s_wrBuff[1] = LOBYTE(memAddr);

    usDelay(10);
    if( i2cPutBytes(handle, addr, s_wrBuff, 2, false) == 2 )
      return i2cGetBytes(handle, addr, data, dataLen);
  }
  
  return 0;  
}
