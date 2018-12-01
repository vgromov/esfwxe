#ifndef _max_4400x_h_
#define _max_4400x_h_

/// MAX4400x R+G+B+E+IR+temp sensor driver API
///
class max4400x
{
public:
  /// MAX4400x adresses, register bit masks
  ///
  enum
  {
    /// Slave address for max44006
    ///
    _6addr0             = 0x88,
    _6addr1             = 0x8A,

    /// Slave address for max44008
    ///
    _8addr0             = 0x80,
    _8addr1             = 0x82,

    /// Interrupt register bits.
    /// Reading the Interrupt Status register clears the PWRON and AMBINTS bits if set, and deasserts the INT pin (INT pin is 
    /// pulled high by the off-chip pullup resistor). The AMBINTS bits are disabled and set to 0 if the respective INTE Interrupt 
    /// Enable bits in Register 0x01 are set to 0.
    ///
    intAMBINTS          = 0x01, ///< The AMBINTS bit in the Interrupt Status register 0x00 is a read-only bit, 
                                ///< and indicates that an ambient light-interrupt condition has occurred.
                                ///< If any of these bits (PWRON, AMBINTS) are set to 1, the INT pin is pulled low.
    intPWRON            = 0x04, ///< The PWRON bit in the Interrupt Status register 0x00 is a read-only bit, 
                                ///< and if set, indicates that a power-on-reset (POR) condition has 
                                ///< occurred, and any user-programmed thresholds may not be valid anymore.
    intSHDN             = 0x08, ///< The SHDN bit in the Interrupt Status register
                                ///< is a read/write bit, and can be used to put the part into and bring out of shutdown 
                                ///< for power saving. All register data is retained during this operation.
    intRESET            = 0x10, ///< The RESET bit in the Interrupt Status register is also a read/write bit, and 
                                ///< can be used to reset all the registers back to a power-on default condition.
    
    /// Main Configuration register bits
    ///
    cfgMaskAMBINTE      = 0x01, ///< Ambient interrupt enable.
    cfgAMBINTE_ON       = 0x01, ///< 1 - Detection of ambient interrupt events is enabled (see the AMBINTS bit for more details).
    cfgAMBINTE_OFF      = 0x00, ///< 0 - The AMBINTS bit and INT pin remain unasserted even if an ambient interrupt 
                                ///< event has occurred. The AMBINTS bit is set to 0 if previously set to 1.
                                ///< An active ambient interrupt can trigger a hardware interrupt (INT pin pulled low) 
                                ///< and set the AMBINTS bit.
                                        
    cfgMaskAMBSEL       = 0x0C, ///< Ambient Interrupt Select.
    cfgAMBSEL_CLEAR     = 0x00, ///< 00 CLEAR channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
    cfgAMBSEL_GREEN     = 0x04, ///< 01 GREEN channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
    cfgAMBSEL_IR        = 0x08, ///< 10 IR channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
    cfgAMBSEL_TEMP      = 0x0C, ///< 11 TEMP channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
    
    cfgMaskMODE         = 0x30, ///< Operating mode selection
    cfgMODE_CL_T        = 0x00, ///< 00 CLEAR + TEMP* channels active
    cfgMODE_CL_IR_T     = 0x10, ///< 01 CLEAR + TEMP* + IR channels active
    cfgMODE_ALL         = 0x20, ///< 10 CLEAR + TEMP* + RGB + IR channels active
                                ///< * When TEMPEN is set to 1
    
    /// Ambient Configuration Register
    ///
    ambCfgMaskAMBPGA    = 0x03, ///< Gain of the clear/red/green/blue/IR channel measurements
    ambCfgAMBPGA_1      = 0x00, ///< No attenuation
    ambCfgAMBPGA_4      = 0x01, ///< 4x attenuation
    ambCfgAMBPGA_16     = 0x02, ///< 16x attenuation
    ambCfgAMBPGA_256    = 0x03, ///< 256x attenuation
    
    ambCfgMaskAMBTIM    = 0x1C, ///< Integration time for the red/green/blue/IR/temp channel ADC conversion (AKA exposition)
                                ///< AMBTIM[2:0] INTEGRATION TIME FULL-SCALE ADC  BIT RESOLUTION  RELATIVE LSB 
                                ///<                (ms)            (COUNTS)                      SIZE FOR FIXED AMBPGA[1:0]
    ambCfgAMBTIM_100    = 0x00, ///<  000           100               16384           14            1x
    ambCfgAMBTIM_25     = 0x04, ///<  001           25                4096            12            4x
    ambCfgAMBTIM_6_25   = 0x08, ///<  010           6.25              1024            10            16x
    ambCfgAMBTIM_1_56   = 0x0C, ///<  011           1.5625            256             8             64x
    ambCfgAMBTIM_400    = 0x10, ///<  100           400               16384           14            1/4x
    
