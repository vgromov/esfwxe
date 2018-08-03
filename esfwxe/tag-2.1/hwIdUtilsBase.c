#include "target.h"
#pragma hdrstop

#include <stdio.h>
#include <string.h>
#include "utils.h"
#include "hwIdUtilsBase.h"
#include "devices/descriptions.h"

// compare firmware versions
int hwIdFwVersionsCompare(const EseVerInfo* ver, int major, int minor)
{
	if(ver)
	{
		if(ver->major > major)
			return 1;
		else if( ver->major == major )
		{
			if( ver->minor > minor )
				return 1;
			else if( ver->minor == minor )
				return 0;
			else
				return -1;
		}
	}

	return -1;
}

#ifdef ES_USE_FWID_FORMATTERS

// Fwd decls
static void fmtEcoE(ESE_STR buff, size_t buffLen, esU16 flags, esU16 type, esU16 year, esU32 order,
  const EseVerInfo* fwver, const esU8* month, const esU8* day, const esU16* region, const EseHwConfigInfo* hwInfo);

static void fmtUniversal(ESE_STR buff, size_t buffLen, esU16 flags, esU16 type, esU16 year, esU32 order,
  const EseVerInfo* fwver, const esU8* month, const esU8* day, const esU16* region, const EseHwConfigInfo* hwInfo);

void fmtEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID* id, esU16 flags)
{
	if( buff && 0 < buffLen && id )
    fmtEcoE(buff, buffLen, flags, id->type, id->num.year, id->num.ser, &id->ver, 0, 0, 0, 0);
}

void fmtEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo* id, esU16 flags)
{
	if( buff && 0 < buffLen && id )
    fmtEcoE(buff, buffLen, flags, id->type, id->year, id->order, &id->ver, 
      &id->month, &id->day, &id->countryCode, &id->hwConfig);
}

void fmtUID(ESE_STR buff, size_t buffLen, const EseUID* key)
{
	if( buff && 0 < buffLen && key )
	{
		// format in 4 groups 8 chars each
		ESE_CSTR str = (ESE_CSTR)key->uid;
		utilsSnprintf(buff, buffLen, "%.8s-%.4s-%.4s-%.4s-%.12s", str,
			str+8, str+12, str+16, str+20);
	}
}

void fmtIdStringFromEseBasicFirmwareID(ESE_STR buff, size_t buffLen, const EseBasicFirmwareID* id, esU16 flags)
{
	if( buff && 0 < buffLen && id )
    fmtUniversal(buff, buffLen, flags, id->type, id->num.year, id->num.ser, &id->ver, 0, 0, 0, 0);
}

void fmtIdStringFromEseFwInfo(ESE_STR buff, size_t buffLen, const EseFwInfo* info, esU16 flags)
{
	if( buff && 0 < buffLen && info )
    fmtUniversal(buff, buffLen, flags, info->type, info->year, info->order, &info->ver, 
      &info->month, &info->day, &info->countryCode, &info->hwConfig);
}

ESE_STR fmtUIDtoIdString(ESE_STR buff, size_t buffLen, ESE_CSTR uid, size_t uidLen)
{
  int len = 0;
	if( buff && 0 < buffLen && uid && uidLen <= buffLen )
		len = utilsSnprintf(buff, buffLen, "U:%.*s", uidLen, uid);
  
  return buff+len;
}

ESE_STR fmtFunctionalToIdString(ESE_STR buff, size_t buffLen, esU32 functional)
{
  int len = 0;
	if( buff && 0 < buffLen )
		len = utilsSnprintf(buff, buffLen, "F:0x%08X", (unsigned)functional);
  
  return buff+len;
}

//--------------------------------------------------------------------------------------------------------------------
//
static __inline ESE_STR fmtBuffInc(ESE_STR buff, size_t* buffLen, int len)
{
  if( 0 < len )
  {
    if( (int)(*buffLen) >= len )
      *buffLen -= len;
    else
      *buffLen = 0;
      
    buff += len;
  }
  
  return buff;
}

static __inline void fmtBuffTerminate(ESE_STR buff, ESE_STR buffEnd)
{
  if( buffEnd <= buff )
  {
    --buffEnd;
    *buffEnd = 0;
  }
}

// Internal ECE-E formatting helper
static void fmtEcoE(ESE_STR buff, size_t buffLen, esU16 flags, esU16 type, esU16 year, esU32 order,
  const EseVerInfo* fwver, const esU8* month, const esU8* day, const esU16* region, const EseHwConfigInfo* hwInfo)
{
  ESE_STR buffEnd = buff+buffLen;
  
  int len = 0;

  // Format description, if this functionality is supported
#if defined(ESE_USE_STRING_DEVICES_INFO) && 1 == ESE_USE_STRING_DEVICES_INFO

  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_DESCR_SHORT) )
    len = utilsSnprintf(
      buff,
      buffLen,
      "%s",
      getDeviceDescrShort(
        eseI18nLang_ru,
        type
      )
    );
  else if( ES_BIT_IS_SET(flags, APPL_ID_FMT_DESCR_LONG) )
    len = utilsSnprintf(
      buff,
      buffLen,
      "%s",
      getDeviceDescrLong(
        eseI18nLang_ru,
        type
      )
    );
  
  buff = fmtBuffInc(buff, &buffLen, len);

