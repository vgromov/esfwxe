#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/core/spi.h>
#include <common/datetime.h>
#include "ds28dg02.h"

// driver code for ds28dg02 RTC|GPIO|EEPROM chip
//

#define MEMBLOCK_LEN								0x40
// unique registration
#define UID													0x18

// PIO POD access
#define POD_PIOSTATE0								0x0A
#define POD_PIOSTATE1								0x0B
#define POD_PIODIR0									0x0C
#define POD_PIODIR1									0x0D
#define POD_PIOCTL0									0x0E
#define POD_PIOCTL1									0x0F
// PIO access
#define PIO_STATE0									0x20
#define PIO_STATE1									0x21
#define PIO_DIR0										0x22
#define PIO_DIR1										0x23
#define PIO_READ0										0x26
#define PIO_READ1										0x27

// RTC calendar
#define RTC_SECS										0x29
#define RTC_MINS										0x2A
#define RTC_HRS											0x2B
#define RTC_DAYOW										0x2C
#define RTC_DAYNUM									0x2D
#define RTC_MONS										0x2E
#define RTC_YRS											0x2F
// RTC alarms
#define RTC_ASECS										0x30
#define RTC_AMINS										0x31
#define RTC_AHRS										0x32
#define RTC_ADAYOW_DAYNUM						0x33
// RTC control|monitor setup
#define RTC_CTL											0x34
// RTC status
#define RTC_STAT										0x35

// chip instructions
#define WRITE_ENABLE 								0x06
#define WRITE_DISABLE								0x04

#define WRITE_STATUS								0x01
#define READ_STATUS									0x05
#define APPLY_CTLREG_PWRONDEFS			0x07

#define WRITE_HMEM									0x0A
#define READ_HMEM										0x0B
#define WRITE_LMEM									0x02
#define READ_LMEM										0x03

#define READY_WAIT_RETRIES					0x0000FFFF

// static dummy buffer for TX|RX instructions
// trailing or prefix byte is ds28 status byte as declared by
// datasheet 
static BYTE s_buff[2];

static BOOL internalWaitUntilReady(spiHANDLE bus)
{
	DWORD retries	= READY_WAIT_RETRIES;
	BOOL result = FALSE;

	s_buff[0] = READ_STATUS;

	// select chip
	ds28dg02_SELECT

	if( spiPutBytes(bus, s_buff, 1) == 1 )
		while( spiGetBytes(bus, s_buff, 1) == 1 && !result && retries-- )
			result = !(s_buff[0] & ds28dg02STATUS_BUSY);
	
	// unselect chip
	ds28dg02_DESELECT

	return result;
}

BOOL ds28dg02ReadStatus(spiHANDLE bus, BYTE* status)
{
	BOOL result = FALSE;

	s_buff[0] = READ_STATUS;

	// select chip
	ds28dg02_SELECT

	if( spiPutBytes(bus, s_buff, 1) == 1 && spiGetBytes(bus, s_buff, 1) == 1 )
	{
		*status = s_buff[0];
		result = TRUE;
	}
	
	// unselect chip
	ds28dg02_DESELECT

	return result;
}

BOOL ds28dg02WriteStatus(spiHANDLE bus, BYTE status)
{
	BOOL result = FALSE;

	s_buff[0] = WRITE_STATUS;
	s_buff[1]	= status;

	// select chip
	ds28dg02_SELECT

	result = spiPutBytes(bus, s_buff, 2) == 2;
	
	// unselect chip
	ds28dg02_DESELECT

	return result;
}

BOOL ds28dg02ApplyPOD(spiHANDLE bus)
{
	BOOL result = FALSE;

	if( internalWaitUntilReady(bus) )
	{
		s_buff[0] = APPLY_CTLREG_PWRONDEFS;
		// select chip
		ds28dg02_SELECT
		result = spiPutBytes(bus, s_buff, 1) == 1;
		// unselect chip
		ds28dg02_DESELECT
	}

	return result;
}

