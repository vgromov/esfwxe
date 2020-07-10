// SPI IO support
#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>

#if defined( USE_FREE_RTOS )
// rtos stuff
#  include <esfwxe/core/rtos_intf.h>
#endif

// driver stuff
#include "spi.h"

// internal port structures
//

enum {
  SpiPortMask = 0x07,
  SpiOpened    = 0x08,
};

#define SPI_DCR_SET_PORT(dcr, port) (dcr) = (((dcr) & ~SpiPortMask) | (port))
#define SPI_DCR_GET_PORT(dcr) ((dcr) & SpiPortMask)

typedef struct 
{
  spiDCB             dcb;
  spiErrorCode      err;
#ifdef USE_FREE_RTOS
  rtosMUTEX_HANDLE   mutex;
#endif
  volatile esU32    dcr;

} spiPortStruct;

// SPI port descriptors
static volatile spiPortStruct s_ports[spiPortCnt];

// interface implementation
//
esBL spiLockPort(spiHANDLE hport, esU32 timeout)
{
#ifdef USE_FREE_RTOS

  if( hport != INVALID_HANDLE )
    return rtosMutexLock( CAST_PORT_HANDLE(spiPortStruct, hport)->mutex, timeout);

  return FALSE;

#else

  return TRUE;

#endif
}

void spiUnlockPort(spiHANDLE hport)
{
#ifdef USE_FREE_RTOS

  if( hport != INVALID_HANDLE )
    rtosMutexUnlock( CAST_PORT_HANDLE(spiPortStruct, hport)->mutex );

#endif
}

void spiGetDCB(spiHANDLE hport, spiDCB* dcb)
{
  if( hport != INVALID_HANDLE && dcb != NULL )
    *dcb = CAST_PORT_HANDLE(spiPortStruct, hport)->dcb;
}

esBL spiIsOpen(spiHANDLE hport)
{
  return hport != INVALID_HANDLE && 
    (CAST_PORT_HANDLE(spiPortStruct, hport)->dcr & SpiOpened);
}

spiErrorCode spiGetErrorCode(spiHANDLE hport)
{
  if( hport != INVALID_HANDLE )
    return CAST_PORT_HANDLE(spiPortStruct, hport)->err;
  else
    return spiInvalidHandle;
}

#if LPC23XX == 1
  #include "lpc23xx/spi_hw.cc"
#endif

#if LPC214X == 1
  #include "lpc214x/spi_hw.cc"
#endif

#if LPC2103 == 1
  #include "lpc2103/spi_hw.cc"
#endif

#if LPC13XX == 1
  #include "lpc13xx/spi_hw.cc"
#endif

void spiPowerUp(spiPort port)
{
  volatile spiDCB* pdcb = &s_ports[port].dcb;
  switch(port)
  {
#ifdef USE_SPI_PORT0
  case spi0:
    SPI0_ENABLE_PWR;
    SPI_CONFIG(0, pdcb);    
  break;
#endif
#ifdef USE_SPI_PORT1
  case spi1:
    SPI1_ENABLE_PWR;
    SPI_CONFIG(1, pdcb);
  break;
#endif
  }
}

void spiPowerDown(spiPort port)
{
  switch(port)
  {
#ifdef USE_SPI_PORT0
  case spi0:
    SPI0_DISABLE_PWR;    
    break;
#endif
#ifdef USE_SPI_PORT1
  case spi1:
    SPI1_DISABLE_PWR;    
    break;
#endif
  }
}

esBL spiOpen(spiHANDLE hport)
{
  if( hport != INVALID_HANDLE && !spiIsOpen(hport) )
  {
    volatile spiPortStruct* ps = CAST_PORT_HANDLE(spiPortStruct, hport);
    spiPowerUp((spiPort)SPI_DCR_GET_PORT(ps->dcr));
    ps->err = spiOK;
    ps->dcr |= SpiOpened;

    return TRUE;
  }

  return FALSE;
}

void spiClose(spiHANDLE hport)
{
  if( spiIsOpen(hport) )
  {
    volatile spiPortStruct* ps = CAST_PORT_HANDLE(spiPortStruct, hport);
    ps->dcr &= ~SpiOpened;
    spiPowerDown((spiPort)SPI_DCR_GET_PORT(ps->dcr));
  }
}

spiHANDLE spiInit( spiPort port, const spiDCB* dcb )
{
  s_ports[ port ].dcb = *dcb;  
  SPI_DCR_SET_PORT(s_ports[ port ].dcr, port);
#ifdef USE_FREE_RTOS
  s_ports[ port ].mutex = rtosMutexCreate();
#endif
  
  spiPowerUp(port);

  return (spiHANDLE)&s_ports[port];
}

