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
	BYTE crc7update(BYTE crc, BYTE data);
	BYTE crc7(BYTE crc, const BYTE* buff, DWORD buffLen);
#endif

// calculate CRC8
#ifdef USE_CRC8
	BYTE crc8update(BYTE crc, BYTE data);
	BYTE crc8(BYTE crc, const BYTE* buff, DWORD buffLen);
#endif

// calculate misc CRC16
//
#ifdef USE_CRC16_CCITT
	WORD crc16ccitt_update(WORD crc, BYTE data);
	WORD crc16ccitt(WORD crc, const BYTE* buff, DWORD buffLen);
#endif

// calculate checksum 16 as of RFC1071
#ifdef USE_CHECKSUM16
	WORD checksum16(const BYTE* buff, DWORD buffLen);
	DWORD calcSumForChecksum16(const BYTE* buff, DWORD buffLen);
	WORD checksum16FromSum( DWORD sum );
#endif

#ifdef __cplusplus
	}
#endif

#endif // _crc_h_
