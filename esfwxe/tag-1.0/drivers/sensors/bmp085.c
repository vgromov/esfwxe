#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/core/twi_i2c.h>
#include "bmp085.h"

// misc constants
enum {
 // BMP085 sensor I2C address
	bmp085Addr	 = 0xEE,
	// registers
	bmp085ConvRequestReg = 0xF4,
	bmp085ConvResultReg = 0xF6,
	// control data
	bmp085UtRequest = 0x2E,
	bmp085UpRequest = 0x34,
};

// up wait in us, depending on precision
static const DWORD c_bmp085UpWait[4] = {
	4500,
	7500,
	13500,
	25500
};

// read sensor calibration data from chip's EEPROM
BOOL bmp085CalibrationGet(i2cHANDLE h, bmp085Calibration* cal)
{
	BYTE calReg = 0xAA; // calibration table start register
	// read calibration
	if( 1 == i2cPutBytes(h, bmp085Addr, &calReg, 1) &&
		sizeof(bmp085Calibration) == i2cGetBytes(h, bmp085Addr, (BYTE*)cal, sizeof(bmp085Calibration)) )
	{
		// convert to little-endian format
		cal->AC1 = SWAPB_WORD(cal->AC1);
		cal->AC2 = SWAPB_WORD(cal->AC2);
		cal->AC3 = SWAPB_WORD(cal->AC3);
		cal->AC4 = SWAPB_WORD(cal->AC4);
		cal->AC5 = SWAPB_WORD(cal->AC5);
		cal->AC6 = SWAPB_WORD(cal->AC6);
		cal->B1 = SWAPB_WORD(cal->B1);
		cal->B2 = SWAPB_WORD(cal->B2);
		cal->MB = SWAPB_WORD(cal->MB);
		cal->MC = SWAPB_WORD(cal->MC);
		cal->MD = SWAPB_WORD(cal->MD);

		return TRUE;
	}

	return FALSE;
}

// non-blocking T conversion start
BOOL bmp085TemperatureStart(i2cHANDLE h)
{
	BYTE tmp[2];
	tmp[0] = bmp085ConvRequestReg;
	tmp[1] = bmp085UtRequest;
	return 2 == i2cPutBytes(h, bmp085Addr, tmp, 2); // start temperature conversion
}

// non-blocking read uncompensated temperature
static BOOL bmp085ReadUt(i2cHANDLE h, long* ut)
{
	BYTE tmp[2];
	// request result	
	tmp[0] = bmp085ConvResultReg;
	if( 1 == i2cPutBytes(h, bmp085Addr, tmp, 1) &&
			2 == i2cGetBytes(h, bmp085Addr, tmp, 2) )
	{
		*ut = ((long)tmp[0] << 8) + (long)tmp[1];
		return TRUE;
	}

	return FALSE;
}

// blocking read uncompensated temperature
static BOOL bmp085ReadUtBlocking(i2cHANDLE h, long* ut)
{
	if( bmp085TemperatureStart(h) ) // start temperature conversion
	{
		// wait 4.5 ms
		usDelay(bmp085WaitT);
		// request result	
		return bmp085ReadUt(h, ut);
	}

	return FALSE;
}

// calculate temperature
static void bmp085CalcT(const bmp085Calibration* cal, long ut, long* b5, long* t)
{
	long x1 ,x2;		
	// calculate true temperature
	x1 = ((ut - (long)cal->AC6) * (long)cal->AC5) >> 15;
	x2 = ((long)cal->MC << 11)/(x1 + cal->MD);
	*t = ((x1 + x2 + 8) >> 4);	// temperature in 0.1 C
	
	if( b5 )
		*b5 = x1 + x2;
}

// non-blocking T conversion read. T conversion must have been previously started by bmp085StartT
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
BOOL bmp085TemperatureGet(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t )
{
	long ut;
	// read uncompensated temperature
	if( bmp085ReadUt(h, &ut) )
	{
		bmp085CalcT(cal, ut, b5, t);

		return TRUE;
	}

	return FALSE;
} 

// blocking T conversion read. 
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
BOOL bmp085TemperatureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t )
{
	long ut;
	// read uncompensated temperature
	if( bmp085ReadUtBlocking(h, &ut) )
	{
		bmp085CalcT(cal, ut, b5, t);

		return TRUE;
	}

	return FALSE;
}

