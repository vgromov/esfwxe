#include <esfwxe/target.h>
#pragma hdrstop
//
//  $Id: usbISR.c 258 2008-10-31 01:02:51Z jcw $
//  $Revision: 258 $
//  $Author: jcw $
//  $Date: 2008-10-30 21:02:51 -0400 (Thu, 30 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbISR.c $
//
#include "usbapi.h"

static TFnDevIntHandler *s_pfnDevIntHandler = NULL;  /** Installed device interrupt handler */
static TFnEPIntHandler  *s_apfnEPIntHandlers [16];   /** Installed endpoint interrupt handlers */
static TFnFrameHandler  *s_pfnFrameHandler = NULL;   /** Installed frame interrupt handlers */

// hardware-dependent part
//

#if LPC23XX == 1
	#include <esfwxe/core/lpc23xx/usb_hw.cc>
#endif

#if LPC214X == 1
	#include <esfwxe/core/lpc214x/usb_hw.cc>
#endif

//
//  Enables or disables an endpoint
//
static void usbHardwareEndpointEnable (int idx, esBL fEnable)
{
  usbHardwareCommandWrite (CMD_EP_SET_STATUS | idx, fEnable ? 0 : EP_DA);
}

//
// Configures an endpoint and enables it
//
void usbHardwareEndpointConfig (esU8 bEP, esU16 wMaxPacketSize)
{
  int idx;
  
  idx = EP2IDX (bEP);
  usbHardwareEndpointRealize (idx, wMaxPacketSize);
  usbHardwareEndpointEnable (idx, TRUE);
}

//
//  Sets the USB address.
//
void usbHardwareSetAddress (esU8 bAddr)
{
  usbHardwareCommandWrite (CMD_DEV_SET_ADDRESS, DEV_EN | bAddr);
}

//
//  Connects or disconnects from the USB bus
//
void usbHardwareConnect (esBL fConnect)
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
void usbHardwareNakIntEnable (esU8 bIntBits)
{
  usbHardwareCommandWrite (CMD_DEV_SET_MODE, bIntBits);
}

//
//  Gets the stalled property of an endpoint
//
esBL usbHardwareEndpointIsStalled (esU8 bEP)
{
  int idx = EP2IDX (bEP);

  return (usbHardwareCommandRead (CMD_EP_SELECT | idx) & EP_STATUS_STALLED);
}

//
//  Sets the stalled property of an endpoint
//
void usbHardwareEndpointStall (esU8 bEP, esBL fStall)
{
  int idx = EP2IDX (bEP);

  usbHardwareCommandWrite (CMD_EP_SET_STATUS | idx, fStall ? EP_ST : 0);
}

//
//  Sets the 'configured' state.
//
void usbHardwareConfigDevice (esBL fConfigured)
{
  usbHardwareCommandWrite (CMD_DEV_CONFIG, fConfigured ? CONF_DEVICE : 0);
}

void usbSetupInterruptHandler(void)
{
	irqInstall( USB_INT, (void*)&usbISRHandler, USB_INTERRUPT_PRIORITY );
}

