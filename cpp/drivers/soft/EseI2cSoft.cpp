#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

#include <stddef.h>

// FreeRTOS
#include <FreeRTOS.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/concept/EseChannel.h>
#include <esfwxe/cpp/drivers/common/EseI2cChannelIoCtlDefs.h>

#include <esfwxe/cpp/os/EseMutex.h>
//----------------------------------------------------------------------

#include "EseI2cSoft.h"
//----------------------------------------------------------------------

EseI2cSoft::EseI2cSoft() ESE_NOTHROW :
ChannelT(0, 0),
m_rate(100000),
m_devAddr(0),
m_memAddr(0),
m_memAddrSize(0),
m_devAddr7bit(true)
{
  m_mx.init();
}
//----------------------------------------------------------------------

EseI2cSoft::~EseI2cSoft() ESE_NOTHROW
{
  m_mx.uninit();
}
//----------------------------------------------------------------------

uint32_t EseI2cSoft::rateGet() const ESE_NOTHROW 
{ 
  return m_rate; 
}
//----------------------------------------------------------------------

bool EseI2cSoft::isMemAccessMode() const ESE_NOTHROW 
{ 
  return 0 != m_memAddrSize; 
}
//----------------------------------------------------------------------

bool EseI2cSoft::isOk() const ESE_NOTHROW
{
  return m_mx.isOk();
}
//----------------------------------------------------------------------

rtosStatus EseI2cSoft::lock(esU32 tmo /*= rtosMaxDelay*/ ) ESE_NOTHROW
{
  return m_mx.lock(tmo);
}
//----------------------------------------------------------------------

rtosStatus EseI2cSoft::unlock() ESE_NOTHROW
{
  return m_mx.unlock();
}
//----------------------------------------------------------------------

esU32 EseI2cSoft::dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW
{
  return EseI2cChannel::maxByteTimeoutMsGet(
    len, 
    m_rate,
    8       ///< Consider we got 8 bit word
  );
}
//----------------------------------------------------------------------

int EseI2cSoft::ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW
{
  switch(ctl)
  {
  case EseI2cChannelIoCtl::ctlDcb:
    return rtosErrorParameterNotSupported;
  case EseI2cChannelIoCtl::ctlRate:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
      rateSet( reinterpret_cast<uintptr_t>(data) );
    break;
  case EseI2cChannelIoCtl::ctlMasterMode:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else if( 0 == reinterpret_cast<uintptr_t>(data) ) //< Request slave mode - not supported
      return rtosErrorParameterNotSupported;
    break;
  case EseI2cChannelIoCtl::ctlDevAddr:
    m_devAddr = reinterpret_cast<uintptr_t>(data);
    break;
  case EseI2cChannelIoCtl::ctlDevAddr7bit:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
    {
      m_devAddr7bit = (0 != reinterpret_cast<uintptr_t>(data)); //< Request 7 bit address
      ES_BIT_CLR(m_flags, flagConfigured);
    }
    break;
  case EseI2cChannelIoCtl::ctlMemAddr:
    m_memAddr = reinterpret_cast<uintptr_t>(data);
    break;
  case EseI2cChannelIoCtl::ctlMemAddrSize:
    m_memAddrSize = reinterpret_cast<uintptr_t>(data);
    break;
  case ctlRxReset:
  case ctlTxReset:
  case ctlReset:
    masterAbort();
    break;
  default:
    return ChannelT::ioCtlSet(
      ctl, 
      data
    );
  }

  return rtosOK;
}
//----------------------------------------------------------------------

int EseI2cSoft::ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    return rtosErrorParameterNotSupported;
  case EseI2cChannelIoCtl::ctlRate:
    *reinterpret_cast<uintptr_t*>(data) = rateGet();
    break;
  case EseI2cChannelIoCtl::ctlMasterMode:
    *reinterpret_cast<uintptr_t*>(data) = true;
    break;
  case ctlDevAddr:
    *reinterpret_cast<uintptr_t*>(data) = m_devAddr;
    break;
  case EseI2cChannelIoCtl::ctlDevAddr7bit:
    *reinterpret_cast<uintptr_t*>(data) = m_devAddr7bit;
    break;
  case ctlMemAddr:
    *reinterpret_cast<uintptr_t*>(data) = m_memAddr;
    break;
  case ctlMemAddrSize:
    *reinterpret_cast<uintptr_t*>(data) = m_memAddrSize;
    break;
  case ctlRxPendingGet:
  case ctlTxPendingGet:
    return rtosErrorParameterNotSupported;
  default:
    return ChannelT::ioCtlGet(
      ctl, 
      data
    );
  }

  return rtosOK;
}
//----------------------------------------------------------------------