#endif
  
  // Format serial
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_SERIAL) )
  {
    if( len && 0 < buffLen )
    {
      *buff = ' ';
      buff = fmtBuffInc(buff, &buffLen, 1);
    }
    
    len = utilsSnprintf(
      buff, 
      buffLen, 
      (month && day) ? 
        "%0.5u-%0.4u" : 
        "%0.5u-%0.2u", 
      (unsigned)order,
      (month && day) ?
        (unsigned)year : 
        (unsigned)(year%100)
    );
    
    buff = fmtBuffInc(buff, &buffLen, len);
   
    if( month && day )
    {
      len = utilsSnprintf(buff, buffLen, "-%0.2u-%0.2u", (unsigned)*month, (unsigned)*day);
      buff = fmtBuffInc(buff, &buffLen, len);
    }
  }
  else if( ES_BIT_IS_SET(flags, APPL_ID_FMT_SERIAL_SHORT) )
  {
    if( len && 0 < buffLen )
    {
      *buff = ' ';
      buff = fmtBuffInc(buff, &buffLen, 1);
    }
    
    len = utilsSnprintf(
      buff, 
      buffLen, 
      "%0.5u-%0.2u", 
      (unsigned)order,
      (unsigned)(year%100)
    );
    
    buff = fmtBuffInc(buff, &buffLen, len);
   
    if( month && day )
    {
      len = utilsSnprintf(buff, buffLen, "%0.2u%0.2u", (unsigned)*month, (unsigned)*day);
      buff = fmtBuffInc(buff, &buffLen, len);
    }
  }
  
  // Format firmware version
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_FW) && fwver )
  {
    if( 0 < len && 0 < buffLen )
    {
      *buff = ' ';
      buff = fmtBuffInc(buff, &buffLen, 1);
    }
    len = utilsSnprintf(buff, buffLen, "v%d.%d", (int)fwver->major, (int)fwver->minor);
    
    buff = fmtBuffInc(buff, &buffLen, len);
  }
  
  // Format firmware region
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_REGION) && region )
  {
    // If firmware version was formatted - concat with it
    if( 0 < len && 0 < buffLen )
    {
      if( ES_BIT_IS_SET(flags, APPL_ID_FMT_FW) && fwver )
        *buff = '.';
      else  
        *buff = ' ';
    
      buff = fmtBuffInc(buff, &buffLen, 1);
    }

    len = utilsSnprintf(buff, buffLen, "%u", (unsigned)*region);

    buff = fmtBuffInc(buff, &buffLen, len);
  }  
  
  // Format hardware config
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_HWINFO) && hwInfo )
  {
    if( 0 < len && 0 < buffLen )
    {
      *buff = ' ';
      
      buff = fmtBuffInc(buff, &buffLen, 1);
    }
    
    len = utilsSnprintf(buff, buffLen, "0x%08X", (unsigned)hwInfo->raw);
    
    buff = fmtBuffInc(buff, &buffLen, len);
  }
  
  fmtBuffTerminate(buff, buffEnd);
}

// Internal Universal ID formatting helper
static __inline ESE_STR fmtUniversalCommaAdd(ESE_STR buff, size_t* buffLen)
{
  if( 0 < *buffLen )
  {
    *buff = ',';
    buff = fmtBuffInc(buff, buffLen, 1);
  }
  
  return buff;
}

static void fmtUniversal(ESE_STR buff, size_t buffLen, esU16 flags, esU16 type, esU16 year, esU32 order,
  const EseVerInfo* fwver, const esU8* month, const esU8* day, const esU16* region, const EseHwConfigInfo* hwInfo)
{
  ESE_STR buffEnd = buff+buffLen;
  
  // Always format type
  int len = utilsSnprintf(buff, buffLen, "T:%u", (unsigned)type);
  buff = fmtBuffInc(buff, &buffLen, len);
  
  // Format serial
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_SERIAL) )
  {
    // Year
    buff = fmtUniversalCommaAdd(buff, &buffLen);
    len = utilsSnprintf(buff, buffLen, "Y:%u", (unsigned)year);
    buff = fmtBuffInc(buff, &buffLen, len);    

    // Month
    if( month )
    {
      buff = fmtUniversalCommaAdd(buff, &buffLen);
      len = utilsSnprintf(buff, buffLen, "M:%u", (unsigned)(*month));
      buff = fmtBuffInc(buff, &buffLen, len);
    }
    
    // Day
    if( day )
    {
      buff = fmtUniversalCommaAdd(buff, &buffLen);
      len = utilsSnprintf(buff, buffLen, "D:%u", (unsigned)(*day));
      buff = fmtBuffInc(buff, &buffLen, len);
    }
    
    // Order
    buff = fmtUniversalCommaAdd(buff, &buffLen);
    len = utilsSnprintf(buff, buffLen, "O:%u", (unsigned)order);
    buff = fmtBuffInc(buff, &buffLen, len);
  }
  
  // Format firmware version
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_FW) && fwver )
  {
    buff = fmtUniversalCommaAdd(buff, &buffLen);
    len = utilsSnprintf(buff, buffLen, "V:%d.%d", (int)fwver->major, (int)fwver->minor);
    buff = fmtBuffInc(buff, &buffLen, len);
  }
  
  // Format firmware region
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_REGION) && region )
  {
    buff = fmtUniversalCommaAdd(buff, &buffLen);
    len = utilsSnprintf(buff, buffLen, "R:%u", (unsigned)(*region));
    buff = fmtBuffInc(buff, &buffLen, len);
  }  
  
  // Format hardware config
  if( ES_BIT_IS_SET(flags, APPL_ID_FMT_HWINFO) && hwInfo )
  {
    buff = fmtUniversalCommaAdd(buff, &buffLen);
    len = utilsSnprintf(buff, buffLen, "H:0x%08X", (unsigned)*((esU32*)hwInfo));
    buff = fmtBuffInc(buff, &buffLen, len);
  }
  
  fmtBuffTerminate(buff, buffEnd);
}

#endif // ES_USE_FWID_FORMATTERS
