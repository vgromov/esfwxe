#include <esfwxe/target.h>
#pragma hdrstop
//
//  $Id: usbser.c 325 2008-11-09 04:58:13Z jcw $
//  $Revision: 325 $
//  $Author: jcw $
//  $Date: 2008-11-08 23:58:13 -0500 (Sat, 08 Nov 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbser/usbser.c $
//

#include <stdio.h>
#include <string.h>

#include <esfwxe/core/rtos_intf.h>

#include <esfwxe/commintf.h>
#include <esfwxe/utils.h>

#include "../usbapi.h"
#include "usbser.h"

#define INT_IN_EP     0x81
#define BULK_OUT_EP   0x05
#define BULK_IN_EP    0x82

#define MAX_PACKET_SIZE 64
#define BULKBUF_SIZE          MAX_PACKET_SIZE

#define LE_WORD(x)    ((x)&0xFF),((x)>>8)

// CDC definitions
#define CS_INTERFACE      0x24
#define CS_ENDPOINT       0x25

#define SET_LINE_CODING         0x20
#define GET_LINE_CODING         0x21
#define SET_CONTROL_LINE_STATE  0x22

#define CONTROLLINE_DTR 0x01
#define CONTROLLINE_RTS 0x02

// state flags
enum {
    flagsInitialized     = 0x0001,
    flagsConnected         = 0x0002,
};

typedef struct {
    esU16 controlLines;
    esU8 abBulkBuf[BULKBUF_SIZE];
    esU8 abClassReqData[8];
    rtosQUEUE_HANDLE rx;
    rtosQUEUE_HANDLE tx;
    rtosMUTEX_HANDLE mutex;                     // mutex for port sharing between tasks
    volatile esU16 flags;
    usbserDCB dcb;

} UsbSer;
static UsbSer s_usbser;

// descriptors block
//
static esU8 s_descriptors[256] = 
{
  //  Device descriptor
  //
  0x12,
  DESC_DEVICE,
  LE_WORD(0x0101),      // bcdUSB
  0x02,                 // bDeviceClass            - Communication Interface Class
  0x00,                 // bDeviceSubClass
  0x00,                 // bDeviceProtocol
  MAX_PACKET_SIZE0,     // bMaxPacketSize
  LE_WORD(0xffff),      // idVendor
  LE_WORD(0x0005),      // idProduct
  LE_WORD(0x0100),      // bcdDevice release number
  0x01,                 // iManufacturer
  0x02,                 // iProduct
  0x03,                 // iSerialNumber
  0x01,                 // bNumConfigurations

  //
  //  Configuration descriptor
  //
  0x09,
  DESC_CONFIGURATION,
  LE_WORD(67),          // wTotalLength
  0x02,                 // bNumInterfaces
  0x01,                 // bConfigurationValue
  0x00,                 // iConfiguration
  0xc0,                 // bmAttributes
  0x32,                 // bMaxPower

  //
  //  Control class interface
  //
  0x09,
  DESC_INTERFACE,
  0x00,                 // bInterfaceNumber
  0x00,                 // bAlternateSetting
  0x01,                 // bNumEndPoints
  0x02,                 // bInterfaceClass        - Communication Interface Class
  0x02,                 // bInterfaceSubClass    - Abstract Control Model
  0x01,                 // bInterfaceProtocol - Common AT commands (also known as “Hayes™ compatible”) linux requires value of 1 for the cdc_acm module
  0x00,                 // iInterface

  //
  //  Header functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x00,
  LE_WORD(0x0110),

  //
  //  Call management functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x01,
  0x01,                 // bmCapabilities = device handles call management
  0x01,                 // bDataInterface

  //
  //  ACM functional descriptor
  //
  0x04,
  CS_INTERFACE,
  0x02,
  0x02,                 // bmCapabilities

  //
  //  Union functional descriptor
  //
  0x05,
  CS_INTERFACE,
  0x06,
  0x00,                 // bMasterInterface
  0x01,                 // bSlaveInterface0

  //
  //  Notification EP
  //
  0x07,
  DESC_ENDPOINT,
  INT_IN_EP,            // bEndpointAddress
  0x03,                 // bmAttributes = intr
  LE_WORD(8),           // wMaxPacketSize
  0x0a,                 // bInterval

  //
  //  Data class interface descriptor
  //
  0x09,
  DESC_INTERFACE,
  0x01,                 // bInterfaceNumber
  0x00,                 // bAlternateSetting
  0x02,                 // bNumEndPoints
  0x0a,                 // bInterfaceClass = data
  0x00,                 // bInterfaceSubClass
  0x00,                 // bInterfaceProtocol
  0x00,                 // iInterface

  //
  //  Data EP OUT
  //
  0x07,
  DESC_ENDPOINT,
  BULK_OUT_EP,              // bEndpointAddress
  0x02,                     // bmAttributes = bulk
  LE_WORD(MAX_PACKET_SIZE), // wMaxPacketSize
  0x00,                     // bInterval

  //
  //  Data EP in
  //
  0x07,
  DESC_ENDPOINT,
  BULK_IN_EP,               // bEndpointAddress
  0x02,                     // bmAttributes = bulk
  LE_WORD(MAX_PACKET_SIZE), // wMaxPacketSize
  0x00,                     // bInterval

  //
  //  String descriptors
  //
  0x04,
  DESC_STRING,
  LE_WORD(0x0409)
};

