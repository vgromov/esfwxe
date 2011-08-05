// FreeRTOS uart IO support
#include <common/target.h>
#pragma hdrstop

#ifdef USE_FREE_RTOS
// rtos stuff
#	include <common/core/rtos_intf.h>
#else
#	include <common/ringbuff.h>	// in case no rtos-es are used, invoke ringbuffer as io queue
#	include <common/userSWI.h>
#endif

#ifdef USE_COMMINTF
#	include <common/commintf.h>
#	include <common/utils.h>
#endif

// driver stuff
#include <common/core/uart.h>
// irq support
#include <common/irq.h>

enum {
	uartPortMask 		= 0x0007,
	uartOpened			= 0x0008,
#ifndef USE_FREE_RTOS
	uartTxReady			= 0x0010,
#endif
};

typedef struct 
{
	WORD							dcr;
	uartDCB 					dcb;
#if defined(USE_FREE_RTOS)
	rtosQUEUE_HANDLE	uartTxQueue;
	rtosQUEUE_HANDLE	uartRxQueue;
	rtosMUTEX_HANDLE 	mutex; 					// mutex for port sharing between tasks
	rtosSEMAPHORE_HANDLE uartTxReady; 		// semaphore signalling transmission is stopped
#else	
	BYTE							txData[UART_TX_BUFFLEN];
	RingBuff					uartTxQueue;
	BYTE							rxData[UART_RX_BUFFLEN];
	RingBuff					uartRxQueue;
#endif
	uartErrorCode			err;
	DWORD 						lineStatus;

} uartPortStruct;

// uart port descriptors
static volatile uartPortStruct ports[uartPortCnt];

// rough max estimate of ms per 2 bytes at specified baud
static DWORD uartGetMaxByteTimeoutMs( DWORD len, DWORD baud, DWORD dataBits, DWORD stopBits, DWORD parityBits )
{
	DWORD result = (len * 2000 * (dataBits + stopBits + parityBits + 1)) / baud;

	return result ? result : 1;
}

DWORD uartSendTimeEstimateGet(uartHANDLE hPort, DWORD len)
{													
	volatile uartPortStruct* ps = CAST_PORT_HANDLE(uartPortStruct, hPort);
	return uartGetMaxByteTimeoutMs(len, ps->dcb.baud, 5+ps->dcb.bits, 1+ps->dcb.stopBits, 
		(ps->dcb.parity != uartNO_PARITY) ? 1 : 0);
}

// return uart port number from the port handle, -1 if handle is invalid
int uartGetPortNum( uartHANDLE hPort )
{
	if( hPort != INVALID_HANDLE )
		return CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask;

	return -1;
}

// try to mutually exclusive acquire port resource
BOOL uartLockPort(uartHANDLE handle, DWORD timeout)
{
	if( handle != INVALID_HANDLE )
	{
#if defined( USE_FREE_RTOS )
		return rtosMutexLock(CAST_PORT_HANDLE(uartPortStruct, handle)->mutex, timeout);
#else
		return TRUE;
#endif
	}

	return FALSE;
}

void uartUnlockPort(uartHANDLE handle)
{
#ifdef USE_FREE_RTOS
	if( handle != INVALID_HANDLE )
		rtosMutexUnlock( CAST_PORT_HANDLE(uartPortStruct, handle)->mutex );
#endif
}

// access dcb
void uartGetDCB(uartHANDLE handle, uartDCB* dcb)
{
	if( handle != INVALID_HANDLE && dcb != NULL )
		*dcb = CAST_PORT_HANDLE(uartPortStruct, handle)->dcb;
}

BOOL uartIsOpen(uartHANDLE hPort)
{
	return hPort != INVALID_HANDLE && (uartOpened == (CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartOpened));
}

// access line status and status code
uartErrorCode uartGetErrorCode(uartHANDLE hPort)
{
	if( hPort != INVALID_HANDLE )
		return CAST_PORT_HANDLE(uartPortStruct, hPort)->err;
	else
		return uartInvalidHandle;
}

static void internal_uartOpen(uartHANDLE hPort);
static void internal_uartClose(uartHANDLE hPort);

// open uart port (merly enable interrupts)
BOOL uartOpen(uartHANDLE hPort)
{
	if( hPort != INVALID_HANDLE && !uartIsOpen(hPort) )
	{
		DEF_PORT_STRUCT_VAR(uartPortStruct, hPort);

		uartPowerUp(hPort);
		ps->err = uartOK;
		ps->lineStatus = 0;
#ifdef USE_FREE_RTOS
		ps->dcr |= uartOpened;
		rtosSemaphoreGive( CAST_PORT_HANDLE(uartPortStruct, hPort)->uartTxReady );
#else
		ps->dcr |= uartOpened|uartTxReady;
#endif
		internal_uartOpen(hPort);	

		return TRUE;
	}

	return FALSE;
}

