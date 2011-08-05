#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <common/utils.h>
#include "ccdTcd.h"

// device-dependent pixel conveyor timings, in terms
// of master clock ticks
//
typedef struct {
	UINT32 ccdFrequency;
	UINT32 icgTickStart;
	UINT32 icgTickEnd;
	UINT32 shTickStart;
	UINT32 ticksPerPixel;
	UINT32 totalPixelTicks;
	UINT32 dummyPixelsTickStart;
	UINT32 dummyPixelsTickEnd;
	UINT32 blackPixelsTickStart;
	UINT32 blackPixelsTickEnd;
	UINT32 dataPixelTickStart;
	UINT32 dataPixelTickEnd;
	UINT8 refCaptureStart;
	UINT8 refCaptureEnd;
	UINT8 dataCaptureStart;
	UINT8 dataCaptureEnd;

} ccdTcdInfo;

static const ccdTcdInfo c_ccdTcd[ccdTcdTypeCnt] = {
	// TCD1304AP|DG
	{
		2000000,		// working frequency
		0,					// icg tick start
		17,					// icg tick end
		2,					// shutter tick start
		8, 					// master ticks per pixel
		3694*8, 		// total pixel ticks count
		17,					// dummy start
		17+31*8,		// dummy end
		17+16*8,		// black start
		17+28*8,		// black end
		17+32*8,		// data start
		17+3679*8,	// data end
		1,					// pixel reference level capture start
		3,					// pixel reference level capture end
		5,					// pixel data level capture start 
		7,					// pixel data level capture end
	}

};

// API
//
static __inline BOOL ccdTcdIsValidType(UINT8 type)
{
	return type > tcdInvalid &&
					type < ccdTcdTypeCnt;
}

void ccdTcdInit(ccdTcdHandle h, ccdTcdType type,
	ccdTcdConfigLinesPfn configLines, ccdTcdLineSetPfn integrationClearGate, ccdTcdLineSetPfn shutter, ccdTcdLineSetPfn masterClock )
{
	if( h )
	{
	 	memset(h, 0, sizeof(ccdTcdStruct));
		if(	ccdTcdIsValidType(type) &&
				configLines &&
				integrationClearGate && 
				shutter && 
				masterClock )
		{
			h->type = type;
			h->configLines = configLines;
			h->integrationClearGate = integrationClearGate;
			h->shutter = shutter;
			h->masterClock	= masterClock;
	
			ccdTcdConfigureLines(h);
			ccdTcdIntegrationTimeSet(h, c_ccdTcd[h->type-1].totalPixelTicks);
			ccdTcdPixelConveyorReset(h);
		}
	}
}

// type info access
ccdTcdType ccdTcdTypeGet(ccdTcdHandle h)
{
	if( h &&
			h->type < ccdTcdTypeCnt )
		return (ccdTcdType)h->type;

	return tcdInvalid;
}

void ccdTcdConfigureLines(ccdTcdHandle h)
{
	if( h &&
			h->configLines )
		h->configLines();
}

// get ccd working frequency
UINT32 ccdTcdWorkingFrequencyGet(ccdTcdHandle h)
{
	if( h &&
			h->type < ccdTcdTypeCnt )
		return c_ccdTcd[h->type-1].ccdFrequency;

	return 0;
}

// get total pixel count
UINT32 ccdTcdTotalPixelCountGet(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		return c_ccdTcd[h->type-1].totalPixelTicks;

	return 0;
}

// get data pixel count
UINT32 ccdTcdDataPixelCountGet(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		return (c_ccdTcd[h->type-1].dataPixelTickEnd - c_ccdTcd[h->type-1].dataPixelTickStart) / c_ccdTcd[h->type-1].ticksPerPixel + 1;

	return 0;
}

// get master ticks per pixel
UINT32 ccdTcdMasterTicksPerPixelGet(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		return c_ccdTcd[h->type-1].ticksPerPixel;

	return 0;
}

