#ifndef _bmp_085_h_
#define _bmp_085_h_

// temperature-compensated i2c pressure sensor from bosh
//

#ifdef __cplusplus
  extern "C" {
#endif

// sensor calibration data struct
typedef struct {
  esI16 AC1;
  esI16 AC2;
  esI16 AC3;
  esU16 AC4;
  esU16 AC5;
  esU16 AC6;
  esI16 B1;
  esI16 B2;
  esI16 MB;
  esI16 MC;
  esI16 MD;

} bmp085Calibration;

// temperature measurement duration in us
#define bmp085WaitT 4500
// get maximum pressure conversion duration, in us, given desired precision
esU32 bmp085PressureWaitGet(esU8 precision);
// read sensor calibration data from chip's EEPROM
esBL bmp085CalibrationGet(i2cHANDLE h, bmp085Calibration* cal);
// start pressure conversion, but do not block until result is ready
esBL bmp05PressureStart(i2cHANDLE h, esU8 precision);
// non-blocking T conversion start
esBL bmp085TemperatureStart(i2cHANDLE h);
// blocking T conversion read. T conversion must have been previously started by bmp085StartT
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
esBL bmp085TemperatureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t );
// non-blocking T conversion read. T conversion must have been previously started by bmp085StartT
// if TRUE, returned is temperature in 0.1 celcius steps
// b5 is optional pointer to calculated value, which may be used for pressure calculations
// if not needed, just pass 0
esBL bmp085TemperatureGet(i2cHANDLE h, const bmp085Calibration* cal, long* b5, long* t );
// blocking pressure get. return FALSE if error occurred
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
esBL bmp085PressureGetBlocking(i2cHANDLE h, const bmp085Calibration* cal, esU8 precision, long* b5, long* p);
// non-blocking pressure get. return FALSE if error occurred. pressure conversion must have previously
// been started by bmp05StartP call.
// precision is value 0(lowest precision, quickiest conversion)..3(highest precision, slowest conversion)
esBL bmp085PressureGet(i2cHANDLE h, const bmp085Calibration* cal, esU8 precision, long* b5, long* p);

#ifdef __cplusplus
  }
#endif

#endif // _bmp_085_h_
