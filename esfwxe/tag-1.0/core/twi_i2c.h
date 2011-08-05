#ifndef _twi_i2c_h_
#define _twi_i2c_h_

// I2C generic driver API 
#include <i2cConfig.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum {
#ifdef USE_I2C_PORT0
	i2c0,
#endif
#ifdef USE_I2C_PORT1
	i2c1,
#endif
#ifdef USE_I2C_PORT2
	i2c2,
#endif
#ifdef USE_I2C_PORT3
	i2c3,
#endif
	i2cPortCnt

} i2cPort;

// i2c common protocol constants
#define TWI_READ						0x01 // read mark bit (set in client address to read from it)
// TWSR values (not bits)
// (taken from avr-libc twi.h - thank you Marek Michalkiewicz)
// Master
#define TW_START						0x08
#define TW_REP_START				0x10
// Master Transmitter
#define TW_MT_SLA_ACK				0x18
#define TW_MT_SLA_NACK			0x20
#define TW_MT_DATA_ACK			0x28
#define TW_MT_DATA_NACK			0x30
#define TW_MT_ARB_LOST			0x38
// Master Receiver
#define TW_MR_ARB_LOST			0x38
#define TW_MR_SLA_ACK				0x40
#define TW_MR_SLA_NACK			0x48
#define TW_MR_DATA_ACK			0x50
#define TW_MR_DATA_NACK			0x58
// Slave Transmitter
#define TW_ST_SLA_ACK				0xA8
#define TW_ST_ARB_LOST_SLA_ACK		0xB0
#define TW_ST_DATA_ACK			0xB8
#define TW_ST_DATA_NACK			0xC0
#define TW_ST_LAST_DATA			0xC8
// Slave Receiver
#define TW_SR_SLA_ACK				0x60
#define TW_SR_ARB_LOST_SLA_ACK		0x68
#define TW_SR_GCALL_ACK			0x70
#define TW_SR_ARB_LOST_GCALL_ACK	0x78
#define TW_SR_DATA_ACK			0x80
#define TW_SR_DATA_NACK			0x88
#define TW_SR_GCALL_DATA_ACK		0x90
#define TW_SR_GCALL_DATA_NACK		0x98
#define TW_SR_STOP					0xA0
// Misc
#define TW_NO_INFO					0xF8
#define TW_BUS_ERROR				0x00

// defines and constants
#define TWCR_CMD_MASK				0x0F
#define TWSR_STATUS_MASK		0xF8

// i2c port mode
typedef enum
{ 
	i2cMaster,
	i2cSlave

} i2cMode;

// i2c device control block
typedef struct
{
	DWORD				baud;  			// when initializing i2c port, this value is considered as
													// desirable speed in Hz. Actually set value may vary. 
	i2cMode			mode;
	BYTE				slaveAddr;	// address this device will use when in slave i2c mode
	BYTE				dummy;			// dummy aligner
		
} i2cDCB;

// i2c line error codes
typedef enum 
{
	i2cInvalidHandle = -1,
	i2cOK,
	i2cNoDev,
	i2cBufferOverflow,
	i2cArbtLost,
	i2cBusError

} i2cErrorCode; 

// i2c port handle type
DEF_VOLATILE_HANDLE(i2cHANDLE);

// try to aquire i2c port for task.
BOOL i2cLockPort(i2cHANDLE handle, DWORD timeout);
// release task locking on port handle
void i2cUnlockPort(i2cHANDLE handle);

// i2c API
void i2cGetDCB(i2cHANDLE handle, i2cDCB* dcb);
i2cHANDLE i2cInit(i2cPort port, const i2cDCB* dcb);

BOOL i2cIsOpen( i2cHANDLE handle );
void i2cOpen( i2cHANDLE handle );
void i2cClose( i2cHANDLE handle );

i2cErrorCode i2cGetErrorCode( i2cHANDLE handle );

// send|receive bytes in the currently set i2c mode
DWORD i2cGetBytes(i2cHANDLE handle, BYTE addr, BYTE* pBytes, DWORD count);
DWORD i2cPutBytes(i2cHANDLE handle, BYTE addr, const BYTE* pBytes, DWORD count); 

#ifdef __cplusplus
	}
#endif

#endif
