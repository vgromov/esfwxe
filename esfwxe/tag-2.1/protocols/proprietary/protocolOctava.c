#include <esfwxe/target.h>
#pragma hdrstop

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <esfwxe/utils.h>
#include <esfwxe/crc.h>
#include <esfwxe/commintf.h>
#include "protocolOctava.h"

// octava data frame reading api
const esU8 c_octavaMagic[4] = {0x11, 0x22, 0x33, 0x44};
esBL octavaProtocolFrameRead(OctavaFrameBuffer* buff, EsChannelIo* chnl)
{
  if( chnlBinaryPatternReceive(chnl, c_octavaMagic, 4, 0) )
  {
    esU32 timeout;
    esU32 toRead, read = 0;
    esU8* bufPos = (esU8*)buff;
    // read octava data header, check data length value
    toRead = 2;
    timeout = 2 * chnlSendTimeEstimateGet(chnl, toRead);
    read = chnlGetBytes(chnl, bufPos, toRead, timeout);

    if( toRead == read &&
      buff->hdr.dataByteLen <= octavaMaxDataSpace_SZE &&
      buff->hdr.dataByteLen > octavaDeviceId_SZE &&
      0 == buff->hdr.dataByteLen % 2 )
    {
      bufPos += read;
      toRead = buff->hdr.dataByteLen+2;
      timeout = 2 * chnlSendTimeEstimateGet(chnl, toRead);
      read = chnlGetBytes(chnl, bufPos, toRead, timeout);
      if( toRead == read )
      {
        esU16 checksum = calcSumForChecksum16(bufPos, read-2);
        return checksum == *((esU16*)(bufPos + (read-2)));
      }
    }
  }

  return FALSE;
}

esBL octavaProtocolFrameWrite(const esU8* data, esU32 dataLen, EsChannelIo* chnl)
{
  if( dataLen && 0 == (dataLen % 2) )
  {  
    esU16 checksum = calcSumForChecksum16(data, dataLen);
    if( 4 == chnlPutBytes(chnl, c_octavaMagic, 4) &&
        2 == chnlPutBytes(chnl, (const esU8*)&dataLen, 2) &&
        dataLen == chnlPutBytes(chnl, data, dataLen) )
      return 2 == chnlPutBytes(chnl, (const esU8*)&checksum, 2);
  }
  
  return FALSE;
}

