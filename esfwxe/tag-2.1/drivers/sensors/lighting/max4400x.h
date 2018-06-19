#ifndef _max_4400x_h_
#define _max_4400x_h_

/// MAX4400x R+G+B+E+IR+temp sensor driver API
///

#ifdef __cplusplus
	extern "C" {
#endif

/// MAX4400x adresses, register bit masks
///
enum
{
  /// Slave address for max44006
  ///
  max44006addr0 							= 0x88,
  max44006addr1 							= 0x8A,

  /// Slave address for max44008
  ///
  max44008addr0 							= 0x80,
  max44008addr1 							= 0x82,

  /// Interrupt register bits.
  /// Reading the Interrupt Status register clears the PWRON and AMBINTS bits if set, and deasserts the INT pin (INT pin is 
  /// pulled high by the off-chip pullup resistor). The AMBINTS bits are disabled and set to 0 if the respective INTE Interrupt 
  /// Enable bits in Register 0x01 are set to 0.
  ///
  max4400xIntAMBINTS          = 0x01, ///< The AMBINTS bit in the Interrupt Status register 0x00 is a read-only bit, 
                                      ///< and indicates that an ambient light-interrupt condition has occurred.
                                      ///< If any of these bits (PWRON, AMBINTS) are set to 1, the INT pin is pulled low.
  max4400xIntPWRON            = 0x04, ///< The PWRON bit in the Interrupt Status register 0x00 is a read-only bit, 
                                      ///< and if set, indicates that a power-on-reset (POR) condition has 
                                      ///< occurred, and any user-programmed thresholds may not be valid anymore.
  max4400xIntSHDN             = 0x08, ///< The SHDN bit in the Interrupt Status register
                                      ///< is a read/write bit, and can be used to put the part into and bring out of shutdown 
                                      ///< for power saving. All register data is retained during this operation.
  max4400xIntRESET            = 0x10, ///< The RESET bit in the Interrupt Status register is also a read/write bit, and 
                                      ///< can be used to reset all the registers back to a power-on default condition.
  
  /// Main Configuration register bits
  ///
  max4400xCfgMaskAMBINTE			= 0x01, ///< Ambient interrupt enable.
	max4400xCfgAMBINTE_ON       = 0x01, ///< 1 - Detection of ambient interrupt events is enabled (see the AMBINTS bit for more details).
  max4400xCfgAMBINTE_OFF      = 0x00, ///< 0 - The AMBINTS bit and INT pin remain unasserted even if an ambient interrupt 
                                      ///< event has occurred. The AMBINTS bit is set to 0 if previously set to 1.
                                      ///< An active ambient interrupt can trigger a hardware interrupt (INT pin pulled low) 
                                      ///< and set the AMBINTS bit.
																			
  max4400xCfgMaskAMBSEL       = 0x0C, ///< Ambient Interrupt Select.
  max4400xCfgAMBSEL_CLEAR     = 0x00, ///< 00 CLEAR channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
  max4400xCfgAMBSEL_GREEN     = 0x04, ///< 01 GREEN channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
  max4400xCfgAMBSEL_IR        = 0x08, ///< 10 IR channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
  max4400xCfgAMBSEL_TEMP      = 0x0C, ///< 11 TEMP channel data is used to compare with ambient interrupt thresholds and ambient timer settings.
  
  max4400xCfgMaskMODE         = 0x30, ///< Operating mode selection
  max4400xCfgMODE_CL_T        = 0x00, ///< 00 CLEAR + TEMP* channels active
  max4400xCfgMODE_CL_IR_T     = 0x10, ///< 01 CLEAR + TEMP* + IR channels active
  max4400xCfgMODE_ALL         = 0x20, ///< 10 CLEAR + TEMP* + RGB + IR channels active
                                      ///< * When TEMPEN is set to 1
  
  /// Ambient Configuration Register
  ///
  max4400xAmbCfgMaskAMBPGA    = 0x03, ///< Gain of the clear/red/green/blue/IR channel measurements
  max4400xAmbCfgAMBPGA_1      = 0x00, ///< No attenuation
  max4400xAmbCfgAMBPGA_4      = 0x01, ///< 4x attenuation
  max4400xAmbCfgAMBPGA_16     = 0x02, ///< 16x attenuation
  max4400xAmbCfgAMBPGA_256    = 0x03, ///< 256x attenuation
  
  max4400xAmbCfgMaskAMBTIM    = 0x1C, ///< Integration time for the red/green/blue/IR/temp channel ADC conversion (AKA exposition)
                                      ///< AMBTIM[2:0] INTEGRATION TIME FULL-SCALE ADC  BIT RESOLUTION  RELATIVE LSB 
                                      ///<                (ms)            (COUNTS)                      SIZE FOR FIXED AMBPGA[1:0]
  max4400xAmbCfgAMBTIM_100    = 0x00, ///<  000           100               16384           14            1x
  max4400xAmbCfgAMBTIM_25     = 0x04, ///<  001           25                4096            12            4x
  max4400xAmbCfgAMBTIM_6_25   = 0x08, ///<  010           6.25              1024            10            16x
  max4400xAmbCfgAMBTIM_1_56   = 0x0C, ///<  011           1.5625            256             8             64x
  max4400xAmbCfgAMBTIM_400    = 0x10, ///<  100           400               16384           14            1/4x
  
	max4400xAmbCfgMaskTEMPEN		= 0x20, ///< Temperature sensor channel enable bit.
  max4400xAmbCfgTEMPEN_ON     = 0x20, ///< 1 - Enables temperature sensor.
  max4400xAmbCfgTEMPEN_OFF    = 0x00, ///< 0 - Disables temperature sensor.
                                                                            
  max4400xAmbCfgCOMPEN_ON     = 0x40, ///< Automatic IR compensation enable bit.
  max4400xAmbCfgCOMPEN_OFF    = 0x00, ///< The integration time of compensation channel is controlled by the AMB mode settings. The compensation is enabled 
                                      ///< only when the clear channel is on. When COMPEN = 1, the CLEAR data is automatically compensated for stray IR 
                                      ///< leakeds and temperature variations. When COMPEN = 0, the IR compensation is disabled, but the output of the IR 
                                      ///< compensation data exits.
                                      ///< 0 - Disables IR compensation. 
                                      ///< 1 - Enables IR compensation. Only for MODE = max4400xCfgMODE_CL_T
                                      
  max4400xAmbCfgMaskTRIM			= 0x80, ///< Automatic gain adjustment on sensor overload
	max4400xAmbCfgTRIM_DEFAULT  = 0x00, ///< 0 - Use factory-programmed gains for all the channels. Ignore any bytes written to TRIM_GAIN_GREEN[6:0], 
  max4400xAmbCfgTRIM_CUSTOM   = 0x80, ///< TRIM_GAIN_RED[6:0], TRIM_GAIN_BLUE[6:0], TRIM_GAIN_CLEAR[6:0], and TRIM_GAIN_IR[6:0] registers. 
                                      ///< 1 - Use bytes written to TRIM_GAIN_GREEN[6:0], TRIM_GAIN_RED[6:0], TRIM_GAIN_BLUE[6:0], TRIM_GAIN_CLEAR[6:0], 
                                      ///< and TRIM_GAIN_IR[6:0] registers to set the gain for each channel.
                                      
  /// Ambient Threshold Persist Timer (trigger) Register
  /// Sets one of four persist values that control a time delay before the interrupt logic reacts to a detected event.
  /// This feature is added in order to reduce false or nuisance interrupts.
  ///                                      NO. OF CONSECUTIVE MEASUREMENTS REQUIRED TO TRIGGER AN INTERRUPT
  max4400xAMBPST_1            = 0x00, ///< 1
  max4400xAMBPST_4            = 0x01, ///< 4
  max4400xAMBPST_8            = 0x02, ///< 8
  max4400xAMBPST_16           = 0x03, ///< 16
};

/// MAX4400x sensor API
///
/// Generic configuration
/// 

/// @brief Read MAX4400X interrupt register contents
/// @param [in] handle I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] intReg Pointer to variable which receives interrupt register contents
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xIntRead(i2cHANDLE handle, esU8 slaveAddr, esU8* intReg);

/// @brief Write MAX4400X interrupt register contents
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] intReg    Bit values to set in interrupt register
/// @return TRUE if operation succeeds, FALSE otherwise
/// 
esBL max4400xIntWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 intReg);

