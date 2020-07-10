#ifndef _devices_descriptions_h_
#define _devices_descriptions_h_

#include <esfwxe/type.h>

// This header creates compile-time generated symbolic names for known devices
//

#ifdef __cplusplus
    extern "C" {
#endif

// device types enumeration
#define DEVICE_ENTRIES_BEGIN                                                                             enum {    
#    define HUB_ENTRY(               CodeName, Sdcr, Ldcr)                              HUB_TYPE_ ## CodeName,
#    define HUB_LIGHT_ENTRY(         CodeName, Sdcr, Ldcr)                              HUB_TYPE_ ## CodeName,
#    define APPLIANCE_ENTRY(         CodeName, Sdcr, Ldcr)                              APPL_TYPE_ ## CodeName,
#define DEVICE_ENTRIES_END                                                                                 DEVICE_TYPES_CNT };
                            
#define OCTAVA_DEVICE_ENTRIES_BEGIN                                                             enum { OCTAVA_DEVICE_ID_BASE = 32000,
# define OCTAVA_APPLIANCE_ENTRY(  CodeName, Sdcr, Ldcr)                       APPL_TYPE_ ## CodeName,
#define OCTAVA_DEVICE_ENTRIES_END                                                                    OCTAVA_DEVICE_ID_END };
                        
#define TENZOR_DEVICE_ENTRIES_BEGIN                                                             enum { TENZOR_DEVICE_ID_BASE = 40000,
# define TENZOR_APPLIANCE_ENTRY(  CodeName, Sdcr, Ldcr)                       APPL_TYPE_ ## CodeName,
#    define TENZOR_HUB_ENTRY(        CodeName, Sdcr, Ldcr)                                HUB_TYPE_ ## CodeName,
#define TENZOR_DEVICE_ENTRIES_END                                                                    TENZOR_DEVICE_ID_END };
                            
#define QUARTA_DEVICE_ENTRIES_BEGIN                                                             enum { QUARTA_DEVICE_ID_BASE = 42000,
# define QUARTA_APPLIANCE_ENTRY(  CodeName, Sdcr, Ldcr)                       APPL_TYPE_ ## CodeName,
#    define QUARTA_HUB_ENTRY(        CodeName, Sdcr, Ldcr)                                HUB_TYPE_ ## CodeName,
#define QUARTA_DEVICE_ENTRIES_END                                                                    QUARTA_DEVICE_ID_END };        
        
#include "devices.cc"

// Optional type characterization API
//
esBL isKnownDevice( esU16 type );
esBL deviceIsHub( esU16 type ); 
esBL deviceIsLightHub( esU16 type );

// Optional description access API
//

#if defined(ESE_USE_STRING_DEVICES_INFO) && 1 == ESE_USE_STRING_DEVICES_INFO

ESE_CSTR getDeviceCodeName( esU16 type );
ESE_CSTR getDeviceDescrShort( eseI18nLangId langId, esU16 type );
ESE_CSTR getDeviceDescrLong( eseI18nLangId langId, esU16 type );
esU16 getDeviceTypeFromCodeName(ESE_CSTR codeName);

#endif

#ifdef __cplusplus
    }
#endif

#endif // _devices_descriptions_h_
