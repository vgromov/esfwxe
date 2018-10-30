#ifndef _hw_id_base_h_
#define _hw_id_base_h_

/// Base typedefs for the standard hardware identification
///
#include "devices/descriptions.h"

#ifdef __cplusplus
    extern "C" {
#endif

#pragma pack(push, 1)

/// Firmware | software version information
///
#pragma anon_unions
typedef union {
  struct
  {
    esU8 major;
    esU8 minor;
  };
  
  esU16 raw;

} EseVerInfo;

/// Device production number
///
typedef struct 
{
  esU16 year;                     ///< Year of production
  esU16 ser;                      ///< Device production (serial) number within year

} EseDeviceNum;

/// Basic software | firmware identification block
///
typedef struct
{
  esU16         type;             ///< Device type (model) id
  EseDeviceNum  num;              ///< Device production number
  EseVerInfo    ver;              ///< Firmware major|minor version

} EseBasicFirmwareID;

/// (G)EseUID in string representation, 
/// no delimiters, commas, or brackets
///
typedef struct {
  esU8 uid[32];
  
} EseUID;

/// Extended core firmware identification structure
/// which may be returned for the core firmware id query
///
typedef struct
{
  EseBasicFirmwareID id;
  EseUID key;
  esU32 funcMask;

}    EseHubFirmwareID;

/// Firmware integrity information struct
///
typedef struct
{
  esU32 crcPrev;                ///< CRC32 of the previous block contents. 0, if this is the first version of block
  esU32 crcCur;                 ///< Current CRC32 (as calculated by CRC32 IEEE 802.3)
  esDT  ts;                     ///< Recent block update timestamp
    
} EseBlockIntegrity;

/// Firmware file && integrity check
///
typedef struct
{
  char name[64];                ///< Readable name of the firmware
  char file[32];                ///< Firmware file used for flashing
  
  /// Firmware integrity information
  EseBlockIntegrity integrity;

} EseFwSwInfo;

/// Universal 64 byte firmware information
///
typedef union {
  struct {
    /// Hardware configuration
    esU16 stdandard;
    esU16 custom;
  };
  
  esU32   raw;
  
} EseHwConfigInfo;

typedef struct {
  /// Device type
  esU16 type;
  /// Order
  esU32 order;
  /// Year
  esU16 year;
  /// Month
  esU8 month;
  /// Day of month
  esU8 day;
  /// Version
  EseVerInfo ver;
  /// Country code, as in ISO 3166-1-numeric 
  esU16 countryCode;
  /// Hardware configuration
  EseHwConfigInfo hwConfig;
  /// Reserved block
  esU8 reserved[46];

} EseFwInfo;

/// Software information structure
/// which is returned as a response to the standard RPC "software id get" request
///
typedef struct
{
  EseFwSwInfo         fwSwInfo;       ///< Firmware file info
  EseBlockIntegrity   calIntegrity;   ///< Calibration integrity check block, if applicable
  EseFwInfo           fwInfo;         ///< Universal 64 byte firmware information
    
} EseSoftwareInfo;

#pragma pack(pop)

// appliance firmware id (aliased EseBasicFirmwareID)
#define ApplianceFirmwareID EseBasicFirmwareID

enum { 
  Type_SZE = sizeof(esU16),
  EseDevNum_SZE = sizeof(EseDeviceNum),
  EseVerInfo_SZE = sizeof(EseVerInfo),
  ApplianceFirmwareID_NoFw_SZE = EseDevNum_SZE+Type_SZE,
  EseBasicFirmwareID_SZE = sizeof(EseBasicFirmwareID),
  ApplianceFirmwareID_SZE = EseBasicFirmwareID_SZE,
  EseUID_SZE = sizeof(EseUID),
  EseHubFirmwareID_SZE = sizeof(EseHubFirmwareID),
  EseBlockIntegrity_SZE = sizeof(EseBlockIntegrity),
  EseFwSwInfo_SZE = sizeof(EseFwSwInfo),
  EseSoftwareInfo_SZE = sizeof(EseSoftwareInfo),
  EseFwInfo_SZE = sizeof(EseFwInfo),
};

#ifdef __cplusplus
    }
#endif

#endif // _hw_id_base_h_
