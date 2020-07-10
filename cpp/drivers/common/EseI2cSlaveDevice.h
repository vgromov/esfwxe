#ifndef _ese_i2c_slave_device_h_
#define _ese_i2c_slave_device_h_

/// Common base implementation of I2C slave device over I2C abstract channel
class EseI2cSlaveDevice
{
public:
  EseI2cSlaveDevice() ESE_NOTHROW ESE_KEEP;
  EseI2cSlaveDevice(EseChannelIntf& i2c, uint16_t addr, uint32_t tmo) ESE_NOTHROW ESE_KEEP;

  /// Check if bus was assigned to device (i.e. device was initialized)
  bool isInitialized() const ESE_NOTHROW ESE_KEEP;

  /// Separate device initializer
  void init(EseChannelIntf& i2c, uint16_t addr, uint32_t tmo) ESE_NOTHROW ESE_KEEP;

  /// Access databus assigned at construction time
  EseChannelIntf& busGet() ESE_NOTHROW ESE_KEEP;
  
  /// Access assigned device address on I2C bus
  uint16_t addrGet() const ESE_NOTHROW ESE_KEEP;
  
  /// Access assigned device IO timeout
  uint32_t timeoutGet() const ESE_NOTHROW ESE_KEEP;
  
protected:
  /// I2C Channel ctl services wrappers, preparing master-slave device IO for specific scenarios
  /// If false is returned - preparation request failed, otherwise, preparation is OK
  ///
  bool chnlMemIoPrepare(uint16_t memaddr, uint16_t memaddrSize) ESE_NOTHROW ESE_KEEP;
  bool chnlSimpleIoPrepare() ESE_NOTHROW ESE_KEEP;
  
protected:
  EseChannelIntf* m_i2c;
  uint32_t m_tmo;
  uint16_t m_addr;

  ESE_NONCOPYABLE(EseI2cSlaveDevice);
};

#endif //< _ese_i2c_slave_device_h_
