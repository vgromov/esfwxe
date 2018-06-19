#include "stdafx.h"
#pragma hdrstop

#include <math.h>
#include "opt3001.h"

/// OPT3001 registers
///
enum {
  /// RW registers
  ///
  opt3001_CONFIG       = 1,
  opt3001_LOWLIMIT     = 2,
  opt3001_HIGHLIMIT    = 3,
  
  /// RO registers
  ///
  opt3001_RESULT       = 0,
  opt3001_MFGID        = 0x7E,
  opt3001_DEVID        = 0x7F
};

opt3001::opt3001(EseI2c& i2c, opt3001::i2cAddr addr, esU32 tmo) ESE_NOTHROW :
m_i2c(i2c),
m_tmo(tmo),
m_addr(addr)
{
}

bool opt3001::regRead(esU8 reg, esU16& data) ESE_NOTHROW
{
	if( 
    2 == m_i2c.masterMemReceive(
      m_addr, 
      reg, 
      1, 
      reinterpret_cast<esU8*>(&data), 
      2, 
      m_tmo
    ) 
  )
  {
    data = SWAPB_WORD(data);
    return true;
  }
    
  return false;
}
  
bool opt3001::regWrite(esU8 reg, esU16 data) ESE_NOTHROW
{
  data = SWAPB_WORD(data);
  
  return 2 == m_i2c.masterMemSend(
    m_addr, 
    reg, 
    1, 
    reinterpret_cast<const esU8*>(&data), 
    2, 
    m_tmo
  );
}

esF opt3001::rawValToVal(const opt3001::Value& val) ESE_NOTHROW
{
  esF result = static_cast<esF>(
    static_cast<esU32>(val.m_data.m_val) * static_cast<esU32>(1 << val.m_data.m_range) 
  ) / 100.f;
  
  if( isnan(result) || result < 0 )
    result = 0;
    
  return result;
}
  
bool opt3001::cfgRead(opt3001::Config& cfg) ESE_NOTHROW
{
  return regRead(
    opt3001_CONFIG,
    cfg.m_raw
  );
}

bool opt3001::cfgWrite(const opt3001::Config& cfg) ESE_NOTHROW
{
  return regWrite(
    opt3001_CONFIG,
    cfg.m_raw
  );
}

bool opt3001::resultRead(opt3001::Value& result) ESE_NOTHROW
{
  return regRead(
    opt3001_RESULT,
    result.m_raw
  );
}

bool opt3001::limitHighRead(opt3001::Value& tu) ESE_NOTHROW
{
  return regRead(
    opt3001_HIGHLIMIT,
    tu.m_raw
  );
}

bool opt3001::limitHighWrite(const opt3001::Value& tu) ESE_NOTHROW
{
  return regWrite(
    opt3001_HIGHLIMIT,
    tu.m_raw
  );
}

bool opt3001::limitLowRead(opt3001::Value& tl) ESE_NOTHROW
{
  return regRead(
    opt3001_LOWLIMIT,
    tl.m_raw
  );
}

bool opt3001::limitLowWrite(const opt3001::Value& tl) ESE_NOTHROW
{
  return regWrite(
    opt3001_LOWLIMIT,
    tl.m_raw
  );
}

bool opt3001::idRead(opt3001::ID& id) ESE_NOTHROW
{
  return regRead(
    opt3001_MFGID,
    id.m_mfg
  ) && 
  regRead(
    opt3001_DEVID,
    id.m_dev
  );
}
