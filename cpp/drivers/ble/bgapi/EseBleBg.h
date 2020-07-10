#ifndef _ese_ble_bg_h_
#define _ese_ble_bg_h_

#include <bgapiConfig.h>

// Forward decls
//
#if defined(ESE_BLEBG_USE_MEMINTF)
class EseMemoryIntf;
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
class EseOsQueueIntf;
class EseOsIntf;
#endif
#ifdef ESE_BLEBG_USE_DBGINTF
class EseDebugIntf;
#endif
class EseBleBgWakeupIntf;
class EseBleBgEvtHandlerIntf;
class EseChannelIntf;

/// BlueGiga BLEXXX HAL, build upon abstract channel
///
class EseBleBg
{
public:
  /// State flags
  enum {
    flagRecentReadOk            = 0x0001,
    flagRecentWriteOk           = 0x0002
  };
  
  enum {
    rxDataBuffLen               = 64   ///< RX data buffer len
  };
  
protected:
  EseBleBg(
#if defined(ESE_BLEBG_USE_MEMINTF)
    const EseMemoryIntf& imem,
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
    const EseOsIntf& ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
    const EseDebugIntf& idbg,
#endif
    EseChannelIntf& chnl, 
    EseBleBgWakeupIntf& iwkup 
  ) ESE_NOTHROW;
  
  ~EseBleBg() ESE_NOTHROW;
  
#if defined(ESE_BLEBG_USE_MEMINTF)
  void* operator new(size_t sze, const EseMemoryIntf& imem) ESE_NOTHROW;
  void operator delete(void* ptr, const EseMemoryIntf& imem) ESE_NOTHROW;
#endif

  static EseBleBg*& instance() ESE_NOTHROW ESE_KEEP;

public:
  /// Singleton instance manipulation and access
  static bool isCreated() ESE_NOTHROW ESE_KEEP;
  static EseBleBg* instanceGet() ESE_NOTHROW ESE_KEEP;
  static EseBleBg* create
  (
#if defined(ESE_BLEBG_USE_MEMINTF)
    const EseMemoryIntf& imem,
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
    const EseOsIntf& ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
    const EseDebugIntf& idbg,
#endif
    EseChannelIntf& chnl, 
    EseBleBgWakeupIntf& iwkup
  ) ESE_NOTHROW ESE_KEEP;

  void destroy() ESE_NOTHROW ESE_KEEP;
  
  /// Status check
  ///
  inline bool recentWriteOk() const ESE_NOTHROW { return ES_BIT_IS_SET(m_flags, flagRecentWriteOk); }
  inline bool recentReadOk() const ESE_NOTHROW { return ES_BIT_IS_SET(m_flags, flagRecentReadOk); }
  bool isWokenUp() const ESE_NOTHROW ESE_KEEP;

  void queuedEvtsReset() ESE_NOTHROW;
  
  /// Assigned Channel interface access
  ///
  inline EseChannelIntf& channelGet() ESE_NOTHROW { return m_chnl; }
  inline const EseChannelIntf& channelGet() const ESE_NOTHROW { return m_chnl; }
  
  /// Commands
  ///
  bool connEncrypt(esU8 handle, bool bondCreate) ESE_NOTHROW;
  bool connStatusGet(esU8 handle = 0) ESE_NOTHROW;
  bool connDisconnect(esU8 handle) ESE_NOTHROW;
  bool gapEndProcedure() ESE_NOTHROW;
  bool gapAdvertisingSet(esU16 adv_interval_min, esU16 adv_interval_max, esU8 adv_channels) ESE_NOTHROW;
  bool gapModeSet(esU8 discover, esU8 connect) ESE_NOTHROW;
  bool attrLocalWrite(esU16 handle, esU8 offset, esU8 value_len, const esU8* value_data) ESE_NOTHROW;
  bool attrSend(esU8 connHandle, esU16 handle, esU8 value_len, const esU8* value_data) ESE_NOTHROW;
  bool attrLocalRead(esU16 handle, esU8 offset, esU8& attrLen, esU8* data, esU8 dataLen) ESE_NOTHROW;
  bool attrUserValueReadResponse(esU8 connHandle, esU8 att_error, esU8 value_len, const esU8* value_data) ESE_NOTHROW;
  bool attrUserValueWriteResponse(esU8 connHandle, esU8 att_error) ESE_NOTHROW;
  bool securityMgrBondableSet(bool bondable) ESE_NOTHROW;
  bool securityMgrBondingDelete(esU8 bonding) ESE_NOTHROW;
  bool securityMgrParametersSet(esU8 mitm, esU8 min_key_size, esU8 io_capabilities) ESE_NOTHROW;
  bool securityMgrPasskeyEntrySet(esU8 hconnection, esU32 passkey) ESE_NOTHROW;
  bool sysHello() ESE_NOTHROW;
  bool sysReset(bool dfu = false) ESE_NOTHROW;
  bool deviceAddrGet(bd_addr_t& addr) ESE_NOTHROW;
  bool hwGpioIrqEnable(esU8 port, esU8 mask) ESE_NOTHROW;
  bool hwGpioWrite(esU8 port, esU8 mask, esU8 data) ESE_NOTHROW;
  bool hwGpioDirectionConfig(esU8 port, esU8 dir) ESE_NOTHROW;
  
