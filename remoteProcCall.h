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
// caller stack packet consists of esU16 proc_id, esU16 proc_signature_id,
// optional arguments data
//
// callee response packet consists of esU16 proc_id, esU16 proc_exec_status,
// optional procedure response in form of byte array
//

// max frame size used for RPC communication
#ifndef ESE_RPC_FRAME_SIZE
# define ESE_RPC_FRAME_SIZE                       512
#endif

// procedure identifiers
//
#define RPID_NOID                                 ((esU16)0xFFFF)       ///< special case - empty DBID

// the standard procedure ids range
#define RPID_STD_BASE                             ((esU16)0)
#define RPID_STD_END                              ((esU16)2047)

// the custom proc ids
#define RPID_CUSTOM_BASE                          ((esU16)(RPID_STD_END+1))
#define RPID_CUSTOM_END                           ((esU16)(RPID_NOID-1))

// standard procedure ids
enum {
  RPID_STD_CAPS_GET                               = RPID_STD_BASE,      ///< Return all procedures supported by the hardware
  RPID_STD_FWID_GET,                                                    ///< Request Firmware identification block. For modern devices,
                                                                        ///< which support dual identification feature, this request will return 
                                                                        ///< legacy identifier. To obtain an actual modern one,
                                                                        ///< the RPID_STD_FWID_MODERN_GET request should be used, if supported.
  RPID_STD_HW_INFO_GET,                                                 ///< Hardware info block
  RPID_STD_STORAGE_INFO_GET,                                            ///< Storage info block
  RPID_STD_HEALTH_INFO_GET,                                             ///< Health status
  RPID_STD_HEALTH_EXEC,                                                 ///< Execute self-test
  RPID_STD_DATETIME_SET,                                                ///< Date time access procedures
  RPID_STD_DATETIME_GET,
  RPID_STD_FACTORY_RESET,                                               ///< Reset to factory defaults
  RPID_STD_SHUTDOWN,                                                    ///< Remotely power down device
  RPID_STD_ENTER_BOOT,                                                  ///< Remotely enter bootloader
  RPID_STD_POWER_STATUS_GET,                                            ///< Return device power status in EsePowerStatus struct
  RPID_STD_DIR_LIST_START,                                              ///< (Re)Start directory listing    
  RPID_STD_DIR_LIST_NEXT_GET,                                           ///< Get next item of directory listing
  RPID_STD_DIR_DELETE,                                                  ///< Delete directory
  RPID_STD_FILE_DELETE,                                                 ///< Delete file
  RPID_STD_FREESPACE_GET,                                               ///< Get free file system space info
  RPID_STD_FILE_READ_START,                                             ///< Prepare file reading sequence
  RPID_STD_FILE_READ_NEXT,                                              ///< Request next file data chunk
  RPID_STD_HW_UID_GET,                                                  ///< Retrieve hardware unique ID (128bit, in text form, i.e. char[32]), if supported by hardware, of course...
  RPID_STD_FINDME,                                                      ///< Perform some kind of visual|audio device identification. useful in network configurations
  RPID_STD_SW_INFO_GET,                                                 ///< Request software information block
  RPID_STD_FWID_GET_MODERN,                                             ///< Request modern firmware identification. For devices with dual identification feature.
  /// For devices, which implement its own ISP
  ///
  RPID_STD_FW_UPLOAD_START,                                             ///< Request FW upload start. Signature && Payload: esU32(Max fw frame size)_RpcSig_esU32(Flags)_esBA(Optional start data)
  RPID_STD_FW_UPLOAD,                                                   ///< Request FW portion upload. Signature && Payload: VOID_RpcSig_esBA(FW portion data)
  RPID_STD_FW_UPLOAD_END                                                ///< Request FW upload end. VOID_RpcSig
};

