// hardware-dependent USB functions for lpc214x core
//
#include <esfwxe/irq.h>
#include <esfwxe/core/rtos_intf.h>

//
//  Protocol engine command codes, device commands 
//
#define CMD_DEV_SET_ADDRESS       0xD0
#define CMD_DEV_CONFIG            0xD8
#define CMD_DEV_SET_MODE          0xF3
#define CMD_DEV_READ_CUR_FRAME_NR 0xF5
#define CMD_DEV_READ_TEST_REG     0xFD
#define CMD_DEV_STATUS            0xFE    /* read/write */
#define CMD_DEV_GET_ERROR_CODE    0xFF
#define CMD_DEV_READ_ERROR_STATUS 0xFB

//
//  Protocol engine command codes, endpoint commands 
//
#define CMD_EP_SELECT             0x00
#define CMD_EP_SELECT_CLEAR       0x40
#define CMD_EP_SET_STATUS         0x40
#define CMD_EP_CLEAR_BUFFER       0xF2
#define CMD_EP_VALIDATE_BUFFER    0xFA

//
//  Set address command 
//
#define DEV_ADDR          (1<<0)
#define DEV_EN            (1<<7)

//
//  Configure device command 
//
#define CONF_DEVICE       (1<<0)

//
//  Set mode command 
//
#define AP_CLK            (1<<0)
#define INAK_CI           (1<<1)
#define INAK_CO           (1<<2)
#define INAK_II           (1<<3)
#define INAK_IO           (1<<4)
#define INAK_BI           (1<<5)
#define INAK_BO           (1<<6)

//
//  Set get device status command 
//  
#define CON               (1<<0)
#define CON_CH            (1<<1)
#define SUS               (1<<2)
#define SUS_CH            (1<<3)
#define RST               (1<<4)

//
//  Get error code command
//  ...
//

//
//  Select Endpoint command read bits 
//
#define EPSTAT_FE         (1<<0)
#define EPSTAT_ST         (1<<1)
#define EPSTAT_STP        (1<<2)
#define EPSTAT_PO         (1<<3)
#define EPSTAT_EPN        (1<<4)
#define EPSTAT_B1FULL     (1<<5)
#define EPSTAT_B2FULL     (1<<6)

//
//  CMD_EP_SET_STATUS command 
//
#define EP_ST             (1<<0)
#define EP_DA             (1<<5)
#define EP_RF_MO          (1<<6)
#define EP_CND_ST         (1<<7)

//
//  Read error status command 
//
#define PID_ERR           (1<<0)
#define UEPKT             (1<<1)
#define DCRC              (1<<2)
#define TIMEOUT           (1<<3)
#define EOP               (1<<4)
#define B_OVRN            (1<<5)
#define BTSTF             (1<<6)
#define TGL_ERR           (1<<7)

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
static void usbWaitForDeviceInterrupt (esU32 dwIntr)
{
  while ((USBDevIntSt & dwIntr) != dwIntr);

  USBDevIntClr = dwIntr;
}

//
//  Local function to send a command to the USB protocol engine
//
static __inline void usbHardwareCommand (esU8 bCmd)
{
  USBDevIntClr = USBINT_CDFULL | USBINT_CCEMPTY;
  USBCmdCode = 0x00000500 | (bCmd << 16);
  usbWaitForDeviceInterrupt(USBINT_CCEMPTY);
}

//
//  Local function to send a command + data to the USB protocol engine
//
static __inline void usbHardwareCommandWrite (esU8 bCmd, esU16 bData)
{
  usbHardwareCommand (bCmd);
  USBCmdCode = 0x00000100 | (bData << 16);
  usbWaitForDeviceInterrupt (USBINT_CCEMPTY);
}

//
//  Local function to send a command to the USB protocol engine and read data
//
static esU8 usbHardwareCommandRead (esU8 bCmd)
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
static void usbHardwareEndpointRealize (int idx, esU16 wMaxPSize)
{
  USBReEp |= (1 << idx);
  USBEpInd = idx;
  USBMaxPSize = wMaxPSize;
  usbWaitForDeviceInterrupt(USBINT_EPRLZED);
}

//
//  Registers an endpoint event callback
//
void usbHardwareRegisterEPIntHandler (esU8 bEP, TFnEPIntHandler *pfnHandler)
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
//  Writes data to an endpoint buffer
//
int usbHardwareEndpointWrite (esU8 bEP, esU8 *pbBuf, int iLen)
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
int usbHardwareEndpointRead (esU8 bEP, esU8 *pbBuf, int iMaxLen)
{
  int i, idx;
  esU32 dwData, dwLen;
  
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
//  USB interrupt handler
//
void usbISR_Handler(void)
{
  esU32 dwStatus;
  esU32 dwIntBit;
  esU32 dwEpIntSt, dwIntMask;
  esU8  bEPStat, bDevStat, bStat;
  esU16 wFrame;
  int i;
  esBL higherPriorityTaskWoken = FALSE;

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
	rtosExitSwitchingIsr( higherPriorityTaskWoken );
}

// freertos usb isr handler
__asm void usbISRHandler(void)
{
	PRESERVE8
	ARM
	INCLUDE freeRTOS/Source/portable/RVDS/ARM7_LPC21xx/portmacro.inc

	portSAVE_CONTEXT 	

; ---------- call ISR worker
	IMPORT 	usbISR_Handler
	BL			usbISR_Handler

	portRESTORE_CONTEXT
}

//
//  Initializes the USB hardware
//     
//  This function assumes that the hardware is connected as shown in
//  section 10.1 of the LPC2148 data sheet:
//  * P0.31 controls a switch to connect a 1.5k pull-up to D+ if low.
//  * P0.23 is connected to USB VCC.
//
esBL usbHardwareInit (void)
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
