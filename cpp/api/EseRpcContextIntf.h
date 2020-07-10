#ifndef _ese_rpc_context_intf_h_
#define _ese_rpc_context_intf_h_

/// Dynamic RPC registry and context interface.
/// Prerequisite is ESE_USE_DYNAMIC_RPC_REGISTRY defined
///

#ifndef ESE_USE_DYNAMIC_RPC_REGISTRY
# error "ESE_USE_DYNAMIC_RPC_REGISTRY should be defined somewhere in target configuration for this functionality to be used"
#endif

class ESE_ABSTRACT EseRpcContextIntf
{
public:
  /// Lockable interface support
  virtual rtosStatus lock() ESE_NOTHROW = 0;
  virtual void unlock() ESE_NOTHROW = 0;

  /// Access encapsulated raw context
  virtual ESE_HRPCCTX ctxRawGet() ESE_NOTHROW = 0;
  
  /// Register RPC handler in RPC context
  virtual bool procedureRegister(int id, int sig, void* pfn) ESE_NOTHROW = 0;

  /// UnRegister RPC handler from RPC context
  virtual void procedureUnRegister(int id) ESE_NOTHROW = 0;

  /// Return count of registered RPC handlers for specific RPC context
  virtual int proceduresCountGet() const ESE_NOTHROW = 0;

  /// Return size of RPC context output memcache
  virtual int memcacheSizeGet() const ESE_NOTHROW = 0;

  /// Return RPC context memcache buffer
  virtual void* memcacheGet() ESE_NOTHROW = 0;

  /// Return RPC context parameter
  virtual void* parameterGet() ESE_NOTHROW = 0;

  /// Execute RPC handler with given ID and signature using specific RPC context
  virtual RpcStatus exec(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen) ESE_NOTHROW = 0;
};

#endif //< _ese_rpc_context_intf_h_