void EseI2cSoft::rateSet(uint32_t rate) ESE_NOTHROW
{
  ESE_ASSERT( !isActive() );
  
  m_rate = rate;
  ES_BIT_CLR(m_flags, flagConfigured);
}
//----------------------------------------------------------------------

size_t EseI2cSoft::masterReceive( esU16 devAddr, esU8* data, size_t toRead, esU32 ESE_UNUSED(tmo) ) ESE_NOTHROW
{
  m_devAddr = devAddr;
  
  if( 
    !data ||
    !toRead ||
    !addrRequest(true) 
  )
    return 0;
  
  esU8* pos = data;
  esU8* end = data + toRead;
  while( pos < end )
  {
    *pos++ = byteRead();
    if( pos < end )
      ack();
    else
      nack();
  }
  stop();
  
  return pos-data;
}
//----------------------------------------------------------------------

size_t EseI2cSoft::bufferSendInternal( const esU8* data, size_t toWrite, esU32 ESE_UNUSED(tmo), bool doStop ) ESE_NOTHROW
{
  const esU8* pos = data;
  const esU8* end = data + toWrite;
  while( pos < end )
  {
    byteWrite(
      *pos++
    );
    if( !ackWait() )
    {
      doStop = true;
      break;
    }
  }
  
  if( doStop )
    stop();
  
  return pos-data;
}
//----------------------------------------------------------------------

size_t EseI2cSoft::masterSend( esU16 devAddr, const esU8* data, size_t toWrite, esU32 tmo, bool doStop /* = true */ ) ESE_NOTHROW
{
  m_devAddr = devAddr;
  
  if( 
    !data ||
    !toWrite ||
    !addrRequest(false) 
  )
    return 0;
  
  return bufferSendInternal(
    data,
    toWrite,
    tmo,
    doStop
  );
}
//----------------------------------------------------------------------

