#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>

#include "EseI2cChannelIoCtlDefs.h"
//----------------------------------------------------------------------

uint32_t EseI2cChannel::maxByteTimeoutMsGet( uint32_t len, uint32_t rate, uint32_t dataBits ) ESE_NOTHROW
{
  uint32_t result = (len * 2000 * (dataBits + 2)) / rate;
  
  return result ? result : 1;
}
//----------------------------------------------------------------------
