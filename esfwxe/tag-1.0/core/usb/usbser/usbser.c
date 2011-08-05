#include <common/target.h>
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

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>

#ifdef USE_COMMINTF
#	include <common/commintf.h>
#	include <common/utils.h>
#endif

#include <common/core/usb/usbapi.h>
#include <common/core/usb/usbser/usbser.h>

#define INT_IN_EP     0x81
#define BULK_OUT_EP   0x05
#define BULK_IN_EP    0x82

#define MAX_PACKET_SIZE 64
#define BULKBUF_SIZE 	 	MAX_PACKET_SIZE

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
	flagsInitialized 	= 0x0001,
	flagsConnected 		= 0x0002,
};

typedef struct {
	WORD controlLines;
	BYTE abBulkBuf[BULKBUF_SIZE];
	BYTE abClassReqData[8];
	xQueueHandle rx;
	xQueueHandle tx;
	xSemaphoreHandle mutex; 					// mutex for port sharing between tasks
	volatile WORD flags;
	usbserDCB dcb;

} UsbSer;
static UsbSer s_usbser;

// descriptors block
//
static BYTE s_descriptors[256] = 
{
  //  Device descriptor
  //
  0x12,
  DESC_DEVICE,
  LE_WORD(0x0101),      // bcdUSB
  0x02,                 // bDeviceClass			- Communication Interface Class
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
  0x02,                 // bInterfaceClass		- Communication Interface Class
  0x02,                 // bInterfaceSubClass	- Abstract Control Model
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
static void usbHardwareFrameIntHandler(WORD UNUSED(wFrame))
{
  usbHardwareNakIntEnable(INACK_BI);
}

//
//  Local function to handle incoming bulk data.  This needs to copy the incoming data to the VCOM RX queue.
//
static int usbserBulkOut(BYTE bEP, BYTE UNUSED(bEPStatus))
{
  int i;
  int iLen;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

  iLen = usbHardwareEndpointRead(bEP, s_usbser.abBulkBuf, BULKBUF_SIZE);

  for (i = 0; i < iLen; i++) 
    xQueueSendFromISR(s_usbser.rx, &s_usbser.abBulkBuf[i], &higherPriorityTaskWoken);

  return higherPriorityTaskWoken;
}

//
//  Local function to handle outgoing bulk data.  This needs to copy any user data in the VCOM TX queue to the bulk endpoint.
//
static int usbserBulkIn(BYTE bEP, BYTE UNUSED(bEPStatus))
{
  int i;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;
  
  for (i = 0; i < MAX_PACKET_SIZE; i++)
    if(xQueueReceiveFromISR(s_usbser.tx, &s_usbser.abBulkBuf[i], &higherPriorityTaskWoken) != pdPASS)
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
static BOOL usbserHandleClassRequest (TSetupPacket *pSetup, int *piLen, BYTE **ppbData)
{
  switch (pSetup->bRequest) 
  {
    case SET_LINE_CODING:
      {
        memcpy((BYTE*)&s_usbser.dcb, *ppbData, 7);
        *piLen = 7;
      }
      break;

    case GET_LINE_CODING:
      {
        *ppbData = (BYTE*)&s_usbser.dcb;
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
static BOOL usbserPutByte(BYTE b, DWORD tmo)
{
  if(!(s_usbser.flags & flagsInitialized))
    return FALSE;
  if(!(s_usbser.controlLines & CONTROLLINE_DTR))
    return TRUE;

  return xQueueSend(s_usbser.tx, &b, tmo) == pdTRUE;
}

DWORD usbserPutBytes(usbserHANDLE UNUSED(usbser), const BYTE* data, DWORD len, DWORD tmo, CommChannelBreak* brk)
{
	const BYTE* beg = data;
	const BYTE* end = data+len;

  if( (s_usbser.flags & flagsConnected) && 
			(s_usbser.controlLines & CONTROLLINE_DTR) )
	{
	  while(!chnlIsBreaking(brk) &&
					data < end && 
					usbserPutByte(*data, tmo) )
	    data++;
	}

  return data-beg;
}

//
//  Reads one character from VCOM port
//
static BOOL usbserGetByte(BYTE* b, DWORD tmo)
{
  if( !(s_usbser.flags & flagsConnected) )
    return FALSE;

  return xQueueReceive(s_usbser.rx, b, tmo) == pdTRUE;
}

// read data buffer from usb queue
DWORD usbserGetBytes(usbserHANDLE UNUSED(usbser), BYTE* data, DWORD len, DWORD tmo, CommChannelBreak* brk)
{
  if( !(s_usbser.flags & flagsConnected) )
    return 0;
	else
	{
		BYTE* beg = data;
		BYTE* end = data+len;
	
	  while( !chnlIsBreaking(brk) &&
			data < end && 
			usbserGetByte(data, tmo) )
	    data++;
	
	  return data-beg;
	}
}

//
//  Default reset handler gets overridden so we can clear line state on
//  disconnect (this is only really useful when the board is not powered
//  from the USB connection, because if it is, power is going away when
//  the cable is unplugged...)
//
static int usbHandleDevStatus(BYTE bDevStatus)
{
	int result = 0;
 	s_usbser.controlLines = 0;

	if( DEV_STATUS_CONNECT != (bDevStatus & DEV_STATUS_CONNECT) && flagsConnected == (s_usbser.flags & flagsConnected) )
	{
		s_usbser.flags &=	~flagsConnected;
		result = usbserOnDisconnect();
	}
	else if( DEV_STATUS_CONNECT == (bDevStatus & DEV_STATUS_CONNECT) && flagsConnected != (s_usbser.flags & flagsConnected) )
	{
		s_usbser.flags |=	flagsConnected;
		result = usbserOnConnect();
	}

	return result;
}

// init serial usb device, providing identification information
//
usbserHANDLE usbserInit(const usbserDCB* dcb, WORD vendorId, WORD productId, 
	const BYTE* mfgUstr, BYTE mfgLen,
	const BYTE* productUstr, BYTE productLen,
	const BYTE* serialUstr, BYTE serialLen)
{
	if( !(s_usbser.flags & flagsInitialized) )
	{
		BYTE* descrPos = s_descriptors;
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
	  s_usbser.rx = xQueueCreate(USBSER_RX_QUEUE_LEN, sizeof(BYTE));
	  s_usbser.tx = xQueueCreate(USBSER_TX_QUEUE_LEN, sizeof(BYTE));
		s_usbser.mutex = xSemaphoreCreateMutex();
 	  s_usbser.flags = flagsInitialized;
		usbSetupInterruptHandler();
	  usbHardwareConnect(TRUE);
	}

	return &s_usbser;
}

// try to mutually exclusive acquire port resource
BOOL usbserLockPort(usbserHANDLE usbser, DWORD tmo)
{
	if( usbser != INVALID_HANDLE )
	{
		if( taskSCHEDULER_RUNNING == xTaskGetSchedulerState()	)
			return xSemaphoreTake(((UsbSer*)usbser)->mutex, tmo) == pdTRUE;
	}

	return FALSE;
}

void usbserUnlockPort(usbserHANDLE usbser)
{
	if( usbser != INVALID_HANDLE && 
			taskSCHEDULER_RUNNING == xTaskGetSchedulerState() )
		xSemaphoreGive( ((UsbSer*)usbser)->mutex );
}

// access dcb
void usbserGetDCB(usbserHANDLE usbser, usbserDCB* dcb)
{
	if( usbser != INVALID_HANDLE && dcb != NULL )
		*dcb = ((UsbSer*)usbser)->dcb;
}

BOOL usbserOpen(usbserHANDLE usbser)
{
	if( usbser )
		return flagsInitialized == (((UsbSer*)usbser)->flags & flagsInitialized);	

	return FALSE;
}

void usbserClose(usbserHANDLE usbser)
{
	// do nothing
}

BOOL usbserIsOpen(usbserHANDLE usbser)
{
	if( usbser )
		return flagsInitialized == (((UsbSer*)usbser)->flags & flagsInitialized);

	return FALSE;
}

#ifdef USE_COMMINTF

static DWORD usbserPutBytesFixedTmo(usbserHANDLE usbser, const BYTE* data, DWORD len, CommChannelBreak* brk)
{
	return usbserPutBytes(usbser, data, len, 1000, brk);
}

// usbserial channel initializer
void usbserChannelInit(CommChannel* chnl, usbserHANDLE usbser)
{
	// perform basic initialization first
	chnlInit(chnl, usbser);
	chnl->m_type = CHNL_UART;
	
	// assign interface implementation
	chnl->lock = usbserLockPort;
	chnl->unlock = usbserUnlockPort;
	chnl->connect = usbserOpen;
	chnl->disconnect = usbserClose;
	chnl->isConnected = usbserIsOpen;
	chnl->putBytes = usbserPutBytesFixedTmo;
	chnl->getBytes = usbserGetBytes;
//	chnl->resetIo = uartFlushRx;
//	chnl->waitTxEmpty = uartWaitTxEmpty;
//	chnl->getError = uartGetErrorCode;
}

#endif // USE_COMMINTF
