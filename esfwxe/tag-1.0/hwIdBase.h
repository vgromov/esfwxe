#ifndef _hw_id_base_h_
#define _hw_id_base_h_

// base typedefs for the standard hardware identification
//
#include "devices/descriptions.h"

#ifdef __cplusplus
	extern "C" {
#endif

#pragma pack(push, 1)
// firmware | software version information
typedef struct
{
	BYTE major;
	BYTE minor;

} VerInfo;

// device production number
typedef struct 
{
	WORD year;		// year of production
	WORD ser;			// device production (serial) number within year

} DeviceNum;

// basic software | firmware identification block
typedef struct
{
	WORD 			type;		// device type (model) id
 	DeviceNum num;		// device production number
	VerInfo 	ver;		// firmware major|minor version

} BasicFirmwareID;

typedef BYTE UID[32];			// (G)UID	in string representation, 
													// no delimiters, commas, or brackets

// extended core firmware identification structure
// which may be returned for the core firmware id query
typedef struct
{
	BasicFirmwareID id;
	UID key;
	DWORD funcMask;

}	HubFirmwareID;

#pragma pack(pop)

// appliance firmware id (aliased BasicFirmwareID)
#define ApplianceFirmwareID BasicFirmwareID

enum { 
	Type_SZE = sizeof(WORD),
	DevNum_SZE = sizeof(DeviceNum),
	VerInfo_SZE = sizeof(VerInfo),
	ApplianceFirmwareID_NoFw_SZE = DevNum_SZE+Type_SZE,
	BasicFirmwareID_SZE = sizeof(BasicFirmwareID),
	UID_SZE = sizeof(UID),
	HubFirmwareID_SZE = sizeof(HubFirmwareID),
};
#define ApplianceFirmwareID_SZE BasicFirmwareID_SZE

#ifdef __cplusplus
	}
#endif

#endif // _hw_id_base_h_
