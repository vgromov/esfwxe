// FreeRTOS uart IO support
#include <esfwxe/target.h>
#pragma hdrstop

#ifdef USE_FREE_RTOS
// rtos stuff
#    include <esfwxe/core/rtos_intf.h>
#else
#    include <esfwxe/ringbuff.h>    // in case no rtos-es are used, invoke ringbuffer as io queue
#    if defined(USE_ISR_SAFE_RB) && !defined(USE_EMULATOR)
#        ifdef USE_USER_SWI
#            include <esfwxe/userSWI.h>
//#        else
//            extern void disableIrq(void);    
//            extern void enableIrq(void);    
#        endif
#    endif
#endif

#include <esfwxe/commintf.h>
#include <esfwxe/utils.h>

// driver stuff
#include <esfwxe/core/uart.h>
// irq support
#include <esfwxe/irq.h>

#pragma Otime

enum {
    uartPortMask         = 0x0007,
    uartOpened            = 0x0008,
#ifndef USE_FREE_RTOS
    uartTxReady            = 0x0010,
#endif
};

// uart ring buffers
#ifndef USE_FREE_RTOS
#    ifdef USE_UART_PORT0
        static volatile esU8 s_rxBuff0[UART0_RX_BUFFLEN];
        static volatile esU8 s_txBuff0[UART0_TX_BUFFLEN];
# endif
#    ifdef USE_UART_PORT1
        static volatile esU8 s_rxBuff1[UART1_RX_BUFFLEN];
        static volatile esU8 s_txBuff1[UART1_TX_BUFFLEN];
# endif
#endif

typedef struct 
{
    uartDCB                     dcb;
#if defined(USE_FREE_RTOS)
    rtosQUEUE_HANDLE  uartTxQueue;
    rtosQUEUE_HANDLE  uartRxQueue;
    rtosMUTEX_HANDLE     mutex;                         // mutex for port sharing between tasks
    rtosSEMAPHORE_HANDLE uartTxReady;     // semaphore signalling transmission is stopped
#else    
    RingBuff                    uartTxQueue;
    RingBuff                    uartRxQueue;
#endif
    uartErrorCode            err;
    esU32                         lineStatus;
    volatile esU32        dcr;

} uartPortStruct;

// uart port descriptors
static volatile uartPortStruct ports[uartPortCnt];

// rough max estimate of ms per 2 bytes at specified baud
static __inline esU32 uartGetMaxByteTimeoutMs( esU32 len, esU32 baud, esU32 dataBits, esU32 stopBits, esU32 parityBits )
{
    esU32 result = (len * 2000 * (dataBits + stopBits + parityBits + 1)) / baud;

    return result ? result : 1;
}

static __inline esU32 internalUartSendTimeEstimateGet(volatile uartPortStruct* ps, esU32 len)
{
    return uartGetMaxByteTimeoutMs(len, ps->dcb.baud, 5+ps->dcb.bits, 1+ps->dcb.stopBits, 
        (ps->dcb.parity != uartNO_PARITY) ? 1 : 0);
}

esU32 uartSendTimeEstimateGet(EseChannelIo* p, esU32 len)
{                                                    
    return internalUartSendTimeEstimateGet(    CAST_PORT_HANDLE(uartPortStruct, p->m_bus), len);
}

// return uart port number from the port handle, -1 if handle is invalid
int uartGetPortNum( uartHANDLE hPort )
{
    if( hPort != INVALID_HANDLE )
        return CAST_PORT_HANDLE(uartPortStruct, hPort)->dcr & uartPortMask;

    return -1;
}

// try to mutually exclusive acquire port resource
esBL uartLockPort(EseChannelIo* p, esU32 timeout)
{
#ifdef USE_RTOS
    return rtosMutexLock(CAST_PORT_HANDLE(uartPortStruct, p->m_bus)->mutex, timeout);
#else
    return TRUE;
#endif
}

void uartUnlockPort(EseChannelIo* p)
{
#ifdef USE_RTOS
    rtosMutexUnlock( CAST_PORT_HANDLE(uartPortStruct, p->m_bus)->mutex );
#endif
}

// access dcb
void uartGetDCB(EseChannelIo* p, uartDCB* dcb)
{
    if( dcb != NULL )
        *dcb = CAST_PORT_HANDLE(uartPortStruct, p->m_bus)->dcb;
}

esBL uartIsOpen(EseChannelIo* p)
{
    return p && ES_BIT_IS_SET(CAST_PORT_HANDLE(uartPortStruct, p->m_bus)->dcr, uartOpened);
}

