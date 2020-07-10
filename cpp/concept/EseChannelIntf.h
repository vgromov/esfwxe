#ifndef _ese_channel_intf_h_
#define _ese_channel_intf_h_

class EseBreakerIntf;

/// An abstract IO channel services declaration
///
class ESE_ABSTRACT EseChannelIntf
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
  /// Return true if channel is created OK
  virtual bool isOk() const ESE_NOTHROW = 0;

  /// Generic channel interface services
  ///
  /// Check if channel is initialzied
  virtual bool isInitialized() const ESE_NOTHROW = 0;
  
  /// Check if channel is active
  virtual bool isActive() const ESE_NOTHROW = 0;

  /// Check channel connection state. By default, active channel is considered connected
  /// For certain channels, for instance USB ones, active and connected states are different
  virtual bool isConnected() const ESE_NOTHROW = 0;
  
  /// Return true if we're collating TX packet
  virtual bool inTxBatch() const ESE_NOTHROW = 0;

  /// Return true if we're collating RX packet
  virtual bool inRxBatch() const ESE_NOTHROW = 0;
  
  /// Return channel error code, 0, if no error detected
  virtual esU32 errorGet() const ESE_NOTHROW = 0;

  /// Channel locking support
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW = 0;
  virtual rtosStatus unlock() ESE_NOTHROW = 0;

  /// Return data block with size len, transfer time (send|receive) estimate, in ms, 
  /// using current channel settings.
  ///
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW = 0;
  
  /// Initialize channel
  virtual void init() ESE_NOTHROW = 0;
  
  /// Uninitialize channel
  virtual void uninit() ESE_NOTHROW = 0;
  
  /// Check channel configuration is up-to-date
  virtual void checkConfigured() ESE_NOTHROW = 0;

  /// Activate channel. Return true, if channel is activated successfully
  virtual bool activate() ESE_NOTHROW = 0;

  /// De activate channel. Actual implementation must not throw any exception
  virtual void deactivate() ESE_NOTHROW = 0;
  
  /// Set|reset breaker interface
  virtual void breakerSet(EseBreakerIntf* breaker) ESE_NOTHROW = 0;

  /// Try to receive items from channel, in specified ms time span.
  /// @data     [out] Data buffer to read
  /// @toRead   [in]  Count of items to read
  /// @tmo      [in]  Reading timeout, in ms
  /// @return count of items actually read
  ///
  virtual size_t receive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW = 0;

  /// Try to send items via channel, in specified ms time span.
  /// @data     [in]  Data buffer to send
  /// @toWrite  [in]  Count of items to write
  /// @tmo      [in]  Sending timeout, in ms
  /// @return count of items actually sent
  ///
  virtual size_t send(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW = 0;
  
  /// Begin packet (output) operation, if supported
  virtual bool txBatchBegin() ESE_NOTHROW = 0;
  
  /// End packet (output) operation, if supported
  /// @ok       [in]  It's ok to complete TX batch, we should reset it, otherwise.
  virtual void txBatchEnd(bool ok) ESE_NOTHROW = 0;

  /// Begin packet (input) operation, if supported
  virtual bool rxBatchBegin() ESE_NOTHROW = 0;
  
  /// End packet (input) operation, if supported
  /// @ok       [in]  It's ok to complete RX batch, we should reset it, otherwise.
  virtual void rxBatchEnd(bool ok) ESE_NOTHROW = 0;  
  
  /// Execute channel implementation-specific control action
  /// @ctl      [in] Specify channel implementation-specific control code.
  /// @data     [in] Optional data for control request. Set to null by default.
  /// @return   status of io control request
  ///
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW = 0;

  /// Request channel implementation-specific control data.
  /// @ctl      [in]  Specify channel implementation-specific control code.
  /// @data     [out] Data, returned by request. Must not be null. 
  ///                 Caller should provide enough space for requested data.
  /// @return   status of io control request
  ///
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW = 0;
  
  /// Templated ioCtlSet wrappers to simplify typed access
  ///
  
  /// Simple data is transferred to ctl function as void* value
  template <typename DataT>
  int ioCtlSimpleSet(esU32 ctl, DataT in) ESE_NOTHROW
  {
    static_assert( sizeof(void*) >= sizeof(DataT), "Simple ioCtl data must fit void*" );
    
    return ioCtlSet(ctl, reinterpret_cast<void*>(in));
  }

  /// Simple data is transferred out from ctl function as by reference
  template <typename DataT>
  int ioCtlSimpleGet(esU32 ctl, DataT& out) ESE_NOTHROW
  {
    uintptr_t tmp;
    int result = ioCtlGet(ctl, &tmp);
    if( rtosOK == result )
      out = tmp;
    
    return result;
  }

protected:
  /// Internal interface services
  ///
  virtual bool isBreaking(esU32 tmo) const ESE_NOTHROW = 0;                                               ///< Interlocked breaking check
  virtual bool doInit() ESE_NOTHROW = 0;                                                                  ///< Return true if channel initialization was successfull
  virtual void doUninit() ESE_NOTHROW = 0;
  virtual bool doCheckConfigured() ESE_NOTHROW = 0;                                                       ///< Return true if channel configuration was successfull
  virtual bool doActivate() ESE_NOTHROW = 0;                                                              ///< Return true if channel was activated successfully
  virtual void doDeactivate() ESE_NOTHROW = 0;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW = 0;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW = 0;
  virtual bool doTxBatchBegin() ESE_NOTHROW = 0;
  virtual void doTxBatchEnd(bool ok) ESE_NOTHROW = 0;
  virtual bool doRxBatchBegin() ESE_NOTHROW = 0;
  virtual void doRxBatchEnd(bool ok) ESE_NOTHROW = 0;
};

#endif //< _ese_channel_intf_h_