// remote procedure call status
//
typedef enum {
  RpcOK,                                                                ///< Procedure call succeeded
  RpcNotImplemented,                                                    ///< RPC with this ID is not implemented/not registered
  RpcStackCorrupt,                                                      ///< RPC stack is corrupt and unusable
  RpcTimedOut,                                                          ///< RPC execution was prematurely cancelled due to timeout
  RpcStackOverflow,                                                     ///< Too many parameters, rpc stack overflow occured
  RpcUnknownSignature,                                                  ///< RPC signature is unknown
  RpcSignatureMismatch,                                                 ///< RPC signature of caller and callee do not match each other
  RpcResponseIdMismatch,                                                ///< RPC procedure idGet of response does not match one of request 
  RpcCommunicationError,                                                ///< RPC failed due to communication layer
  RpcNotReady,                                                          ///< RPC not ready to respond
  
  /// RPC parameter value errors
  RpcParam0ValueError,
  RpcParam1ValueError,
  RpcParam2ValueError,
  RpcParam3ValueError,
  RpcParam4ValueError,
  RpcParam5ValueError,
  RpcCancelled                                                          ///< RPC execution is cancelled

} RpcStatus;

// core of rpc procedural reflection - rpc wrappers. the following defines rpc call signature & wrapper type
// up to 6 parameters are supported
//
// generate rpc reflection signatures
#define RPC_REFLECTION_BEGIN \
    typedef enum { RpcSig_NULL,

#define  RPC_DEF_HANDLER0(ReturnType) \
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
#define  RPC_DEF_HANDLER0_NORETURN() \
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

#include <esfwxe/rpcMap/rpcReflection.cc>

/// Execute procedure at the side of the callee. 
/// endMax marks an end of the stack space allocated 
/// by the reflection engine
///
#ifndef ESE_USE_DYNAMIC_RPC_REGISTRY

RpcStatus rpcExecLocal(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen);

#else //< ESE_USE_DYNAMIC_RPC_REGISTRY

typedef struct tagRemoteProcMoniker
{
  esU16 id;             //< The procedure identifier
  esU16 sig;            //< The procedure call signature
  void* proc;           //< Actual RPC hanlder procedure that should be called
  void* rpcProxy;       //< RPC proxy with signature corrsponding to sig
  struct tagRemoteProcMoniker* prev;
  struct tagRemoteProcMoniker* next;

} RemoteProcMoniker;

/// RPC dynamic registry and execution context
typedef struct tagRemoteProcContext {
  RemoteProcMoniker root;
  esU32 memcacheSize;
  void* param;
  // Memcache follows...
  
} RemoteProcContext;
typedef void* ESE_HRPCCTX;

/// @brief Create RPC dynamic registry and execution context
/// Post-condition - a new RPC dynamic context is created, and one most basic procedure (registered IDs requester) is registered
/// @param [in]       outMemCacheSize - Size of additional memory cache for RPC handlers to use for temporary output parameters
/// @param [in]       param           - An optional arbitrary information parameter which is passed through context
/// @return                           - created context handle, or NULL, if unsuccessful
///
ESE_HRPCCTX rpcContextCreate(int outMemCacheSize, void* param);

/// Remove context, unregistering procedures, and deallocating all memory resources
void rpcContextDelete(ESE_HRPCCTX ctx);

/// Register RPC handler in RPC context
bool rpcProcedureRegister(ESE_HRPCCTX ctx, int id, int sig, void* pfn);

/// UnRegister RPC handler from RPC context
void rpcProcedureUnRegister(ESE_HRPCCTX ctx, int id);

/// Return count of registered RPC handlers for specific RPC context
int rpcContextEntriesCountGet(ESE_HRPCCTX ctx);

/// Return size of RPC context output memcache
int rpcContextMemcacheSizeGet(ESE_HRPCCTX ctx);

/// Return RPC context memcache buffer
void* rpcContextMemcacheGet(ESE_HRPCCTX ctx);

/// Return value of additional parameter passed to rpcContextCreate
void* rpcContextParameterGet(ESE_HRPCCTX ctx);

/// Execute RPC handler with given ID and signature using specific RPC context
RpcStatus rpcExecLocal(ESE_HRPCCTX ctx, esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen);

#endif

// service interface
//
// idGet checks
#define rpcIsStdPID( pId )           ((esBL)((esU16)(pId) >= RPID_STD_BASE && (esU16)(pId) <= RPID_STD_END))
#define rpcIsCustomPID( pId )        ((esBL)((esU16)(pId) >= RPID_CUSTOM_BASE && (esU16)(pId) <= RPID_CUSTOM_END))
#define rpcIsPID( pId )              ((esBL)(rpcIsStdPID(pId) || rpcIsCustomPID(pId)))

#ifdef __cplusplus
    }
#endif

#endif // _Data_Blocks_h_
