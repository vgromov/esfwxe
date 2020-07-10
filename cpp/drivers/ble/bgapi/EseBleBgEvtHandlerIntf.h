#ifndef _ese_ble_bg_evt_handler_intf_h_
#define _ese_ble_bg_evt_handler_intf_h_

/// BluGiga module BLEXXX events
///
struct EseBleBgEvent
{
  // Event and response IDs
  enum {
    /// Events
    btevtNone,                               ///< Special event ID (none, or empty event)
    btevtBoot,                               ///< System boot complete, bootInfo is valid
    btevtBootDfu,                            ///< DFU boot complete, bootInfoDfu is valid
    btevtConnStatus,                         ///< connStatus is valid
    btevtConnDisconnected,                   ///< connDisconnectInfo is valid 
    btevtLocalAttrStatusChanged,             ///< Attribute flags changed event. attrStatus is valid
    btevtAttrUserValueWriteRequest,          ///< Attribute write request from remote device. attrValueWrite is valid
    btevtAttrUserValueReadRequest,           ///< User attribute read request from remote device. attrUserRead is valid
    btevtSmgrBondStatus,                     ///< Return bond status info in event. bondStatus is valid.
    btevtSmgrBondingFail,                    ///< bondingFailInfo is valid
    btevtPasskeyRequested,                   ///< passkeyRequest is valid
    
    /// Specific event ID value. all IDs in range [0, btevtLastID[ are events
    /// Everything greater than btevtLastID is response ID
    ///
    btevtLastID,

    /// Responses
    btrspReset,                              ///< No payload
    btrspHello,                              ///< No payload
    btrspConnStatus,                         ///< Connection status response
    btrspConnEncryptionStart,                ///< Connection encryption start request was sent, encryptionStartInfo is valid
    btrspConnDisconnect,                     ///< Connection disconnect response received, connDisconnect is valid
    btrspGapModeSet,                         ///< Returned is a result of GAP mode set request
    btrspGapAdvertisingSet,                  ///< Returned is a result of advertising parameters change request
    btrspGapEndProcedure,                    ///< Returned is a result of GAP procedure end request 
    btrspLocalAttrWrite,                     ///< Returned is a result of attribute write request
    btrspLocalAttrRead,                      ///< attrValueRead is valid
    btrspAttrUserValueRead,                  ///< User attribute read request from remote device is served with response
    btrspAttrUserValueWrite,                 ///< Attribute write request from remote device is served with response
    btrspAttrSend,                           ///< Attribute notification sending response, result is valid
    btrspHwPortIrqEnable,                    ///< Returned is a result of port IRQ configuration operation
    btrspHwPortDirCfg,                       ///< Returned is a result of port configuration operation
    btrspHwPortWrite,                        ///< Returned is a result of port write operation
    btrspAddressGet,                         ///< deviceAddr is valid
    btrspSmgrBondableSet,                    ///< No payload
    btrspSmgrBondingDelete,                  ///< Bonding was deleted, result is valid
    btrspSmgrParametersSet,                  ///< Security manager paramteres were set
    btrspSmgrPasskeyEntrySet,                ///< Security manager passkey was set
    
    /// Specific response ID value. Use for response ID range checking only
    ///
    btrspLastID
  };

  // Event ID
  uint16_t id;
  
  /// Combined BGAPI data header part (event payload)
  union
  {
    uint8_t raw[30];
    ble_msg_dfu_boot_evt_t bootInfoDfu;
    ble_msg_system_boot_evt_t bootInfo;
    ble_msg_system_address_get_rsp_t deviceAddr;
    ble_msg_connection_status_evt_t connStatus;
    ble_msg_sm_bonding_fail_evt_t bondingFailInfo;
    ble_msg_sm_bond_status_evt_t bondStatus;
    ble_msg_connection_disconnected_evt_t connDisconnectInfo;
    ble_msg_attributes_status_evt_t attrStatus;
    ble_msg_attributes_value_evt_t attrValueWrite;
    ble_msg_attributes_read_rsp_t attrValueRead;
    ble_msg_attributes_user_read_request_evt_t attrUserRead;
    ble_msg_sm_encrypt_start_rsp_t encryptionStartInfo;
    ble_msg_connection_disconnect_rsp_t connDisconnect;
    ble_msg_sm_passkey_request_evt_t passkeyRequest;

    uint16_t result;
  };
  
  /// Specialized data access
  ///
  
  /// Get incoming remote data pointer
  inline const uint8_t* dataWrittenGet() const ESE_NOTHROW
  {
    return data<ble_msg_attributes_value_evt_t>();
  }
  
  /// Get incoming remote data length
  inline uint8_t dataWrittenLenGet() const ESE_NOTHROW
  {
    return attrValueWrite.value.len;
  }

  /// Get incoming local data pointer
  inline const uint8_t* dataLocalReadGet() const ESE_NOTHROW
  {
    return data<ble_msg_attributes_read_rsp_t>();
  }
  
  /// Get incoming local data length
  inline uint8_t dataLocalReadLenGet() const ESE_NOTHROW
  {
    return attrValueRead.value.len;
  }
  
  /// Access the data, which (may) reside beyond the end of header struct
  ///
  template <typename HdrStructT>
  const uint8_t* data() const ESE_NOTHROW
  {
    return raw+sizeof(HdrStructT);
  }
  
  /// Access the data, which (may) reside beyond the end of header struct
  ///
  template <typename HdrStructT>
  uint8_t* data() ESE_NOTHROW
  {
    return raw+sizeof(HdrStructT);
  }
};

/// BGAPI event handler interface
///
class ESE_ABSTRACT EseBleBgEvtHandlerIntf
{
public:
  virtual void evtHandle(const EseBleBgEvent& evt) ESE_NOTHROW = 0;
  virtual void rspErrorHandle(uint8_t respId, uint16_t result) ESE_NOTHROW = 0;
  virtual void rxGarbageHandle(const ble_header& apihdr, const uint8_t* data) ESE_NOTHROW = 0;
  virtual bool isBreaking() ESE_NOTHROW = 0;
};

#endif //< _ese_ble_bg_evt_handler_intf_h_
