#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <common/utils.h>
#include <common/core/twi_i2c.h>
#include "mc24xx256.h"

// page read timeout
#define MC24XX256_PGREAD_TMO	50
// chip page count
#define MC24XX256_PAGE_COUNT	0x00000200
// page size in bytes
#define MC24XX256_PAGE_SIZE		0x00000040
// local page write buffer - address word + page 
static BYTE s_wrBuff[MC24XX256_PAGE_SIZE+2];

// write bytes to the chip. only 3 lower bits are used from the chipAddr
// internally, paged operations are invoked to perform continuous writes
DWORD mc24xx256PutBytes(i2cHANDLE handle, BYTE chipAddr, WORD memAddr, const BYTE* data, DWORD dataLen)
{
	const BYTE* pos = data;
	const BYTE* end = data+dataLen;
	// make chip address
	BYTE addr = MC24XX256_MAKE_ADDR(chipAddr);
	BOOL writtenOk = TRUE;
	while( memAddr < MC24XX256_SIZE && 
				 writtenOk && pos < end )
	{
		// find initial address within current page
		WORD writeLen;
		WORD pageOffs = memAddr % MC24XX256_PAGE_SIZE;
		s_wrBuff[0] = HIBYTE(memAddr);
		s_wrBuff[1] = LOBYTE(memAddr);
		// how many bytes to write this time
		writeLen = MIN(MC24XX256_PAGE_SIZE - pageOffs, end-pos);
		memcpy(s_wrBuff+2, pos, writeLen);
		// perform (possibly partial) page write
		writtenOk =	i2cPutBytes(handle, addr, s_wrBuff, writeLen + 2) == writeLen + 2;
		
		if( writtenOk )
		{
			// wait some time until internal write completes
			usDelay(5000);
			// increment page, memAddr, data position	& result
			memAddr += writeLen;
			pos += writeLen;
		}
	}

	return pos-data;
}

// sequentially read bytes from memory chip
DWORD mc24xx256GetBytes(i2cHANDLE handle, BYTE chipAddr, WORD memAddr, BYTE* data, DWORD dataLen)
{
	if( dataLen && memAddr < MC24XX256_SIZE && 
			(memAddr + dataLen) <= MC24XX256_SIZE )
	{
		// make chip address
		BYTE addr = MC24XX256_MAKE_ADDR(chipAddr);
		// prepare mem address 
		s_wrBuff[0] = HIBYTE(memAddr);
		s_wrBuff[1] = LOBYTE(memAddr);

		usDelay(10);
		if( i2cPutBytes(handle, addr, s_wrBuff, 2) == 2	)
			return i2cGetBytes(handle, addr, data, dataLen);
	}
	
	return 0;	
}