static __inline BOOL internalWriteEnable(spiHANDLE bus)
{
	BOOL result = FALSE;

	s_buff[0] = WRITE_ENABLE;
	// select chip
	ds28dg02_SELECT
	result = spiPutBytes(bus, s_buff, 1) == 1;
	// unselect chip
	ds28dg02_DESELECT

	return result;
}


static __inline void internalWriteDisable(spiHANDLE bus)
{
	s_buff[0] = WRITE_DISABLE;
	// select chip
	ds28dg02_SELECT
	spiPutBytes(bus, s_buff, 1);
	// unselect chip
	ds28dg02_DESELECT
}

// Read bytes from chip.
//
// ds28 protocol requires that each multibyte write packet is terminated with 
// byte which gets written to ds28 status register
DWORD ds28dg02ReadBytesLo(spiHANDLE bus, BYTE bAddr, BYTE* buff, DWORD length)
{
	DWORD result = 0;

	// wait until possible pending EEPROM writes
	if( length && internalWaitUntilReady(bus) )
	{
		s_buff[0] = READ_LMEM;
		s_buff[1]	= bAddr;

		// select chip
		ds28dg02_SELECT 	
		
		if( spiPutBytes(bus, s_buff, 2) == 2 && spiGetBytes(bus, s_buff, 1) == 1 )	// write address & read 1 dummy status byte
			 result = spiGetBytes(bus, buff, length);
	
		// unselect chip
		ds28dg02_DESELECT
	}

	return result;
}

DWORD ds28dg02ReadBytesHi(spiHANDLE bus, BYTE bAddr, BYTE* buff, DWORD length)
{
	DWORD result = 0;

	// wait until possible pending EEPROM writes
	if( length && internalWaitUntilReady(bus) )
	{
		s_buff[0] = READ_HMEM;
		s_buff[1]	= bAddr;

		// select chip
		ds28dg02_SELECT 	
		
		if( spiPutBytes(bus, s_buff, 2) == 2 && spiGetBytes(bus, s_buff, 1) == 1 )	// write address & read 1 dummy status byte 
			 result = spiGetBytes(bus, buff, length);
	
		// unselect chip
		ds28dg02_DESELECT
	}

	return result;
}

// read unique chip id
BOOL ds28dg02ReadUID( spiHANDLE bus, UINT64* uid )
{
	return ds28dg02ReadBytesHi(bus, UID, (BYTE*)uid, 8) == 8;
}

// write bytes to chip
//
DWORD ds28dg02WriteBytesLo(spiHANDLE bus, BYTE bAddr, const BYTE* buff, DWORD length)
{
	DWORD result = 0;

	// wait until possible pending EEPROM writes
	if( length && internalWaitUntilReady(bus) && internalWriteEnable(bus) )
	{
		s_buff[0] = WRITE_LMEM;
		s_buff[1]	= bAddr;

		// select chip
		ds28dg02_SELECT 	
		
		if( spiPutBytes(bus, s_buff, 2) == 2 )
			result = spiPutBytes(bus, buff, length);

		// unselect chip
		ds28dg02_DESELECT

		// ensure we disable WEN bit in case write is failed
		internalWriteDisable(bus);
	}

	return result;
}

DWORD ds28dg02WriteBytesHi(spiHANDLE bus, BYTE bAddr, const BYTE* buff, DWORD length)
{
	DWORD result = 0;

	// wait until possible pending EEPROM writes
	if( length && internalWaitUntilReady(bus) && internalWriteEnable(bus) )
	{
		s_buff[0] = WRITE_HMEM;
		s_buff[1]	= bAddr;

		// select chip
		ds28dg02_SELECT 	
		
		if( spiPutBytes(bus, s_buff, 2) == 2 )
			result = spiPutBytes(bus, buff, length);

		// unselect chip
		ds28dg02_DESELECT

		// ensure we disable WEN bit in case write is failed
		internalWriteDisable(bus);
	}

	return result;
}

BOOL ds28dg02GetPOD_PIODirection( spiHANDLE bus, WORD* dir )
{
	return ds28dg02ReadBytesHi(bus, POD_PIODIR0, (BYTE*)dir, 2) == 2;
}

