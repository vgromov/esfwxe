#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/drivers/common/EseI2cSlaveDevice.h>
//----------------------------------------------------------------------

#if defined(ESE_HTS221_USE_MATHINTF)
# include <esfwxe/cpp/math/api/EseMathIntf.h>
#endif

#include "hts221.h"
//----------------------------------------------------------------------

float HTS221::calibration::temperatureCalculate(
  int raw
#if defined(ESE_HTS221_USE_MATHINTF)
  , const EseMathIntf& imath
#endif
) const ESE_NOTHROW
{
  int traw = (raw > 32767) ? 
    (raw - 65536) : 
    raw;
  
  int tdiv = m_T1_out - m_T0_out;
  
#if defined(ESE_HTS221_USE_MATHINTF)

  return imath.div(
    imath.i2f(
      (m_T1_degC_x8 - m_T0_degC_x8) * (traw - m_T0_out) + tdiv * m_T0_degC_x8 
    ),
    imath.i2f(tdiv * 8)
  );
  
#else

  return (
    static_cast<float>(
      (m_T1_degC_x8 - m_T0_degC_x8) * (traw - m_T0_out) + tdiv * m_T0_degC_x8 
    ) / 
    static_cast<float>(tdiv * 8)
  );
  
#endif
}
//----------------------------------------------------------------------

float HTS221::calibration::humidityCalculate(
  int raw
#if defined(ESE_HTS221_USE_MATHINTF)
  , const EseMathIntf& imath
#endif
) const ESE_NOTHROW
{
  int hdiv = m_H1_T0_out - m_H0_T0_out;
  
#if defined(ESE_HTS221_USE_MATHINTF)

  return imath.div(
    imath.i2f(((m_H1_rH_x2 - m_H0_rH_x2) * (raw - m_H0_T0_out)) + hdiv * m_H0_rH_x2), 
    imath.i2f(hdiv * 2)
  );
  
#else

  return static_cast<float>(((m_H1_rH_x2 - m_H0_rH_x2) * (raw - m_H0_T0_out)) + hdiv * m_H0_rH_x2) / 
    static_cast<float>(hdiv * 2);
    
#endif
}
//----------------------------------------------------------------------
//----------------------------------------------------------------------

/// HTS221 registers
///
enum {
  /// RW registers
  ///
  hst221_AVG_CONFIG     = 0x10,
  hst221_CTL1           = 0x20,
  hst221_CTL2           = 0x21,
  hst221_CTL3           = 0x22,
  
  /// RO registers
  ///
  hst221_WHO_AM_I       = 0x0F,
  hst221_STATUS         = 0x27,
  hst221_H              = 0x28,
  hst221_T              = 0x2A,
  hst221_CAL_H0_rH_x2   = 0x30,
  hst221_CAL_H1_rH_x2   = 0x31,
  hst221_CAL_T0_degC_x8 = 0x32,
  hst221_CAL_T1_degC_x8 = 0x33,
  hst221_CAL_T1_T0_msb  = 0x35,
  hst221_CAL_H0_T0_OUT  = 0x36,
  hst221_CAL_H1_T0_OUT  = 0x3A,
  hst221_CAL_T0_OUT     = 0x3C,
  hst221_CAL_T1_OUT     = 0x3E
};
//----------------------------------------------------------------------

HTS221::HTS221(EseChannelIntf& i2c, HTS221::i2cAddr addr, esU32 tmo) ESE_NOTHROW :
EseI2cSlaveDevice(
  i2c,
  addr,
  tmo
)
{}
//----------------------------------------------------------------------

bool HTS221::registerRead(uint8_t reg, uint8_t len, uint8_t* out) ESE_NOTHROW
{
  if( 
    !len || 
    !out || 
    !chnlMemIoPrepare(
      (len > 1) ? 
        (reg | 0x80) : 
        reg, 
      1
    ) 
  )
    return false;
    
  return len == m_i2c->receive(
    out, 
    len, 
    m_tmo
  ); 
}
//----------------------------------------------------------------------

bool HTS221::registerWrite(uint8_t reg, uint8_t len, const uint8_t* in) ESE_NOTHROW
{
  if( 
    !len || 
    !in ||
    !chnlMemIoPrepare(
      (len > 1) ? 
        (reg | 0x80) : 
        reg, 
      1
    )
  )
    return false;
    
  return len == m_i2c->send(
    in, 
    len, 
    m_tmo
  ); 
}
//----------------------------------------------------------------------

bool HTS221::idRead(HTS221::ID& id) ESE_NOTHROW
{
  uint8_t tmp = 0;
  
  if(
    registerRead(
      hst221_WHO_AM_I,
      1,
      &tmp
    )
  )
  {
    id = static_cast<HTS221::ID>(tmp);
    return true;
  }
    
  return false;
}
//----------------------------------------------------------------------
  