// access line status and status code
uartErrorCode uartGetErrorCode(EseChannelIo* p)
{
    if( p )
        return CAST_PORT_HANDLE(uartPortStruct, p->m_bus)->err;
    else
        return uartInvalidHandle;
}

static void internal_uartOpen(uartHANDLE hPort);
static void internal_uartClose(uartHANDLE hPort);

// open uart port (merly enable interrupts)
esBL uartOpen(EseChannelIo* p)
{
    if( !uartIsOpen(p) )
    {
        uartHANDLE h = p->m_bus;
        DEF_PORT_STRUCT_VAR(uartPortStruct, h);

        uartPowerUp(h);
        ps->err = uartOK;
        ps->lineStatus = 0;
#ifdef USE_FREE_RTOS
        ES_BIT_SET(ps->dcr, uartOpened);
        rtosSemaphoreGive( ps->uartTxReady );
#else
        ES_BIT_SET(ps->dcr, uartOpened|uartTxReady);
#endif
        internal_uartOpen(h);    

        return TRUE;
    }

    return FALSE;
}

// close uart port (merely disable interrupts)
void uartClose(EseChannelIo* p)
{
    if( uartIsOpen(p) )
    {
        uartHANDLE h = p->m_bus;
        internal_uartClose(h);        
#ifdef USE_RTOS
        ES_BIT_CLR(CAST_PORT_HANDLE(uartPortStruct, h)->dcr, uartOpened);
        rtosSemaphoreGive( CAST_PORT_HANDLE(uartPortStruct, h)->uartTxReady );
#else
        ES_BIT_CLR(CAST_PORT_HANDLE(uartPortStruct, h)->dcr, (uartOpened | uartTxReady));
#endif
        uartPowerDown(h);
    }
}

// include particular hardware uart driver implementation
#if LPC23XX == 1
#    include "lpc23xx/uart_hw.cc"
#endif

#if LPC214X == 1
#    include "lpc214x/uart_hw.cc"
#endif

#if LPC2103 == 1
#    include "lpc2103/uart_hw.cc"
#endif

#if LPC11XX == 1
#    include "lpc11xx/uart_hw.cc"
#endif

#if LPC13XX == 1
#    include "lpc13xx/uart_hw.cc"
#endif

/////////////////////////////////// the rest of implementation
//

// reset uart io
//
// timeout specifies how long to wait for the RX queue to consider it empty
void uartReset(EseChannelIo* p)
{
    uartHANDLE h = p->m_bus;
#if defined( USE_RTOS )
  rtosQueueReset( CAST_PORT_HANDLE(uartPortStruct, h)->uartRxQueue );
  rtosQueueReset( CAST_PORT_HANDLE(uartPortStruct, h)->uartTxQueue );
#else
    rbFlush( &CAST_PORT_HANDLE(uartPortStruct, h)->uartRxQueue );
    rbFlush( &CAST_PORT_HANDLE(uartPortStruct, h)->uartTxQueue );
#endif
    // reset error status
    CAST_PORT_HANDLE(uartPortStruct, h)->err = uartOK;        
}

