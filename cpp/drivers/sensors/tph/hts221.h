#ifndef _hts_221_h_
#define _hts_221_h_

/// HTS221 sensor driver
/// Capacitive digital sensor for relative humidity and temperature
///
class HTS221 : public EseI2cSlaveDevice
{
public:
  /// ID value
  ///
  enum ID {
    none          = 0,
    whoAmI        = 0xBC
  };
  
  /// Applicable I2C addresses
  ///
  enum i2cAddr {
    _0xBE         = 0xBE
  };
  
  /// Internal value averaging configuration codes
  ///
  enum avgCfgT {
    T2,
    T4,
    T8,
    T16,
    T32,
    T64,
    T128,
    T256
  };

  enum avgCfgH {
    H4,
    H8,
    H16,
    H32,
    H64,
    H128,
    H256,
    H512
  };
  
  /// Data rate configurations
  ///
  enum dataRate {
    oneShot,
    _1Hz,
    _7Hz,
    _12_5Hz
  };
  
  /// Control register 1
  ///
  class ctl1
  {
  public:
    inline dataRate rateGet() const ESE_NOTHROW { return static_cast<dataRate>(m_rate); }
    inline void rateSet(HTS221::dataRate rate) ESE_NOTHROW { m_rate = static_cast<uint8_t>(rate) & 0x03; }
    
    inline bool dataUpdateBlockGet() const ESE_NOTHROW { return m_blockDataUpdate; }
    inline void dataUpdateBlockSet(bool block) ESE_NOTHROW { m_blockDataUpdate = (block ? 1 : 0); }

    inline bool pwrDownGet() const ESE_NOTHROW { return 0 == m_pwrDown; }
    inline void pwrDownSet(bool down) ESE_NOTHROW { m_pwrDown = (down ? 0 : 1); }
    
  protected:
    uint8_t m_rate              : 2;  ///< Data sampling rate, as in dataRate
    uint8_t m_blockDataUpdate   : 1;  ///< Block (1) /allow (0) Low/High data registers bytes update during data registers read
    uint8_t m_reserved          : 4;
    uint8_t m_pwrDown           : 1;  ///< 0 - Power down mode, 1 - sensor is active
  };
  
  /// Control register 2
  ///
  class ctl2
  {
  public:
    inline bool oneShotGet() const ESE_NOTHROW { return m_oneShot; }
    inline void oneShotStart() ESE_NOTHROW { m_oneShot = 1; }
    
    inline bool heaterIsOn() const ESE_NOTHROW { return 1 == m_heaterOn; }
    inline void heaterOn(bool on) ESE_NOTHROW { m_heaterOn = (on ? 1 : 0); }

    inline bool isNormalMode() const ESE_NOTHROW { return 0 == m_boot; }
    inline void normalModeSet(bool normal) ESE_NOTHROW { m_boot = (normal ? 0 : 1); }
    
  protected:
    uint8_t m_oneShot           : 1;  ///< One shot measurement start (1), waiting for new convertion (0)
    uint8_t m_heaterOn          : 1;  ///< Enable (1) or disable (0) internal heater to recover from condensation, proceed with normal measurements
    uint8_t m_reserved          : 5;
    uint8_t m_boot              : 1;  ///< 0 - Normal operation mode, 1 - request sensor reset, factory calibration data re-read from ROM
  };
  
  /// Control register 3
  ///
  class ctl3
  {
  public:
    inline bool dataReadyOutIsEnabled() const ESE_NOTHROW { return 1 == m_drdyEnable; }
    inline void dataReadyOutEnable(bool enable) ESE_NOTHROW { m_drdyEnable = (enable ? 1 : 0); }
    
    inline bool dataReadyOutIsOD() const ESE_NOTHROW { return 1 == m_drdyOD; }
    inline void dataReadyOutODset(bool od) ESE_NOTHROW { m_drdyOD = (od ? 1 : 0); }

    inline bool dataReadyOutIsActiveLow() const ESE_NOTHROW { return 1 == m_drdyActLow; }
    inline void dataReadyOutActiveLowSet(bool low) ESE_NOTHROW { m_drdyActLow = (low ? 1 : 0); }
    
  protected:
    uint8_t m_reserved0         : 2;
    uint8_t m_drdyEnable        : 1;  ///< Data ready state output on pin 3 is enabled (1), or disabled (0)
    uint8_t m_reserved1         : 3;
    uint8_t m_drdyOD            : 1;  ///< Data ready pin 3 is OD (1), or PP (0)
    uint8_t m_drdyActLow        : 1;  ///< Data ready output pin is active low (1) or high (0)
  };
  
  /// Status register
  ///
  class status
  {
  public:
    inline bool temperatureIsValid() const ESE_NOTHROW { return 1 == m_tempIsOk; }
    inline bool humidityIsValid() const ESE_NOTHROW { return 1 == m_hIsOk; }
  
  protected:
    uint8_t m_tempIsOk          : 1;
    uint8_t m_hIsOk             : 1;
    uint8_t m_reserved          : 6;
  };
  