// close uart port (merely disable interrupts)
void uartClose(uartHANDLE hPort)
{
	if( uartIsOpen(hPort) )
	{
		internal_uartClose(hPort);		
#ifdef USE_FREE_RTOS
		CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr &= ~uartOpened;
		rtosSemaphoreGive( CAST_PORT_HANDLE(uartPortStruct, hPort)->uartTxReady );
#else
		CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr &= ~(uartOpened | uartTxReady);
#endif
		uartPowerDown(hPort);
	}
}

// include particular hardware uart driver implementation
#if LPC23XX == 1
	#include "lpc23xx/uart_hw.cc"
#endif

#if LPC214X == 1
	#include "lpc214x/uart_hw.cc"
#endif

#if LPC2103 == 1
	#include "lpc2103/uart_hw.cc"
#endif

/////////////////////////////////// the rest of implementation
//

// reset uart io
//
// timeout specifies how long to wait for the RX queue to consider it empty
void uartReset(uartHANDLE hPort, DWORD timeout)
{
	if( hPort != INVALID_HANDLE )
	{
#if defined( USE_FREE_RTOS )
		BYTE dummy;
		while( rtosQueuePop( CAST_PORT_HANDLE(uartPortStruct, hPort)->uartRxQueue, &dummy, timeout ) );
#else
		rbFlush( &CAST_PORT_HANDLE(uartPortStruct, hPort)->uartRxQueue );
#endif
		// reset error status
		CAST_PORT_HANDLE(uartPortStruct, hPort)->err = uartOK;		
	}
}

// retrieve char from the RX queue
//
DWORD uartGetBytes( uartHANDLE hPort, BYTE* pBytes, DWORD count, DWORD timeout, CommChannelBreak* brk )
{
	DWORD dwReceived = 0;
	DEF_PORT_STRUCT_VAR(uartPortStruct, hPort);

	if( uartIsOpen(hPort) && 
			pBytes != NULL && 
			count )
	{
		while( dwReceived < count && 
			0 != timeout &&
			!chnlIsBreaking(brk) )
		{
			if( 
#if defined( USE_FREE_RTOS	)		
			rtosQueuePop(ps->uartRxQueue, pBytes, ps->dcb.rxTimeout)
#else
			rbPopTimeoutB( (rbHANDLE)&ps->uartRxQueue, 
				pBytes, ps->dcb.rxTimeout)				  
#endif
			)	
			{		
				++pBytes;
				++dwReceived;
			}
			else
				timeout = 
					(timeout > ps->dcb.rxTimeout)	? timeout-ps->dcb.rxTimeout : 0;
		}
	}

	return dwReceived;
}

// open uart port (merly enable interrupts)
static __inline void internal_uartOpen(uartHANDLE hPort)
{
	switch( CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask )
	{
#ifdef USE_UART_PORT0
	case uart0:
		UART_ENABLE_INT(0); 	// enable UART0 interrupt
		break;
#endif
#ifdef USE_UART_PORT1	
	case uart1:
  	UART_ENABLE_INT(1);	// enable UART1 interrupt
		break;
#endif
	}
}

// close uart port (merely disable interrupts)
static __inline void internal_uartClose(uartHANDLE hPort)
{
	switch( CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask )
	{
#ifdef USE_UART_PORT0
	case uart0:
  	UART_DISABLE_INT(0); // disable UART0 interrupts
		break;
#endif
#ifdef USE_UART_PORT1	
	case uart1:
  	UART_DISABLE_INT(1);	// disable UART1 interrupts
		break;
#endif		
	}
}

void uartPowerUp(uartHANDLE hPort)
{
	if( INVALID_HANDLE == hPort )
		return;

	switch( CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask )
	{
#ifdef USE_UART_PORT0
	case uart0:
		UART_ENABLE_PWR(0); // enable UART0 power
		break;
#endif
#ifdef USE_UART_PORT1	
	case uart1:
		UART_ENABLE_PWR(1); // enable UART1 power
		break;
#endif		
	}	
}