size_t EseI2cSoft::masterMemReceive( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW
{
  ESE_ASSERT(memAddrSize > 0);

  if( memAddrSize > 1 )
    memAddr = SWAPB_WORD(memAddr);

  if( 
    memAddrSize == masterSend(
      devAddr,
      reinterpret_cast<const uint8_t*>(&memAddr),
      memAddrSize,
      tmo,
      false //< Do not stop after mem address write, if all went well
    )
  )
    return masterReceive(
      devAddr,
      data,
      toRead,
      tmo
    );
    
  return 0;
}
//----------------------------------------------------------------------

size_t EseI2cSoft::masterMemSend( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW
{
  ESE_ASSERT(memAddrSize > 0);

  if( memAddrSize > 1 )
    memAddr = SWAPB_WORD(memAddr);

  if(  
    memAddrSize == masterSend(
      devAddr,
      reinterpret_cast<const uint8_t*>(&memAddr),
      memAddrSize,
      tmo,
      false //< Do not stop after mem address write, if all went well
    )
  )
    return bufferSendInternal(
      data,
      toWrite,
      tmo,
      true //< Do stop upon completion
    );
    
  return 0;
}
//----------------------------------------------------------------------

size_t EseI2cSoft::doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW
{
  // Branch either simple or mem receive
  if( m_memAddrSize )
    return masterMemReceive(
      m_devAddr,
      m_memAddr,
      m_memAddrSize,
      data,
      toRead,
      tmo
    );
  else
    return masterReceive(
      m_devAddr,
      data,
      toRead,
      tmo
    );
}
//----------------------------------------------------------------------

size_t EseI2cSoft::doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW
{
  // Branch either simple or mem send
  if( m_memAddrSize )
    return masterMemSend(
      m_devAddr,
      m_memAddr,
      m_memAddrSize,
      data,
      toWrite,
      tmo
    );
  else
    return masterSend(
      m_devAddr,
      data,
      toWrite,
      tmo
    );
}
//----------------------------------------------------------------------

bool EseI2cSoft::start() ESE_NOTHROW
{
  m_error = EseI2cChannelError::noError;

  sdaSet(true);
  sclSet(true);
  delay_4();
  if( !sdaGet() ) //< Someone else is keeping SDA low - arbitration conflict
  {
    m_error = EseI2cChannelError::arbitrationLost;
    return false;
  }
  sdaSet(false);
  delay_4();
  if( sdaGet() ) //< Something is keeping SDA high - bus error
  {
    m_error = EseI2cChannelError::busError;
    return false;
  }
  delay_2();

  return true;
}
//----------------------------------------------------------------------
  
void EseI2cSoft::stop() ESE_NOTHROW
{
  sclSet(false);
  delay_4();
  sdaSet(false);
  delay_4();
  sclSet(true);
  delay_4();
  sdaSet(true);
  delay_4();
}
//----------------------------------------------------------------------

void EseI2cSoft::ack() ESE_NOTHROW
{
  sclSet(false);
  delay_4();
  sdaSet(false);
  delay_4();
  sclSet(true);
  delay_2();
  sclSet(false);
  delay_2();
}
//----------------------------------------------------------------------

void EseI2cSoft::nack() ESE_NOTHROW
{
  sclSet(false);
  delay_4();
  sdaSet(true);
  delay_4();
  sclSet(true);
  delay_2();
  sclSet(false);
  delay_2();
}
//----------------------------------------------------------------------

bool EseI2cSoft::ackWait() ESE_NOTHROW
{
  sclSet(false);
  delay_4();
  sdaSet(true);
  delay_4();
  sclSet(true);
  delay_2();
  if( sdaGet() )  //< SDA not low - no ack from slave
  {
    sclSet(false);
    return false;
  }
  
  sclSet(false);
  return true;
}
//----------------------------------------------------------------------
  
bool EseI2cSoft::addrRequest(bool forRead) ESE_NOTHROW
{
  if( !start() )
    return false;

  if( m_devAddr7bit )
  {
    byteWrite( 
      forRead ? 
        m_devAddr | 0x01 :
        m_devAddr
    );
    
    if( !ackWait() ) 
    {
      stop();
      m_error = EseI2cChannelError::addressNacked;
      return false;
    }
  }
  else
  {
    byteWrite( 
      0xF0 |
      (
        forRead ? 
          (((m_devAddr & 0x300) >> 7) | 0x01 ) :
          ((m_devAddr & 0x300) >> 7)
      )
    );
    
    if( !ackWait() ) 
    {
      stop();
      m_error = EseI2cChannelError::addressNacked;
      return false;
    }
    
    byteWrite(
      m_devAddr & 0xFF
    );

    if( !ackWait() ) 
    {
      stop();
      m_error = EseI2cChannelError::addressNacked;
      return false;
    }
  }
  
  return true;
}
//----------------------------------------------------------------------

void EseI2cSoft::byteWrite(uint8_t b) ESE_NOTHROW
{
  for(uint8_t idx = 0; idx < 8; ++idx)
  {
    sclSet(false);
    delay_4();
    sdaSet( ES_BIT_IS_SET(b, 0x80) );
    b <<= 1;
    delay_4();
    sclSet(true);
    delay_2();
  }
}
//----------------------------------------------------------------------

uint8_t EseI2cSoft::byteRead() ESE_NOTHROW
{
  uint8_t b = 0;

  sdaSet(true); //< Slave will control SDA on reading
  for( uint8_t idx = 0; idx < 8; ++idx ) 
  {
    b <<= 1;
    sclSet(false);
    delay_2();
    sclSet(true);
    delay_4();
    if( sdaGet() ) 
      ES_BIT_SET(b, 0x01);
    delay_4();
  }
  
  return b;
}
//----------------------------------------------------------------------

bool EseI2cSoft::doInit() ESE_NOTHROW
{ 
  return true; 
}
//----------------------------------------------------------------------

void EseI2cSoft::doUninit() ESE_NOTHROW
{}
//----------------------------------------------------------------------

void EseI2cSoft::masterAbort() ESE_NOTHROW 
{}
//----------------------------------------------------------------------

const EseI2cSoft::ChannelT::HandleT& EseI2cSoft::doHandleGet() const ESE_NOTHROW 
{ 
  return *this; 
}
//----------------------------------------------------------------------

EseI2cSoft::ChannelT::HandleT& EseI2cSoft::doHandleGet() ESE_NOTHROW
{ 
  return *this; 
}
//----------------------------------------------------------------------
