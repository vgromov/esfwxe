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
	uart1200	= 1200,	
	uart1800	= 1800,	
	uart2400	= 2400,   
	uart4800	= 4800,
	uart9600	= 9600,		
	uart19200	= 19200,	
	uart38400	= 38400,	
	uart57600	= 57600,	
	uart115200= 115200,
	uart128000= 128000,
	uart153600= 153600,
	uart230400= 230400,
	uart256000= 256000,
	uart460800= 460800,
	uart921600= 921600

} uartBaud;

// uart DCB structure
typedef struct
{
	uartBaud			baud;
	uartParity		parity;
	uartDataBits	bits;
	uartStopBits 	stopBits;
	esU32				txTimeout;
	esU32				rxTimeout;				

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
esBL uartLockPort(EseChannelIo* p, esU32 timeout);
void uartUnlockPort(EseChannelIo* p);

// access dcb
void uartGetDCB(EseChannelIo* p, uartDCB* dcb);
// configure dcb
esBL uartSetDCB(EseChannelIo* p, const uartDCB* dcb);

// return uart port number from the port handle, -1 if handle is invalid
int uartGetPortNum(uartHANDLE hPort);

// initialize port
uartHANDLE uartInit(uartPort port, const uartDCB* dcb);

void uartPowerUp(uartHANDLE hPort);
void uartPowerDown(uartHANDLE hPort);

esBL uartIsOpen(EseChannelIo* p);
esBL uartOpen(EseChannelIo* p);
void uartClose(EseChannelIo* p);

uartErrorCode uartGetErrorCode(EseChannelIo* p);

// esU32 timeout specify additional (ensurance) timeout value which will be
// applied for each RX byte
esU32 uartGetBytes(EseChannelIo* p, esU8* pBytes, esU32 count, esU32 timeout);
esU32 uartPutBytes(EseChannelIo* p, const esU8* Bytes, esU32 count);

// generate outgoing break condition on UART
void uartBreakConditionGenerate(EseChannelIo* p);

// reset uart && cleanup rx queue
//
// timeout specifies hoh long to wait for the RX queue to consider it empty
void uartReset(EseChannelIo* p);

// additional services for EseChannelIo interface
//
// wait (blocking) until TX is empty
esBL uartWaitTxEmpty(EseChannelIo*);
// rate support|change
esBL uartIsRateSupported(EseChannelIo*, esU32 rate);
esU32 uartGetRate(EseChannelIo*);
esBL uartSetRate(EseChannelIo*, esU32 rate);
esU32 uartSendTimeEstimateGet(EseChannelIo*, esU32 len);
// uart channel initializer
void uartChannelInit(EseChannelIo* chnl, uartHANDLE bus);

#ifdef __cplusplus
	}
#endif

#endif