void uartPowerDown(uartHANDLE hPort)
{
	if( INVALID_HANDLE == hPort )
		return;

	switch( CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask )
	{
#ifdef USE_UART_PORT0
	case uart0:
		UART_DISABLE_PWR(0); // disable UART0 power
		break;
#endif
#ifdef USE_UART_PORT1	
	case uart1:
		UART_DISABLE_PWR(1); // disable UART1 power
		break;
#endif		
	}	
}

// configure dcb
BOOL uartSetDCB(uartHANDLE handle, const uartDCB* dcb)
{
	if( handle != INVALID_HANDLE )
	{
		BOOL needReopen = uartIsOpen(handle);
		if( needReopen )
			uartClose( handle );

		// ensure uart periphery is powered up before DCB configuration
		uartPowerUp( handle );

		switch( CAST_PORT_HANDLE(uartPortStruct, handle)->dcr & uartPortMask )
		{
#ifdef USE_UART_PORT0
  	case uart0:
			UART_CONFIG(0, dcb);
			break;
#endif //USE_UART_PORT0 
#ifdef USE_UART_PORT1
		case uart1:
			UART_CONFIG(1, dcb);
	  	break;
#endif // USE_UART_PORT1
		}

		// switch power off afterwards
		uartPowerDown( handle );

		if( needReopen )
			uartOpen( handle );

		return TRUE;
	}

	return FALSE;
}

static BOOL __inline uartIsTxReady(volatile uartPortStruct* ps)
{
#ifdef USE_FREE_RTOS
	return rtosSemaphoreTake(ps->uartTxReady, 0);
#else
	BOOL result = FALSE;
	disableIrq();
	result = uartTxReady == (ps->dcr & uartTxReady);
	enableIrq();

	return result;
#endif
}

static void initTransfer(volatile uartPortStruct* ps, BYTE b)
{
#ifndef USE_FREE_RTOS
	// initiate transfer, the queue will be emptied from the uart ISR
	ps->dcr &= ~uartTxReady;
#endif

	switch( ps->dcr & uartPortMask )
	{
#ifdef USE_UART_PORT0
	case uart0:
		UART_PUT_BYTE(0, b);
		break;
#endif
#ifdef USE_UART_PORT1
	case uart1:
		UART_PUT_BYTE(1, b);				
		break;
#endif
	}
}

DWORD uartPutBytes(uartHANDLE hPort, const BYTE* bytes, DWORD count, CommChannelBreak* brk)
{
	const BYTE* pos = bytes;

	if( uartIsOpen(hPort) && bytes && count )
	{
		const BYTE* end = bytes+count;
 		BYTE dummy = 0;
		DEF_PORT_STRUCT_VAR(uartPortStruct, hPort);
		BOOL ok = TRUE;
		while( !chnlIsBreaking(brk) &&
						pos < end )
		{
			if( !ok )
			{
				// we cannot put any more bytes to the TX queue. check if
				// transfer is running. if not, re-initiate transfer
				// otherwise, cancel sending
				if( uartIsTxReady(ps) &&
#if defined( USE_FREE_RTOS )
					rtosQueuePop( ps->uartTxQueue, &dummy, 0 )
#else
					rbPopB( &ps->uartTxQueue, &dummy )
#endif	
				)
				{
					initTransfer(ps, dummy);
					ok = TRUE;
				}
				else
				{
#if defined( USE_FREE_RTOS )			
					while( rtosQueuePop( ps->uartTxQueue, &dummy, rtosMAX_DELAY ) );
#else
					rbFlush( &ps->uartTxQueue );
#endif	
					break;
				}
			}
			else
			{
				ok = 
#if defined( USE_FREE_RTOS )			
					rtosQueuePushBack( ps->uartTxQueue, pos, ps->dcb.txTimeout );
#else
					rbPushTimeoutB( &ps->uartTxQueue, *pos, ps->dcb.txTimeout );
#endif	
				if( ok )
					++pos;
			}
		}

		// if everything was pushed into buffer and transfer was never initiated && Tx is ready - start transfer finally
		if( pos == end && uartIsTxReady(ps) &&
#if defined( USE_FREE_RTOS )
				rtosQueuePop( ps->uartTxQueue, &dummy, 0 )
#else
				rbPopB( &ps->uartTxQueue, &dummy )
#endif 
		)
			initTransfer(ps, dummy);
	}

	return pos-bytes;
}

