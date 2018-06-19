#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
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

// Internal API helpers
//
static __inline esBL max4400xByteSet(i2cHANDLE handle, esU8 slave, esU8 reg, esU8 b)
{
	esU8 tmp[2];
	tmp[0] = reg;
	tmp[1] = b;
	
	return i2cPutBytes(handle, slave, tmp, 2) == 2;
}

static __inline esBL max4400xWordSet(i2cHANDLE handle, esU8 slave, esU8 reg, esU16 w)
{
	esU8 tmp[3];
	tmp[0] = reg;
	tmp[1] = HIBYTE(w);
  tmp[2] = LOBYTE(w);

	return i2cPutBytes(handle, slave, tmp, 3) == 3;
}

static __inline esBL max4400xByteGet(i2cHANDLE handle, esU8 slave, esU8 reg, esU8* b)
{
	return i2cPutBytes(handle, slave, &reg, 1) == 1 &&
		i2cGetBytes(handle, slave, b, 1) == 1;
}

static __inline esBL max4400xWordGet(i2cHANDLE handle, esU8 slave, esU8 reg, esU16* w)
{
	if( i2cPutBytes(handle, slave, &reg, 1) == 1 &&
		i2cGetBytes(handle, slave, (esU8*)w, 2) == 2 )
  {
    *w = SWAPB_WORD(*w);
    return TRUE;
  }
    
  return FALSE;
}

// API implementation
//
// Generic configuration
//
esBL max4400xIntRead(i2cHANDLE handle, esU8 slaveAddr, esU8* intReg)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRSTATUS, intReg);
}

esBL max4400xIntWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 intReg)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRSTATUS, intReg);
}

esBL max4400xCfgRead(i2cHANDLE handle, esU8 slaveAddr, esU8* cfgReg)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRCFG, cfgReg);
}
  
esBL max4400xCfgWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 cfgReg)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRCFG, cfgReg);
}
  
esBL max4400xAmbCfgRead(i2cHANDLE handle, esU8 slaveAddr, esU8* cfgReg)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRAMBCFG, cfgReg);
}

esBL max4400xAmbCfgWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 cfgReg)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRAMBCFG, cfgReg);
}
  
// Thresholds configuration
//
esBL max4400xThresholdUpperRead(i2cHANDLE handle, esU8 slaveAddr, esU16* tu)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRTHRESHUH, tu);
}

esBL max4400xThresholdUpperWrite(i2cHANDLE handle, esU8 slaveAddr, esU16 tu)
{
  return max4400xWordSet(handle, slaveAddr, max4400xRTHRESHUH, tu);
}
  
esBL max4400xThresholdLowerRead(i2cHANDLE handle, esU8 slaveAddr, esU16* tl)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRTHRESHLH, tl);
}
  
esBL max4400xThresholdLowerWrite(i2cHANDLE handle, esU8 slaveAddr, esU16 tl)
{
  return max4400xWordSet(handle, slaveAddr, max4400xRTHRESHLH, tl);
}

esBL max4400xThresholdLatchTimerRead(i2cHANDLE handle, esU8 slaveAddr, esU8* tt)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRTHLATCH, tt);
}
  
esBL max4400xThresholdLatchTimerWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 tt)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRTHLATCH, tt);
}
  
// Ambient gains configuration
//
esBL max4400xAmbGainClearRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gclear)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRGAINCLEAR, gclear);
}

esBL max4400xAmbGainClearWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gclear)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRGAINCLEAR, gclear);
}

esBL max4400xAmbGainRedRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gred)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRGAINRED, gred);
}

esBL max4400xAmbGainRedWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gred)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRGAINRED, gred);
}
  
esBL max4400xAmbGainGreenRead(i2cHANDLE handle, esU8 slaveAddr, esU8* ggreen)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRGAINGREEN, ggreen);
}
  
esBL max4400xAmbGainGreenWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 ggreen)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRGAINGREEN, ggreen);
}
  
esBL max4400xAmbGainBlueRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gblue)
{  
  return max4400xByteGet(handle, slaveAddr, max4400xRGAINBLUE, gblue);
}

esBL max4400xAmbGainBlueWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gblue)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRGAINBLUE, gblue);
}
  
esBL max4400xAmbGainIrRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gir)
{
  return max4400xByteGet(handle, slaveAddr, max4400xRGAINIR, gir);
}
  
esBL max4400xAmbGainIrWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gir)
{
  return max4400xByteSet(handle, slaveAddr, max4400xRGAINIR, gir);
}
  
// Data channels reading
// 
esBL max4400xClearRead(i2cHANDLE handle, esU8 slaveAddr, esU16* clear)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBCLEARH, clear);
}

esBL max4400xRedRead(i2cHANDLE handle, esU8 slaveAddr, esU16* red)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBREDH, red);
}

esBL max4400xGreenRead(i2cHANDLE handle, esU8 slaveAddr, esU16* green)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBGREENH, green);
}
  
esBL max4400xBlueRead(i2cHANDLE handle, esU8 slaveAddr, esU16* blue)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBBLUEH, blue);
}

esBL max4400xIrRead(i2cHANDLE handle, esU8 slaveAddr, esU16* ir)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBIRH, ir);
}
  
esBL max4400xIrCompRead(i2cHANDLE handle, esU8 slaveAddr, esU16* irComp)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBIRCOMPH, irComp);
}
  
esBL max4400xTempRead(i2cHANDLE handle, esU8 slaveAddr, esU16* temp)
{
  return max4400xWordGet(handle, slaveAddr, max4400xRAMBTEMPH, temp);
}
