#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include <esfwxe/core/twi_i2c.h>
#include <esfwxe/datetime.h>
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
static __inline esBL ds1337SetByte(i2cHANDLE handle, esU8 addr, esU8 b)
{
	esU8 tmp[2];
	tmp[0] = addr;
	tmp[1] = b;
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 2) == 2;
}

static __inline esBL ds1337GetByte(i2cHANDLE handle, esU8 addr, esU8* b)
{
	return i2cPutBytes(handle, DS1337_ADDR, &addr, 1) == 1 &&
		i2cGetBytes(handle, DS1337_ADDR, b, 1) == 1;
}

// RTC initialization
esBL ds1337SetCtl(i2cHANDLE handle, esU8 ctl)
{
	return ds1337SetByte(handle, DS1337_CTL, ctl);
}

esBL ds1337GetCtl(i2cHANDLE handle, esU8* ctl)
{	
	return ds1337GetByte(handle, DS1337_CTL, ctl);
}

// read|write chip status
esBL ds1337SetStatus(i2cHANDLE handle, esU8 status)
{
	return ds1337SetByte(handle, DS1337_STAT, status);
}

esBL ds1337GetStatus(i2cHANDLE handle, esU8* status)
{
	return ds1337GetByte(handle, DS1337_STAT, status);
}

// RTC
//
esBL ds1337SetDateTime(i2cHANDLE handle, const esDT* dt)
{
	esU8 tmp[8];
	int y, mn, d, h, m, s;
	tmp[0] = DS1337_SEC;
	dtDecomposeDateTime(dt,	&y, 0, &mn, &d, &h, &m, &s, 0);
	tmp[1] = BYTE2esBCD(s);
	tmp[2] = BYTE2esBCD(m);
	tmp[3] = BYTE2esBCD(h);
	tmp[4] = BYTE2esBCD( dtGetDayOfWeek(y, mn, d) );
	tmp[5] = BYTE2esBCD(d);
	tmp[6] = BYTE2esBCD(mn);
	tmp[7] = BYTE2esBCD(y%100);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 8) == 8;	
}

esBL ds1337GetDateTime(i2cHANDLE handle, esDT* dt)
{
	esU8 tmp[7];
	tmp[0] = DS1337_SEC;
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 1) == 1 &&
			i2cGetBytes(handle, DS1337_ADDR, tmp, 7) == 7 &&
			dtComposeDateTime(dt, esBCD2BYTE(tmp[6])+2000, esBCD2BYTE(tmp[5]), esBCD2BYTE(tmp[4]),
				esBCD2BYTE(tmp[2]), esBCD2BYTE(tmp[1]), esBCD2BYTE(tmp[0]), 0);
}

// set alarm esDT
esBL ds1337SetAlarm1(i2cHANDLE handle, const esDT* dt, esBL useDayOfWeek, esU8 ignoreMatchMask)
{
	esU8 tmp[5];
  int y, mn, d, h, m, s;
  tmp[0] = DS1337_SEC+DS1337_ALM1_OFFS;
  dtDecomposeDateTime(dt, &y, 0, &mn, &d, &h, &m, &s, 0);
 	tmp[1] = BYTE2esBCD(s) | ((ignoreMatchMask & 0x01) << 7);
 	tmp[2] = BYTE2esBCD(m) | ((ignoreMatchMask & 0x02) << 6);
 	tmp[3] = BYTE2esBCD(h) | ((ignoreMatchMask & 0x04) << 5);
 	tmp[4] = (useDayOfWeek ? BYTE2esBCD( dtGetDayOfWeek(y, mn, d) ) : BYTE2esBCD(d)) |
	  ((ignoreMatchMask & 0x08) << 4);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 5) == 5; 
}

esBL ds1337SetAlarm2(i2cHANDLE handle, const esDT* dt, esBL useDayOfWeek, esU8 ignoreMatchMask)
{
	esU8 tmp[4];
	int y, mn, d, h, m;
	tmp[0] = DS1337_SEC+DS1337_ALM2_OFFS;
	dtDecomposeDateTime(dt,	&y, 0, &mn, &d, &h, &m, 0, 0);
	tmp[1] = BYTE2esBCD(m) | ((ignoreMatchMask & 0x01) << 7);
	tmp[2] = BYTE2esBCD(h) | ((ignoreMatchMask & 0x02) << 6);
	tmp[3] = (useDayOfWeek ? BYTE2esBCD( dtGetDayOfWeek(y, mn, d) ) : BYTE2esBCD(d)) |
		((ignoreMatchMask & 0x04) << 5);
	return i2cPutBytes(handle, DS1337_ADDR, tmp, 4) == 4;	
}

