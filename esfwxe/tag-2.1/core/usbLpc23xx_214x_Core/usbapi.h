#ifndef _usb_api_h_
#define _usb_api_h_

#ifdef __cplusplus
	extern "C" {
#endif

//
//  $Id: usbapi.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbapi.h $
//

#include "usbConfig.h"
#include "usbstruct.h"    // for TSetupPacket

/*************************************************************************
  USB configuration
**************************************************************************/

#define MAX_PACKET_SIZE0  64    /**< maximum packet size for EP 0 */

/*************************************************************************
  USB hardware interface
**************************************************************************/

// endpoint status sent through callback
#define EP_STATUS_DATA    (1<<0)    /**< EP has data */
#define EP_STATUS_STALLED (1<<1)    /**< EP is stalled */
#define EP_STATUS_SETUP   (1<<2)    /**< EP received setup packet */
#define EP_STATUS_ERROR   (1<<3)    /**< EP data was overwritten by setup packet */
#define EP_STATUS_NACKED  (1<<4)    /**< EP sent NAK */

// device status sent through callback
#define DEV_STATUS_CONNECT (1<<0)  /**< device just got connected */
#define DEV_STATUS_SUSPEND (1<<2)  /**< device entered suspend state */
#define DEV_STATUS_RESET   (1<<4)  /**< device just got reset */

// interrupt bits for NACK events in usbHardwareNakIntEnable
// (these bits conveniently coincide with the LPC214x USB controller bit)
#define INACK_NONE  (0)         /**< no interrupts */
#define INACK_CI    (1<<1)      /**< interrupt on NACK for control in */
#define INACK_CO    (1<<2)      /**< interrupt on NACK for control out */
#define INACK_II    (1<<3)      /**< interrupt on NACK for interrupt in */
#define INACK_IO    (1<<4)      /**< interrupt on NACK for interrupt out */
#define INACK_BI    (1<<5)      /**< interrupt on NACK for bulk in */
#define INACK_BO    (1<<6)      /**< interrupt on NACK for bulk out */

esBL usbHardwareInit(void);
void usbSetupInterruptHandler(void);
void usbHardwareNakIntEnable(esU8 bIntBits);
void usbHardwareConnect(esBL fConnect);
void usbHardwareSetAddress(esU8 bAddr);
void usbHardwareConfigDevice(esBL fConfigured);

//
//  Endpoint operations
//
void usbHardwareEndpointConfig(esU8 bEP, esU16 wMaxPacketSize);
int  usbHardwareEndpointRead(esU8 bEP, esU8 *pbBuf, int iMaxLen);
int  usbHardwareEndpointWrite(esU8 bEP, esU8 *pbBuf, int iLen);
void usbHardwareEndpointStall(esU8 bEP, esBL fStall);
esBL usbHardwareEndpointIsStalled(esU8 bEP);

//
//  Endpoint interrupt handler callback
//
typedef int (TFnEPIntHandler) (esU8 bEP, esU8 bEPStatus);
void usbHardwareRegisterEPIntHandler  (esU8 bEP, TFnEPIntHandler *pfnHandler);

//
//  Device status handler callback
//
typedef int (TFnDevIntHandler)  (esU8 bDevStatus);
void usbHardwareRegisterDevIntHandler (TFnDevIntHandler *pfnHandler);

//
//  Frame event handler callback
//
typedef void (TFnFrameHandler)(esU16 wFrame);
void usbHardwareRegisterFrameHandler(TFnFrameHandler *pfnHandler);


/*************************************************************************
  USB application interface
**************************************************************************/

// initialise the complete stack, including HW
esBL usbInit(void);

/** Request handler callback (standard, vendor, class) */
typedef esBL (TFnHandleRequest)(TSetupPacket *pSetup, int *piLen, esU8 **ppbData);
void usbRegisterRequestHandler (int iType, TFnHandleRequest *pfnHandler, esU8 *pbDataStore);
void usbRegisterCustomReqHandler (TFnHandleRequest *pfnHandler);

/** Descriptor handler callback */
typedef esBL (TFnGetDescriptor)(esU16 wTypeIndex, esU16 wLangID, int *piLen, esU8 **ppbData);

/** Default standard request handler */
esBL usbHandleStandardRequest (TSetupPacket *pSetup, int *piLen, esU8 **ppbData);

/** Default EP0 handler */
int usbHandleControlTransfer (esU8 bEP, esU8 bEPStat);

/** Descriptor handling */
void usbRegisterDescriptors (const esU8 *pabDescriptors);
esBL usbGetDescriptor (esU16 wTypeIndex, esU16 wLangID, int *piLen, esU8 **ppbData);

#ifdef __cplusplus
	}
#endif

#endif
