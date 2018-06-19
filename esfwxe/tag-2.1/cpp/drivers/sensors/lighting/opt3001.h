#ifndef _opt_3001_h_
#define _opt_3001_h_

/// OPT3001 White lighting digital sensor with I2C interface
///
class opt3001
{
public:
  /// ID constants
  enum {
    idMfg = 0x5449,
    idDev = 0x3001
  };

  /// Applicable I2C addresses
  ///
  enum i2cAddr {
    _0x88 = 0x88,
    _0x8A = 0x8A,
    _0x8C = 0x8C,
    _0x8E = 0x8E
  };

  /// Sensor Configuration
  ///
  union Config
  {
    esU16 m_raw;
    struct Data {
        esU16 m_faultCnt  : 2;
        esU16 m_rangeMask : 1;
        esU16 m_intPol    : 1;
        esU16 m_intLatch  : 1; 
        esU16 m_lowEvt    : 1;
        esU16 m_highEvt   : 1;
        esU16 m_ready     : 1; 
        esU16 m_overflow  : 1;
        esU16 m_mod       : 2;
        esU16 m_ct        : 1;
        esU16 m_range     : 4;
        
    } m_data;
  };
  
  /// Sensor Value
  union Value
  {
    esU16 m_raw;
    
    struct Data {
      esU16 m_val   : 12;
      esU16 m_range : 4;
      
    } m_data;
  };
  
  /// Sensor ID
  struct ID
  {
    ID() ESE_NOTHROW :
    m_mfg(0),
    m_dev(0)
    {}
  
    esU16 m_mfg;
    esU16 m_dev;
  };
  
public:
  opt3001(EseI2c& i2c, opt3001::i2cAddr addr, esU32 tmo) ESE_NOTHROW;

  /// Access databus assigned at construction time
  inline EseI2c& bus() ESE_NOTHROW { return m_i2c; }

  /// OPT3001 sensor API
  ///
  
  /// Convert OPT3001 raw Value to float representation
  static esF rawValToVal(const opt3001::Value& val) ESE_NOTHROW;
  
  /// @brief Configuration register read
  /// @param [out] cfg   Variable which receives configuration contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool cfgRead(opt3001::Config& cfg) ESE_NOTHROW;

  /// @brief Configuration register write
  /// @param [in] cfg   Configuration data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool cfgWrite(const opt3001::Config& cfg) ESE_NOTHROW;

  /// @brief Read result data
  /// @param [out] result   Variable which receives result
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool resultRead(opt3001::Value& result) ESE_NOTHROW;

  /// Thresholds configuration
  ///

  /// @brief Read upper threshold value
  /// @param [out] tu       Variable that is receiving threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool limitHighRead(opt3001::Value& tu) ESE_NOTHROW;

  /// @brief Write upper threshold value
  /// @param [in] tu        Threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool limitHighWrite(const opt3001::Value& tu) ESE_NOTHROW;

  /// @brief Read lower threshold value
  /// @param [out] tl       Variable that is receiving threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool limitLowRead(opt3001::Value& tl) ESE_NOTHROW;

  /// @brief Write lower threshold value
  /// @param [in] tl        Threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool limitLowWrite(const opt3001::Value& tl) ESE_NOTHROW;

  /// @brief Read ID information
  /// @param [out] tt       Variable that is receiving ID info
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool idRead(opt3001::ID& id) ESE_NOTHROW;
  
protected:
  /// Internal helpers
  ///
 
  /// 16 bit Register read
  bool regRead(esU8 reg, esU16& data) ESE_NOTHROW;
  
  /// 16 bit register write
  bool regWrite(esU8 reg, esU16 data) ESE_NOTHROW;

protected:
  EseI2c& m_i2c;
  esU32 m_tmo;
  esU8 m_addr;    
    
private:
  // Prohibited functionality
  opt3001() ESE_REMOVE_DECL;
  opt3001(const opt3001&) ESE_REMOVE_DECL;
  opt3001& operator=(const opt3001&) ESE_REMOVE_DECL;
};


#endif // _opt_3001_h_