  /// Calibration data block
  ///
  class calibration
  {
  public:
    /// @brief Calculate calibrated temperature value in deg C from the raw one
    /// @param [in] raw   Raw temperature value from sensor
    /// @return           Calculated temperature value.
    ///
    float temperatureCalculate(
      int raw
#if defined(ESE_HTS221_USE_MATHINTF)
      , const EseMathIntf& imath
#endif
    ) const ESE_NOTHROW;
    
    /// @brief Calculate calibrated humidity value in % from the raw one
    /// @param [in] raw   Raw humidity value from sensor.
    /// @return           Calculated humidity value.
    ///
    float humidityCalculate(
      int raw
#if defined(ESE_HTS221_USE_MATHINTF)
      , const EseMathIntf& imath
#endif
    ) const ESE_NOTHROW;
    
  protected:
    int m_H0_rH_x2;
    int m_H1_rH_x2;
    int m_H0_T0_out;
    int m_H1_T0_out;
    int m_T0_degC_x8;
    int m_T1_degC_x8;
    int m_T0_out;
    int m_T1_out;
    
    friend class HTS221;
  };
  
public:
  HTS221() ESE_NOTHROW {}
  HTS221(EseChannelIntf& i2c, HTS221::i2cAddr addr, esU32 tmo) ESE_NOTHROW;

  /// HTS221 sensor API
  ///
  
  /// @brief Read "WHO AM I" ID byte.
  /// @param [out] id   Variable which receives "WHO AM I" ID byte
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   ID byte value should not be considered valid.
  ///
  bool idRead(HTS221::ID& id) ESE_NOTHROW;
  
  /// @brief Read configuration of internal averaging engine
  /// @param [out] cfgT Variable which receives avg control value for temperature
  /// @param [out] cfgH Variable which receives avg control value for humidity
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   control value should not be considered valid.
  ///
  bool avgCtlRead(HTS221::avgCfgT& cfgT, HTS221::avgCfgH& cfgH) ESE_NOTHROW;

  /// @brief Write configuration of internal averaging engine
  /// @param [in] cfgT  Avgeraging control value for temperature
  /// @param [in] cfgH  Avgeraging control value for humidity
  /// @return           true if operation succeeds, false otherwise.
  ///
  bool avgCtlWrite(HTS221::avgCfgT cfgT, HTS221::avgCfgH cfgH) ESE_NOTHROW;
  
  /// @brief Read control register 1
  /// @param [out] ctl  Variable which receives control register 1 value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   control register value should not be considered valid.
  ///
  bool ctlRead(HTS221::ctl1& ctl) ESE_NOTHROW;
  
  /// @brief Write control register 1
  /// @param [in] ctl   Control register 1 value
  /// @return           true if operation succeeds, false otherwise.
  ///
  bool ctlWrite(const HTS221::ctl1& ctl) ESE_NOTHROW;

  /// @brief Read control register 2
  /// @param [out] ctl  Variable which receives control register 2 value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   control register value should not be considered valid.
  ///
  bool ctlRead(HTS221::ctl2& ctl) ESE_NOTHROW;
  
  /// @brief Write control register 2
  /// @param [in] ctl   Control register 2 value
  /// @return           true if operation succeeds, false otherwise.
  ///
  bool ctlWrite(const HTS221::ctl2& ctl) ESE_NOTHROW;

  /// @brief Read control register 3
  /// @param [out] ctl  Variable which receives control register 3 value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   control register value should not be considered valid.
  ///
  bool ctlRead(HTS221::ctl3& ctl) ESE_NOTHROW;
  
  /// @brief Write control register 3
  /// @param [in] ctl   Control register 3 value
  /// @return           true if operation succeeds, false otherwise.
  ///
  bool ctlWrite(const HTS221::ctl3& ctl) ESE_NOTHROW;
  
  /// @brief Read status register
  /// @param [out] stat Variable which receives status register value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   status value should not be considered valid.
  ///
  bool statusRead(HTS221::status& stat) ESE_NOTHROW;
  
  /// @brief Read raw temperature data
  /// @param [out] raw  Variable which receives raw temperature value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   temperature value should not be considered valid.
  ///
  bool temperatureReadRaw(int& raw) ESE_NOTHROW;

  /// @brief Read raw humidity data
  /// @param [out] raw  Variable which receives raw humidity value
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   humidity value should not be considered valid.
  ///
  bool humidityReadRaw(int& raw) ESE_NOTHROW;
  
  /// @brief Read sensor calibration data block
  /// @param [out] cal  Variable which receives sensor calibration data.
  /// @return           true if operation succeeds, false otherwise. If false is returned, 
  ///                   calibration data should not be considered valid.
  ///
  bool calibrationRead(HTS221::calibration& cal) ESE_NOTHROW;
  
protected:
  // Internal helpers
  bool registerRead(uint8_t reg, uint8_t len, uint8_t* out) ESE_NOTHROW;
  bool registerWrite(uint8_t reg, uint8_t len, const uint8_t* in) ESE_NOTHROW;

  ESE_NONCOPYABLE(HTS221);
};

#endif //< _hts_221_h_
