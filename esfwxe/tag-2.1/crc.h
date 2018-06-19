#ifndef _crc_h_
#define _crc_h_

// misc CRC implementations. each one is configurable for table|pure math implementation
// thus allowing for code tuneups in favor of either performance or space
//
#include <crcConfig.h>

#ifdef __cplusplus
	extern "C" {
#endif

// calculate CRC7
#ifdef USE_CRC7
	esU8 crc7update(esU8 crc, esU8 data);
	esU8 crc7(esU8 crc, const esU8* buff, esU32 buffLen);
#endif

// calculate CRC8
#ifdef USE_CRC8
	esU8 crc8update(esU8 crc, esU8 data);
	esU8 crc8(esU8 crc, const esU8* buff, esU32 buffLen);
#endif

// calculate misc CRC16
//
#ifdef USE_CRC16_CCITT
	esU16 crc16ccitt_update(esU16 crc, esU8 data);
	esU16 crc16ccitt(esU16 crc, const esU8* buff, esU32 buffLen);
#endif

// calculate checksum 16 as of RFC1071
#ifdef USE_CHECKSUM16
	esU16 checksum16(const esU8* buff, esU32 buffLen);
	esU32 calcSumForChecksum16(const esU8* buff, esU32 buffLen);
	esU16 checksum16FromSum(esU32 sum);
#endif

// calculate misc CRC32
//
#ifdef USE_CRC32_IEEE_802_3
	esU32 crc32ieee802_3_update(esU32 crc, esU8 data);
	esU32 crc32ieee802_3(esU32 crc, const esU8* buff, esU32 buffLen);
#endif

#ifdef __cplusplus
	}
#endif

#endif // _crc_h_