uartHANDLE uartInit( uartPort port, const uartDCB* dcb )
{
	BOOL ok = FALSE;
	switch( port )
	{
#ifdef USE_UART_PORT0
  case uart0:
		UART_IRQ_UNINIT(0);   // just in case
		UART_ENABLE_PWR(0); 	// enable UART0 power
		UART_SELECT_PINS0;
		UART_CONFIG(0, dcb);
		ok = UART_IRQ_INIT(0);
  break;
#endif //USE_UART_PORT0 
#ifdef USE_UART_PORT1
	case uart1:
		UART_IRQ_UNINIT(1);   // just in case
		UART_ENABLE_PWR(1); 	// enable UART1 power
		UART_SELECT_PINS1;
		UART_CONFIG(1, dcb);
		ok = UART_IRQ_INIT(1);
  break;
#endif // USE_UART_PORT1
	}

	if( ok )
	{
		DEF_PORT_STRUCT_VAR(uartPortStruct, &ports[port]);
		ps->dcb = *dcb;
		ps->dcr = port;

#if defined( USE_FREE_RTOS )
		if( !ps->uartTxQueue )
			ps->uartTxQueue = rtosQueueCreate( UART_TX_BUFFLEN, sizeof(BYTE) );
		if( !ps->uartRxQueue )
			ps->uartRxQueue = rtosQueueCreate( UART_RX_BUFFLEN, sizeof(BYTE) );
		if( !ps->mutex )
			ps->mutex = rtosMutexCreate();
		if( !ps->uartTxReady )
			ps->uartTxReady = rtosSemaphoreCreate(1, 1);
#else
		rbInit( (rbHANDLE)&ps->uartTxQueue, (BYTE*)ps->txData, UART_TX_BUFFLEN );
		rbInit( (rbHANDLE)&ps->uartRxQueue, (BYTE*)ps->rxData, UART_RX_BUFFLEN );
#endif
		
		return ps;
	}	

  return INVALID_HANDLE; 
}

#ifdef USE_COMMINTF

// wait (blocking) until TX is empty
BOOL uartWaitTxEmpty(uartHANDLE hPort, CommChannelBreak* brk)
{
	if( hPort )
	{
		BOOL breaking = chnlIsBreaking( brk );
		while( 	!breaking &&
						!uartIsTxReady( CAST_PORT_HANDLE(uartPortStruct, hPort) ) )
		{
			breaking = chnlIsBreaking( brk );
			if( !breaking )
				usDelay(4000);
		}

		return !breaking;
	}

	return TRUE;
}

// rate support|change
BOOL uartIsRateSupported(uartHANDLE hPort, DWORD rate)
{
	return uart50 == rate ||		
		uart75 == rate ||		
		uart110 == rate ||		
		uart134	== rate ||		
		uart150	== rate ||    
		uart200	== rate ||
		uart300 == rate ||		
		uart600	== rate ||
		uart1200 == rate ||	
		uart1800 == rate ||	
		uart2400 == rate ||   
		uart4800 == rate ||
		uart9600 == rate ||		
		uart19200	== rate ||	
		uart38400	== rate ||	
		uart57600	== rate ||	
		uart115200 == rate;
}

DWORD uartGetRate(uartHANDLE hPort)
{
	if( hPort )
	{
		uartDCB dcb;
		uartGetDCB(hPort, &dcb);
		return dcb.baud;
	}

	return 0;
}

BOOL uartSetRate(uartHANDLE hPort, DWORD rate)
{
	if( uartIsRateSupported(hPort, rate) )
	{	
		if( hPort )
		{
			uartDCB dcb;
			uartGetDCB(hPort, &dcb);
			dcb.baud = (uartBaud)rate;	
			return uartSetDCB(hPort, &dcb);
		}
	}

	return FALSE;
}

// uart channel initializer
void uartChannelInit(CommChannel* chnl, uartHANDLE bus)
{
	// perform basic initialization first
	chnlInit(chnl, bus);
	chnl->m_type = CHNL_UART;
	
	// assign interface implementation
	chnl->setConfig = (ChnlSetConfig)uartSetDCB;
	chnl->getConfig = (ChnlGetConfig)uartGetDCB;
	chnl->lock = uartLockPort;
	chnl->unlock = uartUnlockPort;
	chnl->connect = uartOpen;
	chnl->disconnect = uartClose;
	chnl->isConnected = uartIsOpen;
	chnl->putBytes = uartPutBytes;
	chnl->getBytes = uartGetBytes;
	chnl->resetIo = uartReset;
	chnl->waitTxEmpty = uartWaitTxEmpty;
	chnl->isRateSupported = uartIsRateSupported;
	chnl->getRate = uartGetRate;
	chnl->setRate = uartSetRate;
	chnl->sendTimeEstimateGet = uartSendTimeEstimateGet;
	chnl->getError = uartGetErrorCode;
}

#endif // USE_COMMINTF
