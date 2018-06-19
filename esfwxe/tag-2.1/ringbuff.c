#ifndef USE_EMULATOR
#	include <esfwxe/target.h>
#else
#	include <emulator_stubs.h>
#endif
#pragma hdrstop 

#include "ringbuff.h"
#include <esfwxe/utils.h>

#if defined(USE_ISR_SAFE_RB) && !defined(USE_EMULATOR)
#	ifdef USE_USER_SWI
#		include "userSWI.h"
//#	else
//		extern void disableIrq(void);	
//		extern void enableIrq(void);	
#	endif
#endif

// ring buffer initialization 
//
void rbInit(rbHANDLE buff, void* data, esU32 size)
{
	buff->data 	= data;	// data buffer begin
	buff->mask 	= size-1UL;
	buff->in = buff->out = buff->count = 0;
}

// is empty
#ifdef USE_ISR_SAFE_RB
	esBL rbIsEmpty(rbHANDLE buff)
	{
		esBL result;
		disableIrq();
		result = rbIsEmptyFromIsr(buff);
		enableIrq();

		return result;
	}

	esBL rbIsEmptyFromIsr(rbHANDLE buff)
#else
	esBL rbIsEmpty(rbHANDLE buff)	
#endif	// USE_ISR_SAFE_R
	{
		return (0 == buff->count);		
	}

// is full
#ifdef USE_ISR_SAFE_RB
	esBL rbIsFull(rbHANDLE buff)
	{
		esBL result;
		disableIrq();
		result = rbIsFullFromIsr(buff);
		enableIrq();

		return result;
	}

	esBL rbIsFullFromIsr(rbHANDLE buff)
#else
	esBL rbIsFull(rbHANDLE buff)	
#endif	// USE_ISR_SAFE_R
{
	return buff->mask+1 == buff->count;
}

// elements count
#ifdef USE_ISR_SAFE_RB
	esU32 rbGetCount(rbHANDLE buff)
	{
		esU32 result;
		disableIrq();
		result = rbGetCountFromIsr(buff);
		enableIrq();

		return result;
	}	

	esU32 rbGetCountFromIsr(rbHANDLE buff)
#else
	esU32 rbGetCount(rbHANDLE buff)	
#endif	// USE_ISR_SAFE_RB
{
 	return buff->count;
}

// flush entire buffer
#ifdef USE_ISR_SAFE_RB
	void rbFlush(rbHANDLE buff)
	{
		disableIrq();
		rbFlushFromIsr(buff);
		enableIrq();
	}

	void rbFlushFromIsr(rbHANDLE buff)
#else
	void rbFlush(rbHANDLE buff)	
#endif	// USE_ISR_SAFE_RB
{
	buff->in = buff->out = buff->count = 0;
}

#define RB_IS_NOT_FULL(buff) \
	(buff->mask+1 != buff->count)

// esU8 access
//
#ifdef USE_BYTE_RB

// append byte to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutB(rbHANDLE buff, esU8 b, esU32 timeout)
{
	esBL result = rbPushB(buff, b);
	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPushB(buff, b);
	}

	return result;
}

// append byte to the end of the buffer, return (false) if fail, (true) on success
#ifdef USE_ISR_SAFE_RB
	esBL rbPushB(rbHANDLE buff, esU8 b)
	{
		esBL result;
		disableIrq();
		result = rbPushFromIsrB(buff, b);
		enableIrq();

		return result;
	}

	esBL rbPushFromIsrB(rbHANDLE buff, esU8 b)
#else
	esBL rbPushB(rbHANDLE buff, esU8 b)	
