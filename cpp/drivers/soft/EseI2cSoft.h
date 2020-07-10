#ifndef _ese_i2c_soft_h_
#define _ese_i2c_soft_h_

/// Generic software implementation of I2C bus
/// Specific Implementation should be finalized in derived class,
/// by providing code for pure virtual services.
///
class EseI2cSoft : public EseChannel<
                            EseI2cSoft,
                            EseI2cSoft //< dummy type for template compatibility
                          >
{
protected:
  EseI2cSoft() ESE_NOTHROW ESE_KEEP;

public:
  virtual ~EseI2cSoft() ESE_NOTHROW ESE_KEEP;

  /// EseChannel interface public services
  ///
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus unlock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  /// Direct access to driver-specific DCB
  ///
  void rateSet(uint32_t rate) ESE_NOTHROW ESE_KEEP;
  uint32_t rateGet() const ESE_NOTHROW ESE_KEEP;

  /// Return true if memory address calls mode is active
  bool isMemAccessMode() const ESE_NOTHROW ESE_KEEP;

  /// I2C explicit master services, for compatibility with I2C hardware driver services
  size_t masterReceive( esU16 devAddr, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW ESE_KEEP; 
  size_t masterSend( esU16 devAddr, const esU8* data, size_t toWrite, esU32 tmo, bool doStop = true ) ESE_NOTHROW ESE_KEEP;
  size_t masterMemReceive( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW ESE_KEEP; 
  size_t masterMemSend( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW ESE_KEEP;

protected:
  /// EseChannel interface protected services
  /// All actual configuration work for this channel should be done in
  /// derived configure-activate-deactivate services 
  ///
  virtual bool doInit() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void doUninit() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  /// I2C-specific methods should be implemented in derived class
  virtual void delay_2() const ESE_NOTHROW = 0;
  virtual void delay_4() const ESE_NOTHROW = 0;
  virtual bool sdaGet() ESE_NOTHROW = 0;
  virtual void sdaSet(bool high) ESE_NOTHROW = 0;
  virtual void sclSet(bool high) ESE_NOTHROW = 0;

  // Optional overridable service
  virtual void masterAbort() ESE_NOTHROW ESE_KEEP;
  
  /// Software I2C micro-services
  bool start() ESE_NOTHROW ESE_KEEP;
  void stop() ESE_NOTHROW ESE_KEEP;
  void ack() ESE_NOTHROW ESE_KEEP;
  void nack() ESE_NOTHROW ESE_KEEP;
  bool ackWait() ESE_NOTHROW ESE_KEEP;
  
  /// Software basic IO
  bool addrRequest(bool forRead) ESE_NOTHROW ESE_KEEP;
  void byteWrite(uint8_t b) ESE_NOTHROW ESE_KEEP;
  uint8_t byteRead() ESE_NOTHROW ESE_KEEP;
  size_t bufferSendInternal( const esU8* data, size_t toWrite, esU32 tmo, bool doStop ) ESE_NOTHROW ESE_KEEP;
  
  /// EseChannelT services
  inline const ChannelT::HandleT& doHandleGet() const ESE_NOTHROW ESE_KEEP;
  inline ChannelT::HandleT& doHandleGet() ESE_NOTHROW ESE_KEEP;
    
protected:
  EseMutexRecursive m_mx;
  uint32_t m_rate;
  esU16 m_devAddr;
  esU16 m_memAddr;
  esU16 m_memAddrSize;
  bool m_devAddr7bit;
  
  friend ChannelT;
};

#endif //< _ese_i2c_soft_h_
