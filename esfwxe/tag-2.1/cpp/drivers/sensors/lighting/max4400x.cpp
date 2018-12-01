#include "stdafx.h"
#pragma hdrstop

#include "max4400x.h"

// MAX4400X register addresses
//
enum {
  max4400xRSTATUS     = 0x00,
  max4400xRCFG        = 0x01,
  max4400xRAMBCFG     = 0x02,
  max4400xRAMBCLEARH  = 0x04,
  max4400xRAMBCLEARL  = 0x05,
  max4400xRAMBREDH    = 0x06,
  max4400xRAMBREDL    = 0x07,
  max4400xRAMBGREENH  = 0x08,
  max4400xRAMBGREENL  = 0x09,
  max4400xRAMBBLUEH   = 0x0A,
  max4400xRAMBBLUEL   = 0x0B,
  max4400xRAMBIRH     = 0x0C,
  max4400xRAMBIRL     = 0x0D,
  max4400xRAMBIRCOMPH = 0x0E,
  max4400xRAMBIRCOMPL = 0x0F,
  max4400xRAMBTEMPH   = 0x12,
  max4400xRAMBTEMPL   = 0x13,
  max4400xRTHRESHUH   = 0x14,
  max4400xRTHRESHUL   = 0x15,
  max4400xRTHRESHLH   = 0x16,
  max4400xRTHRESHLL   = 0x17,
  max4400xRTHLATCH    = 0x18,
  max4400xRGAINCLEAR  = 0x1D,
  max4400xRGAINRED    = 0x1E,
  max4400xRGAINGREEN  = 0x1F,
  max4400xRGAINBLUE   = 0x20,
  max4400xRGAINIR     = 0x21
};

void max4400x::AllChannels::reset() ESE_NOTHROW
{
  c = 0;
  r = 0;
  g = 0;
  b = 0;
  ir = 0;
  irComp = 0;
}

// Internal API helpers
//
bool max4400x::byteSet(esU8 reg, esU8 b) ESE_NOTHROW
{
  return 1 == m_i2c.masterMemSend(m_addr, reg, 1, &b, 1, m_tmo);
}

bool max4400x::wordSet(esU8 reg, esU16 w) ESE_NOTHROW
{
  w = SWAPB_WORD(w);
  return 2 == m_i2c.masterMemSend(m_addr, reg, 1, reinterpret_cast<const esU8*>(&w), 2, m_tmo);
}

bool max4400x::byteGet(esU8 reg, esU8& b) ESE_NOTHROW
{
  return 1 == m_i2c.masterMemReceive(m_addr, reg, 1, &b, 1, m_tmo);
}

bool max4400x::wordGet(esU8 reg, esU16& w) ESE_NOTHROW
{
  if( 2 == m_i2c.masterMemReceive(m_addr, reg, 1, reinterpret_cast<esU8*>(&w), 2, m_tmo) )
  {
    w = SWAPB_WORD(w);
    return true;
  }
    
  return false;
}

// Public API implementation
max4400x::max4400x(EseI2c& i2c, esU8 addr, esU32 tmo) ESE_NOTHROW :
m_i2c(i2c),
m_tmo(tmo),
m_addr(addr)
{
}
  
bool max4400x::intRead(esU8& intReg) ESE_NOTHROW
{
  return byteGet(max4400xRSTATUS, intReg);
}

bool max4400x::intWrite(esU8 intReg) ESE_NOTHROW
{
  return byteSet(max4400xRSTATUS, intReg);
}

bool max4400x::cfgRead(esU8& cfgReg) ESE_NOTHROW
{
  return byteGet(max4400xRCFG, cfgReg);  
}

bool max4400x::cfgWrite(esU8 cfgReg) ESE_NOTHROW
{
  return byteSet(max4400xRCFG, cfgReg);  
}

bool max4400x::ambCfgRead(esU8& cfgReg) ESE_NOTHROW
{
  return byteGet(max4400xRAMBCFG, cfgReg);
}

bool max4400x::ambCfgWrite(esU8 cfgReg) ESE_NOTHROW
{
  return byteSet(max4400xRAMBCFG, cfgReg);
}

bool max4400x::thresholdUpperRead(esU16& tu) ESE_NOTHROW
{
  return wordGet(max4400xRTHRESHUH, tu);
}

