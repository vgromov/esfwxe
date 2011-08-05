#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <common/utils.h>
#include "vsp25xx.h"

static __inline BOOL vsp25xxIsValidType(UINT8 type)
{
	return type > vspInvalid &&
			type < vspTypeCnt;
}

// bus api, implementation of vsp25xxBusPutBytes && vsp25xxBusConfig is platform-dependent
void vsp25xxBusInit(vsp25xxBusHANDLE h, vspType vsp25xxType, vsp25xxBusConfigPfn configLines, 
	vsp25xxBusLineSetPfn rloadSet, vsp25xxBusLineSetPfn sloadSet, vsp25xxBusLineSetPfn sclkSet, vsp25xxBusLineSetPfn dataSet,
	vsp25xxBusLineSetPfn ccdRefLevelClkSet,	vsp25xxBusLineSetPfn ccdDataLevelClkSet, vsp25xxBusLineSetPfn ccdADCclkSet,
	vsp25xxBusLineSetPfn ccdOpticalBlackClkSet,	vsp25xxBusLineSetPfn ccdDummyClampClkSet,	vsp25xxBusLineSetPfn ccdBlankClkSet,
	vsp25xxBusCCDdataReadPfn ccdDataRead)
{
	if( h )
	{
		memset(h, 0, sizeof(vsp25xxBus));
		if(	configLines &&
			rloadSet && 
			sloadSet &&
			sclkSet &&
			dataSet &&
			vsp25xxIsValidType(vsp25xxType) )
		{
			h->configLines = configLines;
			h->rloadSet = rloadSet;
			h->sloadSet = sloadSet;
			h->sclkSet = sclkSet;
			h->dataSet = dataSet;
			h->ccdRefLevelClkSet = ccdRefLevelClkSet;
			h->ccdDataLevelClkSet = ccdDataLevelClkSet;
			h->ccdADCclkSet = ccdADCclkSet;
			h->ccdOpticalBlackClkSet = ccdOpticalBlackClkSet;
			h->ccdDummyClampClkSet = ccdDummyClampClkSet;
			h->ccdBlankClkSet = ccdBlankClkSet;
			h->ccdDataRead = ccdDataRead;
			h->dpgaGain = vsp25xxDPGAgainDefault;
			h->type = vsp25xxType;
			h->clockPolarityCtl = vsp25xxCPCtlDefault;
			h->standbySettings = vsp25xxStbyDefault;
			h->dataOutputSettings = vsp25xxDataOutputDefault;
			h->shdSamplingDelay = vsp25xxShdSamplingDelayDefault;
			h->clampLevel = vsp25xxClampLevelDefault;
			h->hotRejection = vsp25xxHotRejectionLevelDefault;
			h->cdsGainControl = vsp25xxCDSgainDefault;
			h->powerControl = vsp25xxPowerCtlDefault;
			// configure the bus for the first time
			vsp25xxBusConfigure(h);
		}
	}
}

void vsp25xxBusConfigure(vsp25xxBusHANDLE h)
{
	if( h &&
		vsp25xxIsValidType(h->type) &&
		h->configLines &&
		h->rloadSet &&
		h->sloadSet &&
		h->sclkSet &&
		h->dataSet )
	{
		// configure lines
		h->configLines(h);
		// set all serial programming lines high
		h->sclkSet(TRUE);
		h->rloadSet(TRUE);
		h->sloadSet(TRUE);
		h->dataSet(TRUE);
		// reset ccd interface lines
		vsp25xxCcdRefLevelClkSet(h, TRUE);
		vsp25xxCcdDataLevelClkSet(h, TRUE);
		vsp25xxCcdADCclkSet(h, TRUE);
		vsp25xxCcdOpticalBlackClkSet(h, TRUE);
		vsp25xxCcdDummyClampClkSet(h, TRUE);
		vsp25xxCcdBlankClkSet(h, FALSE);
	}
}

vspType vsp25xxTypeGet(vsp25xxBusHANDLE h)
{
	if( h &&
			h->type < vspTypeCnt )
		return (vspType)h->type;

	return vspInvalid;
}

// bus data send helper
static void vsp25xxBusPutBits(vsp25xxBusHANDLE h, UINT16 data, UINT8 bits)
{
	if(	h->rloadSet &&
		h->sclkSet &&
		h->dataSet )
	{
		UINT8 cnt;
		for(cnt = 0; cnt < bits; ++cnt)
		{
			h->dataSet(ES_BIT_IS_SET(data, 1));
			h->sclkSet(FALSE);
			// hold down for >= 25ns
			nsDelay(25);
			h->sclkSet(TRUE);
			data >>= 1;
		}
	}
}

// cnt is count of data bits
static BOOL vsp25xxBusPutBytes(vsp25xxBusHANDLE h, UINT16 addr, UINT16 data, UINT8 dataBits)
{
	if( h &&
		vsp25xxIsValidType(h->type) &&
		h->sloadSet )
	{
		UINT8 rloadMode = (h->clockPolarityCtl & vsp25xxCPCtlRLOADmask);
		h->sloadSet(FALSE); // start data load 
		vsp25xxBusPutBits(h, addr, 10); // send address
		vsp25xxBusPutBits(h, data, dataBits); // send data bits
		// finalize data packet sending
		if( vsp25xxCPCtlUpdateByRisingRLOAD == rloadMode )
			h->rloadSet(FALSE);
		// we should have scl set high here.
		h->sloadSet(TRUE); // end data load
		// hold for >= 25ns
		nsDelay(25);
		if( vsp25xxCPCtlUpdateByRisingRLOAD == rloadMode )
			h->rloadSet(TRUE);
		else if( vsp25xxCPCtlUpdateByFallingRLOAD == rloadMode )
		{
			h->rloadSet(FALSE);
			// hold for >= 20ns
			nsDelay(25);
			h->rloadSet(TRUE);
		}
		return TRUE;
	}
	
	return FALSE;
}

