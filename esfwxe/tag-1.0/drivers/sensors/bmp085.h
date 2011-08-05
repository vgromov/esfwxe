#ifndef _bmp_085_h_
#define _bmp_085_h_

// temperature-compensated i2c pressure sensor from bosh
//

#ifdef __cplusplus
	extern "C" {
#endif

// sensor calibration data struct
typedef struct {
	INT16 AC1;
  INT16 AC2;
  INT16 AC3;
  UINT16 AC4;
  UINT16 AC5;
  UINT16 AC6;
  INT16 B1;
  INT16 B2;
  INT16 MB;
  INT16 MC;
  INT16 MD;

} bmp085Calibration;

// temperature measurement duration in us
#define bmp085WaitT 4500
// get maximum pressure conversion duration, in us, given desired precision
DWORD bmp085PressureWaitGet(BYTE precision);
// read sensor calibration data from chip's EEPROM
BOOL bmp085CalibrationGet(i2cHANDLE h, bmp085Calibration* cal);
// start pressure conversion, but do not block until result is ready
BOOL bmp05PressureStart(i2cHANDLE h, BYTE precision);
// non-blocking T conversion start
BOOL bmp085TemperatureStart(i2cHANDLE h);
// blocking T conversion read. T conversion must have been previously started by bmp085StartT
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
BOOL bmp085TemperatureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t );
// non-blocking T conversion read. T conversion must have been previously started by bmp085StartT
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
BOOL bmp085TemperatureGet(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t );
// blocking pressure get. return FALSE if error occurred
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
BOOL bmp085PressureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, BYTE precision, long* b5, long* p);
// non-blocking pressure get. return FALSE if error occurred. pressure conversion must have previously
// been started by bmp05StartP call.
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
BOOL bmp085PressureGet(i2cHANDLE h, const bmp085Calibration* cal, BYTE precision, long* b5, long* p);

#ifdef __cplusplus
	}
#endif

#endif // _bmp_085_h_