bool HTS221::avgCtlRead(HTS221::avgCfgT& cfgT, HTS221::avgCfgH& cfgH) ESE_NOTHROW
{
  uint8_t tmp = 0;
  
  if( 
    registerRead(
      hst221_AVG_CONFIG,
      1,
      &tmp
    )
  )
  {
    cfgH = static_cast<avgCfgH>(tmp & 0x03);
    cfgT = static_cast<avgCfgT>((tmp >> 3) & 0x03);
    
    return true;
  }
    
  return false;
}
//----------------------------------------------------------------------

bool HTS221::avgCtlWrite(HTS221::avgCfgT cfgT, HTS221::avgCfgH cfgH) ESE_NOTHROW
{
  uint8_t tmp = (static_cast<uint8_t>(cfgT) << 3) | static_cast<uint8_t>(cfgH);
  
  return registerWrite(
    hst221_AVG_CONFIG,
    1,
    &tmp
  );
}
//----------------------------------------------------------------------
  
bool HTS221::ctlRead(HTS221::ctl1& ctl) ESE_NOTHROW
{
  return registerRead(
    hst221_CTL1,
    1,
    reinterpret_cast<uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------
  
bool HTS221::ctlWrite(const HTS221::ctl1& ctl) ESE_NOTHROW
{
  return registerWrite(
    hst221_CTL1,
    1,
    reinterpret_cast<const uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------

bool HTS221::ctlRead(HTS221::ctl2& ctl) ESE_NOTHROW
{
  return registerRead(
    hst221_CTL2,
    1,
    reinterpret_cast<uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------
  
bool HTS221::ctlWrite(const HTS221::ctl2& ctl) ESE_NOTHROW
{
  return registerWrite(
    hst221_CTL2,
    1,
    reinterpret_cast<const uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------

bool HTS221::ctlRead(HTS221::ctl3& ctl) ESE_NOTHROW
{
  return registerRead(
    hst221_CTL3,
    1,
    reinterpret_cast<uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------
  
bool HTS221::ctlWrite(const HTS221::ctl3& ctl) ESE_NOTHROW
{
  return registerWrite(
    hst221_CTL3,
    1,
    reinterpret_cast<const uint8_t*>(&ctl)
  );
}
//----------------------------------------------------------------------
  
bool HTS221::statusRead(HTS221::status& stat) ESE_NOTHROW
{
  return registerRead(
    hst221_STATUS,
    1,
    reinterpret_cast<uint8_t*>(&stat)
  );  
}
//----------------------------------------------------------------------
  
bool HTS221::temperatureReadRaw(int& raw) ESE_NOTHROW
{
  int16_t tmp;
  
  if( !
    registerRead(
      hst221_T,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
    
  raw = tmp;
    
  return true;
}
//----------------------------------------------------------------------

bool HTS221::humidityReadRaw(int& raw) ESE_NOTHROW
{
  int16_t tmp;

  if( 
    !registerRead(
      hst221_H,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
    
  raw = tmp;
    
  return true;
}
//----------------------------------------------------------------------
  
bool HTS221::calibrationRead(HTS221::calibration& cal) ESE_NOTHROW
{
  union {
    uint8_t  bb[2];
    int16_t  s;
    
  } tmp;

  if( 
    !registerRead(
      hst221_CAL_H0_rH_x2,
      1,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
  
  cal.m_H0_rH_x2 = tmp.bb[0];
  
  if( 
    !registerRead(
      hst221_CAL_H1_rH_x2,
      1,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;

  cal.m_H1_rH_x2 = tmp.bb[0];
  
  if( 
    !registerRead(
      hst221_CAL_T0_degC_x8,
      1,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;

  cal.m_T0_degC_x8 = tmp.bb[0];

  if( 
    !registerRead(
      hst221_CAL_T1_degC_x8,
      1,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;

  cal.m_T1_degC_x8 = tmp.bb[0];
  
  if( 
    !registerRead(
      hst221_CAL_T1_T0_msb,
      1,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
    
  cal.m_T0_degC_x8 += (tmp.bb[0] & 0x03) << 8;
  cal.m_T1_degC_x8 += (tmp.bb[0] & 0x0C) << 6;

  if( 
    !registerRead(
      hst221_CAL_H0_T0_OUT,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
    
  cal.m_H0_T0_out = tmp.s;

  if( 
    !registerRead(
      hst221_CAL_H1_T0_OUT,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
    
  cal.m_H1_T0_out = tmp.s;
  
  if( 
    !registerRead(
      hst221_CAL_T0_OUT,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;
  
  cal.m_T0_out = tmp.s;
  
  if( 
    !registerRead(
      hst221_CAL_T1_OUT,
      2,
      reinterpret_cast<uint8_t*>(&tmp)
    )
  )
    return false;

  cal.m_T1_out = tmp.s;

  return true;
}
//----------------------------------------------------------------------
