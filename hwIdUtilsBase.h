#ifndef _hardware_id_utils_base_h_
#define _hardware_id_utils_base_h_

// types and utility methods augmenting  hardware and firmware
// identification and hardware-user verification
//

#include "hwIdBase.h"
#include "hwIdUtilsFmtFlags.h"

#ifdef __cplusplus
    extern "C" {
#endif

// compare firmware versions return -1 if ver < major.minor; 0 if ver == major.minor, 1 id ver > major.minor
int hwIdFwVersionsCompare(const EseVerInfo* ver, int major, int minor);

#ifdef ES_USE_FWID_FORMATTERS

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