#endif	// USE_ISR_SAFE_RB
{
	if( RB_IS_NOT_FULL(buff) ) // if we have room in buffer
	{
		((esU8*)buff->data)[buff->mask & buff->in++] = b;
		++buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// pop byte from the buffer with timeout in ms
esBL rbPopTimeoutB(rbHANDLE buff, esU8* b, esU32 timeout)
{		
	esBL result = rbPopB(buff, b);

	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPopB(buff, b);
	}

	return result;
}

// pop byte from front of the buffer
#ifdef USE_ISR_SAFE_RB
	esBL rbPopB(rbHANDLE buff, esU8* b)
	{
		esBL result;
		disableIrq();
		result = rbPopFromIsrB(buff, b);
		enableIrq();

		return result;
	}

	esBL rbPopFromIsrB(rbHANDLE buff, esU8* b)
#else
	esBL rbPopB(rbHANDLE buff, esU8* b)	
#endif	// USE_ISR_SAFE_RB
{
	if( buff->count )
	{
		if(b)
			*b = ((esU8*)buff->data)[buff->mask & buff->out++];
		else
			++buff->out;
		--buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

#endif // USE_BYTE_RB

// esU16 access
//
#ifdef USE_WORD_RB

// append word to the end of the buffer, return FALSE if fail, TRUE on success
#ifdef USE_ISR_SAFE_RB
	esBL rbPushW(rbHANDLE buff, esU16 w)
	{
		esBL result;
		disableIrq();
		result = rbPushFromIsrW(buff, w);
		enableIrq();

		return result;
	}

	esBL rbPushFromIsrW(rbHANDLE buff, esU16 w)
#else	// USE_ISR_SAFE_RB
	esBL rbPushW(rbHANDLE buff, esU16 w)
#endif // USE_ISR_SAFE_RB
{
	if( RB_IS_NOT_FULL(buff) ) // if we have room in buffer
	{
		((esU16*)buff->data)[buff->mask & buff->in++] = w;
		++buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// append word to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutW(rbHANDLE buff, esU16 w, esU32 timeout)
{
	esBL result = rbPushW(buff, w);
	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPushW(buff, w);
	}

	return result;
}

// pop word from front of the buffer
#ifdef USE_ISR_SAFE_RB
	esBL rbPopW(rbHANDLE buff, esU16* w)
	{
		esBL result;
		disableIrq();
		result = rbPopFromIsrW(buff, w);
		enableIrq();

		return result;
	}

	esBL rbPopFromIsrW(rbHANDLE buff, esU16* w)
#else
	esBL rbPopW(rbHANDLE buff, esU16* w)
#endif
{
	if( buff->count )
	{
		if(w)
			*w = ((esU16*)buff->data)[buff->mask & buff->out++];
		else
			++buff->out;
		--buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// pop word from the buffer with timeout in ms
esBL rbPopTimeoutW(rbHANDLE buff, esU16* w, esU32 timeout)
{
	esBL result = rbPopW(buff, w);

	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPopW(buff, w);
	}

	return result;
}

#endif // USE_WORD_RB

// esU32 access
//
#ifdef USE_DWORD_RB
// append dword to the end of the buffer, return FALSE if fail, TRUE on success
#ifdef USE_ISR_SAFE_RB
	esBL rbPushDW(rbHANDLE buff, esU32 dw)
	{
		esBL result;
		disableIrq();
		result = rbPushFromIsrDW(buff, dw);
		enableIrq();

		return result;
	}

	esBL rbPushFromIsrDW(rbHANDLE buff, esU32 dw)
#else	// USE_ISR_SAFE_RB
	esBL rbPushDW(rbHANDLE buff, esU32 dw)
#endif // USE_ISR_SAFE_RB
{
	if( RB_IS_NOT_FULL(buff) ) // if we have room in buffer
	{
		((esU32*)buff->data)[buff->mask & buff->in++] = dw;
		++buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// append dword to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutDW(rbHANDLE buff, esU32 dw, esU32 timeout)
{
	esBL result = rbPushDW(buff, dw);
	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPushDW(buff, dw);
	}

	return result;
}

// pop dword from front of the buffer
#ifdef USE_ISR_SAFE_RB
	esBL rbPopDW(rbHANDLE buff, esU32* dw)
	{
		esBL result;
		disableIrq();
		result = rbPopFromIsrDW(buff, dw);
		enableIrq();

		return result;
	}

	esBL rbPopFromIsrDW(rbHANDLE buff, esU32* dw)
#else
	esBL rbPopDW(rbHANDLE buff, esU32* dw)
#endif
{
	if( buff->count )
	{
		if(dw)
			*dw = ((esU32*)buff->data)[buff->mask & buff->out++];
		else
			++buff->out;
		--buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// pop dword from the buffer with timeout in ms
esBL rbPopTimeoutDW(rbHANDLE buff, esU32* dw, esU32 timeout)
{
	esBL result = rbPopDW(buff, dw);

	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPopDW(buff, dw);
	}

	return result;
}

#endif // USE_DWORD_RB

// esF access
//
#ifdef USE_FLOAT_RB
// append float to the end of the buffer, return FALSE if fail, TRUE on success
#ifdef USE_ISR_SAFE_RB
	esBL rbPushF(rbHANDLE buff, esF f)
	{
		esBL result;
		disableIrq();
		result = rbPushFromIsrF(buff, f);
		enableIrq();

		return result;
	}

	esBL rbPushFromIsrF(rbHANDLE buff, esF f)
#else	// USE_ISR_SAFE_RB
	esBL rbPushF(rbHANDLE buff, esF f)
#endif // USE_ISR_SAFE_RB
{
	if( RB_IS_NOT_FULL(buff) ) // if we have room in buffer
	{
		((esF*)buff->data)[buff->mask & buff->in++] = f;
		++buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// append float to buffer waiting timeout for buffer space to become available
esBL rbPushTimeoutF(rbHANDLE buff, esF f, esU32 timeout)
{
	esBL result = rbPushF(buff, f);
	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPushF(buff, f);
	}

	return result;
}

// pop float from front of the buffer
#ifdef USE_ISR_SAFE_RB
	esBL rbPopF(rbHANDLE buff, esF* f)
	{
		esBL result;
		disableIrq();
		result = rbPopFromIsrF(buff, f);
		enableIrq();

		return result;
	}

	esBL rbPopFromIsrF(rbHANDLE buff, esF* f)
#else
	esBL rbPopF(rbHANDLE buff, esF* f)
#endif
{
	if( buff->count )
	{
		if(f)
			*f = ((esF*)buff->data)[buff->mask & buff->out++];
		else
			++buff->out;
		--buff->count;
		return TRUE;
	}
	else
		return FALSE;
}

// pop float from the buffer with timeout in ms
esBL rbPopTimeoutF(rbHANDLE buff, esF* f, esU32 timeout)
{
	esBL result = rbPopF(buff, f);
	while( !result && timeout-- )
	{
		usDelay(1000);
		result = rbPopF(buff, f);
	}

	return result;
}

#endif // USE_FLOAT_RB
