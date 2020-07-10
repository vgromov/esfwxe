#ifndef _ese_rpc_context_h_
#define _ese_rpc_context_h_

/// Dynamic RPC context interface implementation
///

class EseRpcContext : public EseRpcContextIntf
{
public:
  EseRpcContext(int memcacheSize, void* param = NULL) ESE_NOTHROW ESE_KEEP;
  virtual ~EseRpcContext() ESE_NOTHROW ESE_KEEP;
  
  virtual rtosStatus lock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void unlock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual ESE_HRPCCTX ctxRawGet() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool procedureRegister(int id, int sig, void* pfn) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void procedureUnRegister(int id) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int proceduresCountGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual int memcacheSizeGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* memcacheGet() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void* parameterGet() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual RpcStatus exec(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
protected:
  EseMutexRecursive m_mx;
  ESE_HRPCCTX m_ctx;
  void* m_param;

  ESE_NODEFAULT_CTOR(EseRpcContext);
  ESE_NONCOPYABLE(EseRpcContext);
};

#endif //< _ese_rpc_context_h_
