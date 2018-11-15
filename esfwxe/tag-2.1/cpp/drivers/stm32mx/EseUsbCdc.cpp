#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#include <esfwxe/cpp/concept/EseChannel.h>
#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseSemaphore.h>
#include <esfwxe/cpp/os/EseQueue.h>
#include <esfwxe/cpp/os/EseTask.h>
#include <esfwxe/cpp/os/EseKernel.h>

//----------------------------------------------------------------------
// TODO: conditionally include proper DEFS and HAL for other MCUs
#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>

//----------------------------------------------------------------------
#include <esfwxe/cpp/drivers/stm32mx/EseUsbCdc.h>

#ifndef ESE_USB_ISR_TRACE_CFG
# define ESE_USB_ISR_TRACE_CFG
# define ESE_USB_ISR_TRACE_ON
# define ESE_USB_ISR_TRACE_OFF
#endif

#ifndef ESE_USBCDCRX_TRACE_CFG 
# define ESE_USBCDCRX_TRACE_CFG
# define ESE_USBCDCRX_TRACE_ON
# define ESE_USBCDCRX_TRACE_OFF
#endif

#ifndef ESE_USBCDCTX_TRACE_CFG 
# define ESE_USBCDCTX_TRACE_CFG
# define ESE_USBCDCTX_TRACE_ON
# define ESE_USBCDCTX_TRACE_OFF
#endif

extern "C" 
{

static int8_t CDC_Init_FS     (void);
static int8_t CDC_DeInit_FS   (void);
static int8_t CDC_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS  (uint8_t* pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = 
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,  
  CDC_Receive_FS
};

/// Return default USB_CNTR mask configuration
uint32_t USB_CNTR_MaskGet(void)
{
  return 
    USB_CNTR_CTRM  | 
    USB_CNTR_WKUPM | 
    USB_CNTR_SUSPM | 
//  ERR and ESOF bits never handled at driver callback anyway, so mask them out of ISR handling,
//  to avoid constant flow of these condition on unconnected bus to consume all CPU time on
//  some HW units
//
#ifdef USB_HANDLE_EXTRA_ERRORS
    USB_CNTR_ERRM  | 
    USB_CNTR_ESOFM | 
#endif
    USB_CNTR_RESETM;
}

} // extern "C"

// Check for CDC CubeMX bugs
#if CDC_DATA_HS_MAX_PACKET_SIZE > 256
# error "CDC_DATA_HS_MAX_PACKET_SIZE must not be greater than 256!"
#endif  

//---------------------------------------------------------------------------------
EseUsbCdc EseUsbCdc::_1(USB_CDC_RX_BUFFLEN, USB_CDC_TX_BUFFLEN);

EseUsbCdc::EseUsbCdc(size_t rxLen, size_t txLen) ESE_NOTHROW :
ChannelT(rxLen < staticIoBlockLen ? staticIoBlockLen : rxLen, txLen < staticIoBlockLen ? staticIoBlockLen : txLen),
m_shouldYield(false)
{
  // set-up default line config
  m_lineCfg.bitrate = 9600; /* baud rate*/
  m_lineCfg.format = 0x00;   /* stop bits-1*/
  m_lineCfg.paritytype = 0x00;   /* parity - none*/
  m_lineCfg.datatype = 0x08;    /* nb. of bits 8*/

  /* Init USB_IP */
  /* Link The driver to the stack */
  m_h.pData = &m_devh;
  m_devh.pData = &m_h;

  m_h.Instance = USB;
  m_h.Init.dev_endpoints = 3;
  m_h.Init.speed = PCD_SPEED_FULL;
  m_h.Init.ep0_mps = DEP0CTL_MPS_64;
  m_h.Init.phy_itface = PCD_PHY_EMBEDDED;
  m_h.Init.Sof_enable = DISABLE;
  m_h.Init.low_power_enable = DISABLE;
  m_h.Init.battery_charging_enable = DISABLE;
}

EseUsbCdc::~EseUsbCdc() ESE_NOTHROW
{
  uninit();
}

#pragma Otime
bool EseUsbCdc::isOk() const ESE_NOTHROW
{
  return m_mx.isOk();
}

#pragma Otime
rtosStatus EseUsbCdc::lock(esU32 tmo) ESE_NOTHROW
{
  return m_mx.lock(tmo);
}

#pragma Otime
rtosStatus EseUsbCdc::unlock() ESE_NOTHROW
{
  return m_mx.unlock();
}

