#ifndef _devices_h_
#define _devices_h_

#include <common/type.h>

// this header creates compile-time generated symbolic names for known devices
//

#ifdef __cplusplus
	extern "C" {
#endif

// device types enumeration
#define DEVICE_ENTRIES_BEGIN enum {	
#	define HUB_ENTRY(CodeName, Sdcr, Ldcr)				HUB_TYPE_ ## CodeName,
#	define HUB_LIGHT_ENTRY(CodeName, Sdcr, Ldcr)	HUB_TYPE_ ## CodeName,
#	define APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr)	APPL_TYPE_ ## CodeName,
#define DEVICE_ENTRIES_END \
	DEVICE_TYPES_CNT };
#include "devices.cc"

// optional description access API
//
BOOL isKnownDevice( WORD type );
ES_ASCII_CSTR getDeviceCodeName( WORD type );
ES_ASCII_CSTR getDeviceDescrShort( WORD type );
ES_ASCII_CSTR getDeviceDescrLong( WORD type );
BOOL deviceIsHub( WORD type ); 
BOOL deviceIsLightHub( WORD type );
WORD getDeviceTypeFromCodeName(ES_ASCII_CSTR codeName);

#ifdef __cplusplus
	}
#endif

#endif // _devices_h_
