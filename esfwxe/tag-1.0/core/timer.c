#include <common/target.h>
#pragma hdrstop

#ifdef USE_FREE_RTOS
	#include <FreeRTOS.h>
	#include <semphr.h>
#endif

#include "timer.h"

enum {
	timerUnitMask = 0x0F,
	timerEnabled  = 0x10
};

typedef struct {
	WORD 							dcr;
	tmrDCB 						dcb;
#if defined( USE_FREE_RTOS )
	xSemaphoreHandle 	mutex;
#endif

} tmrDCR;

// timer hardware - specific macros
//
// timer power manipulation macros
#define TIMER_ENABLE_POWER( timerNumber )	 	PCONP |=	PCTIM ## timerNumber
#define TIMER_DISABLE_POWER( timerNumber ) 	PCONP &=	~PCTIM ## timerNumber

// timer start-stop-reset
#define TIMER_ENABLE( timerNumber )	 				T ## timerNumber ## TCR = 1
#define TIMER_DISABLE( timerNumber )				T ## timerNumber ## TCR = 0
#define TIMER_SET_RESET( timerNumber )			T ## timerNumber ## TCR |= 0x02
#define TIMER_CLEAR_RESET( timerNumber )		T ## timerNumber ## TCR &= ~0x02
#define TIMER_IS_ENABLED( timerNumber )			(1 == (T ## timerNumber ## TCR & 1))

