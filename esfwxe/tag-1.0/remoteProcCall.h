// remote procedure call implementation for ekosfera intelligent hardware
//
#ifndef _remote_procedure_call_h_
#define _remote_procedure_call_h_

#ifdef __cplusplus
	extern "C" {
#endif						 

// rpc call consists of stack preparation on the server side, stack transfer over the communication channel
// rpc stack receiving and interpretation on the callee side, stack re-fill on the client side and
// response over the communication channel to the caller
//
// caller stack packet consists of WORD proc_id, WORD proc_signature_id,
// optional arguments data
//
// callee response packet consists of WORD proc_id, WORD proc_exec_status,
// optional procedure response in form of byte array
//

// max frame size used for RPC communication
#define RPC_FRAME_SIZE							512

// procedure identifiers
//
#define RPID_NOID										((WORD)0xFFFF)	// special case - empty DBID

// the standard procedure ids range
#define	RPID_STD_BASE								((WORD)0)
#define RPID_STD_END								((WORD)2047)

// the custom proc ids
#define RPID_CUSTOM_BASE						((WORD)(RPID_STD_END+1))
#define RPID_CUSTOM_END							((WORD)(RPID_NOID-1))

// standard procedure ids
enum {
	RPID_STD_CAPS_GET									= RPID_STD_BASE,		// return all procedures supported by the hardware
	RPID_STD_FWID_GET, 																		// firmware identification block
	RPID_STD_HW_INFO_GET,																	// hardware info block
	RPID_STD_STORAGE_INFO_GET,														// storage info block
	RPID_STD_HEALTH_INFO_GET,															// health status
	RPID_STD_HEALTH_EXEC,																	// execute self-test
	RPID_STD_DATETIME_SET,																// date time access procedures
	RPID_STD_DATETIME_GET,
	RPID_STD_FACTORY_RESET,																// reset to factory defaults
	RPID_STD_SHUTDOWN,																		// remotely power down device
	RPID_STD_ENTER_BOOT,																	// remotely	enter bootloader
	RPID_STD_POWER_STATUS_GET,														// return device power status in PowerStatus struct
	RPID_STD_DIR_LIST_START,															// (re)start directory listing	
	RPID_STD_DIR_LIST_NEXT_GET,														// get next item of directory listing
	RPID_STD_DIR_DELETE,																	// delete directory
	RPID_STD_FILE_DELETE,																	// delete file
	RPID_STD_FREESPACE_GET,																// get free file system space info
	RPID_STD_FILE_READ_START,															// prepare file reading sequence
	RPID_STD_FILE_READ_NEXT,															// request next file data chunk
	RPID_STD_HW_UID_GET,																	// retrieve hardware unique ID (128bit, in text form, i.e. char[32]), if supported by hardware, of course...
	RPID_STD_FINDME,																			// perform some kind of visual|audio device identification. useful in network configurations
};

// remote procedure call status
//
typedef enum {
	RpcOK,														// procedure call succeeded
	RpcNotImplemented,								// rpc with this id is not implemented
	RpcStackCorrupt,									// rpc stack is corrupt and unusable
	RpcTimedOut,											// RPC execution was prematurely cancelled due to timeout
	RpcStackOverflow,									// too many parameters, rpc stack overflow occured
	RpcUnknownSignature,							// RPC signature is unknown
	RpcSignatureMismatch,							// RPC signature of caller and callee do not match each other
	RpcResponseIdMismatch,						// RPC procedure id of response does not match one of request 
	RpcCommunicationError,						// RPC failed due to communication layer
	RpcNotReady,											// RPC not ready to respond
	// rpc parameter value errors
	RpcParam0ValueError,
	RpcParam1ValueError,
	RpcParam2ValueError,
	RpcParam3ValueError,
	RpcParam4ValueError,
	RpcParam5ValueError,
	RpcCancelled,											// RPC execution is cancelled

} RpcStatus;

// core of rpc procedural reflection - rpc wrappers. the following defines rpc call signature & wrapper type
// up to 6 parameters are supported
//
// generate rpc reflection signatures
#define RPC_REFLECTION_BEGIN \
	typedef enum { RpcSig_NULL,

#define	RPC_DEF_HANDLER0(ReturnType) \
	ReturnType ## _RpcSig,
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
	ReturnType ## _RpcSig_ ## Param0Type,
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
	ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type,
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
	ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type,
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
	ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type,
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
	ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type,
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
	ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type,
#define	RPC_DEF_HANDLER0_NORETURN() \
	VOID_RpcSig,
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
	VOID_RpcSig_ ## Param0Type,
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
	VOID_RpcSig_ ## Param0Type ## _ ## Param1Type,
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
	VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type,
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
	VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type,
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
	VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type,
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
	VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type,

#define RPC_REFLECTION_END \
		RpcReflectionSignatureCnt \
	} RpcReflectionSignature;

#include <common/rpcMap/rpcReflection.cc>

// execute procedure at the side of the callee. 
// endMax marks an end of the stack space allocated 
// by the reflection engine
RpcStatus rpcExecLocal(WORD id, WORD sig, BYTE* stack, DWORD* stackLen, DWORD stackMaxLen);

// service interface
//
// id checks
#define rpcIsStdPID( pId )			((BOOL)((WORD)(pId) >= RPID_STD_BASE && (WORD)(pId) <= RPID_STD_END))
#define rpcIsCustomPID( pId )		((BOOL)((WORD)(pId) >= RPID_CUSTOM_BASE && (WORD)(pId) <= RPID_CUSTOM_END))
#define rpcIsPID( pId )					((BOOL)(rpcIsStdPID(pId) || rpcIsCustomPID(pId)))

#ifdef __cplusplus
	}
#endif

#endif // _Data_Blocks_h_
