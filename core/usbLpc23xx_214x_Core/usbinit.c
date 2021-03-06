#include <esfwxe/target.h>
#pragma hdrstop
//
//  $Id: usbinit.c 136 2008-10-13 05:07:39Z jcw $
//  $Revision: 136 $
//  $Author: jcw $
//  $Date: 2008-10-13 01:07:39 -0400 (Mon, 13 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usb/usbinit.c $
//
#include "usbapi.h"


//
//  Data storage area for standard requests
//
static esU8 abStdReqData [8];


//
//  USB reset handler
//
static int usbDefaultHandleReset(esU8 bDevStatus __attribute__ ((unused)))
{
    return FALSE;
}

//
//
//
esBL usbInit (void)
{
  usbHardwareInit();
  usbHardwareRegisterDevIntHandler(usbDefaultHandleReset);
  usbHardwareRegisterEPIntHandler(0x00, usbHandleControlTransfer);
  usbHardwareRegisterEPIntHandler(0x80, usbHandleControlTransfer);
  usbHardwareEndpointConfig(0x00, MAX_PACKET_SIZE0);
  usbHardwareEndpointConfig(0x80, MAX_PACKET_SIZE0);
  usbRegisterRequestHandler(REQTYPE_TYPE_STANDARD, usbHandleStandardRequest, abStdReqData);

  return TRUE;
}