#define TIMER_CONFIG_DCB( timerNumber, dcb ) 	\
		if(dcb->mode == tmrTimer) 								\
		{																					\
			T ## timerNumber ## CTCR = 0;						\
			T ## timerNumber ## CCR = 0; 						\
		}																					\
		else																			\
		{																					\
			switch( dcb->mode )											\
			{																				\
			case tmrCaptureOnRisingEdge:						\
				T ## timerNumber ## CTCR = 1;					\
				break;																\
			case tmrCaptureOnFallingEdge:						\
				T ## timerNumber ## CTCR = 2;					\
				break;																\
			case tmrCaptureOnBothEdges:							\
				T ## timerNumber ## CTCR = 3;					\
				break;																\
			}																				\
			T ## timerNumber ## CTCR |= dcb->counterSrc * 4; \
			T ## timerNumber ## CCR = dcb->captureCtl & ~(0x07 << dcb->counterSrc * 3); \
		}																					\
		T ## timerNumber ## MCR = dcb->matchCtl; \
		T ## timerNumber ## EMR = dcb->matchOutCtl; \
		T ## timerNumber ## PR = ((DWORD)Timer ## timerNumber ## pclk / dcb->timerCountsPerSecond - 1);	\
		s_timers[tmr ## timerNumber ].dcb = *dcb

#define SET_MATCH_REGISTER(timerNum, regNum, regVal) \
	switch( regNum )																		\
	{								 																		\
	case tmrMatch0:																			\
		T ## timerNum	## MR0 = regVal;										\
		break;																						\
	case tmrMatch1:																			\
		T ## timerNum	## MR1 = regVal;										\
		break;																						\
	case tmrMatch2:																			\
		T ## timerNum	## MR2 = regVal;										\
		break;																						\
	case tmrMatch3:																			\
		T ## timerNum	## MR3 = regVal;										\
		break;																						\
	}

// timer device descriptors which are available for configuration
static volatile tmrDCR s_timers[tmrCount];

void tmrPowerUp(tmrUnit tmr)
{
	switch(tmr)
	{
#ifdef USE_TIMER0
		case tmr0:
		TIMER_ENABLE_POWER(0);
		break;
#endif
#ifdef USE_TIMER1
		case tmr1:
		TIMER_ENABLE_POWER(1);
		break;
#endif
#ifdef USE_TIMER2
		case tmr2:
		TIMER_ENABLE_POWER(2);
		break;
#endif
#ifdef USE_TIMER3
		case tmr3:
		TIMER_ENABLE_POWER(3);
		break;
#endif
	}
}

void tmrPowerDown(tmrUnit tmr)
{
	switch(tmr)
	{
#ifdef USE_TIMER0
		case tmr0:
		TIMER_DISABLE_POWER(0);
		break;
#endif
#ifdef USE_TIMER1
		case tmr1:
		TIMER_DISABLE_POWER(1);
		break;
#endif
#ifdef USE_TIMER2
		case tmr2:
		TIMER_DISABLE_POWER(2);
		break;
#endif
#ifdef USE_TIMER3
		case tmr3:
		TIMER_DISABLE_POWER(3);
		break;
#endif
	}
}

BOOL tmrLock(tmrHANDLE htmr, DWORD timeout)
{
	if( htmr != INVALID_HANDLE )
#if defined( USE_FREE_RTOS )		
		return xSemaphoreTake( CAST_PORT_HANDLE(tmrDCR, htmr)->mutex, timeout )  == pdTRUE;
#else
		return TRUE;
#endif

	return FALSE;
}

void tmrUnlock(tmrHANDLE htmr)
{
#if defined( USE_FREE_RTOS )
	if( htmr != INVALID_HANDLE )
		xSemaphoreGive( CAST_PORT_HANDLE(tmrDCR, htmr)->mutex);
#endif
} 

tmrHANDLE tmrInit(tmrUnit tmr, const tmrDCB* dcb)
{
	switch( tmr )
	{
#ifdef USE_TIMER0
	case tmr0:
		s_timers[tmr].dcr = tmr;
#if defined( USE_FREE_RTOS )
		s_timers[tmr].mutex = xSemaphoreCreateMutex();
#endif
		TIMER_ENABLE_POWER(0);
		TIMER_SET_RESET(0);
		TIMER_DISABLE(0);
		TIMER_CONFIG_DCB(0, dcb);
		return (tmrHANDLE)&s_timers[tmr0];
#endif
#ifdef USE_TIMER1
	case tmr1:
		s_timers[tmr].dcr = tmr;
#if defined( USE_FREE_RTOS )
		s_timers[tmr].mutex = xSemaphoreCreateMutex();
#endif
		TIMER_ENABLE_POWER(1);
		TIMER_SET_RESET(1);
		TIMER_DISABLE(1);
		TIMER_CONFIG_DCB(1, dcb);
		return (tmrHANDLE)&s_timers[tmr1];
#endif
#ifdef USE_TIMER2
	case tmr2:
		s_timers[tmr].dcr = tmr;
#if defined( USE_FREE_RTOS )
		s_timers[tmr].mutex = xSemaphoreCreateMutex();
#endif
		TIMER_ENABLE_POWER(2);
		TIMER_SET_RESET(2);
		TIMER_DISABLE(2);
		TIMER_CONFIG_DCB(2, dcb);
		return (tmrHANDLE)&s_timers[tmr2];
#endif
#ifdef USE_TIMER3
	case tmr3:
		s_timers[tmr].dcr = tmr;
#if defined( USE_FREE_RTOS )
		s_timers[tmr].mutex = xSemaphoreCreateMutex();
#endif
		TIMER_ENABLE_POWER(3);
		TIMER_SET_RESET(3);		
		TIMER_DISABLE(3);
		TIMER_CONFIG_DCB(3, dcb);
		return (tmrHANDLE)&s_timers[tmr3];
#endif
	}

	return NULL;
}

void tmrGetDCB(tmrHANDLE htmr, tmrDCB *dcb)
{
	if( htmr != INVALID_HANDLE )
		*dcb = CAST_PORT_HANDLE(tmrDCR, htmr)->dcb;
}

void tmrEnable(tmrHANDLE htmr)
{
	if( htmr != INVALID_HANDLE )
	{
		switch( CAST_PORT_HANDLE(tmrDCR, htmr)->dcr & timerUnitMask )
		{
#ifdef USE_TIMER0
		case tmr0:
			TIMER_CLEAR_RESET(0);
			TIMER_ENABLE(0);
			break;
#endif		
#ifdef USE_TIMER1
		case tmr1:
			TIMER_CLEAR_RESET(1);
			TIMER_ENABLE(1);
			break;
#endif		
#ifdef USE_TIMER2
		case tmr2:
			TIMER_CLEAR_RESET(2);
			TIMER_ENABLE(2);
			break;
#endif		
#ifdef USE_TIMER3
		case tmr3:
			TIMER_CLEAR_RESET(3);
			TIMER_ENABLE(3);
			break;
#endif		
		}
		CAST_PORT_HANDLE(tmrDCR, htmr)->dcr |= timerEnabled;	
	}
}

void tmrDisable(tmrHANDLE htmr)
{
	if( htmr != INVALID_HANDLE )
	{
		switch( CAST_PORT_HANDLE(tmrDCR, htmr)->dcr & timerUnitMask )
		{
#ifdef USE_TIMER0
		case tmr0:
			TIMER_SET_RESET(0);
			TIMER_DISABLE(0);
			break;
#endif		
#ifdef USE_TIMER1
		case tmr1:
			TIMER_SET_RESET(1);
			TIMER_DISABLE(1);
			break;
#endif		
#ifdef USE_TIMER2
		case tmr2:
			TIMER_SET_RESET(2);
			TIMER_DISABLE(2);
			break;
#endif		
#ifdef USE_TIMER3
		case tmr3:
			TIMER_SET_RESET(3);
			TIMER_DISABLE(3);
			break;
#endif		
		}
		
		CAST_PORT_HANDLE(tmrDCR, htmr)->dcr &= ~timerEnabled;	
	}
}

BOOL tmrIsEnabled(tmrHANDLE htmr)
{
	if( htmr != INVALID_HANDLE )
		return timerEnabled == (CAST_PORT_HANDLE(tmrDCR, htmr)->dcr & timerEnabled);
	else	
		return FALSE;	
}

void tmrSetMatchRegister(tmrHANDLE htmr, tmrMatchReg reg, DWORD val)
{
	if( htmr != INVALID_HANDLE )
	{
		switch( CAST_PORT_HANDLE(tmrDCR, htmr)->dcr & timerUnitMask )
		{
#ifdef USE_TIMER0
		case tmr0:
			SET_MATCH_REGISTER(0, reg, val);
			break;
#endif		
#ifdef USE_TIMER1
		case tmr1:
			SET_MATCH_REGISTER(1, reg, val);
			break;
#endif		
#ifdef USE_TIMER2
		case tmr2:
			SET_MATCH_REGISTER(2, reg, val);
			break;
#endif		
#ifdef USE_TIMER3
		case tmr3:
			SET_MATCH_REGISTER(3, reg, val);
			break;
#endif		
		}			
	}
}
