#ifndef _sht_21_h_
#define _sht_21_h_

#ifdef __cplusplus
	extern "C" {
#endif

// driver for Sensitron SHT21 humidity & temperature sensor chip
//
enum {
	// user register bits
	sht21DisableOtpReload 	= 0x02,
	sht21EnableOnchipHeater	= 0x04,
	sht21EndOfBattLess225		= 0x40,
	sht21MsrResolutionMask	= 0x81,
};

// sht21 api
//
BOOL sht21SoftReset(i2cHANDLE h);
BOOL sht21UserRegisterRead(i2cHANDLE h, BYTE* ur);
BOOL sht21UserRegisterWrite(i2cHANDLE h, BYTE ur);
// return estimated max wait for conversion, in us, depending on precision value
DWORD sht21TemperatureMaxWaitGet(BYTE precision);
DWORD sht21HumidityMaxWaitGet(BYTE precision);
// start conversion. precision is 0..3, from least precise, to the most precise one
BOOL sht21TemperatureStart(i2cHANDLE h, BYTE precision);
BOOL sht21HumidityStart(i2cHANDLE h, BYTE precision);
// read conversion result. additional optional flag isHumidity
// signals result of which conversion, T or Rh, was acquired
// result is either relative humidity, in 10ths of %
// or temperature, in 100ths of Celcius
BOOL sht21ResultGet(i2cHANDLE h, long* result, BOOL* isHumidity);
// blocking measurements of T and Rh
BOOL sht21TemperatureGetBlocking(i2cHANDLE h, BYTE precision, long* result);
BOOL sht21HumidityGetBlocking(i2cHANDLE h, BYTE precision, long* result);

#ifdef __cplusplus
	}
#endif

#endif // _sht_21_h_
