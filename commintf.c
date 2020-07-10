#include <esfwxe/target.h>
#include <string.h>
#pragma hdrstop

#include <esfwxe/commintf.h>
#include <esfwxe/trace.h>

// basic channel initialization (must necesserily be called first from
// the code which initializes channel with specific bus impl)
//
void chnlInit(EseChannelIo* channel, busHANDLE bus)
{
  channel->setConfig = 0;
  channel->getConfig = 0;
  channel->lock = 0;
  channel->unlock = 0;
  channel->connect = 0;
  channel->disconnect = 0;
  channel->isConnected = 0;
  channel->bytesPut = 0;
  channel->bytesGet = 0;
  channel->resetIo = 0;
  channel->txBatchBegin = 0;
  channel->txBatchEnd = 0;  
  channel->waitTxEmpty = 0;
  channel->isRateSupported = 0;
  channel->rateGet = 0;
  channel->rateSet = 0;
  channel->sendTimeEstimateGet = 0;
  channel->errorGet = 0;
  channel->m_type = CHNL_UNDEFINED;
  channel->m_bus = bus;
  channel->m_breaker = 0;
}

// channel methods
//
// configuration
esBL chnlSetConfig(EseChannelIo* chnl, const void* config)
{
    if( chnl && config && chnl->setConfig )
        return chnl->setConfig(chnl, config);

    return FALSE;
}

void chnlGetConfig(EseChannelIo* chnl, void* config)
{
    if( chnl && config && chnl->getConfig )
        chnl->getConfig(chnl, config);
}

// check if channel should break io
void chnlBreakerSet(EseChannelIo* chnl, EseChannelIoBreaker* brk)
{
    if(chnl)
        chnl->m_breaker = brk;
}

esBL chnlIsBreaking(EseChannelIo* chnl)
{
    if( chnl &&    chnl->m_breaker && chnl->m_breaker->isBreaking )
        return chnl->m_breaker->isBreaking(chnl->m_breaker);
             
    return FALSE;
}

// locking | unlocking
esBL chnlLock(EseChannelIo* chnl, esU32 timeout)
{
    if( chnl && chnl->lock )
        return chnl->lock(chnl, timeout);

    return FALSE;
}

void chnlUnlock(EseChannelIo* chnl)
{
    if( chnl && chnl->unlock )
        chnl->unlock(chnl);
}

// connect-disconnect, check connection status
esBL chnlConnect(EseChannelIo* chnl)
{
    if( chnl && chnl->connect )
        return chnl->connect(chnl);

    return FALSE;
}

void chnlDisconnect(EseChannelIo* chnl)
{
    if( chnl && chnl->disconnect )
        chnl->disconnect(chnl);
}

esBL chnlIsConnected(EseChannelIo* chnl)
{
    if( chnl && chnl->isConnected )
        return chnl->isConnected(chnl);

    return FALSE;
}

// data io
esU32 chnlPutBytes(EseChannelIo* chnl, const esU8* data, esU32 count)
{
    if( chnl && 
            chnl->bytesPut && 
            !chnlIsBreaking(chnl) )
        return chnl->bytesPut(chnl, data, count);
        
    return 0;        
}

esU32 chnlGetBytes(EseChannelIo* chnl, esU8* data, esU32 count, esU32 timeout)
{
    if( chnl && 
            chnl->bytesGet && 
            !chnlIsBreaking(chnl) )
        return chnl->bytesGet(chnl, data, count, timeout);

    return 0;
}

esBL chnlTxBatchBegin(EseChannelIo* chnl)
{
    if( 
    chnl && 
        chnl->txBatchBegin && 
        !chnlIsBreaking(chnl) 
  )
        return chnl->txBatchBegin(chnl);

    return TRUE;
}

void chnlTxBatchEnd(EseChannelIo* chnl, esBL ok)
{
    if( 
    chnl && 
        chnl->txBatchEnd
  )
        chnl->txBatchEnd(chnl, ok);
}

void chnlResetIo(EseChannelIo* chnl)
{
    if( chnl && chnl->resetIo )
  {
        chnl->resetIo(chnl);    
    ES_DEBUG_TRACE0("Channel was reset\n")
  }
}

esBL chnlWaitTxEmpty(EseChannelIo* chnl)
{
    if( chnl &&    chnl->waitTxEmpty ) 
        return chnl->waitTxEmpty(chnl);
    
    return FALSE;
}

// rate support|change
esBL chnlIsRateSupported(EseChannelIo* chnl, esU32 rate)
{
    return chnl && 
        chnl->isRateSupported &&
        chnl->isRateSupported(chnl, rate);
}

esU32 chnlGetRate(EseChannelIo* chnl)
{
    if( chnl && chnl->rateGet )
        return chnl->rateGet(chnl);

    return 0;
}

esBL chnlSetRate(EseChannelIo* chnl, esU32 rate)
{
    return chnl && chnl->rateSet &&
        chnl->rateSet(chnl, rate);
}

// error report
int chnlGetError(EseChannelIo* chnl)
{
    if( chnl && chnl->errorGet )
        return chnl->errorGet(chnl);

    return 0;
}

// return ms estimate needed to send len bytes over the channel
// 0 is returned if implementation is unknown
esU32 chnlSendTimeEstimateGet(EseChannelIo* chnl, esU32 len)
{
    if( chnl && chnl->sendTimeEstimateGet )
        return chnl->sendTimeEstimateGet(chnl, len);
    
    return 0;
}

// try to receive specific byte from comm channel, in specified count of retries,
esBL chnlSpecificByteReceive(EseChannelIo* chnl, esU8 b, esU32 retries)
{
    while( !chnlIsBreaking(chnl) )
    {
        esU8 r;
        if( 1 == chnlGetBytes(chnl, &r, 1, 0) &&
                r == b )
            return TRUE;
        else if( 0 != retries )
            --retries;
        else
            break;
    }
    
    return FALSE;
}

esBL chnlBinaryPatternReceive(EseChannelIo* chnl, const esU8* pattern, esU32 patternLen, esU32 retries)
{
    const esU8* pos = pattern;
    const esU8* end = pattern+patternLen;
    esBL ok = chnlSpecificByteReceive(chnl, *pos++, retries);
    while(pos < end && ok)
        ok = chnlSpecificByteReceive(chnl, *pos++, 0);
        
    return ok;
}
