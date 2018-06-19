/*----------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *----------------------------------------------------------------------------
 * Name:    usbdesc.c
 * Purpose: USB Descriptors
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
 *          V1.20 Changed string descriptor handling
 *          V1.00 Initial Version
 *----------------------------------------------------------------------------*/

#include "type.h"

#include "usb.h"
#include "usbcfg.h"
#include "usbdesc.h"

#include "audio.h"


/* USB Standard Device Descriptor */
const U8 USB_DeviceDescriptor[] = {
  USB_DEVICE_DESC_SIZE,              /* bLength */
  USB_DEVICE_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0200), /* 2.00 */          /* bcdUSB */
  0x00,                              /* bDeviceClass */
  0x00,                              /* bDeviceSubClass */
  0x00,                              /* bDeviceProtocol */
  USB_MAX_PACKET0,                   /* bMaxPacketSize0 */
  WBVAL(0xC251),                     /* idVendor */
  WBVAL(0x1304),                     /* idProduct */
  WBVAL(0x0100), /* 1.00 */          /* bcdDevice */
  0x01,                              /* iManufacturer */
  0x02,                              /* iProduct */
  0x03,                              /* iSerialNumber */
  0x01                               /* bNumConfigurations: one possible configuration*/
};

/* USB Configuration Descriptor */
/*   All Descriptors (Configuration, Interface, Endpoint, Class, Vendor) */
const U8 USB_ConfigDescriptor[] = {
/* Configuration 1 */
  USB_CONFIGUARTION_DESC_SIZE,       /* bLength */
  USB_CONFIGURATION_DESCRIPTOR_TYPE, /* bDescriptorType */
  WBVAL(                             /* wTotalLength */
    USB_CONFIGUARTION_DESC_SIZE         +
    USB_INTERFACE_DESC_SIZE             +
    AUDIO_CONTROL_INTERFACE_DESC_SZ(1)  +
    AUDIO_INPUT_TERMINAL_DESC_SIZE      +
    AUDIO_FEATURE_UNIT_DESC_SZ(1,1)     +
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE     +
    USB_INTERFACE_DESC_SIZE             +
    USB_INTERFACE_DESC_SIZE             +
    AUDIO_STREAMING_INTERFACE_DESC_SIZE +
    AUDIO_FORMAT_TYPE_I_DESC_SZ(1)      +
    AUDIO_STANDARD_ENDPOINT_DESC_SIZE   +
    AUDIO_STREAMING_ENDPOINT_DESC_SIZE
  ),
  0x02,                              /* bNumInterfaces */
  0x01,                              /* bConfigurationValue */
  0x00,                              /* iConfiguration */
  USB_CONFIG_BUS_POWERED /*|*/       /* bmAttributes */
/*USB_CONFIG_REMOTE_WAKEUP*/,
  USB_CONFIG_POWER_MA(100),          /* bMaxPower */
/* Interface 0, Alternate Setting 0, Audio Control */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0x00,                              /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x00,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,            /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOCONTROL,       /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,          /* bInterfaceProtocol */
  0x00,                              /* iInterface */
/* Audio Control Interface */
  AUDIO_CONTROL_INTERFACE_DESC_SZ(1),/* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_CONTROL_HEADER,              /* bDescriptorSubtype */
  WBVAL(0x0100), /* 1.00 */          /* bcdADC */
  WBVAL(                             /* wTotalLength */
    AUDIO_CONTROL_INTERFACE_DESC_SZ(1) +
    AUDIO_INPUT_TERMINAL_DESC_SIZE     +
    AUDIO_FEATURE_UNIT_DESC_SZ(1,1)    +
    AUDIO_OUTPUT_TERMINAL_DESC_SIZE
  ),
  0x01,                              /* bInCollection */
  0x01,                              /* baInterfaceNr */
/* Audio Input Terminal */
  AUDIO_INPUT_TERMINAL_DESC_SIZE,    /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_CONTROL_INPUT_TERMINAL,      /* bDescriptorSubtype */
  0x01,                              /* bTerminalID */
  WBVAL(AUDIO_TERMINAL_USB_STREAMING),/* wTerminalType */
  0x00,                              /* bAssocTerminal */
  0x01,                              /* bNrChannels */
  WBVAL(AUDIO_CHANNEL_M),            /* wChannelConfig */
  0x00,                              /* iChannelNames */
  0x00,                              /* iTerminal */
/* Audio Feature Unit */
  AUDIO_FEATURE_UNIT_DESC_SZ(1,1),   /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_CONTROL_FEATURE_UNIT,        /* bDescriptorSubtype */
  0x02,                              /* bUnitID */
  0x01,                              /* bSourceID */
  0x01,                              /* bControlSize */
  AUDIO_CONTROL_MUTE |               
  AUDIO_CONTROL_VOLUME,              /* bmaControls(0) */
  0x00,                              /* bmaControls(1) */
  0x00,                              /* iTerminal */
/* Audio Output Terminal */
  AUDIO_OUTPUT_TERMINAL_DESC_SIZE,   /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_CONTROL_OUTPUT_TERMINAL,     /* bDescriptorSubtype */
  0x03,                              /* bTerminalID */
  WBVAL(AUDIO_TERMINAL_SPEAKER),     /* wTerminalType */
  0x00,                              /* bAssocTerminal */
  0x02,                              /* bSourceID */
  0x00,                              /* iTerminal */
/* Interface 1, Alternate Setting 0, Audio Streaming - Zero Bandwith */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0x01,                              /* bInterfaceNumber */
  0x00,                              /* bAlternateSetting */
  0x00,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,            /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,     /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,          /* bInterfaceProtocol */
  0x00,                              /* iInterface */
/* Interface 1, Alternate Setting 1, Audio Streaming - Operational */
  USB_INTERFACE_DESC_SIZE,           /* bLength */
  USB_INTERFACE_DESCRIPTOR_TYPE,     /* bDescriptorType */
  0x01,                              /* bInterfaceNumber */
  0x01,                              /* bAlternateSetting */
  0x01,                              /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,            /* bInterfaceClass */
  AUDIO_SUBCLASS_AUDIOSTREAMING,     /* bInterfaceSubClass */
  AUDIO_PROTOCOL_UNDEFINED,          /* bInterfaceProtocol */
  0x00,                              /* iInterface */
/* Audio Streaming Interface */
  AUDIO_STREAMING_INTERFACE_DESC_SIZE,/* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_STREAMING_GENERAL,           /* bDescriptorSubtype */
  0x01,                              /* bTerminalLink */
  0x01,                              /* bDelay */
  WBVAL(AUDIO_FORMAT_PCM),           /* wFormatTag */
/* Audio Type I Format */
  AUDIO_FORMAT_TYPE_I_DESC_SZ(1),    /* bLength */
  AUDIO_INTERFACE_DESCRIPTOR_TYPE,   /* bDescriptorType */
  AUDIO_STREAMING_FORMAT_TYPE,       /* bDescriptorSubtype */
  AUDIO_FORMAT_TYPE_I,               /* bFormatType */
  0x01,                              /* bNrChannels */
  0x02,                              /* bSubFrameSize */
  16,                                /* bBitResolution */
  0x01,                              /* bSamFreqType */
  B3VAL(32000),                      /* tSamFreq */
/* Endpoint - Standard Descriptor */
  AUDIO_STANDARD_ENDPOINT_DESC_SIZE, /* bLength */
  USB_ENDPOINT_DESCRIPTOR_TYPE,      /* bDescriptorType */
  USB_ENDPOINT_OUT(3),               /* bEndpointAddress */
  USB_ENDPOINT_TYPE_ISOCHRONOUS,     /* bmAttributes */
  WBVAL(64),                         /* wMaxPacketSize */
  0x01,                              /* bInterval */
  0x00,                              /* bRefresh */
  0x00,                              /* bSynchAddress */
/* Endpoint - Audio Streaming */
  AUDIO_STREAMING_ENDPOINT_DESC_SIZE,/* bLength */
  AUDIO_ENDPOINT_DESCRIPTOR_TYPE,    /* bDescriptorType */
  AUDIO_ENDPOINT_GENERAL,            /* bDescriptor */
  0x00,                              /* bmAttributes */
  0x00,                              /* bLockDelayUnits */
  WBVAL(0x0000),                     /* wLockDelay */
/* Terminator */
  0                                  /* bLength */
};