esBL spiSetDCB(spiHANDLE hport, const spiDCB* dcb)
{
  if( !spiIsOpen(hport) && 
      dcb != NULL )
  {
    volatile spiPortStruct* ps = CAST_PORT_HANDLE(spiPortStruct, hport);
    switch(SPI_DCR_GET_PORT(ps->dcr))
    {
#ifdef USE_SPI_PORT0
    case spi0:
      SPI0_ENABLE_PWR;
      SPI_CONFIG(0, dcb);
      break;
#endif  
#ifdef USE_SPI_PORT1
    case spi1:
      SPI1_ENABLE_PWR;
      SPI_CONFIG(1, dcb);
      break;
#endif  
    }    
    ps->dcb = *dcb;

    return TRUE;
  }

  return FALSE;
}

esU32 spiPutBytes( spiHANDLE hport, const esU8* Bytes, esU32 length )
{
  const esU8* pos = Bytes;
  const esU8* end = Bytes + length;

  if( spiIsOpen(hport) && Bytes && length &&
    !((CAST_PORT_HANDLE(spiPortStruct, hport)->dcb.bits > spi8_BITS) && (length % 2)) )
  {
    volatile spiDCB* dcb = &CAST_PORT_HANDLE(spiPortStruct, hport)->dcb;
    esU32 inc = dcb->bits > spi8_BITS ? 2 : 1;

    // start transfer
    while( pos < end )
    {
      volatile esU16 dummy = dcb->bits > spi8_BITS ? *((esU16*)pos) : *pos;
      esU32 retries = SPI_CHECK_FOR_SENT_RETRIES;

      switch( SPI_DCR_GET_PORT(CAST_PORT_HANDLE(spiPortStruct, hport)->dcr) )
      {
#ifdef USE_SPI_PORT0
      case spi0:
        SPI_SEND_WORD(0, dummy);
        while( !SPI_CHECK_FOR_SENT(0) && retries ) --retries;
        SPI_RECEIVE_WORD(0, dummy);
        if( !retries )
          return pos-Bytes;
      break;
#endif
#ifdef USE_SPI_PORT1
      case spi1:
        SPI_SEND_WORD(1, dummy);
        while( !SPI_CHECK_FOR_SENT(1) && retries ) --retries;
        SPI_RECEIVE_WORD(1, dummy);
        if( !retries )
          return pos-Bytes;
      break;
#endif

      }      
      pos += inc;
    }
  }

  return pos-Bytes;
}

esU32 spiGetBytes( spiHANDLE hport, esU8* Bytes, esU32 length )
{  
  esU8* pos = Bytes;
  const esU8* end = Bytes + length;

  if( spiIsOpen(hport) && Bytes && length && 
      !((CAST_PORT_HANDLE(spiPortStruct, hport)->dcb.bits > spi8_BITS) && (length % 2)) )
  {
    volatile spiDCB* dcb = &CAST_PORT_HANDLE(spiPortStruct, hport)->dcb;
    esU32 inc = dcb->bits > spi8_BITS ? 2 : 1;

    while( pos < end )
    {    
      esU32 retries = SPI_CHECK_FOR_SENT_RETRIES;
      switch( SPI_DCR_GET_PORT(CAST_PORT_HANDLE(spiPortStruct, hport)->dcr) )
      {
#ifdef USE_SPI_PORT0
      case spi0:
        SPI_SEND_WORD(0, 0xFFFF);
        while( !SPI_CHECK_FOR_SENT(0) && retries ) --retries;
        if( !retries ) // retries expired
        {
          SPI_RECEIVE_WORD(0, retries);  // flash RFIO
          return pos-Bytes;
        }
        if( dcb->bits > spi8_BITS )
        {
          SPI_RECEIVE_WORD(0, *((esU16*)pos));
        }
        else
        {
          SPI_RECEIVE_WORD(0, *pos);
        }
      break;
#endif
#ifdef USE_SPI_PORT1
      case spi1:
        SPI_SEND_WORD(1, 0xFFFF);
        while( !SPI_CHECK_FOR_SENT(1) && retries ) --retries;
        if( !retries ) // retries expired
        {
          SPI_RECEIVE_WORD(1, retries);  // flash RFIO
          return pos-Bytes;
        }
        if( dcb->bits > spi8_BITS )
        {
          SPI_RECEIVE_WORD(1, *((esU16*)pos));
        }
        else
        {
          SPI_RECEIVE_WORD(1, *pos);
        }
      break;
#endif
      }
      pos += inc;
    }
  }

  return pos-Bytes;
}
