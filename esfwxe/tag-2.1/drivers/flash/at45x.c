#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include <esfwxe/core/spi.h>
#include "at45x.h"

static __inline esBL internal_at45WriteCmd(spiHANDLE bus, esU32 cmd)
{
	esU32 cmdbuf[2];
	esBL result = FALSE;
	esU8 opcode = at45GET_STDCMD_OPCODE( cmd );
	esU32 size = at45STDCMD_SIZE;
	cmdbuf[0] = cmd;
	cmdbuf[1] = 0;

	switch( opcode )
	{
#ifndef at45_REDUCED_FUNCTIONALITY
	case at45READ_MFG_AND_DEVICE_ID:
	case at45ENTER_DEEP_POWER_DOWN:
	case at45RESUME_DEEP_POWER_DOWN:
#endif
	case at45STATUS_READ_ICPLH:
	case at45STATUS_READ_SPI03:
		size = 1;
		break;
	case at45PAGE_READ_ICPLH:
	case at45PAGE_READ_SPI03:
	case at45ARRAY_READ_ICPLH:
	case at45ARRAY_READ_SPI03:
		// 4 extra don't care bytes neded
		size += 4;
		break;
	case at45BUFF1_READ_ICPLH:
	case at45BUFF1_READ_SPI03:
	case at45BUFF2_READ_ICPLH:
	case at45BUFF2_READ_SPI03:
		// 1 extra don't care byte needed
		++size;
		break;
	}

	result = (spiPutBytes( bus, (const esU8*)cmdbuf, size) == size);
	
	return result;	
}

esBL at45WaitUntilReady(spiHANDLE bus, esU8 statusReadOpcode)
{	
	esBL result = FALSE;
	esU8 stat;
	esU32 retries = at45MAX_READY_WAIT;

	SELECT_at45;

	if( internal_at45WriteCmd(bus, statusReadOpcode) )
	{
		while( spiGetBytes( bus, &stat, 1) == 1 && retries-- && !result )
		{
			usDelay(1);
			result = at45STATUS_READY == (stat & at45STATUS_READY);
		}
	}

	DESELECT_at45;

	return result;
}

esBL at45CmdWrite( spiHANDLE bus, esU32 cmd )
{
	esBL result = FALSE;

	SELECT_at45;

	result = internal_at45WriteCmd(bus, cmd);

	DESELECT_at45;

	return result;
}

esU32 at45Write( spiHANDLE bus, esU32 cmd, const esU8* buff, esU32 len )
{
	esU32 result = 0;
	
	SELECT_at45;

	if( internal_at45WriteCmd(bus, cmd) )
		result = spiPutBytes( bus, buff, len);

	DESELECT_at45;

	return result;
}

esU32 at45Read( spiHANDLE bus, esU32 cmd, esU8* buff, esU32 len )
{
	esU32 result = 0;

	SELECT_at45;

	if( internal_at45WriteCmd(bus, cmd) )
		result = spiGetBytes( bus, buff, len);

	DESELECT_at45;

	return result;
}
	
esBL at45StatusRead( spiHANDLE bus, esU8 statusReadOpcode, esU8* stat )
{
	esBL result = FALSE;

	SELECT_at45;	

	result = internal_at45WriteCmd( bus, statusReadOpcode ) && 
		spiGetBytes( bus, stat, 1) == 1;

	DESELECT_at45;

	return result;
}

// small flash chip io check - write predefined pattern to the sram buffer, 
// read it back and compare results
static const esU32 c_at45checkPattern = 0xABBACAAC;
esBL at45IoCheck(spiHANDLE bus, esU8 statusReadOpcode)
{
	if( at45WaitUntilReady(bus, statusReadOpcode) )
	{
		esU32 tmp;
		if( 4 == at45Write(bus, at45MAKE_STDCMD(at45BUFF1_WRITE, 0, 0), (const esU8*)&c_at45checkPattern, 4) &&
				at45WaitUntilReady(bus, statusReadOpcode) )
			{
				return 4 == at45Read(bus, at45MAKE_STDCMD((statusReadOpcode == at45STATUS_READ_SPI03) ? at45BUFF1_READ_SPI03 : at45BUFF1_READ_ICPLH, 0, 0), (esU8*)&tmp, 4) &&
					tmp == c_at45checkPattern;
			}
		}

	return FALSE;
}
