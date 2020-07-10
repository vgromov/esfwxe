#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/ese_assert.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

//----------------------------------------------------------------------
// TODO: conditionally include proper DEFS and HAL for other MCUs
#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>

#include "EseMcuFlash.h"
//-----------------------------------------------------------------------------

EseMcuFlash::EseMcuFlash() ESE_NOTHROW :
m_addr(0),
m_addrEnd(0),
m_addrCur(0)
{
}
//-----------------------------------------------------------------------------

uint32_t EseMcuFlash::dataAddressStartGet() const ESE_NOTHROW 
{ 
  return m_addr; 
}
//-----------------------------------------------------------------------------

uint32_t EseMcuFlash::dataAddressCurrentGet() const ESE_NOTHROW 
{ 
  return m_addrCur; 
}
//-----------------------------------------------------------------------------

void EseMcuFlash::dataWriteAreaSet(uint32_t addr, uint32_t size) ESE_NOTHROW
{
  ES_ASSERT(0 == (addr % 2));
  ES_ASSERT(0 == (size % 2));
  ES_ASSERT(size);

  m_addr = addr;
  m_addrEnd = addr+size;
  m_addrCur = m_addr;
}
//-----------------------------------------------------------------------------

void EseMcuFlash::dataAddressReset() ESE_NOTHROW
{
  m_addrCur = m_addr;
}
//-----------------------------------------------------------------------------

void EseMcuFlash::dataAddressSet(uint32_t addr) ESE_NOTHROW
{
  ESE_ASSERT( m_addr <= addr );
  ESE_ASSERT( m_addrEnd > addr );
  ESE_ASSERT( 0 == (addr % 2) );
  
  m_addrCur = addr;
}
//-----------------------------------------------------------------------------

uint32_t EseMcuFlash::pageFromAddr(uint32_t addr) ESE_NOTHROW
{
  return (addr / FLASH_PAGE_SIZE);
}
//-----------------------------------------------------------------------------

uint32_t EseMcuFlash::pagesCountFromSize(uint32_t size) ESE_NOTHROW
{
  return ( size % FLASH_PAGE_SIZE ) ? 
    (size / FLASH_PAGE_SIZE) + 1 :
    (size / FLASH_PAGE_SIZE);
}
//-----------------------------------------------------------------------------

bool EseMcuFlash::lock() ESE_NOTHROW
{
  return HAL_OK == HAL_FLASH_Lock();
}
//-----------------------------------------------------------------------------

bool EseMcuFlash::unlock() ESE_NOTHROW
{
  return HAL_OK == HAL_FLASH_Unlock();
}
//-----------------------------------------------------------------------------

bool EseMcuFlash::pagesErase() ESE_NOTHROW
{
  if( !unlock() )
    return false;

  dataAddressReset();

  FLASH_EraseInitTypeDef flashCalErase;
  flashCalErase.TypeErase = FLASH_TYPEERASE_PAGES;
  flashCalErase.PageAddress = pageFromAddr(m_addr) * FLASH_PAGE_SIZE;
  flashCalErase.NbPages = pagesCountFromSize(m_addrEnd-flashCalErase.PageAddress);
  
  ES_DEBUG_TRACE2("EseMcuFlash::pagesErase 0x%0.8X (%d)\n", flashCalErase.PageAddress, flashCalErase.NbPages)

  uint32_t pageError = 0xFFFFFFFFU;
  HAL_StatusTypeDef status = HAL_FLASHEx_Erase(
    &flashCalErase,
    &pageError
  );
  
  lock();
  
  return HAL_OK == status && 0xFFFFFFFFU == pageError;
}
//-----------------------------------------------------------------------------

uint32_t EseMcuFlash::dataWrite(uint32_t len, const esU8* data) ESE_NOTHROW
{
  if( 0 == len || NULL == data || !unlock() )
    return 0;

  ESE_ASSERT( 0 == (len % sizeof(uint16_t)) );

  const uint16_t* pos = reinterpret_cast<const uint16_t*>(data);
  const uint16_t* end = pos + (len / sizeof(uint16_t));
  
  HAL_StatusTypeDef status = HAL_OK;
  while( pos < end && m_addrCur < m_addrEnd )
  {
    status = HAL_FLASH_Program(
      FLASH_TYPEPROGRAM_HALFWORD,
      m_addrCur,
      *pos
    );
    
    if( status != HAL_OK )
      break;
    
    m_addrCur += sizeof(uint16_t);
    ++pos;
  }
  
  lock();
  
  return (pos-reinterpret_cast<const uint16_t*>(data))*sizeof(uint16_t);
}
//-----------------------------------------------------------------------------
