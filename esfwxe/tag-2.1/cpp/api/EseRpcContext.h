#ifndef _ese_rpc_context_h_
#define _ese_rpc_context_h_

/// Dynamic RPC context interface implementation
///

class EseRpcContext : public EseRpcContextIntf
{
public:
  EseRpcContext(int memcacheSize, void* param = NULL) ESE_NOTHROW;
  virtual ~EseRpcContext() ESE_NOTHROW;
  
  virtual ESE_HRPCCTX ctxRawGet() ESE_NOTHROW ESE_OVERRIDE;
  virtual bool procedureRegister(int id, int sig, void* pfn) ESE_NOTHROW ESE_OVERRIDE;
  virtual void procedureUnRegister(int id) ESE_NOTHROW ESE_OVERRIDE;
  virtual int proceduresCountGet() const ESE_NOTHROW ESE_OVERRIDE;
  virtual int memcacheSizeGet() const ESE_NOTHROW ESE_OVERRIDE;
  virtual void* memcacheGet() ESE_NOTHROW ESE_OVERRIDE;
  virtual void* parameterGet() ESE_NOTHROW ESE_OVERRIDE;
  virtual RpcStatus exec(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen) ESE_NOTHROW ESE_OVERRIDE;
  
protected:
  ESE_HRPCCTX m_ctx;
  void* m_param;

private:
  EseRpcContext() ESE_NOTHROW ESE_REMOVE_DECL;
  EseRpcContext(const EseRpcContext&) ESE_NOTHROW ESE_REMOVE_DECL;
  EseRpcContext& operator=(const EseRpcContext&) ESE_NOTHROW ESE_REMOVE_DECL;
};

#endif //< _ese_rpc_context_h_
