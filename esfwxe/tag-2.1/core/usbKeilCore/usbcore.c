
#include <esfwxe/target.h>
#include <esfwxe/core/usbKeilCore/usb.h>
#include <esfwxe/core/usbKeilCore/usbcore.h>
#include <esfwxe/core/usbKeilCore/usbuser.h>
#include <esfwxe/core/usbKeilCore/usbdesc.h>
#include <esfwxe/core/usbKeilCore/usbhw.h>
 
#if (USB_CLASS)

#if (USB_AUDIO)
#include <esfwxe/core/usbKeilCore/audio.h>
#include <esfwxe/core/usbKeilCore/adcuser.h>
#endif

#if (USB_HID)
#include <esfwxe/core/usbKeilCore/hid.h>
#include <esfwxe/core/usbKeilCore/hiduser.h>
#endif

#if (USB_MSC)
#include <esfwxe/core/usbKeilCore/msc.h>
#include <esfwxe/core/usbKeilCore/mscuser.h>
extern MSC_CSW CSW;
#endif

#if (USB_CDC)
#include <esfwxe/core/usbKeilCore/cdc.h>
#include <esfwxe/core/usbKeilCore/cdcuser.h>
#endif

#endif

#if (USB_VENDOR)
#include "vendor.h"
#endif

#if LPC13XX == 1
#	include <esfwxe/core/lpc13xx/usbcore.cc>
#	include <esfwxe/core/lpc13xx/usbhw_LPC13xx.cc>
#endif
