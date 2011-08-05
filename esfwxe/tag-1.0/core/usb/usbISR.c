#include <common/target.h>
#pragma hdrstop
//
//  $Id: usbISR.c 258 2008-10-31 01:02:51Z jcw $
//  $Revision: 258 $
//  $Author: jcw $
//  $Date: 2008-10-30 21:02:51 -0400 (Thu, 30 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbISR.c $
//
#include <common/irq.h>
#include <FreeRTOS.h>
#include <task.h>

#include <common/core/usb/usbISR.h>
#include <common/core/usb/usbapi.h>

static TFnDevIntHandler *s_pfnDevIntHandler = NULL;  /** Installed device interrupt handler */
static TFnEPIntHandler  *s_apfnEPIntHandlers [16];   /** Installed endpoint interrupt handlers */
static TFnFrameHandler  *s_pfnFrameHandler = NULL;   /** Installed frame interrupt handlers */

#define EP2IDX(bEP) ((((bEP)&0xF)<<1)|(((bEP)&0x80)>>7))  /** convert from endpoint address to endpoint index */
#define IDX2EP(idx) ((((idx)<<7)&0x80)|(((idx)>>1)&0xF))  /** convert from endpoint index to endpoint address */

enum {
	// interrupt register masks
	USBINT_NONE 		= 0,
	USBINT_ERR			= 0x00000200,
	USBINT_EPRLZED	= 0x00000100,
	USBINT_TxENDPKT	= 0x00000080,
	USBINT_RxENDPKT	= 0x00000040,
	USBINT_CDFULL		=	0x00000020,
	USBINT_CCEMPTY	=	0x00000010,
	USBINT_DEVSTAT	=	0x00000008,
	USBINT_EPSLOW		=	0x00000004,
	USBINT_EPFAST		=	0x00000002,
	USBINT_FRAME		=	0x00000001,
	USBINT_ALL			=	USBINT_ERR|USBINT_EPRLZED|USBINT_TxENDPKT|USBINT_RxENDPKT|
										USBINT_CDFULL|USBINT_CCEMPTY|USBINT_DEVSTAT|
										USBINT_EPSLOW|USBINT_EPFAST|USBINT_FRAME,
	// usb control register
	USBCTL_RDEN			= 0x00000001,
	USBCTL_WREN			= 0x00000002,
	// usp recieve packet length register
	USBRXLEN_LENMASK= 0x000001FF,
	USBRXLEN_PKTRDY = 0x00000800,
	USBRXLEN_DVALID = 0x00000400,
};

//
//  Local function to wait for a device interrupt (and clear it)
//
static void usbWaitForDeviceInterrupt (DWORD dwIntr)
{
  while ((USBDevIntSt & dwIntr) != dwIntr);

  USBDevIntClr = dwIntr;
}

//
//  Local function to send a command to the USB protocol engine
//
static void usbHardwareCommand (BYTE bCmd)
{
  USBDevIntClr = USBINT_CDFULL | USBINT_CCEMPTY;
  USBCmdCode = 0x00000500 | (bCmd << 16);
  usbWaitForDeviceInterrupt(USBINT_CCEMPTY);
}

//
//  Local function to send a command + data to the USB protocol engine
//
static void usbHardwareCommandWrite (BYTE bCmd, WORD bData)
{
  usbHardwareCommand (bCmd);
  USBCmdCode = 0x00000100 | (bData << 16);
  usbWaitForDeviceInterrupt (USBINT_CCEMPTY);
}

//
//  Local function to send a command to the USB protocol engine and read data
//
static BYTE usbHardwareCommandRead (BYTE bCmd)
{
  usbHardwareCommand (bCmd);
  USBCmdCode = 0x00000200 | (bCmd << 16);
  usbWaitForDeviceInterrupt (USBINT_CDFULL);
  return USBCmdData;
}

//
//  'Realizes' an endpoint, meaning that buffer space is reserved for
//  it. An endpoint needs to be realised before it can be used.
//    
//  From experiments, it appears that a USB reset causes USBReEP to
//  re-initialise to 3 (= just the control endpoints).
//  However, a USB bus reset does not disturb the USBMaxPSize settings.
//    
static void usbHardwareEndpointRealize (int idx, WORD wMaxPSize)
{
  USBReEp |= (1 << idx);
  USBEpInd = idx;
  USBMaxPSize = wMaxPSize;
  usbWaitForDeviceInterrupt(USBINT_EPRLZED);
}

//
//  Enables or disables an endpoint
//
static void usbHardwareEndpointEnable (int idx, BOOL fEnable)
{
  usbHardwareCommandWrite (CMD_EP_SET_STATUS | idx, fEnable ? 0 : EP_DA);
}