//
//
//
static void usbHardwareFrameIntHandler(esU16 wFrame)
{
  usbHardwareNakIntEnable(INACK_BI);
}

//
//  Local function to handle incoming bulk data.  This needs to copy the incoming data to the VCOM RX queue.
//
static int usbserBulkOut(esU8 bEP, esU8 bEPStatus)
{
  int i;
  int iLen;
  esBL higherPriorityTaskWoken = FALSE;

  iLen = usbHardwareEndpointRead(bEP, s_usbser.abBulkBuf, BULKBUF_SIZE);

  for (i = 0; i < iLen; i++) 
    rtosQueuePushBackFromIsr(s_usbser.rx, &s_usbser.abBulkBuf[i], &higherPriorityTaskWoken);

  return higherPriorityTaskWoken;
}

//
//  Local function to handle outgoing bulk data.  This needs to copy any user data in the VCOM TX queue to the bulk endpoint.
//
static int usbserBulkIn(esU8 bEP, esU8 bEPStatus)
{
  int i;
  esBL higherPriorityTaskWoken = FALSE;
  
  for (i = 0; i < MAX_PACKET_SIZE; i++)
    if( !rtosQueuePopFromIsr(s_usbser.tx, &s_usbser.abBulkBuf[i], &higherPriorityTaskWoken) )
      break;

  if (i == 0)
    usbHardwareNakIntEnable(INACK_NONE);
  else
    usbHardwareEndpointWrite(bEP, s_usbser.abBulkBuf, i);

  return higherPriorityTaskWoken;
}

//
//  Local function to handle the USB-CDC class requests
//
static esBL usbserHandleClassRequest (TSetupPacket *pSetup, int *piLen, esU8 **ppbData)
{
  switch (pSetup->bRequest) 
  {
    case SET_LINE_CODING:
      {
        memcpy((esU8*)&s_usbser.dcb, *ppbData, 7);
        *piLen = 7;
      }
      break;

    case GET_LINE_CODING:
      {
        *ppbData = (esU8*)&s_usbser.dcb;
        *piLen = 7;
      }
      break;

    case SET_CONTROL_LINE_STATE:
      {
        s_usbser.controlLines = pSetup->wValue;
      }
      break;

    default:
      return FALSE;
  }

  return TRUE;
}

//
//  Writes one character to VCOM port
//
static esBL usbserPutByte(esU8 b, esU32 tmo)
{
  if(!(s_usbser.flags & flagsInitialized))
    return FALSE;
  if(!(s_usbser.controlLines & CONTROLLINE_DTR))
    return TRUE;

  return rtosQueuePushBack(s_usbser.tx, &b, tmo);
}

esU32 usbserPutBytes(EseChannelIo* chnl, const esU8* data, esU32 len, esU32 tmo)
{
    const esU8* beg = data;
    const esU8* end = data+len;

  if( (s_usbser.flags & flagsConnected) && 
            (s_usbser.controlLines & CONTROLLINE_DTR) )
    {
      while(!chnlIsBreaking(chnl) &&
                    data < end && 
                    usbserPutByte(*data, tmo) )
            ++data;
    }

  return data-beg;
}

//
//  Reads one character from VCOM port
//
static esBL usbserGetByte(esU8* b, esU32 tmo)
{
  if( !(s_usbser.flags & flagsConnected) )
    return FALSE;

  return rtosQueuePop(s_usbser.rx, b, tmo);
}

// read data buffer from usb queue
esU32 usbserGetBytes(EseChannelIo* chnl, esU8* data, esU32 len, esU32 tmo)
{
  if( !(s_usbser.flags & flagsConnected) )
    return 0;
    else
    {
        esU8* beg = data;
        esU8* end = data+len;
    
      while( !chnlIsBreaking(chnl) &&
                        data < end && 
                        usbserGetByte(data, tmo) )
            ++data;
    
      return data-beg;
    }
}

//
//  Default reset handler gets overridden so we can clear line state on
//  disconnect (this is only really useful when the board is not powered
//  from the USB connection, because if it is, power is going away when
//  the cable is unplugged...)
//
static int usbHandleDevStatus(esU8 bDevStatus)
{
    int result = 0;
     s_usbser.controlLines = 0;

    if( DEV_STATUS_CONNECT != (bDevStatus & DEV_STATUS_CONNECT) && flagsConnected == (s_usbser.flags & flagsConnected) )
    {
        s_usbser.flags &=    ~flagsConnected;
        result = usbserOnDisconnect();
    }
    else if( DEV_STATUS_CONNECT == (bDevStatus & DEV_STATUS_CONNECT) && flagsConnected != (s_usbser.flags & flagsConnected) )
    {
        s_usbser.flags |=    flagsConnected;
        result = usbserOnConnect();
    }

    return result;
}