/// @brief Main configuration register read
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] cfgReg   Pointer to variable which receives configuration register contents
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xCfgRead(i2cHANDLE handle, esU8 slaveAddr, esU8* cfgReg);

/// @brief Main configuration register write
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] cfgReg   Configuration register contents
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xCfgWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 cfgReg);

/// @brief Read ambient configuration register
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] cfgReg   Pointer to variable which receives configuration register contents
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbCfgRead(i2cHANDLE handle, esU8 slaveAddr, esU8* cfgReg);

/// @brief Write ambient configuration register
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] cfgReg   Configuration register contents
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbCfgWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 cfgReg);

/// Thresholds configuration
///

/// @brief Read upper threshold value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] tu       Pointer to variable that is receiving threshold value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdUpperRead(i2cHANDLE handle, esU8 slaveAddr, esU16* tu);

/// @brief Write upper threshold value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] tu        Threshold value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdUpperWrite(i2cHANDLE handle, esU8 slaveAddr, esU16 tu);

/// @brief Read lower threshold value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] tl       Pointer to variable that is receiving threshold value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdLowerRead(i2cHANDLE handle, esU8 slaveAddr, esU16* tl);

/// @brief Write lower threshold value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] tl        Threshold value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdLowerWrite(i2cHANDLE handle, esU8 slaveAddr, esU16 tl);

