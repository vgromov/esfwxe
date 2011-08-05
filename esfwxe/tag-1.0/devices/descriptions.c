#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <common/utils.h>
#include <common/devices/descriptions.h>

#define DEVICE_FLAG_HUB				0x0001
#define DEVICE_FLAG_HUB_LIGHT	0x0002

// generate device names && descriptions from devices.cc
// device types enumeration
#define DEVICE_ENTRIES_BEGIN static struct { \
	ES_ASCII_CSTR codeName; \
	ES_ASCII_CSTR descrShort;	\
	ES_ASCII_CSTR descrLong; \
	WORD flags; \
	} c_deviceDescrs[] = {
#	define HUB_ENTRY(CodeName, Sdcr, Ldcr)				{ #CodeName, Sdcr, Ldcr, DEVICE_FLAG_HUB },
#	define HUB_LIGHT_ENTRY(CodeName, Sdcr, Ldcr)	{ #CodeName, Sdcr, Ldcr, DEVICE_FLAG_HUB|DEVICE_FLAG_HUB_LIGHT },
#	define APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr)	{ #CodeName, Sdcr, Ldcr, 0 },
#define DEVICE_ENTRIES_END };
#include "devices.cc"

// description access API
//
BOOL isKnownDevice( WORD type )
{
	return type < DEVICE_TYPES_CNT;
}

ES_ASCII_CSTR getDeviceCodeName( WORD type )
{
	if( type < DEVICE_TYPES_CNT )
		return c_deviceDescrs[type].codeName;
	else
		return 0;	
}

ES_ASCII_CSTR getDeviceDescrShort( WORD type )
{
	if( type < DEVICE_TYPES_CNT )
		return c_deviceDescrs[type].descrShort;
	else
		return 0;
}

ES_ASCII_CSTR getDeviceDescrLong( WORD type )
{
	if( type < DEVICE_TYPES_CNT )
		return c_deviceDescrs[type].descrLong;
	else
		return 0;	
}

BOOL deviceIsHub( WORD type )
{
	if( type < DEVICE_TYPES_CNT )
		return DEVICE_FLAG_HUB == (c_deviceDescrs[type].flags & DEVICE_FLAG_HUB);
	else
		return FALSE;	
}

BOOL deviceIsLightHub( WORD type )
{
	if( type < DEVICE_TYPES_CNT )
		return DEVICE_FLAG_HUB_LIGHT == (c_deviceDescrs[type].flags & DEVICE_FLAG_HUB_LIGHT);
	else
		return FALSE;	
}

WORD getDeviceTypeFromCodeName(ES_ASCII_CSTR codeName)
{
	WORD type = 0;
	while( type < DEVICE_TYPES_CNT )
	{
		if( 0 == strcmp( c_deviceDescrs[type].codeName, codeName ) )
			return type;
		++type;
	}
	
	return NO_ID;
}
