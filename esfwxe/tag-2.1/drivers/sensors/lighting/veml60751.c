#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include "veml60751.h"

enum {
  VEML60751_ADDR = 0x20,
  VEML60751_CFG  = 0x00,
  VEML60751_UVA  = 0x07,
  VEML60751_DARK = 0x08,
  VEML60751_UVB  = 0x09,
  VEML60751_VIS  = 0x0A,
  VEML60751_IR   = 0x0B,
  VEML60751_ID   = 0x0C
};

static __inline esBL veml60751read(i2cHANDLE hi2c, esU8 addr, esU8* data)
{
  ES_ASSERT(hi2c);
  ES_ASSERT(data);
  
  return ( 1 == i2cPutBytes(hi2c, VEML60751_ADDR, &addr, 1, false) ) &&
    (2 == i2cGetBytes(hi2c, VEML60751_ADDR, data, 2) );
}

static __inline esBL veml60751write(i2cHANDLE hi2c, esU8 addr, const esU8* data)
{
  ES_ASSERT(hi2c);
  ES_ASSERT(data);
  
  esU8 buff[3];
  buff[0] = addr;
  buff[1] = *data++;
  buff[2] = *data;
  
  return 3 == i2cPutBytes(hi2c, VEML60751_ADDR, buff, 3, true);
}

esBL veml60751idRead(i2cHANDLE hi2c, veml60751id* id)
{
  return veml60751read(hi2c, VEML60751_ID, (esU8*)id);
}

esBL veml60751cfgWrite(i2cHANDLE hi2c, const veml60751cfg* cfg)
{
  return veml60751write(hi2c, VEML60751_CFG, (const esU8*)cfg);
}

esBL veml60751cfgRead(i2cHANDLE hi2c, veml60751cfg* cfg)
{
  return veml60751read(hi2c, VEML60751_CFG, (esU8*)cfg);
}

esBL veml60751uvaRead(i2cHANDLE hi2c, esU16* uva)
{
  return veml60751read(hi2c, VEML60751_UVA, (esU8*)uva);
}

esBL veml60751darkRead(i2cHANDLE hi2c, esU16* dark)
{
  return veml60751read(hi2c, VEML60751_DARK, (esU8*)dark);
}

esBL veml60751uvbRead(i2cHANDLE hi2c, esU16* uvb)
{
  return veml60751read(hi2c, VEML60751_UVB, (esU8*)uvb);
}

esBL veml60751visCompRead(i2cHANDLE hi2c, esU16* visComp)
{
  return veml60751read(hi2c, VEML60751_VIS, (esU8*)visComp);
}

esBL veml60751irCompRead(i2cHANDLE hi2c, esU16* irComp)
{
  return veml60751read(hi2c, VEML60751_IR, (esU8*)irComp);
}

esBL veml60751resultRead(i2cHANDLE hi2c, veml60751result* result)
{
  ES_ASSERT(result);
  
  return veml60751read(hi2c, VEML60751_UVA, (esU8*)&result->m_uva) &&
    veml60751read(hi2c, VEML60751_DARK, (esU8*)&result->m_dark) &&
    veml60751read(hi2c, VEML60751_UVB, (esU8*)&result->m_uvb) &&
    veml60751read(hi2c, VEML60751_VIS, (esU8*)&result->m_visComp) &&
    veml60751read(hi2c, VEML60751_IR, (esU8*)&result->m_irComp);
}

esU32 veml60751uvCalculate(const veml60751result* data, const veml60751cfg* cfg, 
  const veml60751calibration* cal)
{
  ES_ASSERT(data);
  ES_ASSERT(cfg);
  ES_ASSERT(cal);
  ES_ASSERT(cal->m_respDiv > 0);
  
  // TODO: perform sensitivity correcttion, accounting for cfg exposure (&& hdr ?) values
  
  esI32 delta1 = 0;
  if( data->m_visComp > data->m_dark )
    delta1 = data->m_visComp - data->m_dark;
  
  esI32 delta2 = 0;
  if( data->m_irComp > data->m_dark )
    delta1 = data->m_irComp - data->m_dark;
  
  esI32 delta3 = 0;
  if( data->m_uva > data->m_dark )
    delta3 = data->m_uva - data->m_dark;

  esI32 delta4 = 0;
  if( data->m_uvb > data->m_dark )
    delta4 = data->m_uvb - data->m_dark;
  
  esI32 uvi = ((100*delta4 - cal->m_c*delta1 - cal->m_d*delta2)*cal->m_respB + 
               (100*delta3 - cal->m_a*delta1 - cal->m_b*delta2)*cal->m_respA
              ) / (2 * cal->m_respDiv);
              
  if( uvi < 0 )
    uvi = 0;
  
  return uvi;
}