static __inline BYTE bmp085RestrictPrecision(BYTE precision)
{
	return (precision > 3) ? 3 : precision;
}

// get maximum pressure conversion duration, given desired precision
DWORD bmp085PressureWaitGet(BYTE precision)
{
	return c_bmp085UpWait[ bmp085RestrictPrecision(precision) ];
}

// start pressure conversion, but do not block until result is ready
BOOL bmp05PressureStart(i2cHANDLE h, BYTE precision)
{
	BYTE tmp[2];

	precision = bmp085RestrictPrecision(precision);
	// request up
	tmp[0] = bmp085ConvRequestReg;
	tmp[1] = bmp085UpRequest + (precision << 6);
	return 2 == i2cPutBytes(h, bmp085Addr, tmp, 2);
}

// read uncompensated pressure
static BOOL bmp085ReadUp(i2cHANDLE h, BYTE precision, unsigned long* up)
{
	BYTE tmp[3];

	// read conversion results
	tmp[0] = bmp085ConvResultReg;
	if( 1 == i2cPutBytes(h, bmp085Addr, tmp, 1) &&
			3 == i2cGetBytes(h, bmp085Addr, tmp, 3) )
	{
		*up = ((((unsigned long)tmp[0] << 16) + ((unsigned long)tmp[1] << 8) + (unsigned long)tmp[2]) >> 
			(8 - precision));
			 
		return TRUE;
	}

	return FALSE;
}

// blocking read uncompensated pressure
static BOOL bmp085ReadUpBlocking(i2cHANDLE h, BYTE precision, unsigned long* up)
{
	// request up
	if( bmp05PressureStart(h, precision) )
	{
		// wait for conversion to end
		usDelay( c_bmp085UpWait[precision] );
		
		return bmp085ReadUp(h, precision, up);
	}

	return FALSE;
}

// calculate pressure in pa. return FALSE if error occurred
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
static void bmp085PressureCalc(i2cHANDLE h, const bmp085Calibration* cal, BYTE precision, long* b5, unsigned long up, long* p)
{
	// calculate true pressure
	long pp, x1, x2, x3, b3, b6;
 	unsigned long b4, b7;
 
 	b6 = *b5 - 4000;
	// calc b3
 	x1 = (((b6*b6) >> 12) * (long)cal->B2 ) >> 11;
	x2 = (((long)cal->AC2)*b6) >> 11;
	x3 = x1+x2;
	b3 = (((((long)cal->AC1 )*4 + x3) << precision) + 2) >> 2;
	// calc b4
 	x1 = ((long)cal->AC3 * b6) >> 13;
 	x2 = ((long)cal->B1 * ((b6*b6) >> 12) ) >> 16;
 	x3 = ((x1 + x2) + 2) >> 2;
 	b4 = ((unsigned long)cal->AC4 * (unsigned long)(x3 + 32768)) >> 15;
	b7 = ((unsigned long)(up - b3) * (50000 >> precision)); 	
 	if(b7 < 0x80000000)
		pp = (b7 << 1) / b4;
	else
		pp = (b7 / b4) << 1;
 	x1 = (pp*pp) >> 16;
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * pp) >> 16;
	pp += (x1 + x2 + 3791) >> 4;
	*p = pp;
}

// blocking pressure get. return FALSE if error occurred
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
BOOL bmp085PressureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, BYTE precision, long* b5, long* p)
{
	unsigned long up = 0;
	precision = bmp085RestrictPrecision(precision);

	if( bmp085ReadUpBlocking(h, precision, &up) )
	{
		bmp085PressureCalc(h, cal, precision, b5, up, p);
		return TRUE;
	}

	return FALSE;
}

// non-blocking pressure get. return FALSE if error occurred. pressure conversion must have previously
// been started by bmp05StartP call.
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
BOOL bmp085PressureGet(i2cHANDLE h, const bmp085Calibration* cal, BYTE precision, long* b5, long* p)
{
	unsigned long up = 0;
	precision = bmp085RestrictPrecision(precision);

	if( bmp085ReadUp(h, precision, &up) )
	{
		bmp085PressureCalc(h, cal, precision, b5, up, p);
		return TRUE;
	}

	return FALSE;
}
