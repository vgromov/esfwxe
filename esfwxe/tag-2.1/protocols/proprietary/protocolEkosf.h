#ifndef _protocol_ekosf_h_
#define _protocol_ekosf_h_

// Functionality configuration header
#include <protocolEkosfConfig.h>

// ES proprietary protocol header.
// Initial idea: Sergei Arestov
// Implementation: Vsevolod Gromov
//

#ifdef __cplusplus
	extern "C" {
#endif 

// frame types
#define ES_FT_MASTER	 						  ((esU8)0x7B)
#define ES_FT_SLAVE 							  ((esU8)0x7A)

// command & error code bases
//
#define ES_CMD_REQUEST_BASE				  ((esU16)0x0000)
#define ES_CMD_RESPONSE_BASE			  ((esU16)0x8000)
#define ES_ERROR_BASE							  ((esU16)0xFF00)

// basic commands & error codes
//
// commands...
#define ES_CMD_PING								  (ES_CMD_REQUEST_BASE) 		// "are you there" ping request
#define ES_CMD_ADDR_SET						  (ES_CMD_REQUEST_BASE+1) 	// set new device address
#define ES_CMD_RATE_SET						  (ES_CMD_REQUEST_BASE+3) 	// request new data rate. rate is desired bps value, esU32
#define ES_CMD_RPC_EXEC						  (ES_CMD_REQUEST_BASE+32)	// executes remote procedure call on the responder's side
																																	// data contains esU16 == rpc idGet,
																																	// followed by packed rpc stack
																																	// respond either with: 
																																	// ES_ERR_CMD_UNKNOWN (really bad, we cannot do much with such device)
																																	// ES_ERR_NOTREADY	device is busy executing another procedure, etc.
																																	// ES_CMD_RPC_EXEC	+ ES_CMD_RESPONSE_BASE, followed by response stack

// service macros
//
#define ES_MAKE_RESPONSE( cmd )		  ((esU16)((cmd) + ES_CMD_RESPONSE_BASE))
#define ES_IS_ERROR( cmd )				  ((esBL)((esU16)(cmd) >= ES_ERROR_BASE))

// errors...
//
// generic
#define ES_ERR_DATA_SIZE					  (ES_ERROR_BASE) 					// Data size in header does not equal to real data size...
#define ES_ERR_DATA_MISALIGN			  (ES_ERROR_BASE+1) 				// Wrong data size alignment (not multiple of 2)...
#define ES_ERR_DATA_CRC						  (ES_ERROR_BASE+2) 				// CRC data error
#define ES_ERR_CMD_UNKNOWN				  (ES_ERROR_BASE+3) 				// Unknown command
#define ES_ERR_DATA_TOO_LONG 			  (ES_ERROR_BASE+4)				  // pending data packet is too long
// command-related
#define ES_ERR_DATA_SIZE_UNEXPECTED (ES_ERROR_BASE+5) 			  // actual data size is unexpected for this command
#define ES_ERR_WRONG_RATE					  (ES_ERROR_BASE+6) 				// requested rate value is out of range
#define ES_ERR_WRONG_ADDR					  (ES_ERROR_BASE+7) 				// requested device address is out of range
#define ES_ERR_NOTREADY						  (ES_ERROR_BASE+8)				  // device is not ready

/// Mandatory packet header struct, which may optionally be followed by data packet
///
typedef struct
{
	esU8 frameType;   ///< Frame type specifier (ES_FT_MASTER|ES_FT_SLAVE)
	esU8 devAddr;		  ///< Unique peer device address
	esU16 command;		///< Command or error code response
	esU16 dataLen;		///< If no data follow header, just set this field to 0
	esU16 packetNum;  ///< Server marks its packet's sequence	number by this field
										///  Peer should respond with packet with the same sequence number
	esU16 reserved;   ///< Reserved space, normally set to all 0s
	esU16 crc;				///< 2 byte CRC as of RFC1070

} ProtocolEkosfHdr;

// constants
//

// generics
enum {
	ProtocolEkosfHdr_SZE = sizeof(ProtocolEkosfHdr),
};

/// Functionality interface
///
/// This method should be implemented in target application.
/// Its result should be effective wait for tmo milliseconds.
///
void ekosfWait(esU32 tmo);

// common standard defs
//
#if defined(ESE_USE_RPC_STD_MASTER_IMPL) || defined(ESE_USE_RPC_STD_SLAVE_IMPL)

#include <esfwxe/remoteProcCall.h>

// monitor interface forward decl
struct tagEseProtocolIoMonitor;

#if !defined(_BORLANDC_) && !defined(_MSC_VER)
#	pragma anon_unions
#endif

/// ES protocol standard IO state struct
///
typedef struct {
	union {
		esU8 data[RPC_FRAME_SIZE];		// io data
		struct {
			ProtocolEkosfHdr  hdr; 			// request|response header
			union {
				esU8			      rawData[RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE];// raw data (incl. checksum16) following header
				struct {
					esU16				  idGet;				// rpc idGet
					esU16				  sigOrStat;// rpc signature or returned status
					esU8				  stack[RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE-6];	// rpc stack space (6 = two WORDs and 1 CRC16 at the end of data)
				};
			};
		};
	};

	esU32 packetTmo; 							  // packet timeout in ms
	esU32 pendingRate;							// value for pending rate change
	EseChannelIo* chnl;							// communication channel associated with this io state
	struct tagEseProtocolIoMonitor* mon;			// protocol monitor interface
	esU16 err;											// error reported from protocol communication
	esI16 pendingAddr;							// pending address value for address set command, -1 if nothing to change
	esU8 addr;											// our|recently contacted peer address

} EseStdIoState;

// default io parameters
enum {
	ekosfDefLegacyPeerAddr 	= 5,			// default peer address
	ekosfBroadcastPeerAddr 	= 255,		// default peer address AKA broadcast address. slave will always respond to broadcast packets as well as to its own address packets
	ekosfDefPacketTmo 			=         // default packet timeout in ms.
#ifndef ES_DEF_PACKET_TMO
    100,
#else
    ES_DEF_PACKET_TMO,
#endif    
	ekosfHdrFrameReceiveMaxRetries=   // max count of tries to receive full header frame, identified either by the ES_FT_MASTER or ES_FT_SLAVE markers
#ifndef ES_HDR_FRAME_MAX_RETRIES
    10, 
#else
    ES_HDR_FRAME_MAX_RETRIES,
#endif
	
  ekosfMaxAutoSetupAddrTmo = 255 + ekosfDefPacketTmo,
  
	EseStdIoState_SZE 		  = sizeof(EseStdIoState),
	ekosfMaxDataLen 				= RPC_FRAME_SIZE-ProtocolEkosfHdr_SZE, // max data length is frame size except header length
	ekosfRpcStackOffs 			= 4,				// offset from the beginning of raw data to the start of RPC stack
	ekosfRpcMinDataLen  		= ekosfRpcStackOffs + 2, // minimum data length (incl checksum16) of RPC data packet
	ekosfRpcMaxStackLen			= ekosfMaxDataLen-ekosfRpcMinDataLen, // max length available for rpc stack 	
};

#endif // defined( ESE_USE_RPC_STD_MASTER_IMPL ) || defined(ESE_USE_RPC_STD_SLAVE_IMPL)

// standard server-side services implementation
//
#ifdef ESE_USE_RPC_STD_MASTER_IMPL

// protocol frame monitor services.
//
typedef struct tagEseProtocolIoMonitor {
	void (*protocolFrameSent)(const EseStdIoState* io);
	void (*protocolFrameSendingFailed)(const EseStdIoState* io);
	void (*protocolFrameReceived)(const EseStdIoState* io);
	void (*protocolFrameReceptionFailed)(const EseStdIoState* io);

} EseProtocolIoMonitor;

// protocol monitor API
void ekosfProtocolMonitorInit(EseProtocolIoMonitor* mon);
void ekosfOnFrameSent(const EseStdIoState* io);
void ekosfOnFrameSendingFailed(const EseStdIoState* io);
void ekosfOnFrameReceived(const EseStdIoState* io);
void ekosfOnFrameReceptionFailed(const EseStdIoState* io);

// ------------------------------------------------------------
//
// initialize master-specific io state structure
void ekosfStdMasterIoInit( EseStdIoState* io, EseChannelIo* chnl );
// set protocol monitor
void ekosfProtocolMonitorSet( EseStdIoState* io, EseProtocolIoMonitor* mon );
// ping slave device with io-specified address
esBL ekosfPingSlave( EseStdIoState* io );
// request slave device address change
esBL ekosfSlaveAddrSet( EseStdIoState* io, esU8 newAddr );
// request slave device channel rate change
esBL ekosfSlaveRateSet( EseStdIoState* io, esU32 rate);
// protocol implementation of the single remote procedure call
// rpc stack data must be already formed externally in server data block
// stackSize returns data stack size after successfull call
RpcStatus ekosfDoRPC( EseStdIoState* io, esU16 rpcId, esU16 rpcSig, esU32* stackSize);
// network devices enumeration service with address range auto setup. optional onEnum callback may be used
// to filter enumerated device net|break enumeration process, if FALSE is retured from onEnum.
// returned is esU8 = count of enumerated devices, 0 if process failed|aborted
//
typedef esBL (*ekosfPfnOnEnumDevice)( EseStdIoState* io, esU8 addr, void* data );
esU8 ekosfSlaveEnumerate( EseStdIoState* io, esU8 startAddr, esU8 endAddr, ekosfPfnOnEnumDevice onEnum, void* data);

// rpc call prototypes auto-declaration
#define RPC_REFLECTION_BEGIN
#define	RPC_DEF_HANDLER0(ReturnType) \
RpcStatus ReturnType ## _Rpc(EseStdIoState* io, esU16 rpcId, ReturnType* ret );
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0);
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1);
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2);
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3);
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4);
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5);
#define	RPC_DEF_HANDLER0_NORETURN() \
RpcStatus VOID_Rpc(EseStdIoState* io, esU16 rpcId );
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
RpcStatus VOID_Rpc_ ## Param0Type (EseStdIoState* io, esU16 rpcId, Param0Type p0);
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1);
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2);
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3);
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4);
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5);
#define RPC_REFLECTION_END

#include <esfwxe/rpcMap/rpcReflection.cc>

#endif //	ESE_USE_RPC_STD_MASTER_IMPL

// standard listener client stuff
//
#ifdef ESE_USE_RPC_STD_SLAVE_IMPL

// initialize slave-specific iostate structure
void ekosfStdSlaveIoInit( EseStdIoState* io, EseChannelIo* chnl );
// client command execution returns true if master request was properly parsed and replied
esBL ekosfStdSlaveExecute( EseStdIoState* client );

#endif // ESE_USE_RPC_STD_SLAVE_IMPL

#ifdef __cplusplus
	}
#endif 

#endif // _protocol_ekosf_h_
