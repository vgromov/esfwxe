// I2C aka "two wire" IO support
#include <common/target.h>
#pragma hdrstop

#include <common/utils.h>

#if defined(USE_FREE_RTOS)
// rtos stuff
#	include <common/core/rtos_intf.h>
#endif 

// driver stuff
#include "twi_i2c.h"
#include <common/irq.h>

enum {
	i2cPortMask = 0x0007,
	i2cOpened		= 0x0008,
	i2cReady		= 0x0010,
};

// i2c port struct
typedef struct
{
	WORD 					dcr;
	i2cDCB 				dcb;
	i2cErrorCode	err;
	DWORD					lineStatus;
#if defined(USE_FREE_RTOS)
	// one port may not be shared among different threads, so set-up mutex for high level access
	rtosMUTEX_HANDLE  mutex;
#endif
	// direct reference to the data requested|posted to i2c IO
	// at the beginning of the data send|receive buffIO points to data
	// buffLen specifies buffer length and buffPos is 0
	BYTE					addrIO;
	BYTE*					pos;
	const BYTE*		end;

} i2cPortStruct;

static volatile i2cPortStruct ports[i2cPortCnt];

// try to aquire i2c port for task.
BOOL i2cLockPort(i2cHANDLE handle, DWORD timeout)
{
#if defined(USE_FREE_RTOS)
	if( handle != INVALID_HANDLE )
		return rtosMutexLock( CAST_PORT_HANDLE(i2cPortStruct, handle)->mutex, timeout );

	return FALSE;
#else
	return TRUE;
#endif
}

// release task locking on port handle
void i2cUnlockPort(i2cHANDLE handle)
{
#if defined(USE_FREE_RTOS)
	if( handle != INVALID_HANDLE )
		rtosMutexUnlock( CAST_PORT_HANDLE(i2cPortStruct, handle)->mutex );
#endif
}

// common (MCU-independent) API implementation part
void i2cGetDCB(i2cHANDLE handle, i2cDCB* dcb)
{
	if( handle != INVALID_HANDLE && dcb != NULL )
		*dcb = CAST_PORT_HANDLE(i2cPortStruct, handle)->dcb;
}

BOOL i2cIsOpen( i2cHANDLE handle )
{
	return handle != INVALID_HANDLE && (i2cOpened == (CAST_PORT_HANDLE(i2cPortStruct, handle)->dcr & i2cOpened));
}

// access line status and status code
i2cErrorCode i2cGetErrorCode( i2cHANDLE handle )
{
	if( handle != INVALID_HANDLE )
		return CAST_PORT_HANDLE(i2cPortStruct, handle)->err;

	return i2cInvalidHandle;
}

////////////////////////////////////////////// MCU and hw - dependent part goes here
#if LPC23XX == 1
	#include "lpc23xx/twi_i2c_hw.cc"
#endif

#if LPC214X == 1
	#include "lpc214x/twi_i2c_hw.cc"
#endif

#if LPC2103 == 1
	#include "lpc2103/twi_i2c_hw.cc"
#endif 
///////////////////////////////////////////////////////////// rest of implementation
//

void i2cOpen( i2cHANDLE handle )
{
	if( handle != INVALID_HANDLE && !i2cIsOpen(handle) )
	{
		DEF_PORT_STRUCT_VAR(i2cPortStruct, handle);
		switch(ps->dcr & i2cPortMask)
		{
	#ifdef USE_I2C_PORT0
		case i2c0:
			I2C_ENABLE_PWR(0); // enable power
			I2C_ENABLE(0);
			break;
	#endif
	#ifdef USE_I2C_PORT1
		case i2c1:
			I2C_ENABLE(1);
			I2C_ENABLE_PWR(1); // enable power
			break;
	#endif
	#ifdef USE_I2C_PORT2
		case i2c2:
			I2C_ENABLE(2);
			I2C_ENABLE_PWR(2); // enable power
			break;
	#endif
		}

		ps->err = i2cOK;
		ps->lineStatus = 0;
		ps->dcr |= i2cReady | i2cOpened;
	}
}

void i2cClose( i2cHANDLE handle )
{	
	if( i2cIsOpen(handle) )
	{
		DEF_PORT_STRUCT_VAR(i2cPortStruct, handle);
		switch(ps->dcr & i2cPortMask)
		{
	#ifdef USE_I2C_PORT0
		case i2c0:
			I2C_RESET(0);
			I2C_DISABLE_PWR(0); // disable power
			break;
	#endif
	#ifdef USE_I2C_PORT1
		case i2c1:
			I2C_RESET(1);
			I2C_DISABLE_PWR(1); // disable power
			break;
	#endif
	#ifdef USE_I2C_PORT2
		case i2c2:
			I2C_RESET(2);
			I2C_DISABLE_PWR(2); // disable power
			break;
	#endif
		}

		ps->dcr &= ~(i2cReady | i2cOpened);
	}
}