void EseUsbCdc::dcbSet(const PCD_InitTypeDef& dcb) ESE_NOTHROW
{
  m_h.Init = dcb;
  if( m_h.Init.speed != PCD_SPEED_FULL )
    m_h.Init.speed = PCD_SPEED_FULL;
  ES_BIT_CLR(m_flags, flagConfigured);
}

/// Channel implementation
bool EseUsbCdc::doInit() ESE_NOTHROW
{
  m_mx.init();
  m_txIdle.init();
  m_rx.init(m_rxLen);
  m_tx.init(m_txLen);
  
  m_txIdle.unlock();
  
  ESE_USB_ISR_TRACE_CFG
  ESE_USBCDCTX_TRACE_CFG
  ESE_USBCDCRX_TRACE_CFG
  
  return true;
}

void EseUsbCdc::doUninit() ESE_NOTHROW
{
  m_rx.uninit();
  m_txIdle.uninit();
  m_tx.uninit();
  m_mx.uninit();
  
  ESE_USB_ISR_TRACE_OFF
  ESE_USBCDCTX_TRACE_OFF
  ESE_USBCDCRX_TRACE_OFF
}

bool EseUsbCdc::doCheckConfigured() ESE_NOTHROW
{
  EseIsrCriticalSection cs;
  return HAL_OK == HAL_PCD_Init(&m_h);
}

#pragma Otime
esU32 EseUsbCdc::dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW
{
  // estimate xfer rate over full speed USB bus, using lowest speed (0,5 MBPS) estimate 
  return 1 + ((len / 500) << 2);
}

#pragma Otime
bool EseUsbCdc::doActivate() ESE_NOTHROW
{
  m_rx.reset();
  m_tx.reset();
  m_txIdle.unlock();

  USBD_Init(&m_devh, &FS_Desc, DEVICE_FS);
  HAL_PCD_DevDisconnect(&m_h);
  HAL_PCD_Init(&m_h);

  // Offet from the beginning of PM memory, to actual packet buffers
  // The math is as follows: num of endpoints used * 2 (reserved for in and out pipes) * 2 (2 descriptors per pipe) * 2 (2 byte per descriptor)
  size_t pmaOffs = m_h.Init.dev_endpoints * 8;

  HAL_PCDEx_PMAConfig(&m_h, 0x00,       PCD_SNG_BUF, pmaOffs );     // EP0 (control in) - always must be there, for USB to function properly
  pmaOffs += USB_FS_MAX_PACKET_SIZE;
  HAL_PCDEx_PMAConfig(&m_h, 0x80,       PCD_SNG_BUF, pmaOffs );     // EP0 (control out) - always must be there, for USB to function properly
  pmaOffs += USB_FS_MAX_PACKET_SIZE;

  HAL_PCDEx_PMAConfig(&m_h, CDC_IN_EP,  PCD_SNG_BUF, pmaOffs );     // CDC commuinication IN
  pmaOffs += CDC_DATA_FS_MAX_PACKET_SIZE;
  HAL_PCDEx_PMAConfig(&m_h, CDC_OUT_EP, PCD_SNG_BUF, pmaOffs );     // CDC commuinication OUT
  pmaOffs += CDC_DATA_FS_MAX_PACKET_SIZE;
  HAL_PCDEx_PMAConfig(&m_h, CDC_CMD_EP, PCD_SNG_BUF, pmaOffs );     // CDC command EP

  USBD_CDC.DataIn = &onTxComplete; ///< Set up our own DataIn handler
  USBD_RegisterClass(&m_devh, &USBD_CDC);
  USBD_CDC_RegisterInterface(&m_devh, &USBD_Interface_fops_FS);

  // Configure and enable USB IRQs
  HAL_NVIC_SetPriority(USB_LP_IRQn, USB_LP_IRQ_PRIORITY, 0); 
  
  // Cleanup some interrupt flags beforhand, if set
  if( __HAL_PCD_GET_FLAG(&m_h, USB_ISTR_ERR) )
  {
    __HAL_PCD_CLEAR_FLAG(&m_h, USB_ISTR_ERR); 
  }
  
  if( __HAL_PCD_GET_FLAG(&m_h, USB_ISTR_ESOF) )
  {
    __HAL_PCD_CLEAR_FLAG(&m_h, USB_ISTR_ESOF); 
  }
  
  // Cleanup potentially pending interrupt
  HAL_NVIC_ClearPendingIRQ(USB_LP_IRQn);

  // Enable interrupt handling
  HAL_NVIC_EnableIRQ(USB_LP_IRQn);
  
  // Switch USB connection pin on
  m_error = USBD_Start(&m_devh);
  
  return 0 == m_error;
}