  /// Responses && events
  ///
  
  // Blocking call helper. First, it checks if request was sucessfully written, if writtenOkCheck is set. 
  // Then, it waits for specified event to occur, for specified count of retries.
  // Each retry includes reading from the device first, then checking for the response|event generated
  // True is returned only if response was read successfully, and it's with specified ID.
  //
  bool blockingWaitForResponseOrEvent(esU8 expectedId, bool writtenOkCheck = true) ESE_NOTHROW;
  
  /// Normal BLE112 events processing
  void process() ESE_NOTHROW;
  
  /// Pluggable event handler callback
  void evtHandlerSet(EseBleBgEvtHandlerIntf* handler) ESE_NOTHROW { m_ihandler = handler; }
  
  static inline bool isPositiveResult(esU16 result) ESE_NOTHROW { return 0 == result; }
  static EseBleBgEvent& internalEvtAccess() ESE_NOTHROW;
  static void swrite(esU8 len1, esU8* data1, esU16 len2, esU8* data2) ESE_NOTHROW;
  
#if defined(ESE_BLEBG_USE_MEMINTF)
  /// Memory interface access
  static const EseMemoryIntf& memIntfGet() ESE_NOTHROW ESE_KEEP;
#endif

#if defined(ESE_BLEBG_USE_DBGINTF)
  static const EseDebugIntf& dbgIntfGet() ESE_NOTHROW ESE_KEEP;
#endif
  
  /// Invalid (connection, bonding, etc.) handle value
  static const esU8 sc_invalidHandle = 0xFF;

protected:
#if defined(USE_BLE_TRACE) && defined(USE_BLE_RX_TRACE)
  void bleBufTrace(const char* bufname, const esU8* data, esU32 len) const ESE_NOTHROW;
#endif 

  // Internal read stage helper
  size_t internalRead(esU8* dest, size_t len) ESE_NOTHROW;

  /// BGAPI endpoint reading implementation
  void read() ESE_NOTHROW;
  
  // Device write helpers, for BGAPI IO
  void write(esU8 len1, esU8* data1, esU16 len2, esU8* data2) ESE_NOTHROW;  
  
  void evtHandle() ESE_NOTHROW;
  void evtPost() ESE_NOTHROW;
  void evtQueuedHandle() ESE_NOTHROW;
  void rspErrorHandle(esU8 rspId, esU16 result) ESE_NOTHROW;
  void rxGarbageHandle(const ble_header& apihdr, const esU8* data) ESE_NOTHROW;
  bool handlerIsBreaking() ESE_NOTHROW;

protected:
  EseBleBgEvent m_evt;
#if defined(ESE_BLEBG_USE_OSINTF)
  EseIntfPtr<EseOsQueueIntf> m_evtsQueue;
  const EseOsIntf& m_ios;
#else
  EseQueue<EseBleBgEvent> m_evtsQueue;
#endif
#if defined(ESE_BLEBG_USE_MEMINTF)
  const EseMemoryIntf& m_imem;
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
  const EseDebugIntf& m_idbg;
#endif
  EseChannelIntf& m_chnl;
  EseBleBgEvtHandlerIntf* m_ihandler;
  EseBleBgWakeupIntf& m_iwkup;
  esU8* m_buff;
  esU32 m_flags;
  volatile bool m_handlingQueue;
  
  ESE_NODEFAULT_CTOR(EseBleBg);
  ESE_NONCOPYABLE(EseBleBg);
};

#endif // _ese_ble_bg_h_
