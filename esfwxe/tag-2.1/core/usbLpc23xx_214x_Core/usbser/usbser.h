//
//  $Id: usbser.h 42 2008-10-04 18:40:36Z jcw $
//  $Revision: 42 $
//  $Author: jcw $
//  $Date: 2008-10-04 14:40:36 -0400 (Sat, 04 Oct 2008) $
//  $HeadURL: http://tinymicros.com/svn_public/arm/lpc2148_demo/trunk/usbser/usbser.h $
//

#ifndef _USBSER_H_
#define _USBSER_H_

#ifdef __cplusplus
	extern "C" {
#endif

DEF_VOLATILE_HANDLE(usbserHANDLE);

#pragma pack(push, 1)
typedef struct 
{
  esU32   rate;
  esU8    stopBit;
  esU8    parity;
  esU8    dataBits;
	
} usbserDCB;
#pragma pack(pop)

// usbserial io channel api
//
esU32 usbserPutBytes(EseChannelIo* chnl, const esU8* data, esU32 len, esU32 tmo);
esU32 usbserGetBytes(EseChannelIo* chnl, esU8* data, esU32 len, esU32 tmo);
usbserHANDLE usbserInit(const usbserDCB* dcb, 
	esU16 vendorId, esU16 productId, 
	const esU8* mfgUstr, esU8 mfgLen,
	const esU8* productUstr, esU8 productLen,
	const esU8* serialUstr, esU8 serialLen);
// try to mutually exclusive acquire port resource
esBL usbserLockPort(EseChannelIo* chnl, esU32 tmo);
void usbserUnlockPort(EseChannelIo* chnl);
// access dcb
void usbserGetDCB(EseChannelIo* chnl, usbserDCB* dcb);
esBL usbserOpen(EseChannelIo* chnl);
void usbserClose(EseChannelIo* chnl);
esBL usbserIsOpen(EseChannelIo* chnl);

// application-defined callbacks.
// NB! those two get called from ISR
int usbserOnConnect(void);
int usbserOnDisconnect(void);

// usbser channel initializer
void usbserChannelInit(EseChannelIo* chnl, usbserHANDLE usbser);

#ifdef __cplusplus
	}
#endif

#endif
