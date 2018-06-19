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
  /// USB CDC channel - specific IO ctl codes
  enum {
    ctlDcb        = ChannelT::stdCtlCodesEnd,  ///< Get|Set USB CDC DCB
    ctlHwConnect,                              ///< HW-dependent USB connection|disconnection
  };

protected:
  EseUsbCdc(size_t rxLen, size_t txLen) ESE_NOTHROW;

public:
  virtual ~EseUsbCdc() ESE_NOTHROW;

  /// Public EseChannel interface services
  ///
  virtual bool isOk() const ESE_NOTHROW ESE_OVERRIDE;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay ) ESE_NOTHROW ESE_OVERRIDE;
  virtual rtosStatus unlock() ESE_NOTHROW ESE_OVERRIDE;
  virtual esU32 dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW ESE_OVERRIDE;
  virtual int ioCtlSet(esU32 ctl, void* data = 0) ESE_NOTHROW ESE_OVERRIDE;
  virtual int ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW ESE_OVERRIDE;
  /// Return true if USB device is actually connected
  virtual bool isConnected() const ESE_NOTHROW ESE_OVERRIDE { return m_flags & flagConnected; }

  /// Direct access to the device's DCB
  const PCD_InitTypeDef& dcbGet() const ESE_NOTHROW { return m_h.Init; }
  void dcbSet(const PCD_InitTypeDef& dcb) ESE_NOTHROW;

  /// USB device handle access
  inline USBD_HandleTypeDef& devHandleGet() ESE_NOTHROW { return m_devh; }

  /// Driver singleton instance
  static EseUsbCdc _1;

  /// ISR internal callback services
  ///
  inline bool shouldYield() const ESE_NOTHROW { return m_shouldYield; }
  inline void resetYieldFlag() ESE_NOTHROW { m_shouldYield = false; }

  void usbBuffersSetup() ESE_NOTHROW;
  void onRxFromIsr(uint8_t* data, uint32_t* len) ESE_NOTHROW;
  void onSetLineCodingFromIsr(const uint8_t* pbuf) ESE_NOTHROW;
  void onGetLineCodingFromIsr(uint8_t* pbuf) ESE_NOTHROW;
  void onConnect(bool connected) ESE_NOTHROW;

protected:
  /// Channel protected services implementation
  virtual bool doInit() ESE_NOTHROW ESE_OVERRIDE;
  virtual void doUninit() ESE_NOTHROW ESE_OVERRIDE;
  virtual bool doCheckConfigured() ESE_NOTHROW ESE_OVERRIDE;
  virtual bool doActivate() ESE_NOTHROW ESE_OVERRIDE;
  virtual void doDeactivate() ESE_NOTHROW ESE_OVERRIDE;
  virtual size_t doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE;
  virtual size_t doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW ESE_OVERRIDE;
  
  /// ChannelT services
  inline const ChannelT::HandleT& doHandleGet() const ESE_NOTHROW { return m_h; }
  inline ChannelT::HandleT& doHandleGet() ESE_NOTHROW { return m_h; }
  
  // Custom USB handler(s)
  static uint8_t onTxComplete(USBD_HandleTypeDef *pdev, uint8_t epnum) ESE_NOTHROW;
  uint8_t continueTxFromIsr() ESE_NOTHROW;

protected:
  EseMutex m_mx;
  EseMutex m_txIdle; 
  ChannelT::HandleT m_h;
  USBD_HandleTypeDef m_devh;
  USBD_CDC_LineCodingTypeDef m_lineCfg;
  esU8 m_tmpRx[staticIoBlockLen];
  esU8 m_tmpTx[staticIoBlockLen];
  EseQueue<esU8> m_rx;
  EseQueue<esU8> m_tx;
  volatile bool m_shouldYield;

  friend ChannelT;
};

#endif // _usb_cdc_h_