#pragma Otime
void EseUsbCdc::doDeactivate() ESE_NOTHROW
{
  HAL_PCD_DevDisconnect(&m_h);
  USBD_Stop(&m_devh);

  {
    EseIsrCriticalSection cs;
    USBD_DeInit(&m_devh);
  }

  HAL_NVIC_DisableIRQ(USB_LP_IRQn);  
  m_rx.reset();
  m_tx.reset();
  m_txIdle.unlock();
}

// Internal ISR helpers
#pragma Otime
void EseUsbCdc::usbBuffersSetup() ESE_NOTHROW
{
  USBD_CDC_SetTxBuffer(&m_devh, m_tmpTx, 0);
  USBD_CDC_SetRxBuffer(&m_devh, m_tmpRx);
}

#pragma Otime
void EseUsbCdc::onRxFromIsr(uint8_t* data, uint32_t* len) ESE_NOTHROW
{
  uint8_t* end = data + (*len);
  while( data < end )
  {
    esU8 item = *data++;
    if( rtosErrorQueueFull == m_rx.pushBackFromISR(item, m_shouldYield) )
    {
      esU8 tmp;
      m_rx.popFrontFromISR(tmp, m_shouldYield);
      m_rx.pushBackFromISR(item, m_shouldYield);
      m_error = rtosErrorRxOverrun;
    }
  }

  // Always continue data listening
  USBD_CDC_ReceivePacket(&m_devh);
}

#pragma Otime
void EseUsbCdc::onSetLineCodingFromIsr(const uint8_t* pbuf) ESE_NOTHROW
{
  m_lineCfg.bitrate    = (uint32_t)(pbuf[0] | (pbuf[1] << 8) |
                         (pbuf[2] << 16) | (pbuf[3] << 24));
  m_lineCfg.format     = pbuf[4];
  m_lineCfg.paritytype = pbuf[5];
  m_lineCfg.datatype   = pbuf[6];	  
}

#pragma Otime
void EseUsbCdc::onGetLineCodingFromIsr(uint8_t* pbuf) ESE_NOTHROW
{
  pbuf[0] = (uint8_t)(m_lineCfg.bitrate);
  pbuf[1] = (uint8_t)(m_lineCfg.bitrate >> 8);
  pbuf[2] = (uint8_t)(m_lineCfg.bitrate >> 16);
  pbuf[3] = (uint8_t)(m_lineCfg.bitrate >> 24);
  pbuf[4] = m_lineCfg.format;
  pbuf[5] = m_lineCfg.paritytype;
  pbuf[6] = m_lineCfg.datatype; 
}

#pragma Otime
void EseUsbCdc::onConnect(bool connected) ESE_NOTHROW
{
  if( connected )
    m_flags |= flagConnected;
  else
    m_flags &= ~flagConnected;
}

#pragma Otime
uint8_t EseUsbCdc::continueTxFromIsr() ESE_NOTHROW
{
  // fill as much data from tx as we can fill in m_tmpTx
  esU8 item;
  esU8* pos = m_tmpTx;
  esU8* end = pos + staticIoBlockLen;
  while( 
    pos < end && 
    rtosOK == m_tx.popFrontFromISR(
      item, 
      m_shouldYield
    ) 
  )
    *pos++ = item;
  
  if( m_tmpTx != pos )
  {
    USBD_CDC_SetTxBuffer(&m_devh, m_tmpTx, pos-m_tmpTx);   
    m_error = USBD_CDC_TransmitPacket(&m_devh);

    if( USBD_OK != m_error )
      m_txIdle.unlockFromISR(
        m_shouldYield
      );
    
    return static_cast<uint8_t>(m_error);
  }
  else
  {
    // we're done transmitting TX, set signal TX is idle then
    m_txIdle.unlockFromISR(
      m_shouldYield
    );
  }

  return USBD_OK;
}

