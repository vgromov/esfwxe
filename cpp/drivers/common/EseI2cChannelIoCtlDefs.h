#ifndef _ese_i2c_channel_ioctl_defs_h_
#define _ese_i2c_channel_ioctl_defs_h_

/// I2C channel - specific IO ctl codes
enum EseI2cChannelIoCtl
{
  ctlDcb            = EseChannelIntf::stdCtlCodesEnd,   ///< Get|Set I2C DCB
  ctlRate,                                              ///< I2C clock rate
  ctlMasterMode,                                        ///< I2C device master mode
  ctlDevAddr,                                           ///< Bus Device address
  ctlDevAddr7bit,                                       ///< Bus Device address mode - 7 biit or 10 bit
  ctlMemAddr,                                           ///< Prepare device memory access call
  ctlMemAddrSize                                        ///< Prepare device memory access call (memory address register size)
                                                        ///  Setting mem address size to 0 effectively causes 
                                                        ///  simple device calls to be used instead of memory ones
};

/// I2C channel error codes
enum EseI2cChannelError 
{
  noError                                         = 0,
  busError,
  addressNacked,                                        ///< Master address frame NACKed by slave - no such address or slave not ready
  arbitrationLost,
  overrunOrUnderrun,
  timeout,
  unknownError                                          ///< Undefined - unknown error
};


namespace EseI2cChannel
{

/// Rough max estimate of ms per 2 bytes at specified baud
uint32_t maxByteTimeoutMsGet( uint32_t len, uint32_t rate, uint32_t dataBits ) ESE_NOTHROW ESE_KEEP;

}

#endif //< _ese_i2c_channel_ioctl_defs_h_