// TI AFE VSP2566 programming interface.
// all hardware functions are write-only, no error conditions are returned,
// read capability is supported through software only
//
// active polarity selection control
//
void vsp25xxClockPolarityCtlSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 0, val, 6) )
		h->clockPolarityCtl = val;
}

void vsp25xxClockPolarityCtlSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxClockPolarityCtlSet(h, vsp25xxCPCtlDefault);
}

UINT8 vsp25xxClockPolarityCtlGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->clockPolarityCtl;
		
	return 0;
}

// standby settings
//
void vsp25xxStandbySettingSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 1, val, 6) )
		h->standbySettings = val;
}

void vsp25xxStandbySettingSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxStandbySettingSet(h, vsp25xxStbyDefault);
}

UINT8 vsp25xxStandbySettingGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->standbySettings;
	
	return 0;
}

// data output settings
//
void vsp25xxDataOutputSettingSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 2, val, 6) )
		h->dataOutputSettings = val;
}

void vsp25xxDataOutputSettingSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxDataOutputSettingSet( h, vsp25xxDataOutputDefault );
}

UINT8 vsp25xxDataOutputSettingGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->dataOutputSettings;
		
	return 0;
}

// SHP-to-SHD sampling delay 
//
void vsp25xxShdSamplingDelaySet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 3, val, 6) )
		h->shdSamplingDelay = val;
}

void vsp25xxShdSamplingDelaySetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxShdSamplingDelaySet(h, vsp25xxShdSamplingDelayDefault);
}

UINT8 vsp25xxShdSamplingDelayGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->shdSamplingDelay;
		
	return 0;
}

// clamping level
//
void vsp25xxClampLevelSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 4, val, 6) )
		h->clampLevel = val;
}

void vsp25xxClampLevelSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxClampLevelSet(h, vsp25xxClampLevelDefault);
}

UINT8 vsp25xxClampLevelGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->clampLevel;
	
	return 0;
}

// Saturated Pixel Threshold Level
//
void vsp25xxHotRejectionSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 5, val, 6) )
		h->hotRejection = val;
}

void vsp25xxHotRejectionSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxHotRejectionSet(h, vsp25xxHotRejectionLevelDefault|vsp25xxHotRejectionEnable);
}

UINT8 vsp25xxHotRejectionGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->hotRejection;
	
	return 0;
}

// DPGA gain settings
//
void vsp25xxDPGAgainSet(vsp25xxBusHANDLE h, UINT16 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 6, val, 12) )
		h->dpgaGain	= val;			
}

void vsp25xxDPGAgainSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxDPGAgainSet(h, vsp25xxDPGAgainDefault);
}

UINT16 vsp25xxDPGAgainGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->dpgaGain;
		
	return 0;
}

// CDS gain control
//
void vsp25xxCDSgainSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 8, val, 6) )
		h->cdsGainControl = val;
}

void vsp25xxCDSgainSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxCDSgainSet(h, vsp25xxCDSgainDefault);
}

UINT8 vsp25xxCDSgainGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->cdsGainControl;
		
	return 0;
}

// power modes control
//
void vsp25xxPowerCtlSet(vsp25xxBusHANDLE h, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			vsp25xxBusPutBytes(h, 9, val, 6) )
		h->powerControl = val;
}

void vsp25xxPowerCtlSetDefault(vsp25xxBusHANDLE h)
{
	vsp25xxPowerCtlSet(h, vsp25xxPowerCtlDefault);
}

UINT8 vsp25xxPowerCtlGet(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) )
		return h->powerControl;
	
	return 0;
}

// DAC1|2 output write
//
void vsp25xxDACwrite(vsp25xxBusHANDLE h, UINT8 dac, UINT8 val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			dac < 2 &&
			vsp25xxBusPutBytes(h, 10+dac, val, 12) )
		h->dac[dac] = val;
}

void vsp25xxDACwriteDefault(vsp25xxBusHANDLE h, UINT8 dac)
{
	vsp25xxDACwrite(h, dac, 0);
}

UINT8 vsp25xxDACget(vsp25xxBusHANDLE h, UINT8 dac)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			dac < 2 )
		return h->dac[dac];
	
	return 0;
}

// TI AFE VSP2566 CCD interface control && parallel data RO interface.
//
void vsp25xxCcdRefLevelClkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdRefLevelClkSet )
		h->ccdRefLevelClkSet(val);
}

void vsp25xxCcdDataLevelClkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdDataLevelClkSet )
		h->ccdDataLevelClkSet(val);
}

void vsp25xxCcdADCclkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdADCclkSet )
		h->ccdADCclkSet(val);
}

void vsp25xxCcdOpticalBlackClkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdOpticalBlackClkSet )
		h->ccdOpticalBlackClkSet(val);
}

void vsp25xxCcdDummyClampClkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdDummyClampClkSet )
		h->ccdDummyClampClkSet(val);
}

void vsp25xxCcdBlankClkSet(vsp25xxBusHANDLE h, BOOL val)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdBlankClkSet )
		h->ccdBlankClkSet(val);
}

UINT16 vsp25xxCcdDataRead(vsp25xxBusHANDLE h)
{
	if( h &&
			vsp25xxIsValidType(h->type) &&
			h->ccdDataRead )
		return h->ccdDataRead();
		
	return 0;	
}
