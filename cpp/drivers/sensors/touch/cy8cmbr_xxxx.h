#ifndef _cy8cmbr_xxxx_h_
#define _cy8cmbr_xxxx_h_

/// Cypress capacitive CY input controller driver over I2C bus
///
class CY8CMBRXXXX : public EseI2cSlaveDevice
{
public:
  /// Default device address
  enum {
    i2cAddrDefault        = 0x6E
  };
  
  /// Device-specific timeouts, in ms
  enum {
    tmoSoftReset          = 100,    ///< Typical time to wait after device is soft-reset
    tmoConfigAccept       = 250,    ///< Typical time to wait after configuration is uploaded to device
                                    ///  and save operation is initiated.
    requestRetriesCnt     = 10      ///< Default register request retries count, controller reference
                                    ///  states, that due to automatic power mode transitions, chip may
                                    ///  NACK up to N I2C address requests.
  };
  
#pragma pack(push, 1)
  /// Device ID
  struct HwId
  {
    inline HwId() ESE_NOTHROW :
    m_family(0),
    m_id(0),
    m_rev(0)
    {}

    uint8_t m_family;
    uint16_t m_id;
    uint16_t m_rev;
  };
#pragma pack(pop)

  /// Known device IDs
  ///
  enum {
    /// CY8CMBR3102
    ///
    idCY8CMBR3102         = 0x0A01,
    familyCY8CMBR3102     = 0x9A,
    
    /// CY8CMBR3106S
    ///
    idCY8CMBR3106S        = 0x0A06,
    familyCY8CMBR3106S    = 0x9A,
    
    /// CY8CMBR3108
    ///
    idCY8CMBR3108         = 0x0A03,
    familyCY8CMBR3108     = 0x9A,
    
    /// CY8CMBR3110
    ///
    idCY8CMBR3110         = 0x0A02,
    familyCY8CMBR3110     = 0x9A,
    
    /// CY8CMBR3116
    ///
    idCY8CMBR3116         = 0x0A05,
    familyCY8CMBR3116     = 0x9A
  };

  /// HW Configuration block.
  /// Last two bytes should contain valid block CRC [MSB,LSB]
  /// CRC algo is CRC-16/CCITT-FALSE, may be calculated here: https://crccalc.com
  ///
  struct ConfigBlock 
  {
    uint8_t m_data[128];
  };

public:
  CY8CMBRXXXX() ESE_NOTHROW {}
  CY8CMBRXXXX(EseChannelIntf& i2c, uint8_t addr, esU32 tmo) ESE_NOTHROW;

  /// @brief Read hardware device ID
  /// @param [out] id   Device ID.
  /// @return           true if operation succeeds, false otherwise.
  ///                   If false is returned, Device ID data must be considered invalid
  ///
  bool hwIdRead(HwId& id) ESE_NOTHROW;

  /// @brief Write hardware configuration block, app-defined
  /// @param [in] cfg   Configuration block.
  /// @return           true if operation succeeds, false otherwise.
  ///
  bool cfgBlockWrite(const ConfigBlock& cfg) ESE_NOTHROW;

  /// @brief Read hardware configuration block
  /// @param [out] cfg  Configuration block.
  /// @return           true if operation succeeds, false otherwise.
  ///                   If false is returned, a contents of cfg block 
  ///                   must be considered invalid.
  ///
  bool cfgBlockRead(ConfigBlock& cfg) ESE_NOTHROW;
  
  /// @brief Execute software reset on chip
  /// @return           true if operation succeeds, false otherwise.
  ///                   If true is returned, an application must wait for tmoSoftReset
  ///
  bool softReset() ESE_NOTHROW;
  
  /// Register access helpers
  ///
  bool registerRead(uint8_t reg, size_t len, uint8_t* out, unsigned retries = requestRetriesCnt) ESE_NOTHROW;
  bool registerWrite(uint8_t reg, size_t len, const uint8_t* in, unsigned retries = requestRetriesCnt) ESE_NOTHROW;

  ESE_NONCOPYABLE(CY8CMBRXXXX);
};

#endif //< _cy8cmbr_xxxx_h_