// get|set integration time. integration time is measured in pixel units, i.e. the shortest integration time is 1 pixel = ccdTcdMasterTicksPerPixelGet() ticks,
// the longest = ccdTcdTotalPixelCountGet() pixels = ccdTcdMasterTicksPerPixelGet() * ccdTcdTotalPixelCountGet() ticks
void ccdTcdIntegrationTimeSet(ccdTcdHandle h, UINT32 pixels)
{
	if( h &&
			ccdTcdIsValidType(h->type) &&
			pixels > 0 &&
			pixels <= c_ccdTcd[h->type-1].totalPixelTicks )
		h->integrationDuration = pixels * c_ccdTcd[h->type-1].ticksPerPixel;
}

UINT32 ccdTcdIntegrationTimeGet(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		return h->integrationDuration / c_ccdTcd[h->type-1].ticksPerPixel;

	return 0;
}

// internal shutter timing update helper. must be called at the end of 
// each shutter start-end cycle
static void ccdTcdUpdateShutterStartEnd(ccdTcdHandle h)
{
	h->shutterStart = h->shutterEnd + h->integrationDuration;
	h->shutterEnd = h->shutterStart + c_ccdTcd[h->type-1].ticksPerPixel;
}

// reset pixel conveyor
void ccdTcdPixelConveyorReset(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
	{
		h->flags = 0;
		h->masterTickCnt = 0;
		h->masterClock(FALSE);
		// reset icg && shutter lines
		h->integrationClearGate(TRUE);
		h->shutter(FALSE);
		// reset integration controls
		h->shutterStart = c_ccdTcd[h->type-1].shTickStart;
		h->shutterEnd = h->shutterStart + c_ccdTcd[h->type-1].ticksPerPixel;
		// reset in-pixel data capture controls
		h->pixelTickCnt = 0;
	}
}

