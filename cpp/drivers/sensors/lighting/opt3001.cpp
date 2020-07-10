#include <esfwxe/target.h>

#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/concept/EseChannel.h>
#include <esfwxe/cpp/drivers/common/EseI2cSlaveDevice.h>

#if defined(ESE_OPT3001_USE_MATHINTF)
# include <esfwxe/cpp/math/api/EseMathIntf.h>
#endif

#include "opt3001.h"
//----------------------------------------------------------------------

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
//----------------------------------------------------------------------
//----------------------------------------------------------------------

OPT3001::OPT3001(EseChannelIntf& i2c, OPT3001::i2cAddr addr, esU32 tmo) ESE_NOTHROW :
EseI2cSlaveDevice(
  i2c,
  addr,
  tmo
)
{
}
//----------------------------------------------------------------------

bool OPT3001::registerRead(esU8 reg, esU16& data) ESE_NOTHROW
{
  if( 
    !chnlMemIoPrepare(
      reg, 
      1
    ) 
  )
    return false;
    
  if(
    2 == m_i2c->receive(
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
//----------------------------------------------------------------------

bool OPT3001::registerWrite(esU8 reg, esU16 data) ESE_NOTHROW
{
  if( 
    !chnlMemIoPrepare(
      reg, 
      1
    )
  )
    return false;
    
  data = SWAPB_WORD(data);
  return 2 == m_i2c->send(
    reinterpret_cast<const esU8*>(&data), 
    2, 
    m_tmo
  );
}
//----------------------------------------------------------------------

esF OPT3001::rawValToVal(
  const OPT3001::Value& val
#if defined(ESE_OPT3001_USE_MATHINTF)
  , const EseMathIntf& imath
#endif
) ESE_NOTHROW
{
  esF result = 0.f;

#if defined(ESE_OPT3001_USE_MATHINTF)

  result = imath.div(
    imath.ui2f(
      static_cast<esU32>(val.m_data.m_val) * 
      static_cast<esU32>(1 << val.m_data.m_range)
    ),
    100.f
  );
  
  if( 
    imath.isNaN(result) || 
    0 > imath.cmp(
      result,
      0.f
    )
  )
    result = 0.f;

#else

  result = static_cast<esF>(
    static_cast<esU32>(val.m_data.m_val) * static_cast<esU32>(1 << val.m_data.m_range) 
  ) / 100.f;
  
  if( isnan(result) || result < 0 )
    result = 0;

#endif

  return result;
}
//----------------------------------------------------------------------

bool OPT3001::cfgRead(OPT3001::Config& cfg) ESE_NOTHROW
{
  return registerRead(
    opt3001_CONFIG,
    cfg.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::cfgWrite(const OPT3001::Config& cfg) ESE_NOTHROW
{
  return registerWrite(
    opt3001_CONFIG,
    cfg.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::resultRead(OPT3001::Value& result) ESE_NOTHROW
{
  return registerRead(
    opt3001_RESULT,
    result.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::limitHighRead(OPT3001::Value& tu) ESE_NOTHROW
{
  return registerRead(
    opt3001_HIGHLIMIT,
    tu.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::limitHighWrite(const OPT3001::Value& tu) ESE_NOTHROW
{
  return registerWrite(
    opt3001_HIGHLIMIT,
    tu.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::limitLowRead(OPT3001::Value& tl) ESE_NOTHROW
{
  return registerRead(
    opt3001_LOWLIMIT,
    tl.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::limitLowWrite(const OPT3001::Value& tl) ESE_NOTHROW
{
  return registerWrite(
    opt3001_LOWLIMIT,
    tl.m_raw
  );
}
//----------------------------------------------------------------------

bool OPT3001::idRead(OPT3001::ID& id) ESE_NOTHROW
{
  return registerRead(
    opt3001_MFGID,
    id.m_mfg
  ) && 
  registerRead(
    opt3001_DEVID,
    id.m_dev
  );
}
//----------------------------------------------------------------------