// retrieve char from the RX queue
//
esU32 uartGetBytes(EseChannelIo* p, esU8* pBytes, esU32 count, esU32 timeout)
{
    esU32 dwReceived = 0;
    DEF_PORT_STRUCT_VAR(uartPortStruct, p->m_bus);

    if( uartIsOpen(p) && 
            pBytes != NULL && 
            count )
    {
        if( !timeout )
            timeout = 1;
    
        while( dwReceived < count &&
            timeout && 
            !chnlIsBreaking(p) )
        {
            if( 
#if defined( USE_FREE_RTOS    )        
            rtosQueuePop(ps->uartRxQueue, pBytes, ps->dcb.rxTimeout)
#else
            rbPopTimeoutB( (rbHANDLE)&ps->uartRxQueue, pBytes, ps->dcb.rxTimeout)                  
#endif
            )    
            {        
                ++pBytes;
                ++dwReceived;
            }
            else
                timeout = 
                    (timeout > ps->dcb.rxTimeout)    ? timeout-ps->dcb.rxTimeout : 0;
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
        UART_ENABLE_INT(0);     // enable UART0 interrupt
        break;
#endif
#ifdef USE_UART_PORT1    
    case uart1:
      UART_ENABLE_INT(1);    // enable UART1 interrupt
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
      UART_DISABLE_INT(1);    // disable UART1 interrupts
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
esBL uartSetDCB(EseChannelIo* p, const uartDCB* dcb)
{
    uartHANDLE h = p->m_bus;
    esBL needReopen = uartIsOpen(p);
    if( needReopen )
        uartClose(p);

    // ensure uart periphery is powered up before DCB configuration
    uartPowerUp(h);

    switch( CAST_PORT_HANDLE(uartPortStruct, h)->dcr & uartPortMask )
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
    uartPowerDown( h );
    if( needReopen )
        uartOpen(p);

    return TRUE;
}

static __inline esBL uartIsTxReady(volatile uartPortStruct* ps)
{
#ifdef USE_RTOS
    return rtosSemaphoreTake(ps->uartTxReady, 0);
#else
    esBL result = FALSE;
    disableIrq();
    result = ES_BIT_IS_SET(ps->dcr, uartTxReady);
    enableIrq();

    return result;
#endif
}

static __inline void initTransfer(volatile uartPortStruct* ps, esU8 b)
{
#ifndef USE_RTOS
    // initiate transfer, the queue will be emptied from the uart ISR
    ES_BIT_CLR(ps->dcr, uartTxReady);
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

// generate outgoing break condition on UART
static __inline void internalUartBreakConditionGenerate(volatile uartPortStruct* ps)
{
    esU32 ustmo = 1000*internalUartSendTimeEstimateGet(ps, 1);
    switch( ps->dcr & uartPortMask )
    {
#ifdef USE_UART_PORT0
    case uart0:
        UART_BREAK_SET(0);
        usDelay(ustmo);
        UART_BREAK_UNSET(0);
        break;
#endif
#ifdef USE_UART_PORT1
    case uart1:
        UART_BREAK_SET(1);
        usDelay(ustmo);
        UART_BREAK_UNSET(1);
        break;
#endif
    }
}

// generate outgoing break condition on UART
void uartBreakConditionGenerate(EseChannelIo* p)
{
    if( uartIsOpen(p) )
        internalUartBreakConditionGenerate(CAST_PORT_HANDLE(uartPortStruct, p->m_bus));
}

esU32 uartPutBytes(EseChannelIo* p, const esU8* bytes, esU32 count)
{
    const esU8* pos = bytes;

    if( uartIsOpen(p) && bytes && count )
    {
        const esU8* end = bytes+count;
        uartHANDLE h = p->m_bus;
         esU8 dummy = 0;
        DEF_PORT_STRUCT_VAR(uartPortStruct, h);
        esBL ok = TRUE;
        while( !chnlIsBreaking(p) &&
                        pos < end )
        {
            if( !ok )
            {
                // we cannot put any more bytes to the TX queue. check if
                // transfer is running. if not, re-initiate transfer
                // otherwise, cancel sending
                if( uartIsTxReady(ps) &&
#if defined( USE_RTOS )
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
#if defined( USE_RTOS )            
                    while( rtosQueuePop( ps->uartTxQueue, &dummy, 50 ) );
#else
                    rbFlush( &ps->uartTxQueue );
#endif    
                    break;
                }
            }
            else
            {
                ok = 
#if defined( USE_RTOS )            
                    rtosQueuePushBack( ps->uartTxQueue, pos, ps->dcb.txTimeout );
#else
                    rbPushTimeoutB( &ps->uartTxQueue, *pos, ps->dcb.txTimeout );
#endif    
                if( ok )
                    ++pos;
            }
        }

        if( chnlIsBreaking(p) )
            internalUartBreakConditionGenerate(ps);

        // if everything was pushed into buffer and transfer was never initiated && Tx is ready - start transfer finally
        if( pos == end && uartIsTxReady(ps) &&
#if defined( USE_RTOS )
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
    esBL ok = FALSE;
    switch( port )
    {
#ifdef USE_UART_PORT0
  case uart0:
        UART_IRQ_UNINIT(0);   // just in case
    UART_ENABLE_PWR(0);
       UART_SELECT_PINS0;
    UART_CONFIG(0, dcb);
        ok = UART_IRQ_INIT(0);
    break;
#endif //USE_UART_PORT0 
#ifdef USE_UART_PORT1
    case uart1:
        UART_IRQ_UNINIT(1);   // just in case
    UART_ENABLE_PWR(1);
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
        if( !ps->mutex )
            ps->mutex = rtosMutexCreate();
        if( !ps->uartTxReady )
            ps->uartTxReady = rtosSemaphoreCreate(1, 1);
#endif

        switch( port )
        {
#ifdef USE_UART_PORT0
      case uart0:
#    if defined( USE_FREE_RTOS )
            if( !ps->uartTxQueue ) ps->uartTxQueue = rtosQueueCreate( UART0_TX_BUFFLEN, sizeof(esU8) );
            if( !ps->uartRxQueue ) ps->uartRxQueue = rtosQueueCreate( UART0_RX_BUFFLEN, sizeof(esU8) );
#    else
            rbInit( (rbHANDLE)&ps->uartTxQueue, (esU8*)s_txBuff0, UART0_TX_BUFFLEN );
            rbInit( (rbHANDLE)&ps->uartRxQueue, (esU8*)s_rxBuff0, UART0_RX_BUFFLEN );
#    endif
            break;
#endif
#ifdef USE_UART_PORT1
      case uart1:
#    if defined( USE_FREE_RTOS )
            if( !ps->uartTxQueue ) ps->uartTxQueue = rtosQueueCreate( UART1_TX_BUFFLEN, sizeof(esU8) );
            if( !ps->uartRxQueue ) ps->uartRxQueue = rtosQueueCreate( UART1_RX_BUFFLEN, sizeof(esU8) );
#    else
            rbInit( (rbHANDLE)&ps->uartTxQueue, (esU8*)s_txBuff1, UART1_TX_BUFFLEN );
            rbInit( (rbHANDLE)&ps->uartRxQueue, (esU8*)s_rxBuff1, UART1_RX_BUFFLEN );
# endif
            break;
#endif
        }
        
        return ps;
    }    

  return INVALID_HANDLE; 
}

// wait (blocking) until TX is empty
esBL uartWaitTxEmpty(EseChannelIo* p)
{
    esBL breaking = chnlIsBreaking( p );
    while(     !breaking &&
                    !uartIsTxReady( CAST_PORT_HANDLE(uartPortStruct, p->m_bus) ) )
    {
        breaking = chnlIsBreaking(p);
        if( !breaking )
            msDelay(4);
    }

    return !breaking;
}

// rate support|change
esBL uartIsRateSupported(EseChannelIo* p, esU32 rate)
{
    return uart50 == rate ||        
        uart75 == rate ||        
        uart110 == rate ||        
        uart134    == rate ||        
        uart150    == rate ||    
        uart200    == rate ||
        uart300 == rate ||        
        uart600    == rate ||
        uart1200 == rate ||    
        uart1800 == rate ||    
        uart2400 == rate ||   
        uart4800 == rate ||
        uart9600 == rate ||        
        uart19200    == rate ||    
        uart38400    == rate ||    
        uart57600    == rate ||    
        uart115200 == rate ||
        uart128000 == rate ||
        uart153600 == rate ||
        uart230400 == rate ||
        uart256000 == rate ||
        uart460800 == rate ||
        uart921600 == rate;
}

esU32 uartGetRate(EseChannelIo* p)
{
    uartDCB dcb;
    uartGetDCB(p, &dcb);
    return dcb.baud;
}

esBL uartSetRate(EseChannelIo* p, esU32 rate)
{
    if( uartIsRateSupported(p, rate) )
    {    
        uartDCB dcb;
        uartGetDCB(p, &dcb);
        dcb.baud = (uartBaud)rate;    
        return uartSetDCB(p, &dcb);
    }

    return FALSE;
}

// uart channel initializer
void uartChannelInit(EseChannelIo* chnl, uartHANDLE bus)
{
    // perform basic initialization first
    chnlInit(chnl, (busHANDLE)bus);
    chnl->m_type = CHNL_UART;
    
    // assign interface implementation
    chnl->setConfig = (ChnlSetConfig)uartSetDCB;
    chnl->getConfig = (ChnlGetConfig)uartGetDCB;
    chnl->lock = uartLockPort;
    chnl->unlock = uartUnlockPort;
    chnl->connect = uartOpen;
    chnl->disconnect = uartClose;
    chnl->isConnected = uartIsOpen;
    chnl->bytesPut = uartPutBytes;
    chnl->bytesGet = uartGetBytes;
    chnl->resetIo = uartReset;
    chnl->waitTxEmpty = uartWaitTxEmpty;
    chnl->isRateSupported = uartIsRateSupported;
    chnl->rateGet = uartGetRate;
    chnl->rateSet = uartSetRate;
    chnl->sendTimeEstimateGet = uartSendTimeEstimateGet;
    chnl->errorGet = uartGetErrorCode;
}
