#include <esfwxe/target.h>
#pragma hdrstop

#include <string.h>
#include <esfwxe/utils.h>
#include <esfwxe/ese_i18n.h>
#include <esfwxe/devices/descriptions.h>

#define DEVICE_FLAG_HUB				0x0001
#define DEVICE_FLAG_HUB_LIGHT	0x0002

#if defined(ESE_USE_STRING_DEVICES_INFO) && 1 == ESE_USE_STRING_DEVICES_INFO

// generate device names && descriptions from devices.cc
// device types enumeration
#define DEVICE_ENTRIES_BEGIN \
static const struct { \
	ESE_CSTR codeName; \
	int descrShortId;	\
	int descrLongId; \
	esU16 flags; \
	} c_deviceDescrs[] = {
#	define HUB_ENTRY(CodeName, SdcrId, LdcrId)		    { #CodeName, SdcrId, LdcrId, DEVICE_FLAG_HUB },
#	define HUB_LIGHT_ENTRY(CodeName, SdcrId, LdcrId)  { #CodeName, SdcrId, LdcrId, DEVICE_FLAG_HUB|DEVICE_FLAG_HUB_LIGHT },
#	define APPLIANCE_ENTRY(CodeName, SdcrId, LdcrId)	{ #CodeName, SdcrId, LdcrId, 0 },
#define DEVICE_ENTRIES_END };

#define OCTAVA_DEVICE_ENTRIES_BEGIN \
static const struct { \
	ESE_CSTR codeName; \
	int descrShortId;	\
	int descrLongId; \
	} c_octavaDeviceDescrs[] = {
# define OCTAVA_APPLIANCE_ENTRY(CodeName, SdcrId, LdcrId) { #CodeName, SdcrId, LdcrId }, 
#define OCTAVA_DEVICE_ENTRIES_END	};

#define TENZOR_DEVICE_ENTRIES_BEGIN \
static const struct { \
	ESE_CSTR codeName; \
	int descrShortId;	\
	int descrLongId; \
	esU16 flags; \
	} c_tenzorDeviceDescrs[] = {
# define TENZOR_APPLIANCE_ENTRY(CodeName, SdcrId, LdcrId) { #CodeName, SdcrId, LdcrId, 0 }, 
#	define TENZOR_HUB_ENTRY(CodeName, SdcrId, LdcrId)				{ #CodeName, SdcrId, LdcrId, DEVICE_FLAG_HUB },
#define TENZOR_DEVICE_ENTRIES_END	};

#define QUARTA_DEVICE_ENTRIES_BEGIN \
static const struct { \
	ESE_CSTR codeName; \
	int descrShortId;	\
	int descrLongId; \
	esU16 flags; \
	} c_quartaDeviceDescrs[] = {
# define QUARTA_APPLIANCE_ENTRY(CodeName, SdcrId, LdcrId) { #CodeName, SdcrId, LdcrId, 0 }, 
#	define QUARTA_HUB_ENTRY(CodeName, SdcrId, LdcrId)				{ #CodeName, SdcrId, LdcrId, DEVICE_FLAG_HUB },
#define QUARTA_DEVICE_ENTRIES_END	};

#else // ESE_USE_STRING_DEVICES_INFO

// generate device names && descriptions from devices.cc
// device types enumeration
#define DEVICE_ENTRIES_BEGIN \
static const struct { \
	esU16 flags; \
  } c_deviceDescrs[] = {
#	define HUB_ENTRY(CodeName, Sdcr, Ldcr)				{ DEVICE_FLAG_HUB },
#	define HUB_LIGHT_ENTRY(CodeName, Sdcr, Ldcr)	{ DEVICE_FLAG_HUB|DEVICE_FLAG_HUB_LIGHT },
#	define APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr)	{ 0 },
#define DEVICE_ENTRIES_END };

#define OCTAVA_DEVICE_ENTRIES_BEGIN
# define OCTAVA_APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr) 
#define OCTAVA_DEVICE_ENTRIES_END

#define TENZOR_DEVICE_ENTRIES_BEGIN \
static const struct { \
	esU16 flags; \
	} c_tenzorDeviceDescrs[] = {
#	define TENZOR_HUB_ENTRY(CodeName, Sdcr, Ldcr)				{ DEVICE_FLAG_HUB },
# define TENZOR_APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr) { 0 }, 
#define TENZOR_DEVICE_ENTRIES_END	};

#define QUARTA_DEVICE_ENTRIES_BEGIN \
static const struct { \
	esU16 flags; \
	} c_quartaDeviceDescrs[] = {
# define QUARTA_APPLIANCE_ENTRY(CodeName, Sdcr, Ldcr) { 0 }, 
#	define QUARTA_HUB_ENTRY(CodeName, Sdcr, Ldcr)				{ DEVICE_FLAG_HUB },
#define QUARTA_DEVICE_ENTRIES_END	};

#endif // ESE_USE_STRING_DEVICES_INFO 

#include "devices.cc"

// description access API
//
static __inline esBL isOctavaDevice( int type )
{
	return OCTAVA_DEVICE_ID_BASE < type && 
		type < OCTAVA_DEVICE_ID_END;
}

static __inline esBL isTenzorDevice( int type )
{
	return TENZOR_DEVICE_ID_BASE < type && 
		type < TENZOR_DEVICE_ID_END;
}

static __inline int tenzorTypeToOffs( int type )
{
	return type-TENZOR_DEVICE_ID_BASE-1;
}

static __inline esBL isQuartaDevice( int type )
{
	return QUARTA_DEVICE_ID_BASE < type && 
		type < QUARTA_DEVICE_ID_END;
}

static __inline int quartaTypeToOffs( int type )
{
	return type-QUARTA_DEVICE_ID_BASE-1;
}

esBL isKnownDevice( esU16 type )
{
	return type < DEVICE_TYPES_CNT ||
		isOctavaDevice(type) ||
		isTenzorDevice(type) ||
		isQuartaDevice(type);
}

esBL deviceIsHub( esU16 type )
{
	if( type < DEVICE_TYPES_CNT )
		return DEVICE_FLAG_HUB == (c_deviceDescrs[type].flags & DEVICE_FLAG_HUB);
	else if( isTenzorDevice(type) )
		return DEVICE_FLAG_HUB == (c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].flags & DEVICE_FLAG_HUB);
	else if( isQuartaDevice(type) )
		return DEVICE_FLAG_HUB == (c_quartaDeviceDescrs[quartaTypeToOffs(type)].flags & DEVICE_FLAG_HUB);
	else
		return FALSE;
}

esBL deviceIsLightHub( esU16 type )
{
	if( type < DEVICE_TYPES_CNT )
		return DEVICE_FLAG_HUB_LIGHT == (c_deviceDescrs[type].flags & DEVICE_FLAG_HUB_LIGHT);
	else if( isTenzorDevice(type) )
		return DEVICE_FLAG_HUB_LIGHT == (c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].flags & DEVICE_FLAG_HUB_LIGHT);
	else if( isQuartaDevice(type) )
		return DEVICE_FLAG_HUB_LIGHT == (c_quartaDeviceDescrs[quartaTypeToOffs(type)].flags & DEVICE_FLAG_HUB_LIGHT);
	else
		return FALSE;	
}

#if defined(ESE_USE_STRING_DEVICES_INFO) && 1 == ESE_USE_STRING_DEVICES_INFO

static __inline int octavaTypeToOffs( int type )
{
	return type-OCTAVA_DEVICE_ID_BASE-1;
}

ESE_CSTR getDeviceCodeName( esU16 type )
{
	if( type < DEVICE_TYPES_CNT )
		return c_deviceDescrs[type].codeName;
	else if( isOctavaDevice(type) )
		return c_octavaDeviceDescrs[octavaTypeToOffs(type)].codeName;
	else if( isTenzorDevice(type) )
		return c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].codeName;
	else if( isQuartaDevice(type) )
		return c_quartaDeviceDescrs[quartaTypeToOffs(type)].codeName;
	else
		return NULL;
}

ESE_CSTR getDeviceDescrShort( eseI18nLangId langId, esU16 type )
{
	if( type < DEVICE_TYPES_CNT )
		return eseI(langId, c_deviceDescrs[type].descrShortId);
	else if( isOctavaDevice(type) )
		return eseI(langId, c_octavaDeviceDescrs[octavaTypeToOffs(type)].descrShortId);
	else if( isTenzorDevice(type) )
		return eseI(langId, c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].descrShortId);
	else if( isQuartaDevice(type) )
		return eseI(langId, c_quartaDeviceDescrs[quartaTypeToOffs(type)].descrShortId);
	else
		return NULL;
}

ESE_CSTR getDeviceDescrLong( eseI18nLangId langId, esU16 type )
{
	if( type < DEVICE_TYPES_CNT )
		return eseI(langId, c_deviceDescrs[type].descrLongId);
	else if( isOctavaDevice(type) )
		return eseI(langId, c_octavaDeviceDescrs[octavaTypeToOffs(type)].descrLongId);
	else if( isTenzorDevice(type) )
		return eseI(langId, c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].descrLongId);
	else if( isQuartaDevice(type) )
		return eseI(langId, c_quartaDeviceDescrs[quartaTypeToOffs(type)].descrLongId);
	else
		return NULL;
}

esU16 getDeviceTypeFromCodeName(ESE_CSTR codeName)
{
	esU16 type = 0;
	// Ekosfera branch
	while( type < DEVICE_TYPES_CNT )
	{
		if( 0 == strcmp( c_deviceDescrs[type].codeName, codeName ) )
			return type;
		++type;
	}
	// Octava branch
	type = OCTAVA_DEVICE_ID_BASE+1;
	while( OCTAVA_DEVICE_ID_END > type )
	{
		if( 0 == strcmp( c_octavaDeviceDescrs[octavaTypeToOffs(type)].codeName, codeName ) )
			return type;
		
		++type;
	}
	// Tenzor branch
	type = TENZOR_DEVICE_ID_BASE+1;
	while( TENZOR_DEVICE_ID_END > type )
	{
		if( 0 == strcmp( c_tenzorDeviceDescrs[tenzorTypeToOffs(type)].codeName, codeName ) )
			return type;
		
		++type;
	}
	// Quarta branch
	type = QUARTA_DEVICE_ID_BASE+1;
	while( QUARTA_DEVICE_ID_END > type )
	{
		if( 0 == strcmp( c_quartaDeviceDescrs[quartaTypeToOffs(type)].codeName, codeName ) )
			return type;
		
		++type;
	}
	
	return NO_ID;
}

#endif // #if defined(ESE_USE_STRING_DEVICES_INFO) && 1 == ESE_USE_STRING_DEVICES_INFO
