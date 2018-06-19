#ifndef _ese_uart_h_
#define _ese_uart_h_

#include <uartConfig.h>

/// UART driver channel implementation
///
class EseUart : public EseChannel<
                         EseUart,
                         UART_HandleTypeDef
                       >
{
public:
  /// UART channel - specific IO ctl codes
  enum {
    ctlDcb = ChannelT::stdCtlCodesEnd,  ///< Get|Set UART DCB
    ctlDcbEx,                           ///< Get|Set UART DCBEX
    ctlLineBreak                        ///< Send line break condition
  };

protected:
  /// Misc consts
  enum {
    txoBuffSze = 16
  };

protected:
  EseUart(USART_TypeDef* hw, size_t rxLen, size_t txLen) ESE_NOTHROW;

public:
  virtual ~EseUart() ESE_NOTHROW;

  /// EseChannel interface public services
  ///
  virtual bool isOk() const ESE_NOTHROW;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW;
  virtual rtosStatus unlock() ESE_NOTHROW;
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW;
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW;

  /// Direct access to driver-specific DCB
  ///
  void dcbSet(const UART_InitTypeDef& dcb) ESE_NOTHROW;
  inline const UART_InitTypeDef& dcbGet() const ESE_NOTHROW { return m_h.Init; }

  /// Direct access to driver-specific DCBex
  ///
  void dcbexSet(const UART_AdvFeatureInitTypeDef& dcbex) ESE_NOTHROW;
  const UART_AdvFeatureInitTypeDef& dcbexGet() const ESE_NOTHROW;
  
  /// ISR internal callback services
  ///
  inline bool shouldYield() const ESE_NOTHROW { return m_shouldYield; }
  inline void resetYieldFlag() ESE_NOTHROW { m_shouldYield = false; }
  void onError() ESE_NOTHROW;
  void onTxComplete() ESE_NOTHROW;
  void onItemRxComplete() ESE_NOTHROW;

public:
#ifdef USE_UART_PORT1
  static EseUart _1;
#endif
#ifdef USE_UART_PORT2
  static EseUart _2;
#endif
#ifdef USE_UART_PORT3
  static EseUart _3;
#endif

protected:
  /// Internal helpers
  void txCheckInit() ESE_NOTHROW;

  /// EseChannel interface protected services
  virtual bool doInit() ESE_NOTHROW;
  virtual void doUninit() ESE_NOTHROW;
  virtual bool doCheckConfigured() ESE_NOTHROW;
  virtual bool doActivate() ESE_NOTHROW;
  virtual void doDeactivate() ESE_NOTHROW;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW;
  /// Rate validity checking
  bool isValidRate(esU32 rate) const ESE_NOTHROW;
  /// ChannelT services
  inline const ChannelT::HandleT& doHandleGet() const ESE_NOTHROW { return m_h; }
  inline ChannelT::HandleT& doHandleGet() ESE_NOTHROW { return m_h; }
    
protected:
  EseMutex m_mx;
  EseQueue<esU8> m_rx;
  EseQueue<esU8> m_tx;
  ChannelT::HandleT m_h;
  esU8 m_txtmp[txoBuffSze]; //< Independent small TX buffer
  esU8 m_rxtmp;
  volatile bool m_shouldYield;
  
  friend ChannelT;
};

#endif // _ese_uart_h_
