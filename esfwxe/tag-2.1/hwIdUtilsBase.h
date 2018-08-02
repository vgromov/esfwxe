#ifndef _hardware_id_utils_base_h_
#define _hardware_id_utils_base_h_

// types and utility methods augmenting	hardware and firmware
// identification and hardware-user verification
//

#include "hwIdBase.h"

#ifdef __cplusplus
	extern "C" {
#endif

// compare firmware versions return -1 if ver < major.minor; 0 if ver == major.minor, 1 id ver > major.minor
int hwIdFwVersionsCompare(const EseVerInfo* ver, int major, int minor);

#ifdef ES_USE_FWID_FORMATTERS

/// ApplianceFwID standard formatting flags
///
enum {
	APPL_ID_FMT_DESCR_SHORT = 0x0001,
	APPL_ID_FMT_DESCR_LONG	= 0x0002,
	APPL_ID_FMT_SERIAL			= 0x0004,
	APPL_ID_FMT_FW					= 0x0008,
  APPL_ID_FMT_REGION      = 0x0010,
  APPL_ID_FMT_HWINFO      = 0x0020,
  APPL_ID_FMT_SERIAL_SHORT= 0x0040, ///< Shortened serial number version OOOOO-YYMMDD
  
  /// Specific formatting masks
  ///

  /// Perform full formatting
  ///

  /// Use short device description
  APPL_ID_FMT_ALL_SHORT   = APPL_ID_FMT_DESCR_SHORT|
                            APPL_ID_FMT_SERIAL|
                            APPL_ID_FMT_FW|
                            APPL_ID_FMT_REGION|
                            APPL_ID_FMT_HWINFO,
                            
  /// Use long device description
  APPL_ID_FMT_ALL_LONG    = APPL_ID_FMT_DESCR_LONG|
                            APPL_ID_FMT_SERIAL|
                            APPL_ID_FMT_FW|
                            APPL_ID_FMT_REGION|
                            APPL_ID_FMT_HWINFO,
  
  /// Perform formatting using only essential parts, like description and serial
  ///

  /// Use short device description
  APPL_ID_FMT_MAIN_SHORT  = APPL_ID_FMT_DESCR_SHORT|
                            APPL_ID_FMT_SERIAL,
              
  /// Use long device description
  APPL_ID_FMT_MAIN_LONG   = APPL_ID_FMT_DESCR_LONG|
                            APPL_ID_FMT_SERIAL
  
};

/// Format ECO-E specific device ID string from EseBasicFirmwareID
void fmtEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID* id, esU16 flags);

/// Format ECO-E specific device ID string from EseFwInfo
void fmtEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo* id, esU16 flags);

/// UID key standard formatting
void fmtUID(ESE_STR buff, size_t buffLen, const EseUID* key);

/// Format device ID string parts using universal ID string specs, and ECO-E specific EseBasicFirmwareID.
/// flags have the same meaning as above, but description bits are omitted by design
///
void fmtIdStringFromEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID* id, esU16 flags);

/// Format device ID string parts using universal ID string specs, and universal EseFwInfo.
/// flags have the same meaning as above, but description bits are omitted by design
///
void fmtIdStringFromEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo* info, esU16 flags);

/// Format UID string in universal format 
ESE_STR fmtUIDtoIdString(ESE_STR buff, size_t buffLen, ESE_CSTR uid, size_t uidLen);

/// Format functional mask in universal format
ESE_STR fmtFunctionalToIdString(ESE_STR buff, size_t buffLen, esU32 functional);

#endif

#ifdef __cplusplus
	}
#endif

#endif // _hardware_id_utils_base_h_