    ambCfgMaskTEMPEN    = 0x20, ///< Temperature sensor channel enable bit.
    ambCfgTEMPEN_ON     = 0x20, ///< 1 - Enables temperature sensor.
    ambCfgTEMPEN_OFF    = 0x00, ///< 0 - Disables temperature sensor.
                                                                              
    ambCfgCOMPEN_ON     = 0x40, ///< Automatic IR compensation enable bit.
    ambCfgCOMPEN_OFF    = 0x00, ///< The integration time of compensation channel is controlled by the AMB mode settings. The compensation is enabled 
                                ///< only when the clear channel is on. When COMPEN = 1, the CLEAR data is automatically compensated for stray IR 
                                ///< leakeds and temperature variations. When COMPEN = 0, the IR compensation is disabled, but the output of the IR 
                                ///< compensation data exits.
                                ///< 0 - Disables IR compensation. 
                                ///< 1 - Enables IR compensation. Only for MODE = CfgMODE_CL_T
                                        
    ambCfgMaskTRIM      = 0x80, ///< Automatic gain adjustment on sensor overload
    ambCfgTRIM_DEFAULT  = 0x00, ///< 0 - Use factory-programmed gains for all the channels. Ignore any bytes written to TRIM_GAIN_GREEN[6:0], 
    ambCfgTRIM_CUSTOM   = 0x80, ///< TRIM_GAIN_RED[6:0], TRIM_GAIN_BLUE[6:0], TRIM_GAIN_CLEAR[6:0], and TRIM_GAIN_IR[6:0] registers. 
                                ///< 1 - Use bytes written to TRIM_GAIN_GREEN[6:0], TRIM_GAIN_RED[6:0], TRIM_GAIN_BLUE[6:0], TRIM_GAIN_CLEAR[6:0], 
                                ///< and TRIM_GAIN_IR[6:0] registers to set the gain for each channel.
                                        
    /// Ambient Threshold Persist Timer (trigger) Register
    /// Sets one of four persist values that control a time delay before the interrupt logic reacts to a detected event.
    /// This feature is added in order to reduce false or nuisance interrupts.
    ///               NO. OF CONSECUTIVE MEASUREMENTS REQUIRED TO TRIGGER AN INTERRUPT
    ambThreshDelay1     = 0x00, ///< 1
    ambThreshDelay4     = 0x01, ///< 4
    ambThreshDelay8     = 0x02, ///< 8
    ambThreshDelay16    = 0x03, ///< 16
  };

  /// All channel data. To be used in 'all' reading call
  struct AllChannels
  {
    esU16 c;
    esU16 r;
    esU16 g;
    esU16 b;
    esU16 ir;
    esU16 irComp;

    AllChannels() ESE_NOTHROW { reset(); }
    void reset() ESE_NOTHROW;
  };

public:
  max4400x(EseI2c& i2c, esU8 addr, esU32 tmo) ESE_NOTHROW;

  /// Access databus assigned at construction time
  inline EseI2c& bus() ESE_NOTHROW { return m_i2c; }

  /// MAX4400x sensor API
  ///
  /// Generic configuration
  /// 

  /// @brief Read MAX4400X interrupt register contents
  /// @param [out] intReg   Variable which receives interrupt register contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool intRead(esU8& intReg) ESE_NOTHROW;

  /// @brief Write MAX4400X interrupt register contents
  /// @param [in] intReg    Bit values to set in interrupt register
  /// @return true if operation succeeds, FALSE otherwise
  /// 
  bool intWrite(esU8 intReg) ESE_NOTHROW;

  /// @brief Main configuration register read
  /// @param [out] cfgReg   Variable which receives configuration register contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool cfgRead(esU8& cfgReg) ESE_NOTHROW;

  /// @brief Main configuration register write
  /// @param [in] cfgReg   Configuration register contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool cfgWrite(esU8 cfgReg) ESE_NOTHROW;

  /// @brief Read ambient configuration register
  /// @param [out] cfgReg   Variable which receives configuration register contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambCfgRead(esU8& cfgReg) ESE_NOTHROW;

  /// @brief Write ambient configuration register
  /// @param [in] cfgReg   Configuration register contents
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambCfgWrite(esU8 cfgReg) ESE_NOTHROW;

