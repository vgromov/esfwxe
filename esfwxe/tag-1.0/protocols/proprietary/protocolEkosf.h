#ifndef _protocol_ekosf_h_
#define _protocol_ekosf_h_

// include configuration header
#include <protocolEkosfConfig.h>

// ekosfera proprietary protocol header.
// idea: Sergei Arestov
// implementation: Vsevolod Gromov
//

#ifdef __cplusplus
	extern "C" {
#endif 

// frame types
#define EKOSF_FT_MASTER	 							((BYTE)0x7B)
#define EKOSF_FT_SLAVE 								((BYTE)0x7A)

// command & error code bases
//
#define EKOSF_CMD_REQUEST_BASE				((WORD)0x0000)
#define EKOSF_CMD_RESPONSE_BASE				((WORD)0x8000)
#define EKOSF_ERROR_BASE							((WORD)0xFF00)

// basic commands & error codes
//
// commands...
#define EKOSF_CMD_PING								(EKOSF_CMD_REQUEST_BASE) 		// "are you there" ping request
#define EKOSF_CMD_ADDR_SET						(EKOSF_CMD_REQUEST_BASE+1) 	// set new device address
#define EKOSF_CMD_RATE_SET						(EKOSF_CMD_REQUEST_BASE+3) 	// request new data rate. rate is desired bps value, DWORD 
#define EKOSF_CMD_RPC_EXEC						(EKOSF_CMD_REQUEST_BASE+32)	// executes remote procedure call on the responder's side
																																	// data contains WORD == rpc id,
																																	// followed by packed rpc stack
																																	// respond either with: 
																																	// EKOSF_ERR_CMD_UNKNOWN (really bad, we cannot do much with such device)
																																	// EKOSF_ERR_NOTREADY	device is busy executing another procedure, etc.
																																	// EKOSF_CMD_RPC_EXEC	+ EKOSF_CMD_RESPONSE_BASE, followed by response stack

// service macros
//
#define EKOSF_MAKE_RESPONSE( cmd )		((WORD)((cmd) + EKOSF_CMD_RESPONSE_BASE))
#define EKOSF_IS_ERROR( cmd )					((BOOL)((WORD)(cmd) >= EKOSF_ERROR_BASE))

// errors...
//
// generic
#define EKOSF_ERR_DATA_SIZE						(EKOSF_ERROR_BASE) 					// Data size in header does not equal to real data size...
#define EKOSF_ERR_DATA_MISALIGN				(EKOSF_ERROR_BASE+1) 				// Wrong data size alignment (not multiple of 2)...
#define EKOSF_ERR_DATA_CRC						(EKOSF_ERROR_BASE+2) 				// CRC data error
#define EKOSF_ERR_CMD_UNKNOWN					(EKOSF_ERROR_BASE+3) 				// Unknown command
#define EKOSF_ERR_DATA_TOO_LONG 			(EKOSF_ERROR_BASE+4)				// pending data packet is too long
// command-related
#define EKOSF_ERR_DATA_SIZE_UNEXPECTED (EKOSF_ERROR_BASE+5) 			// actual data size is unexpected for this command
#define EKOSF_ERR_WRONG_RATE					(EKOSF_ERROR_BASE+6) 				// requested rate value is out of range
#define EKOSF_ERR_WRONG_ADDR					(EKOSF_ERROR_BASE+7) 				// requested device address is out of range
#define EKOSF_ERR_NOTREADY						(EKOSF_ERROR_BASE+8)				// device is not ready


// mandatory packet header struct, which may optionally be followed by data packet
//
typedef struct
{
	UINT8 frameType;
	UINT8 devAddr;		// unique peer device address
	UINT16 command;		// command or error code response
	UINT16 dataLen;		// if no data follow header, just set this field to 0
	UINT16 packetNum; // server marks its packet's sequence	number by this field
										// peer should respond with packet with the same sequence number
	UINT16 reserved;
	UINT16 crc;				// 2 byte CRC as of RFC1070

} ProtocolEkosfHdr;

// constants
//

// generics
enum {
	ProtocolEkosfHdr_SZE = sizeof(ProtocolEkosfHdr),
};

// functionality interface
//
// this method should be implemented in target application.
// its result should be effective wait for tmo milliseconds.
void ekosfWait(DWORD tmo);

// common standard defs
//
#if defined(EKOSF_USE_STD_MASTER) || defined(EKOSF_USE_STD_SLAVE)

#include <common/remoteProcCall.h>

#if !defined(_BORLANDC_) && !defined(_MSC_VER)
#	pragma anon_unions
#endif

// ekosfera protocol standard IO state struct
typedef struct {
	union {
		UINT8 data[RPC_FRAME_SIZE];		// io data
		struct {
			ProtocolEkosfHdr	hdr; 			// request|response header
			union {
				UINT8						rawData[RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE];// raw data (incl. checksum16) following header		
				struct {
					UINT16				id;				// rpc id
					UINT16				sigOrStat;// rpc signature or returned status
					UINT8					stack[RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE-6];	// rpc stack space (6 = two WORDs and 1 CRC16 at the end of data)
				};
			};
		};
	};

	UINT32 packetTmo; 							// packet timeout in ms
	UINT32 pendingRate;							// value for pending rate change
	CommChannel* chnl;							// communication channel associated with this Io state
	UINT16 err;											// error reported from protocol communication
	INT16 pendingAddr;							// pending address value for address set command, -1 if nothing to change
	UINT8 addr;											// our|recently contacted peer address
	UINT8 retry;										// current packet retry counter
	UINT8 retries;									// packet retries count

} EkosfStdIoState;

// default io parameters
enum {
	ekosfDefLegacyPeerAddr 	= 5,			// default peer address
	ekosfBroadcastPeerAddr 	= 255,		// default peer address AKA broadcast address. slave will always respond to broadcast packets as well as to its own address packets
	ekosfDefPacketTmo 			= 100,		// default packet timeout in ms.
	ekosfGarbageRecoveryTmo = 1,			// default garbage RX recovery timeout. device will discard all data packets received during this interval, until nothing is returned
	ekosfHdrFrameReceiveMaxRetries= 10, // max count of tries to receive full header frame, identified either by the EKOSF_FT_MASTER or EKOSF_FT_SLAVE markers
	ekosfDefMasterPacketRetries= 3,	// default master packet retry count
	ekosfMaxAutoSetupAddrTmo = 255 + ekosfDefPacketTmo,
	EkosfStdIoState_SZE 		= sizeof(EkosfStdIoState),
	ekosfMaxDataLen 				= RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE, // max data length is frame size except header length
	ekosfRpcStackOffs 			= 4,				// offset from the beginning of raw data to the start of RPC stack
	ekosfRpcMinDataLen  		= ekosfRpcStackOffs + 2, // minimum data length (incl checksum16) of RPC data packet
	ekosfRpcMaxStackLen			= ekosfMaxDataLen-ekosfRpcMinDataLen, // max length available for rpc stack 	
};

#endif // defined( EKOSF_USE_STD_MASTER ) || defined(EKOSF_USE_STD_SLAVE)

// standard server-side services implementation
//
#ifdef EKOSF_USE_STD_MASTER

// initialize master-specific io state structure
void ekosfStdMasterIoInit( EkosfStdIoState* io, CommChannel* chnl );
// ping slave device with io-specified address
BOOL ekosfPingSlave( EkosfStdIoState* io, CommChannelBreak* brk );
// request slave device address change
BOOL ekosfSlaveAddrSet( EkosfStdIoState* io, BYTE newAddr, CommChannelBreak* brk );
// request slave device channel rate change
BOOL ekosfSlaveRateSet( EkosfStdIoState* io, DWORD rate, CommChannelBreak* brk );
// protocol implementation of the single remote procedure call
// rpc stack data must be already formed externally in server data block
// stackSize returns data stack size after successfull call
RpcStatus ekosfDoRPC( EkosfStdIoState* io, WORD rpcId, WORD rpcSig, DWORD* stackSize, CommChannelBreak* brk );
// network devices enumeration service with address range auto setup. optional onEnum callback may be used
// to filter enumerated device net|break enumeration process, if FALSE is retured from onEnum.
// returned is BYTE = count of enumerated devices, 0 if process failed|aborted
//
typedef BOOL (*ekosfPfnOnEnumDevice)( EkosfStdIoState* io, BYTE addr, void* data );
BYTE ekosfSlaveEnumerate( EkosfStdIoState* io, BYTE startAddr, BYTE endAddr, ekosfPfnOnEnumDevice onEnum, void* data, CommChannelBreak* brk );

// rpc call prototypes auto-declaration
#define RPC_REFLECTION_BEGIN
#define	RPC_DEF_HANDLER0(ReturnType) \
RpcStatus ReturnType ## _Rpc(EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret );
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0);
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1);
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2);
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3);
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4);
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5);
#define	RPC_DEF_HANDLER0_NORETURN() \
RpcStatus VOID_Rpc(EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk );
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0);
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1);
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2);
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3);
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4);
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (EkosfStdIoState* io, WORD rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5);
#define RPC_REFLECTION_END

#include <common/rpcMap/rpcReflection.cc>

#endif //	EKOSF_USE_STD_MASTER

// standard listener client stuff
//
#ifdef EKOSF_USE_STD_SLAVE

// initialize slave-specific iostate structure
void ekosfStdSlaveIoInit( EkosfStdIoState* io, CommChannel* chnl );
// client command execution returns true if master request was properly parsed and replied
BOOL ekosfStdSlaveExecute( EkosfStdIoState* client, CommChannelBreak* brk );

#endif // EKOSF_USE_STD_SLAVE

#ifdef __cplusplus
	}
#endif 

#endif // _protocol_ekosf_h_