BOOL ds28dg02SetPOD_PIODirection( spiHANDLE bus, WORD dir )
{
	return ds28dg02WriteBytesHi(bus, POD_PIODIR0, (const BYTE*)&dir, 2) == 2;
}

BOOL ds28dg02GetPOD_PIOState( spiHANDLE bus, WORD* state )
{
	return ds28dg02ReadBytesHi(bus, POD_PIOSTATE0, (BYTE*)state, 2) == 2;
}

BOOL ds28dg02SetPOD_PIOState( spiHANDLE bus, WORD state )
{
	return ds28dg02WriteBytesHi(bus, POD_PIOSTATE0, (const BYTE*)&state, 2) == 2;
}

BOOL ds28dg02GetPOD_PIOCtl( spiHANDLE bus, WORD* ctl )
{
	return ds28dg02ReadBytesHi(bus, POD_PIOCTL0, (BYTE*)ctl, 2) == 2;
}

BOOL ds28dg02SetPOD_PIOCtl( spiHANDLE bus, WORD ctl )
{	
	return ds28dg02WriteBytesHi(bus, POD_PIOCTL0, (const BYTE*)&ctl, 2) == 2;
}

BOOL ds28dg02SetPIODirection( spiHANDLE bus, WORD state )
{
	return ds28dg02WriteBytesHi(bus, PIO_DIR0, (const BYTE*)&state, 2 ) == 2;
}

BOOL ds28dg02GetPIODirection( spiHANDLE bus, WORD* state )
{
	return ds28dg02ReadBytesHi(bus, PIO_DIR0, (BYTE*)state, 2 ) == 2;
}

BOOL ds28dg02SetPIOState( spiHANDLE bus, WORD state )
{
	return ds28dg02WriteBytesHi(bus, PIO_STATE0, (const BYTE*)&state, 2 ) == 2;
}

BOOL ds28dg02GetPIOState( spiHANDLE bus, WORD* state )
{
	return ds28dg02ReadBytesHi(bus, PIO_STATE0, (BYTE*)state, 2 ) == 2;
}

BOOL ds28dg02GetPIOInput( spiHANDLE bus, WORD* state )
{
	return ds28dg02ReadBytesHi(bus, PIO_READ0, (BYTE*)state, 2 ) == 2;
}

// RTC getters and setters
BOOL ds28dg02SetRTC_Ctl(spiHANDLE bus, BYTE ctl)
{
	return ds28dg02WriteBytesHi(bus, RTC_CTL, &ctl, 1) == 1;
}

BOOL ds28dg02GetRTC_Ctl(spiHANDLE bus, BYTE* ctl)
{
	return ds28dg02ReadBytesHi(bus, RTC_CTL, ctl, 1) == 1;
}

BOOL ds28dg02SetDateTime(spiHANDLE bus, const DATETIME* dt)
{
	BYTE tmp[7];
	int y, mn, d, h, m, s;
	dtDecomposeDateTime(dt,	&y, 0, &mn, &d, &h, &m, &s, 0);
	tmp[0] = BYTE2BCDBYTE(s);
	tmp[1] = BYTE2BCDBYTE(m);
	tmp[2] = BYTE2BCDBYTE(h);
	tmp[3] = BYTE2BCDBYTE( dtGetDayOfWeek(y, mn, d) );
	tmp[4] = BYTE2BCDBYTE(d);
	tmp[5] = BYTE2BCDBYTE(mn);
	tmp[6] = BYTE2BCDBYTE(y%100);

	return ds28dg02WriteBytesHi(bus, RTC_SECS, tmp, 7) == 7;	
}

BOOL ds28dg02GetDateTime(spiHANDLE bus, DATETIME* dt)
{
	BYTE tmp[7];
	return ds28dg02ReadBytesHi(bus, RTC_SECS, tmp, 7) == 7 &&
		dtComposeDateTime(dt, BCDBYTE2BYTE(tmp[6])+2000, BCDBYTE2BYTE(tmp[5]), BCDBYTE2BYTE(tmp[4]),
				BCDBYTE2BYTE(tmp[2]), BCDBYTE2BYTE(tmp[1]), BCDBYTE2BYTE(tmp[0]), 0);	
}