/// @brief Read threshold latch timer value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] tt       Pointer to variable that is receiving latch timer value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdLatchTimerRead(i2cHANDLE handle, esU8 slaveAddr, esU8* tt);

/// @brief Write threshold latch timer value
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] tt        Latch timer value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xThresholdLatchTimerWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 tt);

/// Ambient gains configuration
/// 

/// @brief Read clear ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] gclear   Pointer to variable receiving gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainClearRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gclear);

/// @brief Write clear ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] gclear    Gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainClearWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gclear);

/// @brief Read red ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] gred     Pointer to variable receiving gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainRedRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gred);

/// @brief Write red ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] gred      Gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainRedWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gred);

/// @brief Read green ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] ggreen   Pointer to variable receiving gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainGreenRead(i2cHANDLE handle, esU8 slaveAddr, esU8* ggreen);

/// @brief Write green ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] ggreen    Gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainGreenWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 ggreen);

/// @brief Read blue ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] gblue    Pointer to variable receiving gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainBlueRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gblue);

/// @brief Write blue ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [in] gblue     Gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainBlueWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gblue);

/// @brief Read IR ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] gir      Pointer to variable receiving gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainIrRead(i2cHANDLE handle, esU8 slaveAddr, esU8* gir);

/// @brief Write IR ADC channel gain
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] gir      Gain value
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xAmbGainIrWrite(i2cHANDLE handle, esU8 slaveAddr, esU8 gir);

/// Data channels reading
/// 

/// @brief Read clear channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] clear    Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xClearRead(i2cHANDLE handle, esU8 slaveAddr, esU16* clear);

/// @brief Read red channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] red      Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xRedRead(i2cHANDLE handle, esU8 slaveAddr, esU16* red);

/// @brief Read green channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] green    Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xGreenRead(i2cHANDLE handle, esU8 slaveAddr, esU16* green);
  
/// @brief Read blue channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] blue     Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xBlueRead(i2cHANDLE handle, esU8 slaveAddr, esU16* blue);

/// @brief Read IR channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] ir       Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xIrRead(i2cHANDLE handle, esU8 slaveAddr, esU16* ir);

/// @brief Read IR compensation channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] irComp   Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xIrCompRead(i2cHANDLE handle, esU8 slaveAddr, esU16* irComp);
  
/// @brief Read temperature channel data
/// @param [in] handle    I2C bus handle
/// @param [in] slaveAddr MAX4400X I2C slave address
/// @param [out] temp     Pointer to variable receiving data
/// @return TRUE if operation succeeds, FALSE otherwise
///
esBL max4400xTempRead(i2cHANDLE handle, esU8 slaveAddr, esU16* temp);

#ifdef __cplusplus
  }
#endif
  
#endif // _max_4400x_h_
 