/* USB String Descriptor (optional) */
const U8 USB_StringDescriptor[] = {
/* Index 0x00: LANGID Codes */
  0x04,                              /* bLength */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  WBVAL(0x0409), /* US English */    /* wLANGID */
/* Index 0x01: Manufacturer */
  (13*2 + 2),                        /* bLength (13 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'K',0,
  'e',0,
  'i',0,
  'l',0,
  ' ',0,
  'S',0,
  'o',0,
  'f',0,
  't',0,
  'w',0,
  'a',0,
  'r',0,
  'e',0,
/* Index 0x02: Product */
  (20*2 + 2),                        /* bLength ( 20 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  'K',0,
  'e',0,
  'i',0,
  'l',0,
  ' ',0,
  'M',0,
  'C',0,
  'B',0,
  '2',0,
  '1',0,
  '4',0,
  '0',0,
  ' ',0,
  'S',0,
  'p',0,
  'e',0,
  'a',0,
  'k',0,
  'e',0,
  'r',0,
/* Index 0x03: Serial Number */
  (12*2 + 2),                        /* bLength (12 Char + Type + lenght) */
  USB_STRING_DESCRIPTOR_TYPE,        /* bDescriptorType */
  '0',0,                             /* allowed characters are       */
  '0',0,                             /*   0x0030 - 0x0039 ('0'..'9') */
  '0',0,                             /*   0x0041 - 0x0046 ('A'..'F') */
  '1',0,                             /*   length >= 26               */
  'A',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
  '0',0,
};
