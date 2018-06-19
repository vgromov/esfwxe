#include "stdafx.h"
#pragma hdrstop

#include "EseChannel.h"

EseChannelIntf::EseChannelIntf(size_t rxLen, size_t txLen) ESE_NOTHROW : 
m_breaker(0),
m_flags(0),
m_error(0),
m_rxLen(rxLen),
m_txLen(txLen)
{}

void EseChannelIntf::init() ESE_NOTHROW
{
  if( !ES_BIT_IS_SET(m_flags, flagInitialized) )
  {
    if( doInit() )
      ES_BIT_SET(m_flags, flagInitialized);
  }
  
  checkConfigured();
}
  
void EseChannelIntf::uninit() ESE_NOTHROW
{
  deactivate();
  
  if( ES_BIT_IS_SET(m_flags, flagInitialized) )
  {
    doUninit();
    ES_BIT_CLR(m_flags, flagInitialized);
  }
}
  
void EseChannelIntf::checkConfigured() ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagInitialized) && 
      !ES_BIT_IS_SET(m_flags, flagConfigured) )
  {
    if( doCheckConfigured() )
      ES_BIT_SET(m_flags, flagConfigured);
  }
}

bool EseChannelIntf::activate() ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagInitialized) && 
      !ES_BIT_IS_SET(m_flags, flagActive) )
  {
    m_error = 0;
    
    checkConfigured();
    if( doActivate() )
      ES_BIT_SET(m_flags, flagActive);
  }  

  return (m_flags & flagActive);
}

void EseChannelIntf::deactivate() ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagInitialized) && 
      ES_BIT_IS_SET(m_flags, flagActive) )
  {
    doDeactivate();
    
    ES_BIT_CLR(m_flags, flagActive);
  }
}

void EseChannelIntf::breakerSet(EseBreakerIntf* breaker) ESE_NOTHROW
{
  m_breaker = breaker;
}

bool EseChannelIntf::isBreaking(esU32 tmo) const ESE_NOTHROW
{
  if( !m_breaker )
    return false;
    
  return m_breaker->isBreaking(tmo);
}

size_t EseChannelIntf::receive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagActive) && 
      data && 
      toRead )
  {
    m_error = 0;
    return doReceive(data, toRead, tmo);
  }

  return 0;
}

size_t EseChannelIntf::send(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagActive) && 
      data && 
      toWrite )
  {
    m_error = 0;
    return doSend(data, toWrite, tmo);
  }

  return 0;
}

bool EseChannelIntf::txBatchBegin() ESE_NOTHROW
{
  if( !ES_BIT_IS_SET(m_flags, flagTxBatch) )
  {
    ES_BIT_SET(m_flags, flagTxBatch);
    
    return doTxBatchBegin();
  }
  
  return false;
}

void EseChannelIntf::txBatchEnd(bool ok) ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagTxBatch) )
  {
    doTxBatchEnd(ok);

    ES_BIT_CLR(m_flags, flagTxBatch);
  }
}

bool EseChannelIntf::rxBatchBegin() ESE_NOTHROW
{
  if( !ES_BIT_IS_SET(m_flags, flagRxBatch) )
  {
    ES_BIT_SET(m_flags, flagRxBatch);
    
    return doRxBatchBegin();
  }
  
  return false;
}

void EseChannelIntf::rxBatchEnd(bool ok) ESE_NOTHROW
{
  if( ES_BIT_IS_SET(m_flags, flagRxBatch) )
  {
    ES_BIT_CLR(m_flags, flagRxBatch);
    
    doRxBatchEnd(ok);
  }
}

int EseChannelIntf::ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlIsValidRate:
  case ctlRxPendingGet:
  case ctlTxPendingGet:
    return rtosErrorParameterNotSupported;
  case ctlRate:
    return rtosOK;
  default:
    return rtosErrorParameterUnknown;
  }
}

int EseChannelIntf::ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW
{
  switch( ctl )
  {
  case ctlTxLen:
    *reinterpret_cast<size_t*>(data) = m_txLen;
    break;
  case ctlRxLen:
    *reinterpret_cast<size_t*>(data) = m_rxLen;
    break;
  case ctlRate:
    *reinterpret_cast<size_t*>(data) = 0;
    break;
  case ctlIsValidRate:
    // By default, any rate will do
    break;
  case ctlRxReset:
  case ctlTxReset:
  case ctlReset:
    return rtosErrorParameterNotSupported;
  default:
    return rtosErrorParameterUnknown;
  }

  return rtosOK;
}