// do single master tick on pixel conveyor
void ccdTcdPixelConveyorDoMasterTick(ccdTcdHandle h)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
	{
		const ccdTcdInfo* info = &c_ccdTcd[h->type-1];
		// toggle clock line state bit
		if( ES_BIT_IS_SET(h->flags, ccdTcdfMasterClockHigh) )
			ES_BIT_CLR(h->flags, ccdTcdfMasterClockHigh);
		else
			ES_BIT_SET(h->flags, ccdTcdfMasterClockHigh);
		// set master clock line high for even ticks, 
		// low for odd ones
		h->masterClock( ES_BIT_IS_SET(h->flags, ccdTcdfMasterClockHigh) );
		// handle ICG line state
		if( info->icgTickStart == h->masterTickCnt )
			h->integrationClearGate(FALSE);
		else if( info->icgTickEnd == h->masterTickCnt )
			h->integrationClearGate(TRUE);
		// handle shutter line
		if( h->shutterStart == h->masterTickCnt )
			h->shutter(TRUE);
		else if( h->shutterEnd == h->masterTickCnt )
		{
			h->shutter(FALSE);
			ccdTcdUpdateShutterStartEnd(h);
		}
		// handle dummy pixels
		if( info->dummyPixelsTickStart == h->masterTickCnt )
		{
			ES_BIT_SET(h->flags, ccdTcdfInDummyPixel);
			h->pixelTickCnt = 0;
			if( h->onDummyPixels )
				h->onDummyPixels(TRUE);
		}
		else if( info->dummyPixelsTickEnd == h->masterTickCnt )
		{
			ES_BIT_CLR(h->flags, ccdTcdfInDummyPixel);
			if( h->onDummyPixels )
				h->onDummyPixels(FALSE);
		}
		// handle black pixels
		if( info->blackPixelsTickStart == h->masterTickCnt )
		{
			ES_BIT_SET(h->flags, ccdTcdfInBlackPixel);
			h->pixelTickCnt = 0;
			if( h->onBlackPixels )
				h->onBlackPixels(TRUE);
		}
		else if( info->blackPixelsTickEnd == h->masterTickCnt )
		{
			ES_BIT_CLR(h->flags, ccdTcdfInBlackPixel);
			if( h->onBlackPixels )
				h->onBlackPixels(FALSE);
		}
		// handle data pixels
		if( info->dataPixelTickStart == h->masterTickCnt )
		{
			ES_BIT_SET(h->flags, ccdTcdfInDataPixel);
			h->pixelTickCnt = 0;
			if(	h->onDataPixels )
				h->onDataPixels(TRUE);
		}
		else if( info->dataPixelTickEnd == h->masterTickCnt )
		{
			ES_BIT_CLR(h->flags, ccdTcdfInDataPixel);
			ES_BIT_SET(h->flags, ccdTcdfInTrailingPixel);
			h->pixelTickCnt = 0;
			if(	h->onDataPixels )
				h->onDataPixels(FALSE);
			if( h->onTrailingPixels )
				h->onTrailingPixels(TRUE);
		}
		// handle trailing pixels
		if( ES_BIT_IS_SET(h->flags, ccdTcdfInTrailingPixel) &&
				h->masterTickCnt >= info->dummyPixelsTickStart + info->totalPixelTicks )
		{
			ES_BIT_CLR(h->flags, ccdTcdfInTrailingPixel);
			if( h->onTrailingPixels )
				h->onTrailingPixels(FALSE);
		}
		// handle per-pixel output levels
		if( ES_BIT_MASK_MATCH(h->flags, ccdTcdfInPixelMask) )
		{
			if( info->refCaptureStart == h->pixelTickCnt &&
					h->onPixelRefData )
				h->onPixelRefData(TRUE);
			else if( info->refCaptureEnd == h->pixelTickCnt &&
				h->onPixelRefData )
				h->onPixelRefData(FALSE);

			if( info->dataCaptureStart == h->pixelTickCnt &&
					h->onPixelData )
				h->onPixelData(TRUE);
			else if( info->dataCaptureEnd == h->pixelTickCnt &&
				h->onPixelData )
				h->onPixelData(FALSE);
			// call pixel tick handler
			if(h->onPixelTick)
				h->onPixelTick(h->pixelTickCnt, h->flags & ccdTcdfInPixelMask);
			// update|reset pixel tick counter
			if( ++h->pixelTickCnt >= info->ticksPerPixel )
				h->pixelTickCnt = 0;
		}
		// call external handler for master clock tick
		if( h->onMasterClock )
			h->onMasterClock( ES_BIT_IS_SET(h->flags, ccdTcdfMasterClockHigh) );
		// increment master tick counter && call end-cycle handler
		if( h->masterTickCnt < info->totalPixelTicks+info->dummyPixelsTickStart )
			++h->masterTickCnt;
		else if( h->onConveyorEndReached )
			h->onConveyorEndReached(h);			
	}
}

// external handlers set-up
void ccdTcdOnMasterClockSet(ccdTcdHandle h, ccdTcdLineSetPfn onMasterClock)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onMasterClock = onMasterClock;
}

void ccdTcdOnDummyPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onDummyPixels)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onDummyPixels = onDummyPixels;
}

void ccdTcdOnBlackPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onBlackPixels)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onBlackPixels = onBlackPixels;
}

void ccdTcdOnDataPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onDataPixels)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onDataPixels = onDataPixels;
}

void ccdTcdOnTrailingPixelsSet(ccdTcdHandle h, ccdTcdLineSetPfn onTrailingPixels)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onTrailingPixels = onTrailingPixels;
}

void ccdTcdOnPixelRefDataSet(ccdTcdHandle h, ccdTcdLineSetPfn onPixelRefData)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onPixelRefData = onPixelRefData;
}

void ccdTcdOnPixelDataSet(ccdTcdHandle h, ccdTcdLineSetPfn onPixelData)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onPixelData = onPixelData;
}

void ccdTcdOnPixelTickSet(ccdTcdHandle h, ccdTcdOnInPixelTickPfn onPixelTick)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onPixelTick = onPixelTick;
}

void ccdTcdConveyorEndReachedSet(ccdTcdHandle h, ccdTcdConveyorEndReachedPfn onConveyorEndReached)
{
	if( h &&
			ccdTcdIsValidType(h->type) )
		h->onConveyorEndReached = onConveyorEndReached;
}
