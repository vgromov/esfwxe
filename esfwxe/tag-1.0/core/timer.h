#ifndef _timer_h_
#define _timer_h_

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
	tmrTimer,
	tmrCaptureOnRisingEdge,
	tmrCaptureOnFallingEdge,
	tmrCaptureOnBothEdges

} tmrMode;

typedef enum {
	interruptOnMatchMR0 = 0x0001,
	resetOnMatchMR0		 	= 0x0002,
	stopOnMatchMR0			= 0x0004,
	interruptOnMatchMR1 = 0x0008,
	resetOnMatchMR1		  = 0x0010,
	stopOnMatchMR1			= 0x0020,
	interruptOnMatchMR2 = 0x0040,
	resetOnMatchMR2		  = 0x0080,
	stopOnMatchMR2			= 0x0100,
	interruptOnMatchMR3 = 0x0200,
	resetOnMatchMR3		  = 0x0400,
	stopOnMatchMR3			= 0x0800,

} tmrMatchCtl;

// NOTE: If Counter mode is selected for a particular CAP input in the CTCR, the 3 bits for
// that input in this register should be programmed as 000, but capture and/or interrupt can
// be selected for the other 3 CAP inputs.
typedef enum {
	capCtlNone,
	capCtlLoadCR0onRisingEdge	= 0x0001,
	capCtlLoadCR0onFallingEdge= 0x0002,
	capCtlIntOnCaptureCR0			= 0x0004,
	capCtlLoadCR1onRisingEdge	= 0x0008,
	capCtlLoadCR1onFallingEdge= 0x0010,
	capCtlIntOnCaptureCR1			= 0x0020,
	capCtlLoadCR2onRisingEdge	= 0x0040,
	capCtlLoadCR2onFallingEdge=	0x0080,
	capCtlIntOnCaptureCR2			= 0x0100,
	capCtlLoadCR3onRisingEdge	= 0x0200,
	capCtlLoadCR3onFallingEdge=	0x0400,
	capCtlIntOnCaptureCR3			= 0x0800,

} tmrCaptureCtl;

// output control fields should be given values from 
// matchOutputCtl	enum
typedef enum {
	matchOutDisable,
	matchOut0Enable 		= 0x0001,
	matchOut1Enable 		= 0x0002,
	matchOut2Enable 		= 0x0004,
	matchOut3Enable 		= 0x0008,
 	matchOut0CtlClr			= 0x0010,
	matchOut0CtlSet			= 0x0020,
	matchOut0CtlToggle	= matchOut0CtlClr|matchOut0CtlSet,
	matchOut0CtlMask		=	matchOut0CtlToggle,
 	matchOut1CtlClr			= 0x0040,
	matchOut1CtlSet			= 0x0080,
	matchOut1CtlToggle	= matchOut1CtlClr|matchOut1CtlSet,
	matchOut1CtlMask		=	matchOut1CtlToggle,
 	matchOut2CtlClr			= 0x0100,
	matchOut2CtlSet			= 0x0200,
	matchOut2CtlToggle	= matchOut2CtlClr|matchOut2CtlSet,
	matchOut2CtlMask		=	matchOut2CtlToggle,
 	matchOut3CtlClr			= 0x0400,
	matchOut3CtlSet			= 0x0800,
	matchOut3CtlToggle	= matchOut3CtlClr|matchOut3CtlSet,
	matchOut3CtlMask		=	matchOut3CtlToggle

} tmrMatchOutCtl;

typedef enum {
	counterSrc0,
	counterSrc1,
	counterSrc2,
	counterSrc3,
	counterSrcNone = counterSrc0

} tmrCounterSrc;

// device control block for timer
typedef struct
{
	tmrMode 			mode;
	tmrCounterSrc	counterSrc;
	WORD 					matchCtl;
	WORD					captureCtl;
	WORD 					matchOutCtl;
	DWORD					timerCountsPerSecond;

} tmrDCB;

enum { tmrDCB_SZE = sizeof(tmrDCB) };

typedef enum
{
	tmrMatch0,
	tmrMatch1,
	tmrMatch2,
	tmrMatch3
	
} tmrMatchReg;

DEF_VOLATILE_HANDLE( tmrHANDLE );

#include "timerConfig.h"

typedef enum
{
#ifdef USE_TIMER0
	tmr0,
#endif

#ifdef USE_TIMER1
	tmr1,
#endif

#ifdef USE_TIMER2
	tmr2,
#endif

#ifdef USE_TIMER3
	tmr3,
#endif

	tmrCount

} tmrUnit;

void tmrPowerUp(tmrUnit tmr);
void tmrPowerDown(tmrUnit tmr);
tmrHANDLE tmrInit(tmrUnit tmr, const tmrDCB* dcb);
BOOL tmrLock(tmrHANDLE htmr, DWORD timeout);
void tmrUnlock(tmrHANDLE htmr); 
void tmrGetDCB(tmrHANDLE htmr, tmrDCB *dcb);
void tmrEnable(tmrHANDLE htmr);
void tmrDisable(tmrHANDLE htmr);
BOOL tmrIsEnabled(tmrHANDLE htmr);
void tmrSetMatchRegister(tmrHANDLE htmr, tmrMatchReg reg, DWORD val);

#ifdef __cplusplus
	}
#endif 

#endif // _timer_h_