//
// Configures an endpoint and enables it
//
void usbHardwareEndpointConfig (BYTE bEP, WORD wMaxPacketSize)
{
  int idx;
  
  idx = EP2IDX (bEP);
  usbHardwareEndpointRealize (idx, wMaxPacketSize);
  usbHardwareEndpointEnable (idx, TRUE);
}

//
//  Registers an endpoint event callback
//
void usbHardwareRegisterEPIntHandler (BYTE bEP, TFnEPIntHandler *pfnHandler)
{
  int idx;
  
  idx = EP2IDX (bEP);

  s_apfnEPIntHandlers [idx / 2] = pfnHandler;
  USBEpIntEn |= (1 << idx);
  USBDevIntEn |= USBINT_EPSLOW;
}

//
//  Registers an device status callback
//
void usbHardwareRegisterDevIntHandler (TFnDevIntHandler *pfnHandler)
{
  s_pfnDevIntHandler = pfnHandler;
  USBDevIntEn |= USBINT_DEVSTAT;
}

//
//  Registers the frame callback
//
void usbHardwareRegisterFrameHandler (TFnFrameHandler *pfnHandler)
{
  s_pfnFrameHandler = pfnHandler;
  USBDevIntEn |= USBINT_FRAME;
}

//
//  Sets the USB address.
//
void usbHardwareSetAddress (BYTE bAddr)
{
  usbHardwareCommandWrite (CMD_DEV_SET_ADDRESS, DEV_EN | bAddr);
}

//
//  Connects or disconnects from the USB bus
//
void usbHardwareConnect (BOOL fConnect)
{
  usbHardwareCommandWrite (CMD_DEV_STATUS, fConnect ? CON : 0);
}

//
//  Enables interrupt on NAK condition
//    
//  For IN endpoints a NAK is generated when the host wants to read data
//  from the device, but none is available in the endpoint buffer.
//  For OUT endpoints a NAK is generated when the host wants to write data
//  to the device, but the endpoint buffer is still full.
//  
//  The endpoint interrupt handlers can distinguish regular (ACK) interrupts
//  from NAK interrupt by checking the bits in their bEPStatus argument.
//  
void usbHardwareNakIntEnable (BYTE bIntBits)
{
  usbHardwareCommandWrite (CMD_DEV_SET_MODE, bIntBits);
}

//
//  Gets the stalled property of an endpoint
//
BOOL usbHardwareEndpointIsStalled (BYTE bEP)
{
  int idx = EP2IDX (bEP);

  return (usbHardwareCommandRead (CMD_EP_SELECT | idx) & EP_STATUS_STALLED);
}

//
//  Sets the stalled property of an endpoint
//
void usbHardwareEndpointStall (BYTE bEP, BOOL fStall)
{
  int idx = EP2IDX (bEP);

  usbHardwareCommandWrite (CMD_EP_SET_STATUS | idx, fStall ? EP_ST : 0);
}

//
//  Writes data to an endpoint buffer
//
int usbHardwareEndpointWrite (BYTE bEP, BYTE *pbBuf, int iLen)
{
  int idx;
  
  idx = EP2IDX (bEP);
  
  USBCtrl = USBCTL_WREN | ((bEP & 0xf) << 2);
  USBTxPLen = iLen;

  while (USBCtrl & USBCTL_WREN) 
  {
    USBTxData = (pbBuf [3] << 24) | (pbBuf [2] << 16) | (pbBuf [1] << 8) | pbBuf [0];
    pbBuf += 4;
  }

  USBCtrl = 0;

  usbHardwareCommand (CMD_EP_SELECT | idx);
  usbHardwareCommand (CMD_EP_VALIDATE_BUFFER);
  
  return iLen;
}

//
//  Reads data from an endpoint buffer
//
int usbHardwareEndpointRead (BYTE bEP, BYTE *pbBuf, int iMaxLen)
{
  int i, idx;
  DWORD dwData, dwLen;
  
  idx = EP2IDX (bEP);
  
  USBCtrl = USBCTL_RDEN | ((bEP & 0xf) << 2);
  
  do 
  {
    dwLen = USBRxPLen;
  } 
  while ((dwLen & USBRXLEN_PKTRDY) == 0);
  
  if ((dwLen & USBRXLEN_DVALID) == 0)
    return -1;
  
  dwLen &= USBRXLEN_LENMASK;
  
  for (dwData = 0, i = 0; i < (int) dwLen; i++)
  {
    if (!(i % 4))
      dwData = USBRxData;

    if (pbBuf && (i < iMaxLen))
      pbBuf [i] = dwData & 0xff;

    dwData >>= 8;
  }

  USBCtrl = 0;

  usbHardwareCommand (CMD_EP_SELECT | idx);
  usbHardwareCommand (CMD_EP_CLEAR_BUFFER);
  
  return dwLen;
}


