#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/crc.h>

#include "EseCrcIntf.h"
#include "EseCrcintfImpl.h"
//-----------------------------------------------------------------------------

#ifdef USE_CRC7

EseCrc7::EseCrc7(uint32_t val) ESE_NOTHROW :
EseCrcIntfImpl(val)
{}
//-----------------------------------------------------------------------------

uint32_t EseCrc7::update(uint8_t val) ESE_NOTHROW
{
  m_crc = crc7update(
    m_crc, 
    val
  );
  
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc7::update(uint16_t val) ESE_NOTHROW
{
  m_crc = crc7(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc7::update(uint32_t val) ESE_NOTHROW
{
  m_crc = crc7(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc7::update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW
{
  m_crc = crc7(
    m_crc, 
    buff,
    buffLen
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef USE_CRC8

EseCrc8::EseCrc8(uint32_t val) ESE_NOTHROW :
EseCrcIntfImpl(val)
{}
//-----------------------------------------------------------------------------

uint32_t EseCrc8::update(uint8_t val) ESE_NOTHROW
{
  m_crc = crc8update(
    m_crc, 
    val
  );
  
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc8::update(uint16_t val) ESE_NOTHROW
{
  m_crc = crc8(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc8::update(uint32_t val) ESE_NOTHROW
{
  m_crc = crc8(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc8::update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW
{
  m_crc = crc8(
    m_crc, 
    buff,
    buffLen
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef USE_CRC16_CCITT

EseCrc16_CCIIT::EseCrc16_CCIIT(uint32_t val) ESE_NOTHROW :
EseCrcIntfImpl(val)
{}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_CCIIT::update(uint8_t val) ESE_NOTHROW
{
  m_crc = crc16ccitt_update(
    m_crc, 
    val
  );
  
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_CCIIT::update(uint16_t val) ESE_NOTHROW
{
  m_crc = crc16ccitt(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_CCIIT::update(uint32_t val) ESE_NOTHROW
{
  m_crc = crc16ccitt(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_CCIIT::update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW
{
  m_crc = crc16ccitt(
    m_crc, 
    buff,
    buffLen
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

#ifdef USE_CHECKSUM16

EseCrc16_RFC1071::EseCrc16_RFC1071(uint32_t val) ESE_NOTHROW :
EseCrcIntfImpl(val)
{}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_RFC1071::update(uint8_t val) ESE_NOTHROW
{
  m_crc += val;
  
  return get();
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_RFC1071::update(uint16_t val) ESE_NOTHROW
{
  m_crc += calcSumForChecksum16(
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return get();
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_RFC1071::update(uint32_t val) ESE_NOTHROW
{
  m_crc += calcSumForChecksum16(
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return get();
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_RFC1071::update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW
{
  m_crc += calcSumForChecksum16(
    buff,
    buffLen
  );
    
  return get();
}
//-----------------------------------------------------------------------------

uint32_t EseCrc16_RFC1071::get() const ESE_NOTHROW
{
  return checksum16FromSum(
    m_crc
  );
}
//-----------------------------------------------------------------------------

#endif


#ifdef USE_CRC32_IEEE_802_3

EseCrc32_IEEE_802_3::EseCrc32_IEEE_802_3(uint32_t val) ESE_NOTHROW :
EseCrcIntfImpl(val)
{}
//-----------------------------------------------------------------------------

uint32_t EseCrc32_IEEE_802_3::update(uint8_t val) ESE_NOTHROW
{
  m_crc = crc32ieee802_3_update(
    m_crc, 
    val
  );
  
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc32_IEEE_802_3::update(uint16_t val) ESE_NOTHROW
{
  m_crc = crc32ieee802_3(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc32_IEEE_802_3::update(uint32_t val) ESE_NOTHROW
{
  m_crc = crc32ieee802_3(
    m_crc, 
    reinterpret_cast<const uint8_t*>(&val),
    sizeof(val)
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

uint32_t EseCrc32_IEEE_802_3::update(const uint8_t* buff, uint32_t buffLen) ESE_NOTHROW
{
  m_crc = crc32ieee802_3(
    m_crc, 
    buff,
    buffLen
  );
    
  return m_crc;
}
//-----------------------------------------------------------------------------

#endif
