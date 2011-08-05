#ifndef _ccd_tcd_h_
#define _ccd_tcd_h_

#ifdef __cplusplus
	extern "C" {
#endif

// driver api abstraction for the toshiba CCD sensors
//

enum {
	// internal state flags
	//
	// in pixel bits
	ccdTcdfInDummyPixel 		= 0x01,
	ccdTcdfInBlackPixel			= 0x02,
	ccdTcdfInDataPixel			= 0x04,
	ccdTcdfInTrailingPixel 	= 0x08,
	ccdTcdfInPixelMask			= ccdTcdfInDummyPixel|
														ccdTcdfInBlackPixel|
														ccdTcdfInDataPixel|
														ccdTcdfInTrailingPixel,
	// master clock line state bit
	ccdTcdfMasterClockHigh	= 0x10,
};

// chip types
typedef enum {
	tcdInvalid,
	tcd1304xx,
	// must go last
	ccdTcdTypeCnt

} ccdTcdType;

// ccd sensor structure
typedef struct _ccdTcdStruct ccdTcdStruct;
typedef ccdTcdStruct* ccdTcdHandle;
// installable driver handlers
typedef void (*ccdTcdConfigLinesPfn)(void);
typedef void (*ccdTcdLineSetPfn)(BOOL val);
typedef void (*ccdTcdConveyorEndReachedPfn)(ccdTcdHandle h);
typedef void (*ccdTcdOnInPixelTickPfn)(UINT8 tick, UINT8 pixelFlags);
// ccd structure itself
typedef struct _ccdTcdStruct {
	ccdTcdConfigLinesPfn configLines;
	ccdTcdLineSetPfn integrationClearGate;
	ccdTcdLineSetPfn shutter;
	ccdTcdLineSetPfn masterClock;
	// external handlers
	ccdTcdLineSetPfn onMasterClock;
	ccdTcdLineSetPfn onDummyPixels;
	ccdTcdLineSetPfn onBlackPixels;
	ccdTcdLineSetPfn onDataPixels;
	ccdTcdLineSetPfn onTrailingPixels;
	ccdTcdLineSetPfn onPixelRefData;
	ccdTcdLineSetPfn onPixelData;
	ccdTcdOnInPixelTickPfn onPixelTick;
	ccdTcdConveyorEndReachedPfn onConveyorEndReached;
	// master tick counter
	UINT32 masterTickCnt;
	// integration controls
	UINT32 integrationDuration;
	UINT32 shutterStart;
	UINT32 shutterEnd;
	// pixel data levels capture controls
	UINT8 pixelTickCnt;
	// helper internal state flags
	UINT8 flags;
	// ccd chip type
	UINT8 type;

} ccdTcdStruct;

// API
//
// one-time ccd struct initializer
void ccdTcdInit(ccdTcdHandle h, ccdTcdType type,
	ccdTcdConfigLinesPfn configLines, ccdTcdLineSetPfn integrationClearGate, ccdTcdLineSetPfn shutter, ccdTcdLineSetPfn masterClock );
// type info access
ccdTcdType ccdTcdTypeGet(ccdTcdHandle h);
// configure ccd io lines
void ccdTcdConfigureLines(ccdTcdHandle h);
// get ccd working frequency
UINT32 ccdTcdWorkingFrequencyGet(ccdTcdHandle h);
// get total pixel count
UINT32 ccdTcdTotalPixelCountGet(ccdTcdHandle h);
// get data pixel count
UINT32 ccdTcdDataPixelCountGet(ccdTcdHandle h);
// get master ticks per pixel
UINT32 ccdTcdMasterTicksPerPixelGet(ccdTcdHandle h);
// get|set integration time. integration time is measured in pixel units, i.e. the shortest integration time is 1 pixel = ccdTcdMasterTicksPerPixelGet() ticks,
// the longest = ccdTcdTotalPixelCountGet() pixels = ccdTcdMasterTicksPerPixelGet() ticks
void ccdTcdIntegrationTimeSet(ccdTcdHandle h, UINT32 pixels);
UINT32 ccdTcdIntegrationTimeGet(ccdTcdHandle h);
// reset pixel conveyor
void ccdTcdPixelConveyorReset(ccdTcdHandle h);
// do single master tick on pixel conveyor
void ccdTcdPixelConveyorDoMasterTick(ccdTcdHandle h);
// external handlers set-up
void ccdTcdOnMasterClockSet(ccdTcdHandle h, ccdTcdLineSetPfn onMasterClock);
void ccdTcdOnDummyPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onDummyPixels);
void ccdTcdOnBlackPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onBlackPixels);
void ccdTcdOnDataPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onDataPixels);
void ccdTcdOnTrailingPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onTrailingPixels);
void ccdTcdOnPixelRefDataSet(ccdTcdHandle h, ccdTcdLineSetPfn onPixelRefData);
void ccdTcdOnPixelDataSet(ccdTcdHandle h, ccdTcdLineSetPfn onPixelData);
void ccdTcdOnPixelTickSet(ccdTcdHandle h, ccdTcdOnInPixelTickPfn onPixelTick);
void ccdTcdConveyorEndReachedSet(ccdTcdHandle h, ccdTcdConveyorEndReachedPfn onConveyorEndReached);

#ifdef __cplusplus
	}
#endif

#endif // _ccd_tcd_h_
