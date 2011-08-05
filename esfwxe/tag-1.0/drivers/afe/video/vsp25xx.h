#ifndef _vsp_25xx_h_
#define _vsp_25xx_h_

#ifdef __cplusplus
	extern "C" {
#endif

// TI AFE bus abstraction
//
// vsp chip types
typedef enum {
	vspInvalid,
	vsp2560,
	vsp2562,
	vsp2566,
	// special value, must go last
	vspTypeCnt

} vspType;

// vsp chip bus
typedef struct _vsp25xxBusHANDLE vsp25xxBus;
typedef vsp25xxBus* vsp25xxBusHANDLE;
// vsp bus handler procedure types
typedef void (*vsp25xxBusConfigPfn)(vsp25xxBusHANDLE h);
typedef void (*vsp25xxBusLineSetPfn)(BOOL set);
typedef UINT16 (*vsp25xxBusCCDdataReadPfn)(void);
// vsp bus structure
typedef struct _vsp25xxBusHANDLE {
	vsp25xxBusConfigPfn configLines;
	vsp25xxBusLineSetPfn rloadSet;
	vsp25xxBusLineSetPfn sloadSet;
	vsp25xxBusLineSetPfn sclkSet;
	vsp25xxBusLineSetPfn dataSet;
	vsp25xxBusLineSetPfn ccdRefLevelClkSet;
	vsp25xxBusLineSetPfn ccdDataLevelClkSet;
	vsp25xxBusLineSetPfn ccdADCclkSet;
	vsp25xxBusLineSetPfn ccdOpticalBlackClkSet;
	vsp25xxBusLineSetPfn ccdDummyClampClkSet;
	vsp25xxBusLineSetPfn ccdBlankClkSet;
	vsp25xxBusCCDdataReadPfn ccdDataRead;
	UINT16 dpgaGain;
	UINT8 type;
	UINT8 clockPolarityCtl;
	UINT8 standbySettings;
	UINT8 dataOutputSettings;
	UINT8 shdSamplingDelay;
	UINT8 clampLevel;
	UINT8 hotRejection;
	UINT8 cdsGainControl;
	UINT8 powerControl;
	UINT8 dac[2];
		
} vsp25xxBus;

// bus api, implementations of vsp25xxBusConfig && xxxSet handlers are application-dependent
void vsp25xxBusInit(vsp25xxBusHANDLE h, vspType vsp25xxType, vsp25xxBusConfigPfn configLines, 
	vsp25xxBusLineSetPfn rloadSet, vsp25xxBusLineSetPfn sloadSet, vsp25xxBusLineSetPfn sclkSet, vsp25xxBusLineSetPfn dataSet,
	vsp25xxBusLineSetPfn ccdRefLevelClkSet,	vsp25xxBusLineSetPfn ccdDataLevelClkSet, vsp25xxBusLineSetPfn ccdADCclkSet,
	vsp25xxBusLineSetPfn ccdOpticalBlackClkSet,	vsp25xxBusLineSetPfn ccdDummyClampClkSet,	vsp25xxBusLineSetPfn ccdBlankClkSet,
	vsp25xxBusCCDdataReadPfn ccdDataRead);

void vsp25xxBusConfigure(vsp25xxBusHANDLE h);
vspType vsp25xxTypeGet(vsp25xxBusHANDLE h);

// TI AFE VSP2566 serial programming interface.
// all hardware functions are write-only, no error conditions are returned,
// read capability is supported through software only
//
// active polarity selection control
//
#define vsp25xxCPCtlDefault								0
// RLOAD control values 
#define vsp25xxCPCtlUpdateByRisingRLOAD 	0x02
#define vsp25xxCPCtlUpdateByFallingRLOAD 	0x06
#define vsp25xxCPCtlRLOADmask							0x07
// level invertion bits
#define vsp25xxCPCtlCPLDMactiveHigh				0x08
#define vsp25xxCPCtlCPLOBactiveHign				0x10
#define vsp25xxCPCtlSHP_SHDactiveHigh			0x20
void vsp25xxClockPolarityCtlSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxClockPolarityCtlSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxClockPolarityCtlGet(vsp25xxBusHANDLE h);

// standby settings
//
#define vsp25xxStbyDefault								0x06
// standby control bits
#define vsp25xxStby												0x01
#define vsp25xxStbyDAC1										0x02
#define vsp25xxStbyDAC2										0x04
#define vsp25xxStbyTestEnable							0x08
void vsp25xxStandbySettingSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxStandbySettingSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxStandbySettingGet(vsp25xxBusHANDLE h);

// data output settings
//
#define vsp25xxDataOutputDefault			0
#define vsp25xxDataOutputDelay0ns			0
#define vsp25xxDataOutputDelay2ns			0x01
#define vsp25xxDataOutputDelay4ns			0x02
#define vsp25xxDataOutputDelay6ns			0x03
// data output disable bit
#define vsp25xxDataOutputDisable			0x04
void vsp25xxDataOutputSettingSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxDataOutputSettingSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxDataOutputSettingGet(vsp25xxBusHANDLE h);

// SHP-to-SHD sampling delay 
//
#define vsp25xxShdSamplingDelayDefault		0
#define vsp25xxShdSamplingDelay0ns			0
#define vsp25xxShdSamplingDelay2ns			1
void vsp25xxShdSamplingDelaySet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxShdSamplingDelaySetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxShdSamplingDelayGet(vsp25xxBusHANDLE h);

// clamping level
//
#define vsp25xxClampLevelMin				0
#define vsp25xxClampLevelDefault			8
#define vsp25xxClampLevelMax				31
void vsp25xxClampLevelSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxClampLevelSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxClampLevelGet(vsp25xxBusHANDLE h);

// Saturated Pixel Threshold Level
//
#define vsp25xxHotRejectionLevelMin			0
#define vsp25xxHotRejectionLevelDefault		31
#define vsp25xxHotRejectionLevelMax			31
// hot pixel rejection enabling bit
#define vsp25xxHotRejectionEnable			0x20
void vsp25xxHotRejectionSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxHotRejectionSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxHotRejectionGet(vsp25xxBusHANDLE h);

// DPGA gain settings
//
#define vsp25xxDPGAgainMin 					0
#define vsp25xxDPGAgainDefault 				192
#define vsp25xxDPGAgainMax 					1023
void vsp25xxDPGAgainSet(vsp25xxBusHANDLE h, UINT16 val);
void vsp25xxDPGAgainSetDefault(vsp25xxBusHANDLE h);
UINT16 vsp25xxDPGAgainGet(vsp25xxBusHANDLE h);

// CDS gain control
//
#define vsp25xxCDSgainMinus3db				7
#define vsp25xxCDSgain0db 					0
#define vsp25xxCDSgain6db 					1
#define vsp25xxCDSgain12db					2
#define vsp25xxCDSgain18db					3
#define vsp25xxCDSgainDefault				0
void vsp25xxCDSgainSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxCDSgainSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxCDSgainGet(vsp25xxBusHANDLE h);

// power modes control
//
// DAC power control for OB loop time constant
#define vsp25xxPowerCtlOBloopIDACx1 		0x00
#define vsp25xxPowerCtlOBloopIDACx2 		0x01
#define vsp25xxPowerCtlOBloopIDACx4 		0x02
#define vsp25xxPowerCtlOBloopIDACx8 		0x03
// other power control bits
#define vsp25xxPowerCtlCDStrim				0x04
#define vsp25xxPowerCtlADCtrim				0x08
#define vsp25xxPowerCtlRefTrim				0x10
#define vsp25xxPowerCtlGBAtrim				0x20
// default value
#define vsp25xxPowerCtlDefault				0
void vsp25xxPowerCtlSet(vsp25xxBusHANDLE h, UINT8 val);
void vsp25xxPowerCtlSetDefault(vsp25xxBusHANDLE h);
UINT8 vsp25xxPowerCtlGet(vsp25xxBusHANDLE h);

// DAC1|2 output write
//
void vsp25xxDACwrite(vsp25xxBusHANDLE h, UINT8 dac, UINT8 val);
void vsp25xxDACwriteDefault(vsp25xxBusHANDLE h, UINT8 dac);
UINT8 vsp25xxDACget(vsp25xxBusHANDLE h, UINT8 dac);

// TI AFE VSP2566 CCD interface control && parallel data RO interface.
//
void vsp25xxCcdRefLevelClkSet(vsp25xxBusHANDLE h, BOOL val);
void vsp25xxCcdDataLevelClkSet(vsp25xxBusHANDLE h, BOOL val);
void vsp25xxCcdADCclkSet(vsp25xxBusHANDLE h, BOOL val);
void vsp25xxCcdOpticalBlackClkSet(vsp25xxBusHANDLE h, BOOL val);
void vsp25xxCcdDummyClampClkSet(vsp25xxBusHANDLE h, BOOL val);
void vsp25xxCcdBlankClkSet(vsp25xxBusHANDLE h, BOOL val);
UINT16 vsp25xxCcdDataRead(vsp25xxBusHANDLE h);

#ifdef __cplusplus
	}
#endif

#endif // _vsp_25xx_h_
