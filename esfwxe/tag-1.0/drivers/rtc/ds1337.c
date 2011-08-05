#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/core/twi_i2c.h>
#include <common/datetime.h>
#include "ds1337.h"

// RTC chip address
#define DS1337_ADDR 0xD0

// RTC register addresses
#define DS1337_SEC	0x00
#define DS1337_MIN	0x01
#define DS1337_HR		0x02
#define DS1337_DOW	0x03
#define DS1337_DAY	0x04
#define DS1337_MON	0x05
#define DS1337_YEAR	0x06
#define DS1337_CTL	0x0E
#define DS1337_STAT	0x0F

// alarm register offsets
#define DS1337_ALM1_OFFS 0x07
#define DS1337_ALM2_OFFS 0x0B

// internal adressed byte access
static __inline BOOL ds1337SetByte(i2cHANDLE handle, BYTE addr, BYTE b)
{
	BYTE tmp[2];
	tmp[0] = addr;
	tmp[1] = b;
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 2) == 2;
}

static __inline BOOL ds1337GetByte(i2cHANDLE handle, BYTE addr, BYTE* b)
{
	return i2cPutBytes(handle, DS1337_ADDR, &addr, 1) == 1 &&
		i2cGetBytes(handle, DS1337_ADDR, b, 1) == 1;
}

// RTC initialization
BOOL ds1337SetCtl(i2cHANDLE handle, BYTE ctl)
{
	return ds1337SetByte(handle, DS1337_CTL, ctl);
}

BOOL ds1337GetCtl(i2cHANDLE handle, BYTE* ctl)
{	
	return ds1337GetByte(handle, DS1337_CTL, ctl);
}

// read|write chip status
BOOL ds1337SetStatus(i2cHANDLE handle, BYTE status)
{
	return ds1337SetByte(handle, DS1337_STAT, status);
}

BOOL ds1337GetStatus(i2cHANDLE handle, BYTE* status)
{
	return ds1337GetByte(handle, DS1337_STAT, status);
}

// RTC
//
BOOL ds1337SetDateTime(i2cHANDLE handle, const DATETIME* dt)
{
	BYTE tmp[8];
	int y, mn, d, h, m, s;
	tmp[0] = DS1337_SEC;
	dtDecomposeDateTime(dt,	&y, 0, &mn, &d, &h, &m, &s, 0);
	tmp[1] = BYTE2BCDBYTE(s);
	tmp[2] = BYTE2BCDBYTE(m);
	tmp[3] = BYTE2BCDBYTE(h);
	tmp[4] = BYTE2BCDBYTE( dtGetDayOfWeek(y, mn, d) );
	tmp[5] = BYTE2BCDBYTE(d);
	tmp[6] = BYTE2BCDBYTE(mn);
	tmp[7] = BYTE2BCDBYTE(y%100);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 8) == 8;	
}

BOOL ds1337GetDateTime(i2cHANDLE handle, DATETIME* dt)
{
	BYTE tmp[7];
	tmp[0] = DS1337_SEC;
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 1) == 1 &&
			i2cGetBytes(handle, DS1337_ADDR, tmp, 7) == 7 &&
			dtComposeDateTime(dt, BCDBYTE2BYTE(tmp[6])+2000, BCDBYTE2BYTE(tmp[5]), BCDBYTE2BYTE(tmp[4]),
				BCDBYTE2BYTE(tmp[2]), BCDBYTE2BYTE(tmp[1]), BCDBYTE2BYTE(tmp[0]), 0);
}

// set alarm DATETIME
BOOL ds1337SetAlarm1(i2cHANDLE handle, const DATETIME* dt, BOOL useDayOfWeek, BYTE ignoreMatchMask)
{
	BYTE tmp[5];
  int y, mn, d, h, m, s;
  tmp[0] = DS1337_SEC+DS1337_ALM1_OFFS;
  dtDecomposeDateTime(dt, &y, 0, &mn, &d, &h, &m, &s, 0);
 	tmp[1] = BYTE2BCDBYTE(s) | ((ignoreMatchMask & 0x01) << 7);
 	tmp[2] = BYTE2BCDBYTE(m) | ((ignoreMatchMask & 0x02) << 6);
 	tmp[3] = BYTE2BCDBYTE(h) | ((ignoreMatchMask & 0x04) << 5);
 	tmp[4] = (useDayOfWeek ? BYTE2BCDBYTE( dtGetDayOfWeek(y, mn, d) ) : BYTE2BCDBYTE(d)) |
	  ((ignoreMatchMask & 0x08) << 4);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 5) == 5; 
}

BOOL ds1337SetAlarm2(i2cHANDLE handle, const DATETIME* dt, BOOL useDayOfWeek, BYTE ignoreMatchMask)
{
	BYTE tmp[4];
	int y, mn, d, h, m;
	tmp[0] = DS1337_SEC+DS1337_ALM2_OFFS;
	dtDecomposeDateTime(dt,	&y, 0, &mn, &d, &h, &m, 0, 0);
	tmp[1] = BYTE2BCDBYTE(m) | ((ignoreMatchMask & 0x01) << 7);
	tmp[2] = BYTE2BCDBYTE(h) | ((ignoreMatchMask & 0x02) << 6);
	tmp[3] = (useDayOfWeek ? BYTE2BCDBYTE( dtGetDayOfWeek(y, mn, d) ) : BYTE2BCDBYTE(d)) |
		((ignoreMatchMask & 0x04) << 5);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 4) == 4;	
}

