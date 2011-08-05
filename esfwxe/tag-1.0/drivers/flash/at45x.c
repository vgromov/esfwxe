#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>
#include <common/core/spi.h>
#include "at45x.h"

#define at45MAX_READY_WAIT 0x0000FFFF

static __inline BOOL internal_at45WriteCmd(spiHANDLE bus, DWORD cmd)
{
	DWORD cmdbuf[2];
	BOOL result = FALSE;
	BYTE opcode = at45GET_STDCMD_OPCODE( cmd );
	DWORD size = at45STDCMD_SIZE;
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

	result = (spiPutBytes( bus, (const BYTE*)cmdbuf, size) == size);
	
	return result;	
}

BOOL at45WaitUntilReady(spiHANDLE bus, BYTE statusReadOpcode)
{	
	BOOL result = FALSE;
	BYTE stat;
	DWORD retries = at45MAX_READY_WAIT;

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

BOOL at45CmdWrite( spiHANDLE bus, DWORD cmd )
{
	BOOL result = FALSE;

	SELECT_at45;

	result = internal_at45WriteCmd(bus, cmd);

	DESELECT_at45;

	return result;
}

DWORD at45Write( spiHANDLE bus, DWORD cmd, const BYTE* buff, DWORD len )
{
	DWORD result = 0;
	
	SELECT_at45;

	if( internal_at45WriteCmd(bus, cmd) )
		result = spiPutBytes( bus, buff, len);

	DESELECT_at45;

	return result;
}

DWORD at45Read( spiHANDLE bus, DWORD cmd, BYTE* buff, DWORD len )
{
	DWORD result = 0;

	SELECT_at45;

	if( internal_at45WriteCmd(bus, cmd) )
		result = spiGetBytes( bus, buff, len);

	DESELECT_at45;

	return result;
}
	
BOOL at45StatusRead( spiHANDLE bus, BYTE statusReadOpcode, BYTE* stat )
{
	BOOL result = FALSE;

	SELECT_at45;	

	result = internal_at45WriteCmd( bus, statusReadOpcode ) && 
		spiGetBytes( bus, stat, 1) == 1;

	DESELECT_at45;

	return result;
}

// small flash chip io check - write predefined pattern to the sram buffer, 
// read it back and compare results
static const DWORD c_at45checkPattern = 0xABBACAAC;
BOOL at45IoCheck(spiHANDLE bus, BYTE statusReadOpcode)
{
	if( at45WaitUntilReady(bus, statusReadOpcode) )
	{
		DWORD tmp;
		if( 4 == at45Write(bus, at45MAKE_STDCMD(at45BUFF1_WRITE, 0, 0), (const BYTE*)&c_at45checkPattern, 4) &&
				at45WaitUntilReady(bus, statusReadOpcode) )
			{
				return 4 == at45Read(bus, at45MAKE_STDCMD((statusReadOpcode == at45STATUS_READ_SPI03) ? at45BUFF1_READ_SPI03 : at45BUFF1_READ_ICPLH, 0, 0), (BYTE*)&tmp, 4) &&
					tmp == c_at45checkPattern;
			}
		}

	return FALSE;
}
