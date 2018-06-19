#ifndef _ese_i2c_h_
#define _ese_i2c_h_

#include <i2cConfig.h>

/// EseI2c driver channel implementation
///
class EseI2c : public EseChannel<
                        EseI2c,
                        I2C_HandleTypeDef
                      >
{
public:
  /// I2C channel - specific IO ctl codes
  enum {
    ctlDcb      = ChannelT::stdCtlCodesEnd,   ///< Get|Set I2C DCB
    ctlDevAddr,                               ///< Remote device address
    ctlMemAddr,                               ///< Prepare device memory access call
    ctlMemAddrSize,                           ///< Prepare device memory access call (memory address register size)
                                              ///  Setting mem address size to 0 effectively causes 
                                              ///  simple device calls to be used instead of memory ones
  };

protected:
  EseI2c(I2C_TypeDef* hw) ESE_NOTHROW;

public:
  virtual ~EseI2c() ESE_NOTHROW;

  /// EseChannel interface public services
  ///
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW ESE_OVERRIDE;
  virtual rtosStatus unlock() ESE_NOTHROW ESE_OVERRIDE;
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW ESE_OVERRIDE;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW ESE_OVERRIDE;

  /// Direct access to driver-specific DCB
  ///
  void dcbSet(const I2C_InitTypeDef& dcb) ESE_NOTHROW;
  inline const I2C_InitTypeDef& dcbGet() const ESE_NOTHROW { return m_h.Init; }

  /// Return true if device is configured as I2C master (own address is 0)
  bool isMaster() const ESE_NOTHROW { return 0 == m_h.Init.OwnAddress1 && 0 == m_h.Init.OwnAddress2; }

  /// Return true if memory address calls mode is active
  bool isMemAccessMode() const ESE_NOTHROW { return 0 != m_memAddrSize; }

  /// I2C explicit master services
  size_t masterReceive( esU16 devAddr, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW; 
  size_t masterSend( esU16 devAddr, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW;
  size_t masterMemReceive( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW; 
  size_t masterMemSend( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW;

  /// ISR internal callback services
  ///
  inline bool shouldYieldEv() const ESE_NOTHROW { return m_shouldYieldEv; }
  inline void resetYieldFlagEv() ESE_NOTHROW { m_shouldYieldEv = false; }
  inline bool shouldYieldErr() const ESE_NOTHROW { return m_shouldYieldErr; }
  inline void resetYieldFlagErr() ESE_NOTHROW { m_shouldYieldErr = false; }
  
  void onError() ESE_NOTHROW;
  void onTxComplete() ESE_NOTHROW;
  void onRxComplete() ESE_NOTHROW;

public:
#ifdef USE_I2C_PORT1
  static EseI2c _1;
#endif
#ifdef USE_I2C_PORT2
  static EseI2c _2;
#endif

protected:
  /// EseChannel interface protected services
  virtual bool doInit() ESE_NOTHROW ESE_OVERRIDE;
  virtual void doUninit() ESE_NOTHROW ESE_OVERRIDE;
  virtual bool doCheckConfigured() ESE_NOTHROW ESE_OVERRIDE;
  virtual bool doActivate() ESE_NOTHROW ESE_OVERRIDE;
  virtual void doDeactivate() ESE_NOTHROW ESE_OVERRIDE;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE;
  /// EseChannelT services
  inline const ChannelT::HandleT& doHandleGet() const ESE_NOTHROW { return m_h; }
  inline ChannelT::HandleT& doHandleGet() ESE_NOTHROW { return m_h; }
    
protected:
  EseMutex m_mx;
  EseSemaphore m_sem;
  ChannelT::HandleT m_h;
  esU16 m_devAddr;
  esU16 m_memAddr;
  esU16 m_memAddrSize;
  volatile bool m_shouldYieldEv;
  volatile bool m_shouldYieldErr;
  
  friend ChannelT;
};

#endif // _ese_i2c_h_
