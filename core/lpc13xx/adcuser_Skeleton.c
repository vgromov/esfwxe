/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    adcuser.c
 * Purpose: Audio Device Class Custom User Module
 * Version: V1.20
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2008 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 * History:
 *          V1.20 
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include <esfwxe/target.h>
#include <esfwxe/core/usbKeilCore/usb.h>
#include <esfwxe/core/usbKeilCore/usbcore.h>
#include <esfwxe/core/usbKeilCore/audio.h>
#include <esfwxe/core/usbKeilCore/adcuser.h>

      U16 VolCur = 0x0100;     /* Volume Current Value */
const U16 VolMin = 0x0000;     /* Volume Minimum Value */
const U16 VolMax = 0x0100;     /* Volume Maximum Value */
const U16 VolRes = 0x0004;     /* Volume Resolution */


/*
 *  Audio Device Class Interface Get Request Callback
 *   Called automatically on ADC Interface Get Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

esBL ADC_IF_GetRequest (void) {

/*
  Interface = SetupPacket.wIndex.WB.L;
  EntityID  = SetupPacket.wIndex.WB.H;
  Request   = SetupPacket.bRequest;
  Value     = SetupPacket.wValue.W;
  ...
*/

  if (SetupPacket.wIndex.W == 0x0200) {
    /* Feature Unit: Interface = 0, ID = 2 */
    if (SetupPacket.wValue.WB.L == 0) {
      /* Master Channel */
      switch (SetupPacket.wValue.WB.H) {
        case AUDIO_MUTE_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_GET_CUR:
              EP0Buf[0] = Mute;
              return (__TRUE);
          }
          break;
        case AUDIO_VOLUME_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_GET_CUR:
              *((__packed U16 *)EP0Buf) = VolCur;
              return (__TRUE);
            case AUDIO_REQUEST_GET_MIN:
              *((__packed U16 *)EP0Buf) = VolMin;
              return (__TRUE);
            case AUDIO_REQUEST_GET_MAX:
              *((__packed U16 *)EP0Buf) = VolMax;
              return (__TRUE);
            case AUDIO_REQUEST_GET_RES:
              *((__packed U16 *)EP0Buf) = VolRes;
              return (__TRUE);
          }
          break;
      }
    }
  }

  return (__FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class Interface Set Request Callback
 *   Called automatically on ADC Interface Set Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

esBL ADC_IF_SetRequest (void) {

/*
  Interface = SetupPacket.wIndex.WB.L;
  EntityID  = SetupPacket.wIndex.WB.H;
  Request   = SetupPacket.bRequest;
  Value     = SetupPacket.wValue.W;
  ...
*/

  if (SetupPacket.wIndex.W == 0x0200) {
    /* Feature Unit: Interface = 0, ID = 2 */
    if (SetupPacket.wValue.WB.L == 0) {
      /* Master Channel */
      switch (SetupPacket.wValue.WB.H) {
        case AUDIO_MUTE_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_SET_CUR:
              Mute = EP0Buf[0];
              return (__TRUE);
          }
          break;
        case AUDIO_VOLUME_CONTROL:
          switch (SetupPacket.bRequest) {
            case AUDIO_REQUEST_SET_CUR:
              VolCur = *((__packed U16 *)EP0Buf);
              return (__TRUE);
          }
          break;
      }
    }
  }

  return (__FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class EndPoint Get Request Callback
 *   Called automatically on ADC EndPoint Get Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

esBL ADC_EP_GetRequest (void) {

/*
  EndPoint = SetupPacket.wIndex.WB.L;
  Request  = SetupPacket.bRequest;
  Value    = SetupPacket.wValue.W;
  ...
*/
  return (__FALSE);  /* Not Supported */
}


/*
 *  Audio Device Class EndPoint Set Request Callback
 *   Called automatically on ADC EndPoint Set Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

esBL ADC_EP_SetRequest (void) {

/*
  EndPoint = SetupPacket.wIndex.WB.L;
  Request  = SetupPacket.bRequest;
  Value    = SetupPacket.wValue.W;
  ...
*/
  return (__FALSE);  /* Not Supported */
}