static BOOL i2cStart(i2cHANDLE handle)
{
	if( i2cIsOpen(handle) )
	{
		DWORD dwCurRetry = 0;
		DEF_PORT_STRUCT_VAR(i2cPortStruct, handle);

		switch(ps->dcr & i2cPortMask)
		{
	#ifdef USE_I2C_PORT0
		case i2c0:
	  	I2C_SEND_START(0);	// issue start
			break;
	#endif
	#ifdef USE_I2C_PORT1
		case i2c1:
	  	I2C_SEND_START(1);	// issue start
			break;
	#endif
	#ifdef USE_I2C_PORT2
		case i2c2:
	  	I2C_SEND_START(2);	// issue start
			break;
	#endif
		}
		// wait some time until start condition is set
	 	while( (i2cReady == (ps->dcr & i2cReady)) && dwCurRetry < MAX_I2C_RETRIES )
			++dwCurRetry;
				
		return dwCurRetry < MAX_I2C_RETRIES;
	}

	return FALSE;
}

static void i2cWaitForStop(i2cHANDLE handle)
{
	if( i2cIsOpen(handle) )
		while( (i2cReady != (CAST_PORT_HANDLE(i2cPortStruct, handle)->dcr & i2cReady) ) );
}

#ifdef USE_I2C_MASTER_MODE
static DWORD i2cMasterIo(i2cHANDLE handle, BYTE addrRW, BYTE* buff, DWORD length)
{
	DWORD result = 0;

	if( length )
	{
		DEF_PORT_STRUCT_VAR(i2cPortStruct, handle);
		// initialize address and buffer info
		ps->addrIO = addrRW;
		ps->pos = buff;
		ps->end = buff+length;
		ps->err = i2cOK;
		ps->lineStatus = 0;

		if( i2cStart(handle) )		
			i2cWaitForStop(handle);
	
		result = ps->pos-buff;
	}

	return result;
}

#endif // USE_I2C_MASTER_MODE

#ifdef USE_I2C_SLAVE_MODE
#error "Not implemented"
#endif // USE_I2C_SLAVE_MODE

DWORD i2cGetBytes(i2cHANDLE handle, BYTE addr, BYTE* pBytes, DWORD count)
{
	if( i2cIsOpen(handle) && pBytes && count )
	{
#ifdef USE_I2C_MASTER_MODE
		if( CAST_PORT_HANDLE(i2cPortStruct, handle)->dcb.mode == i2cMaster )
			return i2cMasterIo(handle, addr | TWI_READ, pBytes, count);
#endif
#ifdef USE_I2C_SLAVE_MODE
	#error "Not implemented"
#endif
	}

	return 0;
}

DWORD i2cPutBytes(i2cHANDLE handle, BYTE addr, const BYTE* pBytes, DWORD count)
{
	if( i2cIsOpen(handle) && pBytes && count )
	{
#ifdef USE_I2C_MASTER_MODE
		if( CAST_PORT_HANDLE(i2cPortStruct, handle)->dcb.mode == i2cMaster )
			return i2cMasterIo(handle, addr, (BYTE*)pBytes, count);
#endif
#ifdef USE_I2C_SLAVE_MODE
	#error "Not implemented"
#endif
	}

	return 0;
}

// initialize i2c according to the dcb data
i2cHANDLE i2cInit(i2cPort port, const i2cDCB* dcb)
{
	switch( port )
	{
#ifdef USE_I2C_PORT0
	case i2c0:
		I2C_CONFIG(0, dcb);
		switch( dcb->mode )
		{
#ifdef USE_I2C_SLAVE_MODE 
		case i2cSlave: 	
			I2C_INIT_SLAVE(0, dcb)
			break;
#endif // USE_I2C_SLAVE_MODE
#ifdef USE_I2C_MASTER_MODE
		case i2cMaster:
			I2C_INIT_MASTER(0, dcb)
			break;
#endif // USE_I2C_MASTER_MODE
		}
		break;
#endif // USE_I2C_PORT0

#ifdef USE_I2C_PORT1
	case i2c1:
		I2C_CONFIG(1, dcb);
		switch( dcb->mode )
		{
#ifdef USE_I2C_SLAVE_MODE 
		case i2cSlave: 	
			I2C_INIT_SLAVE(1, dcb)
			break;
#endif // USE_I2C_SLAVE_MODE
#ifdef USE_I2C_MASTER_MODE
		case i2cMaster:
			I2C_INIT_MASTER(1, dcb)
			break;
#endif // USE_I2C_MASTER_MODE
		}
		break;
#endif // USE_I2C_PORT1

#ifdef USE_I2C_PORT2
	case i2c2:
		I2C_CONFIG(2, dcb);
		switch( dcb->mode )
		{
#ifdef USE_I2C_SLAVE_MODE 
		case i2cSlave: 	
			I2C_INIT_SLAVE(2, dcb)
			break;
#endif // USE_I2C_SLAVE_MODE
#ifdef USE_I2C_MASTER_MODE
		case i2cMaster:
			I2C_INIT_MASTER(2, dcb)
			break;
#endif // USE_I2C_MASTER_MODE
		}
		break;
#endif // USE_I2C_PORT2
	}

	return INVALID_HANDLE;
}
