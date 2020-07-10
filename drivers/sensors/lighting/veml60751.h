#ifndef _veml_60751_h_
#define _veml_60751_h_

#ifdef __cplusplus
  extern "C" {
#endif

/// Consts, control structures
///

/// Integration times (exposure), to be assigned to cfg register
///
enum {
  veml60751exp50ms,
  veml60751exp100ms,
  veml60751exp200ms,
  veml60751exp400ms,
  veml60751exp800ms
};

/// Configuration register contents
///
typedef struct 
{
  esU16 m_shutdown   : 1; ///< 1 = shut down, 0 - power on
  esU16 m_continuous : 1; ///< Continuous measurement mode 0 - normal operation (trigger) 1 - continuous measurements
  esU16 m_measureOnce: 1; ///< Write 1 to initiate one-time measurement. Bit is cleared upon measurement start
  esU16 m_hdrOn      : 1; ///< High Dynamic Range mode, 0 - normal, 1 - HDR is on
  esU16 m_exposure   : 3; ///< Exposure time, ome of veml60751expXXXms values
  esU16 m_reserved   : 9;
  
} veml60751cfg;

typedef struct {
  esU8 m_devId;
  
  struct {
    esU8 m_ver      : 4; ///< Version code
    esU8 m_slaveAddr: 2; ///< Slave address (0 == 0x20)
    esU8 m_company  : 2; ///< Company code
    
  } m_chipId;
  
} veml60751id;

/// Measurement results
///
typedef struct {
  esU16 m_uva;
  esU16 m_dark;
  esU16 m_uvb;  
  esU16 m_visComp;
  esU16 m_irComp;
  
} veml60751result;

/// Calibration data
/// a,b,c,d are correction factors in 0.01 ths, found during sensor calibration in experimental setup, see application node
/// respA, respB - is responsivity, in UV 0.01 ths per count, respDiv is resp scaling factor, to provide required accuracy
///
typedef struct {
  esI32 m_a;
  esI32 m_b;
  esI32 m_c;
  esI32 m_d;
  esI32 m_respA;
  esI32 m_respB;
  esI32 m_respDiv;

} veml60751calibration;

/// API
///

esBL veml60751idRead(i2cHANDLE hi2c, veml60751id* id);

esBL veml60751cfgWrite(i2cHANDLE hi2c, const veml60751cfg* cfg);
esBL veml60751cfgRead(i2cHANDLE hi2c, veml60751cfg* cfg);

esBL veml60751uvaRead(i2cHANDLE hi2c, esU16* uva);
esBL veml60751darkRead(i2cHANDLE hi2c, esU16* dark);
esBL veml60751uvbRead(i2cHANDLE hi2c, esU16* uvb);
esBL veml60751visCompRead(i2cHANDLE hi2c, esU16* visComp);
esBL veml60751irCompRead(i2cHANDLE hi2c, esU16* irComp);

esBL veml60751resultRead(i2cHANDLE hi2c, veml60751result* result);

/// Calculate UV index in 0.01 ths
///
esU32 veml60751uvCalculate(const veml60751result* data, const veml60751cfg* cfg, const veml60751calibration* cal);

/// Return ms from exposure value
esU32 __inline veml60751exposureMsGet(esU32 exp)
{
  switch(exp)
  {
  case veml60751exp50ms:
    return 50;
  case veml60751exp100ms:
    return 100;
  case veml60751exp200ms:
    return 200;
  case veml60751exp400ms:
    return 400;
  case veml60751exp800ms:
    return 800;
  default:
    return 50;
  }
}

#ifdef __cplusplus
  }
#endif

#endif // _veml_60751_h_
