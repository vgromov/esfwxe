#ifndef _ese_channel_h_
#define _ese_channel_h_

/// Generic byte IO channel base interface implementation.
/// Each channel object is implicitly derived from it through the EseChannelIntf template
/// Each derived channel object must implement the following public services:
///   rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW;
///   rtosStatus unlock() ESE_NOTHROW;
///   esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW;
///
/// and the following protected services:
///   void doInit() ESE_NOTHROW;
///   void doUninit() ESE_NOTHROW;
///   bool doCheckConfigured() ESE_NOTHROW;
///   bool doActivate() ESE_NOTHROW;
///   void doDeactivate() ESE_NOTHROW;
///   size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW;
///   size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW;
///
/// Optionally, the following public services may be overridden:
///   int ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW;
///   int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW;
///
class EseChannelIntfImpl : public EseChannelIntf
{
public:
  EseChannelIntfImpl(size_t rxLen, size_t txLen) ESE_NOTHROW ESE_KEEP;
  virtual ~EseChannelIntfImpl() ESE_NOTHROW ESE_KEEP {}

  /// Partial implementation of EseChannelIntf
  ///
  virtual bool isInitialized() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isActive() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool isConnected() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool inTxBatch() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool inRxBatch() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esU32 errorGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void init() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void uninit() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void checkConfigured() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool activate() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void deactivate() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void breakerSet(EseBreakerIntf* breaker) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t receive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t send(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool txBatchBegin() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void txBatchEnd(bool ok) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool rxBatchBegin() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void rxBatchEnd(bool ok) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;  
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

protected:
  /// Internal interface services
  ///
  virtual bool isBreaking(esU32 tmo) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;                             ///< Interlocked breaking check
  virtual bool doTxBatchBegin() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return true; }
  virtual void doTxBatchEnd(bool ok) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP {}
  virtual bool doRxBatchBegin() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return true; }
  virtual void doRxBatchEnd(bool ok) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP {}

protected:
  EseBreakerIntf* m_breaker;
  volatile esU32 m_flags;
  volatile esU32 m_error;
  size_t m_rxLen;
  size_t m_txLen;  

  ESE_NODEFAULT_CTOR(EseChannelIntfImpl);
  ESE_NONCOPYABLE(EseChannelIntfImpl);
};

/// Generic io channel concept
///
template <
  typename DerivedT,
  typename DriverHandleT
>
class EseChannel : public EseChannelIntfImpl
{
public:
  typedef DriverHandleT HandleT; 
  typedef EseChannel< DerivedT, HandleT > ChannelT;

public:
  EseChannel(size_t rxLen, size_t txLen) ESE_NOTHROW :
  EseChannelIntfImpl(rxLen, txLen)
  {}

  /// Return device-specific driver constant handle
  const DriverHandleT& handleGet() const ESE_NOTHROW
  {
    return static_cast<const DerivedT*>(this)->doHandleGet();
  }

  /// Return device-specific driver handle
  DriverHandleT& handleGet() ESE_NOTHROW
  {
    return static_cast<DerivedT*>(this)->doHandleGet();
  }
};

#endif // _ese_channel_h_
