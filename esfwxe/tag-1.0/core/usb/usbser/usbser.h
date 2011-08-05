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
  DWORD   rate;
  BYTE    stopBit;
  BYTE    parity;
  BYTE    dataBits;
	
} usbserDCB;
#pragma pack(pop)

// usbserial io channel api
//
DWORD usbserPutBytes(usbserHANDLE usbser, const BYTE* data, DWORD len, DWORD tmo, CommChannelBreak* brk);
DWORD usbserGetBytes(usbserHANDLE usbser, BYTE* data, DWORD len, DWORD tmo, CommChannelBreak* brk);
usbserHANDLE usbserInit(const usbserDCB* dcb, 
	WORD vendorId, WORD productId, 
	const BYTE* mfgUstr, BYTE mfgLen,
	const BYTE* productUstr, BYTE productLen,
	const BYTE* serialUstr, BYTE serialLen);
// try to mutually exclusive acquire port resource
BOOL usbserLockPort(usbserHANDLE usbser, DWORD tmo);
void usbserUnlockPort(usbserHANDLE usbser);
// access dcb
void usbserGetDCB(usbserHANDLE usbser, usbserDCB* dcb);
BOOL usbserOpen(usbserHANDLE usbser);
void usbserClose(usbserHANDLE usbser);
BOOL usbserIsOpen(usbserHANDLE usbser);

// application-defined callbacks.
// NB! those two get called from ISR
int usbserOnConnect(void);
int usbserOnDisconnect(void);

#ifdef USE_COMMINTF

// additional services for CommChannel interface
//
// wait (blocking) until TX is empty
//void usbserWaitTxEmpty(usbserHANDLE usbser);
// rate support|change
//BOOL usbserIsRateSupported(usbserHANDLE usbser, DWORD rate);
//DWORD usbserGetRate(usbserHANDLE usbser);
//BOOL usbserSetRate(usbserHANDLE usbser, DWORD rate);
// uart channel initializer
void usbserChannelInit(CommChannel* chnl, usbserHANDLE usbser);

#endif // USE_COMMINTF

#ifdef __cplusplus
	}
#endif

#endif