bool max4400x::thresholdUpperWrite(esU16 tu) ESE_NOTHROW
{
  return wordSet(max4400xRTHRESHUH, tu);
}

bool max4400x::thresholdLowerRead(esU16& tl) ESE_NOTHROW
{
  return wordGet(max4400xRTHRESHLH, tl);
}

bool max4400x::thresholdLowerWrite(esU16 tl) ESE_NOTHROW
{
  return wordSet(max4400xRTHRESHLH, tl);
}

bool max4400x::thresholdLatchTimerRead(esU8& tt) ESE_NOTHROW
{
  return byteGet(max4400xRTHLATCH, tt);
}

bool max4400x::thresholdLatchTimerWrite(esU8 tt) ESE_NOTHROW
{
  return byteSet(max4400xRTHLATCH, tt);
}

bool max4400x::ambGainClearRead(esU8& gclear) ESE_NOTHROW
{
  return byteGet(max4400xRGAINCLEAR, gclear);
}

bool max4400x::ambGainClearWrite(esU8 gclear) ESE_NOTHROW
{
  return byteSet(max4400xRGAINCLEAR, gclear);
}

bool max4400x::ambGainRedRead(esU8& gred) ESE_NOTHROW
{
  return byteGet(max4400xRGAINRED, gred);
}

bool max4400x::ambGainRedWrite(esU8 gred) ESE_NOTHROW
{
  return byteSet(max4400xRGAINRED, gred);
}

bool max4400x::ambGainGreenRead(esU8& ggreen) ESE_NOTHROW
{
  return byteGet(max4400xRGAINGREEN, ggreen);
}

bool max4400x::ambGainGreenWrite(esU8 ggreen) ESE_NOTHROW
{
  return byteSet(max4400xRGAINGREEN, ggreen);
}

bool max4400x::ambGainBlueRead(esU8& gblue) ESE_NOTHROW
{
  return byteGet(max4400xRGAINBLUE, gblue);
}

bool max4400x::ambGainBlueWrite(esU8 gblue) ESE_NOTHROW
{
  return byteSet(max4400xRGAINBLUE, gblue);
}

bool max4400x::ambGainIrRead(esU8& gir) ESE_NOTHROW
{
  return byteGet(max4400xRGAINIR, gir);
}

bool max4400x::ambGainIrWrite(esU8 gir) ESE_NOTHROW
{
  return byteSet(max4400xRGAINIR, gir);
}

bool max4400x::clearRead(esU16& clear) ESE_NOTHROW
{
  return wordGet(max4400xRAMBCLEARH, clear);
}

bool max4400x::redRead(esU16& red) ESE_NOTHROW
{
  return wordGet(max4400xRAMBREDH, red);
}

bool max4400x::greenRead(esU16& green) ESE_NOTHROW
{
  return wordGet(max4400xRAMBGREENH, green);
}
  
bool max4400x::blueRead(esU16& blue) ESE_NOTHROW
{
  return wordGet(max4400xRAMBBLUEH, blue);
}

bool max4400x::irRead(esU16& ir) ESE_NOTHROW
{
  return wordGet(max4400xRAMBIRH, ir);
}

bool max4400x::irCompRead(esU16& irComp) ESE_NOTHROW
{
  return wordGet(max4400xRAMBIRCOMPH, irComp);
}
  
bool max4400x::tempRead(esU16& temp) ESE_NOTHROW
{
  return wordGet(max4400xRAMBTEMPH, temp);
}

bool max4400x::allRead(max4400x::AllChannels& data) ESE_NOTHROW
{
  if( sizeof(max4400x::AllChannels) == m_i2c.masterMemReceive(m_addr, max4400xRAMBCLEARH, 1, reinterpret_cast<esU8*>(&data), sizeof(max4400x::AllChannels), m_tmo) )
  {
    data.c = SWAPB_WORD(data.c);
    data.r = SWAPB_WORD(data.r);
    data.g = SWAPB_WORD(data.g);
    data.b = SWAPB_WORD(data.b);
    data.ir = SWAPB_WORD(data.ir);
    data.irComp = SWAPB_WORD(data.irComp);

    return true;
  }
    
  return false;  
}
