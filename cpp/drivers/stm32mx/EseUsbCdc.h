#ifndef _usb_cdc_h_
#define _usb_cdc_h_

#include <usbCdcConfig.h>

extern "C" 
{
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
}

/// USB CDC device driver channel implementation
///
class EseUsbCdc : public EseChannel<
                             EseUsbCdc,
                             PCD_HandleTypeDef
                           >
{
protected:
  // Misc. constants
  enum { 
    staticIoBlockLen  = 64,
    flagConnected     = 0x00010000
  };

public:
  /// USB connection callback type (NB! it's being called from inside ISR)
  typedef void (*OnConnectedT)(bool connected, bool& shouldYield);
  
  /// USB CDC channel - specific IO ctl codes
  enum {
    ctlDcb            = ChannelT::stdCtlCodesEnd,  ///< Get|Set USB CDC DCB
    ctlHwConnect,                                  ///< HW-dependent USB connection|disconnection
  };

protected:
  EseUsbCdc(size_t rxLen, size_t txLen) ESE_NOTHROW ESE_KEEP;

public:
  virtual ~EseUsbCdc() ESE_NOTHROW ESE_KEEP;

  /// Public EseChannel interface services
  ///
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus unlock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  /// Return true if USB device is actually connected
  virtual bool isConnected() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  /// Direct access to the device's DCB
  const PCD_InitTypeDef& dcbGet() const ESE_NOTHROW ESE_KEEP;
  void dcbSet(const PCD_InitTypeDef& dcb) ESE_NOTHROW ESE_KEEP;

  /// USB device handle access
  USBD_HandleTypeDef& devHandleGet() ESE_NOTHROW ESE_KEEP;
  
  /// Pluggable connection callback access
  void onConnectedSet(OnConnectedT proc) ESE_NOTHROW ESE_KEEP;

  /// Driver singleton instance
  static EseUsbCdc _1;

  /// ISR internal callback services
  ///
  bool shouldYield() const ESE_NOTHROW ESE_KEEP;
  void resetYieldFlag() ESE_NOTHROW ESE_KEEP;

  void usbBuffersSetup() ESE_NOTHROW ESE_KEEP;
  void onRxFromIsr(uint8_t* data, uint32_t* len) ESE_NOTHROW ESE_KEEP;
  void onSetLineCodingFromIsr(const uint8_t* pbuf) ESE_NOTHROW ESE_KEEP;
  void onGetLineCodingFromIsr(uint8_t* pbuf) ESE_NOTHROW ESE_KEEP;
  void onConnect(bool connected) ESE_NOTHROW ESE_KEEP;

protected:
  /// Channel protected services implementation
  virtual bool doInit() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void doUninit() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool doCheckConfigured() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool doActivate() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void doDeactivate() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  /// ChannelT services
  const ChannelT::HandleT& doHandleGet() const ESE_NOTHROW ESE_KEEP;
  ChannelT::HandleT& doHandleGet() ESE_NOTHROW ESE_KEEP;
  
  // Custom USB handler(s)
  static uint8_t onTxComplete(USBD_HandleTypeDef *pdev, uint8_t epnum) ESE_NOTHROW ESE_KEEP;
  uint8_t continueTxFromIsr() ESE_NOTHROW ESE_KEEP;
  
  // TX status helpers
  //
  void txIdleSet() ESE_NOTHROW;
  void txIdleSetFromISR() ESE_NOTHROW;
  void txActiveSet() ESE_NOTHROW;
  rtosStatus txIdleCheck() ESE_NOTHROW;

protected:
  EseMutex m_mx;
  EseSemaphore m_txIdle; 
  ChannelT::HandleT m_h;
  USBD_HandleTypeDef m_devh;
  USBD_CDC_LineCodingTypeDef m_lineCfg;
  esU8 m_tmpRx[staticIoBlockLen];
  esU8 m_tmpTx[staticIoBlockLen];
  EseQueue<esU8> m_rx;
  EseQueue<esU8> m_tx;
  OnConnectedT m_onConnected;
  volatile ptrdiff_t m_prevBlockLen;
  volatile bool m_txActive;
  volatile bool m_shouldYield;

  friend ChannelT;
};

#endif // _usb_cdc_h_