// init serial usb device, providing identification information
//
usbserHANDLE usbserInit(const usbserDCB* dcb, esU16 vendorId, esU16 productId, 
    const esU8* mfgUstr, esU8 mfgLen,
    const esU8* productUstr, esU8 productLen,
    const esU8* serialUstr, esU8 serialLen)
{
    if( !(s_usbser.flags & flagsInitialized) )
    {
        esU8* descrPos = s_descriptors;
      usbInit();
        // pre populate dcb and descriptors
        s_usbser.dcb = *dcb;
        // vendorId
        descrPos += 8;
        memcpy(descrPos, &vendorId, 2);
        descrPos += 2;
        memcpy(descrPos, &productId, 2);
        // mfg unicode string
        descrPos = s_descriptors+89;
        *descrPos++ = mfgLen+2;
        *descrPos++ = DESC_STRING;
        memcpy(descrPos, mfgUstr, mfgLen);
        descrPos += mfgLen;
        // product unicode string
        *descrPos++ = productLen+2;
        *descrPos++ = DESC_STRING;
         memcpy(descrPos, productUstr, productLen);
        descrPos += productLen;
        // serial unicode string
        *descrPos++ = serialLen+2;
        *descrPos++ = DESC_STRING;
         memcpy(descrPos, serialUstr, serialLen);
        descrPos += serialLen;
        // finally, terminate descriptor with 0
        *descrPos = 0;
        // init usb request handlers
      usbHardwareRegisterDevIntHandler(usbHandleDevStatus);
      usbRegisterDescriptors(s_descriptors);
      usbRegisterRequestHandler(REQTYPE_TYPE_CLASS, usbserHandleClassRequest, s_usbser.abClassReqData);
      usbHardwareRegisterEPIntHandler(INT_IN_EP, NULL);
      usbHardwareRegisterEPIntHandler(BULK_IN_EP, usbserBulkIn);
      usbHardwareRegisterEPIntHandler(BULK_OUT_EP, usbserBulkOut);
      usbHardwareRegisterFrameHandler(usbHardwareFrameIntHandler);
      usbHardwareNakIntEnable(INACK_BI);
        
        // finalize init
      s_usbser.rx = rtosQueueCreate(USBSER_RX_QUEUE_LEN, sizeof(esU8));
      s_usbser.tx = rtosQueueCreate(USBSER_TX_QUEUE_LEN, sizeof(esU8));
        s_usbser.mutex = rtosMutexCreate();
       s_usbser.flags = flagsInitialized;
        usbSetupInterruptHandler();
      usbHardwareConnect(TRUE);
    }

    return &s_usbser;
}

// try to mutually exclusive acquire port resource
esBL usbserLockPort(EseChannelIo* chnl, esU32 tmo)
{
    if( chnl && chnl->m_bus != INVALID_HANDLE &&
            rtosSchedulerIsRunning() )
        return rtosMutexLock(((UsbSer*)chnl->m_bus)->mutex, tmo);

    return FALSE;
}

void usbserUnlockPort(EseChannelIo* chnl)
{
    if( chnl && chnl->m_bus != INVALID_HANDLE && 
            rtosSchedulerIsRunning() )
        rtosMutexUnlock( ((UsbSer*)chnl->m_bus)->mutex );
}

// access dcb
void usbserGetDCB(EseChannelIo* chnl, usbserDCB* dcb)
{
    if( chnl && chnl->m_bus != INVALID_HANDLE && dcb != NULL )
        *dcb = ((UsbSer*)chnl->m_bus)->dcb;
}

esBL usbserOpen(EseChannelIo* chnl)
{
    if( chnl && chnl->m_bus )
        return flagsInitialized == (((UsbSer*)chnl->m_bus)->flags & flagsInitialized);    

    return FALSE;
}

void usbserClose(EseChannelIo* chnl)
{
    // do nothing
}

esBL usbserIsOpen(EseChannelIo* chnl)
{
    if( chnl && chnl->m_bus )
        return flagsInitialized == (((UsbSer*)chnl->m_bus)->flags & flagsInitialized);

    return FALSE;
}

static esU32 usbserPutBytesFixedTmo(EseChannelIo* chnl, const esU8* data, esU32 len)
{
    return usbserPutBytes(chnl, data, len, 1000);
}

// usbserial channel initializer
void usbserChannelInit(EseChannelIo* chnl, usbserHANDLE usbser)
{
    // perform basic initialization first
    chnlInit(chnl, (busHANDLE)usbser);
    chnl->m_type = CHNL_UART;
    
    // assign interface implementation
    chnl->lock = usbserLockPort;
    chnl->unlock = usbserUnlockPort;
    chnl->connect = usbserOpen;
    chnl->disconnect = usbserClose;
    chnl->isConnected = usbserIsOpen;
    chnl->bytesPut = usbserPutBytesFixedTmo;
    chnl->bytesGet = usbserGetBytes;
//    chnl->resetIo = uartFlushRx;
//    chnl->waitTxEmpty = uartWaitTxEmpty;
//    chnl->getError = uartGetErrorCode;
}
