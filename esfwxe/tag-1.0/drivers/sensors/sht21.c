#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/crc.h>
#include <common/core/twi_i2c.h>
#include "sht21.h"

// consts
enum {
	// chip I2C address
	sht21Addr						= 0x80,
	// control codes
	sht21StartT 				= 0xF3, // start T measurements in normal I2C mode (no hold master) 
  sht21StartRh 				= 0xF5, // start RH measurement	in normal I2C mode (no hold master)
	sht21UserWrite 			= 0xE6, // write user register 
	sht21UserRead 			= 0xE7, // read user register  
	sht21Softreset			= 0xFE, // perform chip reset
	// status bits and mask
	sht21StatRh					= 0x02,
	sht21StatMask				= 0x03,
	// soft reset duration in us
	sht21softResetWait	= 15000,
};

// temperature conversion wait times, in us, depending on precision
static const DWORD c_sht21waitT[4] = {
	11000,
	22000,
	43000,
	85000
};

// humidity conversion wait times, in us, depending on precision
static const DWORD c_sht21waitRh[4] = {
	4000,
	9000,
	15000,
	29000
};

// temperature precision masks
static const BYTE c_sht21precT[4] = {
	0x81,	// 11 bits
	0x01,	// 12 bits
	0x80,	// 13 bits
	0x00	// 14 bits	
};

// humidity precision masks
static const BYTE c_sht21precRh[4] = {
	0x01,	// 8 bits
	0x80,	// 10 bits
	0x81,	// 11 bits
	0x00	// 12 bits	
};

// sht21 api
//
BOOL sht21SoftReset(i2cHANDLE h)
{
	BYTE tmp = sht21Softreset;
	if( 1 == i2cPutBytes(h, sht21Addr, &tmp, 1) )
	{
		// wait for sht21 chip to reset
		usDelay(sht21softResetWait);
		return TRUE;
	}

	return FALSE;
}

BOOL sht21UserRegisterRead(i2cHANDLE h, BYTE* ur)
{
	BYTE tmp = sht21UserRead;
	return 	1 == i2cPutBytes(h, sht21Addr, &tmp, 1) &&
					1 == i2cGetBytes(h, sht21Addr, ur, 1);
}

BOOL sht21UserRegisterWrite(i2cHANDLE h, BYTE ur)
{
	BYTE tmp[2];
	tmp[0] = sht21UserWrite;
	tmp[1] = ur;
	return 2 == i2cPutBytes(h, sht21Addr, tmp, 2);	
}

static __inline BYTE sht21RestrictPrecision(BYTE precision)
{
	if( precision > 3 )
		precision = 3;
	
	return precision;
}

// return estimated max wait for conversion, in us, depending on precision value
DWORD sht21TemperatureMaxWaitGet(BYTE precision)
{
	precision = sht21RestrictPrecision(precision);
	
	return c_sht21waitT[precision];
}

DWORD sht21HumidityMaxWaitGet(BYTE precision)
{
	precision = sht21RestrictPrecision(precision);
	
	return c_sht21waitRh[precision];
}

// start conversion. precision is 0..3, from least precise, to the most precise one
BOOL sht21TemperatureStart(i2cHANDLE h, BYTE precision)
{
	BYTE ur;
	BYTE cmd = sht21StartT;
	precision = sht21RestrictPrecision(precision);
	// configure user register for temperature conversion
	if( sht21UserRegisterRead(h, &ur) )
	{
		// reset existing conversion precision bits
		ur &=	~sht21MsrResolutionMask;
		// set the new ones
		ur |= c_sht21precT[precision];
		// write user register back and start T conversion
		return sht21UserRegisterWrite(h, ur) &&
			1 == i2cPutBytes(h, sht21Addr, &cmd, 1);
	}
	
	return FALSE;
}

BOOL sht21HumidityStart(i2cHANDLE h, BYTE precision)
{
	BYTE ur;
	BYTE cmd = sht21StartRh;
	precision = sht21RestrictPrecision(precision);
	// configure user register for humidity conversion
	if( sht21UserRegisterRead(h, &ur) )
	{
		// reset existing conversion precision bits
		ur &=	~sht21MsrResolutionMask;
		// set the new ones
		ur |= c_sht21precRh[precision];
		// write user register back and start Rh conversion
		return sht21UserRegisterWrite(h, ur) &&
			1 == i2cPutBytes(h, sht21Addr, &cmd, 1);
	}
	
	return FALSE;
}

// read conversion result. additional optional flag isHumidity
// signals result of which conversion, T or Rh, was acquired.
// result is either relative humidity, in 10ths of %
// or temperature, in 100ths of Celcius
BOOL sht21ResultGet(i2cHANDLE h, long* result, BOOL* isHumidity)
{
	BYTE tmp[3];
	if( 3 == i2cGetBytes(h, sht21Addr, tmp, 3) )
	{
		if( 0 == crc8(0, tmp, 3) )
		{
			long raw;
			BOOL isH = sht21StatRh == (tmp[1] & sht21StatRh);
			// clear status bits from LSB 
			tmp[1] &= ~sht21StatMask; 
			raw = ((long)tmp[0] << 8) + (long)tmp[1];
			// calculate result
		 	if( isH )
				// humidity, in 10ths of percent				
				*result = -60 + ((1250*raw) >> 16);	
			else
				// temperature, in 100ths of celcius
				*result = -4685 + ((17572*raw) >> 16); 

			if( isHumidity )
				*isHumidity = isH;

			return TRUE;
		}
	}
	
	return FALSE;
}

// blocking measurements of T and Rh
BOOL sht21TemperatureGetBlocking(i2cHANDLE h, BYTE precision, long* result)
{
	if( sht21TemperatureStart(h, precision) )
	{
		usDelay( c_sht21waitT[precision] );
		return sht21ResultGet(h, result, 0);
	}

	return FALSE;
}

BOOL sht21HumidityGetBlocking(i2cHANDLE h, BYTE precision, long* result)
{
	if( sht21HumidityStart(h, precision) )
	{
		usDelay( c_sht21waitRh[precision] );
		return sht21ResultGet(h, result, 0);
	}

	return FALSE;
}