#pragma Otime
uint8_t EseUsbCdc::onTxComplete(USBD_HandleTypeDef *pdev, uint8_t epnum) ESE_NOTHROW
{
  USBD_CDC_HandleTypeDef* hcdc = reinterpret_cast<USBD_CDC_HandleTypeDef*>(pdev->pClassData);
  
  if(pdev->pClassData != NULL)
  {
    hcdc->TxState = 0;
    return EseUsbCdc::_1.continueTxFromIsr();
  }

  return USBD_FAIL;
}

#pragma Otime
size_t EseUsbCdc::doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW
{
  esU32 tmoTicks = (tmo == rtosMaxDelay) ? 
    portMAX_DELAY :
    pdMS_TO_TICKS(tmo);

  ESE_USBCDCRX_TRACE_ON

  esU32 prevTs = EseTask::tickCountGet();
  esU32 ts = prevTs;
  esU32 ticksPerByte = tmoTicks/toRead;
  if( !ticksPerByte )
    ticksPerByte = 1;
  
  esU8* pos = data;

  while( toRead )
  {
    rtosStatus stat = m_rx.popFront(
      *pos, 
      ticksPerByte, 
      true
    );
    
    if( rtosOK == stat )
    {
      ESE_USBCDCRX_TRACE_OFF
      
      --toRead;
      ++pos;
      
      ESE_USBCDCRX_TRACE_ON
    }
    
    if( tmoTicks != portMAX_DELAY )
    {
      ts = EseTask::tickCountGet();
      esU32 delta = (ts < prevTs) ? prevTs-ts : ts-prevTs;
      
      if( tmoTicks > delta )
        tmoTicks -= delta;
      else
        break;

      prevTs = ts;
    }
  }
 
  ESE_USBCDCRX_TRACE_OFF
  
  return pos-data;
}

#pragma Otime
size_t EseUsbCdc::doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW
{
  const esU8* pos = data;

  // 1) If there is no ongoing TX operation, we may stuff data from tx queue directly into static buffer
  // with zero waiting time as well
  //
  
  rtosStatus stat = m_txIdle.lock(1);
  
  ESE_USBCDCTX_TRACE_ON
  
  if( rtosOK == stat )
  {
    esU8* sPos = m_tmpTx;
    esU8* sEnd = m_tmpTx+staticIoBlockLen;
    while( sPos < sEnd && toWrite )
    {
      ESE_USBCDCTX_TRACE_OFF
      
      *sPos++ = *pos++;
      --toWrite;
      
      ESE_USBCDCTX_TRACE_ON
    }
      
    // if we still have data to write - append these data to the end of tx queue,
    // we should have free space there now
    while( toWrite )
    {
      ESE_USBCDCTX_TRACE_OFF
    
      stat = m_tx.pushBack(
        *pos, 
        0
      );
      
      ESE_USBCDCTX_TRACE_ON

      if( rtosOK != stat )
        break;
    
      --toWrite;
      ++pos;
    }
    
    // Initiate transmission explicitly
    // Wrap in critical section, to prevent messing with IO irq routines
    // which may gain access to m_devh at this time
    {
      EseIsrCriticalSection cs;

      USBD_CDC_SetTxBuffer(&m_devh, m_tmpTx, sPos-m_tmpTx);
      m_error = USBD_CDC_TransmitPacket(&m_devh);
    }
    
    if( USBD_OK != m_error )
    {
      m_txIdle.unlock();
      
      ESE_USBCDCTX_TRACE_OFF
  
      return 0;    
    }
  }
  
  if( toWrite )
  {
    // 3) Push the rest of data (if any) in tx queue with specified timeout;
    // transmission IRQ routine will drain tx in background, in (at most) staticIoBlockLen chunks
    esU32 tmoTicks = (tmo == rtosMaxDelay) ? 
      portMAX_DELAY :
      pdMS_TO_TICKS(tmo);

    // Always give us at least 1 OS tick to write
    if( 0 == tmoTicks )
      tmoTicks = 1;

    esU32 prevTs = EseTask::tickCountGet();
    esU32 ts = prevTs;

    while( toWrite )
    {
      ESE_USBCDCTX_TRACE_OFF
      
      stat = m_tx.pushBack(
        *pos, 
        tmoTicks, 
        true
      );

      ESE_USBCDCTX_TRACE_ON
      
      if( rtosOK != stat )
        break;
    
      if( tmoTicks != portMAX_DELAY )
      {
        ts = EseTask::tickCountGet();
        esU32 delta = (ts < prevTs) ? prevTs-ts : ts-prevTs;
        
        if( tmoTicks > delta )
          tmoTicks -= delta;
        else
          tmoTicks = 1;

        prevTs = ts;
      }

      --toWrite;
      ++pos;
    }
  }
  
  ESE_USBCDCTX_TRACE_OFF
 
  return pos-data;
}