//
//  Sets the 'configured' state.
//
void usbHardwareConfigDevice (BOOL fConfigured)
{
  usbHardwareCommandWrite (CMD_DEV_CONFIG, fConfigured ? CONF_DEVICE : 0);
}

//
//  USB interrupt handler
//
void usbISR_Handler(void)
{
  DWORD dwStatus;
  DWORD dwIntBit;
  DWORD dwEpIntSt, dwIntMask;
  BYTE  bEPStat, bDevStat, bStat;
  WORD wFrame;
  int i;
  portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

  dwStatus = USBDevIntSt;
  
  if (dwStatus & USBINT_FRAME) 
  {
    USBDevIntClr = USBINT_FRAME;

    if (s_pfnFrameHandler != NULL)
    {
      wFrame = usbHardwareCommandRead (CMD_DEV_READ_CUR_FRAME_NR);
      s_pfnFrameHandler (wFrame);
    }
  }

  if (dwStatus & USBINT_DEVSTAT) 
  {
    USBDevIntClr = USBINT_DEVSTAT;
    bDevStat = usbHardwareCommandRead (CMD_DEV_STATUS);

    if (bDevStat & (CON_CH | SUS_CH | RST)) 
    {
      bStat = ((bDevStat & CON) ? DEV_STATUS_CONNECT : 0) |
              ((bDevStat & SUS) ? DEV_STATUS_SUSPEND : 0) |
              ((bDevStat & RST) ? DEV_STATUS_RESET   : 0);
      
      if (s_pfnDevIntHandler != NULL)
        higherPriorityTaskWoken |= s_pfnDevIntHandler(bStat);
    }
  }
  
  if (dwStatus & USBINT_EPSLOW) 
  {
    USBDevIntClr = USBINT_EPSLOW;
    dwIntMask = 0xffffffff;

    for (i = 0; i < 32; i++) 
    {
      dwIntBit = (1 << i);
      dwIntMask <<= 1;
      dwEpIntSt = USBEpIntSt;

      if (dwEpIntSt & dwIntBit) 
      {
        USBEpIntClr = dwIntBit;
        usbWaitForDeviceInterrupt(USBINT_CDFULL);
        bEPStat = USBCmdData;

        bStat = ((bEPStat & EPSTAT_FE)  ? EP_STATUS_DATA    : 0) |
                ((bEPStat & EPSTAT_ST)  ? EP_STATUS_STALLED : 0) |
                ((bEPStat & EPSTAT_STP) ? EP_STATUS_SETUP   : 0) |
                ((bEPStat & EPSTAT_EPN) ? EP_STATUS_NACKED  : 0) |
                ((bEPStat & EPSTAT_PO)  ? EP_STATUS_ERROR   : 0);

        if (s_apfnEPIntHandlers [i / 2])
          higherPriorityTaskWoken |= s_apfnEPIntHandlers [i / 2] (IDX2EP (i), bStat);
      }

      if (!(dwEpIntSt & dwIntMask))
        break;
    }
  }
  
	VICVectAddr = 0;

  // we can switch context if necessary
	portEXIT_SWITCHING_ISR( higherPriorityTaskWoken );
}

// freertos usb isr handler
__asm void usbISRHandler(void)
{
	PRESERVE8
	ARM
	INCLUDE c:/FreeRTOS/Source/portable/RealView/ARM7/portmacro.inc

	portSAVE_CONTEXT 	

; ---------- call ISR worker
	IMPORT 	usbISR_Handler
	BL			usbISR_Handler

	portRESTORE_CONTEXT
}

void usbSetupInterruptHandler(void)
{
	install_irq( USB_INT, (void*)&usbISRHandler, USB_INTERRUPT_PRIORITY );
}

//
//  Initializes the USB hardware
//     
//  This function assumes that the hardware is connected as shown in
//  section 10.1 of the LPC2148 data sheet:
//  * P0.31 controls a switch to connect a 1.5k pull-up to D+ if low.
//  * P0.23 is connected to USB VCC.
//
BOOL usbHardwareInit (void)
{
  //  Configure P0.23 for Vbus sense, P0.31 as USB connect indicator
  //
  PINSEL1 &= ~0xC000C000;
  FIO0DIR &= ~0x00800000;
  PINSEL1 |= 0x80004000;

  //  Disable/clear all interrupts for now
  //
  USBDevIntEn = USBINT_NONE;
  USBDevIntClr = USBINT_ALL;
  USBDevIntPri = USBINT_NONE;
  USBEpIntEn = USBINT_NONE;
  USBEpIntClr = USBINT_ALL;
  USBEpIntPri = USBINT_NONE;

  //  By default, only ACKs generate interrupts
  //
  usbHardwareNakIntEnable(0);

  return TRUE;
}
