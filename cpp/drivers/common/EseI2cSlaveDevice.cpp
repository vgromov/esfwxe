#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/drivers/common/EseI2cChannelIoCtlDefs.h>
//----------------------------------------------------------------------

#include "EseI2cSlaveDevice.h"
//----------------------------------------------------------------------

EseI2cSlaveDevice::EseI2cSlaveDevice() ESE_NOTHROW :
m_i2c(nullptr),
m_tmo(0),
m_addr(0)
{}
//----------------------------------------------------------------------

EseI2cSlaveDevice::EseI2cSlaveDevice(EseChannelIntf& i2c, uint16_t addr, uint32_t tmo) ESE_NOTHROW :
m_i2c(nullptr),
m_tmo(0),
m_addr(0)
{
  init(
   i2c, 
   addr, 
   tmo
  );
}
//----------------------------------------------------------------------

bool EseI2cSlaveDevice::isInitialized() const ESE_NOTHROW
{
  return nullptr != m_i2c;
}
//----------------------------------------------------------------------

void EseI2cSlaveDevice::init(EseChannelIntf& i2c, uint16_t addr, uint32_t tmo) ESE_NOTHROW
{
  m_i2c = &i2c;
  m_tmo = tmo;
  m_addr = addr;
}
//----------------------------------------------------------------------

bool EseI2cSlaveDevice::chnlMemIoPrepare(uint16_t memaddr, uint16_t memaddrSize) ESE_NOTHROW
{
  if( !m_i2c )
    return false;

  int stat = m_i2c->ioCtlSimpleSet<uint16_t>(
    EseI2cChannelIoCtl::ctlDevAddr,
    m_addr
  );
  if(rtosOK != stat)
    return false;
  
  stat = m_i2c->ioCtlSimpleSet<uint16_t>(
    EseI2cChannelIoCtl::ctlMemAddr,
    memaddr
  );
  if(rtosOK != stat)
    return false;

  stat = m_i2c->ioCtlSimpleSet<uint16_t>(
    EseI2cChannelIoCtl::ctlMemAddrSize,
    memaddrSize
  );
  return rtosOK == stat;
}
//----------------------------------------------------------------------

bool EseI2cSlaveDevice::chnlSimpleIoPrepare() ESE_NOTHROW
{
  if( !m_i2c )
    return false;

  int stat = m_i2c->ioCtlSimpleSet<esU16>(
    EseI2cChannelIoCtl::ctlDevAddr,
    m_addr
  );
  if(rtosOK != stat)
    return false;

  stat = m_i2c->ioCtlSimpleSet<esU16>(
    EseI2cChannelIoCtl::ctlMemAddrSize,
    0
  );
  return rtosOK == stat;
}
//----------------------------------------------------------------------

EseChannelIntf& EseI2cSlaveDevice::busGet() ESE_NOTHROW
{ 
  return *m_i2c; 
}
//----------------------------------------------------------------------

uint16_t EseI2cSlaveDevice::addrGet() const ESE_NOTHROW 
{ 
  return m_addr; 
}
//----------------------------------------------------------------------

uint32_t EseI2cSlaveDevice::timeoutGet() const ESE_NOTHROW 
{ 
  return m_tmo; 
}
//----------------------------------------------------------------------

