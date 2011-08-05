// unified uartial driver. tuned up freeRTOS source

#ifndef SERIAL_COMMS_H
#define SERIAL_COMMS_H

#ifdef __cplusplus
	extern "C" {
#endif

typedef enum 
{ 
	uartNO_PARITY = -1, 
	uartODD_PARITY, 
	uartEVEN_PARITY, 
	uartMARK_PARITY, 
	uartSPACE_PARITY 
} uartParity;

typedef enum 
{ 
	uartSTOP_1, 
	uartSTOP_2 
} uartStopBits;

typedef enum 
{ 
	uartBITS_5, 
	uartBITS_6, 
	uartBITS_7, 
	uartBITS_8 
} uartDataBits;

typedef enum 
{ 
	uart50 		= 50,		
	uart75 		= 75,		
	uart110		= 110,		
	uart134		= 134,		
	uart150		= 150,    
	uart200		= 200,
	uart300		= 300,		
	uart600		= 600,		
	uart1200		= 1200,	
	uart1800		= 1800,	
	uart2400		= 2400,   
	uart4800		= 4800,
	uart9600		= 9600,		
	uart19200	= 19200,	
	uart38400	= 38400,	
	uart57600	= 57600,	
	uart115200	= 115200
} uartBaud;

// uart DCB structure
typedef struct
{
	uartBaud			baud;
	uartParity		parity;
	uartDataBits	bits;
	uartStopBits stopBits;
	WORD				txTimeout;
	WORD				rxTimeout;				

} uartDCB;

// uart status bits
typedef enum {
	uartInvalidHandle 	= -1,
	uartOK,
	uartRXOverflow,
	uartLineError,
	uartTimeout	

} uartErrorCode;

// uart port handle type
DEF_VOLATILE_HANDLE(uartHANDLE);

#include "uartConfig.h"

typedef enum {
#ifdef USE_UART_PORT0
	uart0,
#endif
#ifdef USE_UART_PORT1
	uart1,
#endif

	uartPortCnt

} uartPort;

// try to mutually exclusive acquire port resource
BOOL uartLockPort(uartHANDLE handle, DWORD timeout);
void uartUnlockPort(uartHANDLE handle);

// access dcb
void uartGetDCB(uartHANDLE handle, uartDCB* dcb);
// configure dcb
BOOL uartSetDCB(uartHANDLE handle, const uartDCB* dcb);

// return uart port number from the port handle, -1 if handle is invalid
int uartGetPortNum( uartHANDLE hPort );

// initialize port
uartHANDLE uartInit( uartPort port, const uartDCB* dcb );

void uartPowerUp(uartHANDLE hPort);
void uartPowerDown(uartHANDLE hPort);

BOOL uartIsOpen(uartHANDLE hPort);
BOOL uartOpen(uartHANDLE hPort);
void uartClose(uartHANDLE hPort);

uartErrorCode uartGetErrorCode(uartHANDLE hPort);

// DWORD timeout specify additional (ensurance) timeout value which will be 
// applied for each RX byte
DWORD uartGetBytes(uartHANDLE hPort, BYTE* pBytes, DWORD count, DWORD timeout, CommChannelBreak* brk);
DWORD uartPutBytes(uartHANDLE hPort, const BYTE* Bytes, DWORD count, CommChannelBreak* brk); 

// reset uart && cleanup rx queue
//
// timeout specifies hoh long to wait for the RX queue to consider it empty
void uartReset(uartHANDLE hPort, DWORD timeout);

#ifdef USE_COMMINTF

// additional services for CommChannel interface
//
// wait (blocking) until TX is empty
BOOL uartWaitTxEmpty(uartHANDLE hPort, CommChannelBreak* brk);
// rate support|change
BOOL uartIsRateSupported(uartHANDLE hPort, DWORD rate);
DWORD uartGetRate(uartHANDLE hPort);
BOOL uartSetRate(uartHANDLE hPort, DWORD rate);
DWORD uartSendTimeEstimateGet(uartHANDLE hPort, DWORD len);
// uart channel initializer
void uartChannelInit(CommChannel* chnl, uartHANDLE bus);

#endif // USE_COMMINTF

#ifdef __cplusplus
	}
#endif

#endif

