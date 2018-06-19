#ifndef _ese_channel_h_
#define _ese_channel_h_

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseBreakerIntf.h>

/// Generic byte IO channel base interface.
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
class EseChannelIntf
{
public:
  /// Common IO Control codes
  ///
  enum {
    ctlRxLen,         ///< Return|Set length _in items_ of RX queue. Setting RX IO buffer length may be not supported by all channel implementations.
    ctlTxLen,         ///< Return|Set length _in items_ of TX queue. Setting TX IO buffer length may be not supported by all channel implementations.
    ctlRxPendingGet,  ///< Return count of items, currently pending in RX queue
    ctlTxPendingGet,  ///< Return count of items, currently pending in TX queue
    ctlRxReset,       ///< Reset RX IO queue contents
    ctlTxReset,       ///< Reset TX IO queue contents
    ctlReset,         ///< Reset currently pending IO, along with IO queues
    ctlRate,          ///< Get|Set IO channel data rate. Not all channels supports rate change. Default implementation just returns 0, and reports OK
    ctlIsValidRate,   ///< Check if channel supports rate specified in data parameter. Return rtosOK if valid, rtosErrorParameterValueInvalid otherwise
    //---------------------------------------------------------------------------------------------------------------------------
    stdCtlCodesEnd    ///< Special value, _must go last_ in this enumeration, _must not_ be used directly. Its sole purpose is to 
                      ///  provide an initial value for misc. specific channel ctl. codes
   };

  enum {
    flagConfigured  = 0x00000001,
    flagInitialized = 0x00000002,
    flagActive      = 0x00000004,
    flagTxBatch     = 0x00000008, ///< We're in the middle of TX batch operation
    flagRxBatch     = 0x00000010 ///< We're in the middle of RX batch operation
  };

public:
  EseChannelIntf(size_t rxLen, size_t txLen) ESE_NOTHROW;
  virtual ~EseChannelIntf() ESE_NOTHROW {}

  /// Return true if channel is created OK
  virtual bool isOk() const ESE_NOTHROW = 0;

  /// Generic channel interface services
  ///
  /// Check if channel is active
  inline bool isActive() const ESE_NOTHROW { return m_flags & flagActive; }

  /// Check channel connection state. By default, active channel is considered connected
  /// For certain channels, for instance USB ones, active and connected states are different
  virtual bool isConnected() const ESE_NOTHROW { return isActive(); }
  
  /// Return true if we're collating TX packet
  inline bool inTxBatch() const ESE_NOTHROW { return m_flags & flagTxBatch; }

  /// Return true if we're collating RX packet
  inline bool inRxBatch() const ESE_NOTHROW { return m_flags & flagRxBatch; }
  
  /// Return channel error code, 0, if no error detected
  esU32 errorGet() const ESE_NOTHROW { return m_error; }

  /// Channel locking support
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW = 0;
  virtual rtosStatus unlock() ESE_NOTHROW = 0;

  /// Return data block with size len, transfer time (send|receive) estimate, in ms, 
  /// using current channel settings.
  ///
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW = 0;
  
  /// Initialize channel
  void init() ESE_NOTHROW;
  
  /// Uninitialize channel
  void uninit() ESE_NOTHROW;
  
  /// Check channel configuration is up-to-date
  void checkConfigured() ESE_NOTHROW;

  /// Activate channel. Return true, if channel is activated successfully
  bool activate() ESE_NOTHROW;

  /// De activate channel. Actual implementation must not throw any exception
  void deactivate() ESE_NOTHROW;
  
  /// Set|reset breaker interface
  void breakerSet(EseBreakerIntf* breaker) ESE_NOTHROW;

  /// Try to receive items from channel, in specified ms time span.
  /// @data     [out] Data buffer to read
  /// @toRead   [in]  Count of items to read
  /// @tmo      [in]  Reading timeout, in ms
  /// @return count of items actually read
  ///
  size_t receive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW;

  /// Try to send items via channel, in specified ms time span.
  /// @data     [in]  Data buffer to send
  /// @toWrite  [in]  Count of items to write
  /// @tmo      [in]  Sending timeout, in ms
  /// @return count of items actually sent
  ///
  size_t send(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW;
  
  /// Begin packet (output) operation, if supported
  bool txBatchBegin() ESE_NOTHROW;
  
  /// End packet (output) operation, if supported
  /// @ok       [in]  It's ok to complete TX batch, we should reset it, otherwise.
  void txBatchEnd(bool ok) ESE_NOTHROW;

  /// Begin packet (input) operation, if supported
  bool rxBatchBegin() ESE_NOTHROW;
  
  /// End packet (input) operation, if supported
  /// @ok       [in]  It's ok to complete RX batch, we should reset it, otherwise.
  void rxBatchEnd(bool ok) ESE_NOTHROW;  
  
  /// Execute channel implementation-specific control action
  /// @ctl      [in] Specify channel implementation-specific control code.
  /// @data     [in] Optional data for control request. Set to null by default.
  /// @return   status of io control request
  ///
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW;

  /// Request channel implementation-specific control data.
  /// @ctl      [in]  Specify channel implementation-specific control code.
  /// @data     [out] Data, returned by request. Must not be null. 
  ///                 Caller should provide enough space for requested data.
  /// @return   status of io control request
  ///
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW;

protected:
  /// Internal interface services
  ///
  bool isBreaking(esU32 tmo) const ESE_NOTHROW;                                                           ///< Interlocked breaking check
  virtual bool doInit() ESE_NOTHROW = 0;                                                                  ///< Return true if channel initialization was successfull
  virtual void doUninit() ESE_NOTHROW = 0;
  virtual bool doCheckConfigured() ESE_NOTHROW = 0;                                                       ///< Return true if channel configuration was successfull
  virtual bool doActivate() ESE_NOTHROW = 0;                                                              ///< Return true if channel was activated successfully
  virtual void doDeactivate() ESE_NOTHROW = 0;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW = 0;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW = 0;
  virtual bool doTxBatchBegin() ESE_NOTHROW { return true; }
  virtual void doTxBatchEnd(bool ok) ESE_NOTHROW {}
  virtual bool doRxBatchBegin() ESE_NOTHROW { return true; }
  virtual void doRxBatchEnd(bool ok) ESE_NOTHROW {}

protected:
  EseBreakerIntf* m_breaker;
  volatile esU32 m_flags;
  volatile esU32 m_error;
  size_t m_rxLen;
  size_t m_txLen;  

private:
  EseChannelIntf() ESE_NOTHROW ESE_REMOVE_DECL;
  EseChannelIntf(const EseChannelIntf&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseChannelIntf& operator=(const EseChannelIntf&) ESE_NOTHROW ESE_REMOVE_DECL;
};

/// Generic io channel concept
///
template <typename DerivedT,
          typename DriverHandleT
          >
class EseChannel : public EseChannelIntf
{
public:
  typedef DriverHandleT HandleT; 
  typedef EseChannel< DerivedT, HandleT > ChannelT;

public:
  EseChannel(size_t rxLen, size_t txLen) ESE_NOTHROW :
  EseChannelIntf(rxLen, txLen)
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