int EseUsbCdc::ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
      dcbSet(*reinterpret_cast<const PCD_InitTypeDef*>(data));
    break;
  case ctlRate:
    // For USB CDC channel, we are free to set any line rate without de-activating the channel,
    // nothing is wrong with that.
    m_lineCfg.bitrate = reinterpret_cast<esU32>(data);
    break;
  case ctlRxReset:
    m_rx.reset();
    break;
  case ctlTxReset:
    m_tx.reset();
    break;
  case ctlReset:
    m_tx.reset();
    m_rx.reset();
    break;
  case ctlHwConnect:
    if( reinterpret_cast<int>(data) )
      HAL_PCD_DevConnect(&m_h);
    else
      HAL_PCD_DevDisconnect(&m_h);
    break;
  default:
    return ChannelT::ioCtlSet(ctl, data);
  }

  return rtosOK;
}

int EseUsbCdc::ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    *reinterpret_cast<PCD_InitTypeDef*>(data) = m_h.Init;
    break;
  case ctlRate:
    *reinterpret_cast<esU32*>(data) = m_lineCfg.bitrate;
    break;
  case ctlIsValidRate:
    if( reinterpret_cast<esU32>(data) > 921600 )
      return rtosErrorParameterValueInvalid;
    break;
  case ctlRxPendingGet:
    *reinterpret_cast<size_t*>(data) = m_rx.countGet();
     break;
  case ctlTxPendingGet:
    *reinterpret_cast<size_t*>(data) = m_tx.countGet();
    break;
  case ctlHwConnect:
    return rtosErrorParameterNotSupported;
  default:
    return ChannelT::ioCtlGet(ctl, data);
  }

  return rtosOK;
}

//------------------------------------------------------------------------------------------
extern "C" 
{
// Empty USB Device Low Level initialization, which should replace one, generated by CubeMX - usbd_conf.c file
#pragma Otime
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef* ESE_UNUSED(pdev))
{ 
  return USBD_OK;
}

// USB CDC IRQ handler
//
#pragma Otime
void USB_LP_IRQHandler(void) ESE_NOTHROW
{
  EseUsbCdc::_1.resetYieldFlag();
  HAL_NVIC_ClearPendingIRQ(USB_LP_IRQn);

  ESE_USB_ISR_TRACE_ON
  
  HAL_PCD_IRQHandler(&EseUsbCdc::_1.handleGet());

  ESE_USB_ISR_TRACE_OFF

  portEND_SWITCHING_ISR(EseUsbCdc::_1.shouldYield() ? pdTRUE : pdFALSE);
}

//------------------------------------------------------------------------------------------------
// CDC interface

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  CDC_Init_FS
  *         Initializes the CDC media low layer over the FS USB IP
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
#pragma Otime
static int8_t CDC_Init_FS(void)
{
  /* Set Application Buffers */
  EseUsbCdc::_1.usbBuffersSetup();
  EseUsbCdc::_1.onConnect(true);
  
  return (USBD_OK);
}

/**
  * @brief  CDC_DeInit_FS
  *         DeInitializes the CDC media low layer
  * @param  None
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
#pragma Otime
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 5 */ 
  EseUsbCdc::_1.onConnect(false);
  return (USBD_OK);
  /* USER CODE END 5 */ 
}

/**
  * @brief  CDC_Control_FS
  *         Manage the CDC class requests
  * @param  Cmd: Command code            
  * @param  Buf: Buffer containing command data (request parameters)
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
#pragma Otime
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  /* USER CODE BEGIN 6 */
  switch (cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:
 
    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:
 
    break;

  case CDC_SET_COMM_FEATURE:
 
    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  case CDC_SET_LINE_CODING:   
    EseUsbCdc::_1.onSetLineCodingFromIsr(pbuf);
    break;

  case CDC_GET_LINE_CODING:     
    EseUsbCdc::_1.onGetLineCodingFromIsr(pbuf);
    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:
 
    break;    
    
  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Receive_FS
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the opeartion: USBD_OK if all operations are OK else USBD_FAIL
  */
#pragma Otime
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  EseUsbCdc::_1.onRxFromIsr(Buf, Len);

  return (USBD_OK);
}

} // extern "C"