  /// Thresholds configuration
  ///

  /// @brief Read upper threshold value
  /// @param [out] tu       Variable that is receiving threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdUpperRead(esU16& tu) ESE_NOTHROW;

  /// @brief Write upper threshold value
  /// @param [in] tu        Threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdUpperWrite(esU16 tu) ESE_NOTHROW;

  /// @brief Read lower threshold value
  /// @param [out] tl       Variable that is receiving threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdLowerRead(esU16& tl) ESE_NOTHROW;

  /// @brief Write lower threshold value
  /// @param [in] tl        Threshold value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdLowerWrite(esU16 tl) ESE_NOTHROW;

  /// @brief Read threshold latch timer value
  /// @param [out] tt       Variable that is receiving latch timer value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdLatchTimerRead(esU8& tt) ESE_NOTHROW;

  /// @brief Write threshold latch timer value
  /// @param [in] tt        Latch timer value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool thresholdLatchTimerWrite(esU8 tt) ESE_NOTHROW;

  /// Ambient gains configuration
  /// 

  /// @brief Read clear ADC channel gain
  /// @param [out] gclear   Variable receiving gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainClearRead(esU8& gclear) ESE_NOTHROW;

  /// @brief Write clear ADC channel gain
  /// @param [in] gclear    Gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainClearWrite(esU8 gclear) ESE_NOTHROW;

  /// @brief Read red ADC channel gain
  /// @param [out] gred     Variable receiving gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainRedRead(esU8& gred) ESE_NOTHROW;

  /// @brief Write red ADC channel gain
  /// @param [in] gred      Gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainRedWrite(esU8 gred) ESE_NOTHROW;

  /// @brief Read green ADC channel gain
  /// @param [out] ggreen   Pointer to variable receiving gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainGreenRead(esU8& ggreen) ESE_NOTHROW;

  /// @brief Write green ADC channel gain
  /// @param [in] ggreen    Gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainGreenWrite(esU8 ggreen) ESE_NOTHROW;

  /// @brief Read blue ADC channel gain
  /// @param [out] gblue    Variable receiving gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainBlueRead(esU8& gblue) ESE_NOTHROW;

  /// @brief Write blue ADC channel gain
  /// @param [in] gblue     Gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainBlueWrite(esU8 gblue) ESE_NOTHROW;

  /// @brief Read IR ADC channel gain
  /// @param [out] gir      Variable receiving gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainIrRead(esU8& gir) ESE_NOTHROW;

  /// @brief Write IR ADC channel gain
  /// @param [out] gir      Gain value
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool ambGainIrWrite(esU8 gir) ESE_NOTHROW;

  /// Data channels reading
  /// 

  /// @brief Read clear channel data
  /// @param [out] clear    Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool clearRead(esU16& clear) ESE_NOTHROW;

  /// @brief Read red channel data
  /// @param [out] red      Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool redRead(esU16& red) ESE_NOTHROW;

  /// @brief Read green channel data
  /// @param [out] green    Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool greenRead(esU16& green) ESE_NOTHROW;
    
  /// @brief Read blue channel data
  /// @param [out] blue     Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool blueRead(esU16& blue) ESE_NOTHROW;

  /// @brief Read IR channel data
  /// @param [out] ir       Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool irRead(esU16& ir) ESE_NOTHROW;

  /// @brief Read IR compensation channel data
  /// @param [out] irComp   Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool irCompRead(esU16& irComp) ESE_NOTHROW;
    
  /// @brief Read temperature channel data
  /// @param [out] temp     Variable receiving data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool tempRead(esU16& temp) ESE_NOTHROW;

  /// @brief Read all channel data, except temperature
  /// @param [out] data Variable receiving read channels data
  /// @return true if operation succeeds, FALSE otherwise
  ///
  bool allRead(AllChannels& data) ESE_NOTHROW;

protected:
  // Internal IO helpers
  bool byteSet(esU8 reg, esU8 b) ESE_NOTHROW;
  bool wordSet(esU8 reg, esU16 w) ESE_NOTHROW;
  bool byteGet(esU8 reg, esU8& b) ESE_NOTHROW;
  bool wordGet(esU8 reg, esU16& w) ESE_NOTHROW;

protected:
  EseI2c& m_i2c;
  esU32 m_tmo;
  esU8 m_addr;

private:
  max4400x() ESE_REMOVE_DECL;
  max4400x(const max4400x&) ESE_REMOVE_DECL;
  max4400x& operator=(const max4400x&) ESE_REMOVE_DECL;
};
  
#endif // _max_4400x_h_
