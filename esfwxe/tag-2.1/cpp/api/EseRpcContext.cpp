#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/remoteProcCall.h>

#include "EseRpcContextIntf.h"
#include "EseRpcContext.h"
//----------------------------------------------------------------------------------------------

EseRpcContext::EseRpcContext(int memcacheSize, void* param /*= NULL*/) ESE_NOTHROW :
m_ctx(NULL),
m_param(param)
{
  m_ctx = rpcContextCreate(
    memcacheSize,
    reinterpret_cast<EseRpcContextIntf*>(this)
  );
}
//----------------------------------------------------------------------------------------------

EseRpcContext::~EseRpcContext() ESE_NOTHROW
{
  if( m_ctx )
  {
    rpcContextDelete(m_ctx);
    m_ctx = NULL;
  }
}
//----------------------------------------------------------------------------------------------

ESE_HRPCCTX EseRpcContext::ctxRawGet() ESE_NOTHROW
{
  return m_ctx;
}
//----------------------------------------------------------------------------------------------

bool EseRpcContext::procedureRegister(int id, int sig, void* pfn) ESE_NOTHROW
{
  return rpcProcedureRegister(
    m_ctx,
    id,
    sig,
    pfn
  );
}
//----------------------------------------------------------------------------------------------

void EseRpcContext::procedureUnRegister(int id) ESE_NOTHROW
{
  rpcProcedureUnRegister(
    m_ctx,
    id
  );
}
//----------------------------------------------------------------------------------------------

int EseRpcContext::proceduresCountGet() const ESE_NOTHROW
{
  return rpcContextEntriesCountGet(
    m_ctx
  );
}
//----------------------------------------------------------------------------------------------

int EseRpcContext::memcacheSizeGet() const ESE_NOTHROW
{
  return rpcContextMemcacheSizeGet(
    m_ctx
  );
}
//----------------------------------------------------------------------------------------------

void* EseRpcContext::memcacheGet() ESE_NOTHROW
{
  return rpcContextMemcacheGet(
    m_ctx
  );
}
//----------------------------------------------------------------------------------------------

void* EseRpcContext::parameterGet() ESE_NOTHROW
{
  return m_param;
}
//----------------------------------------------------------------------------------------------

RpcStatus EseRpcContext::exec(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen) ESE_NOTHROW
{
  return rpcExecLocal(
    m_ctx, 
    id, 
    sig, 
    stack, 
    stackLen, 
    stackMaxLen
  );
}
//----------------------------------------------------------------------------------------------

#include <esfwxe/remoteProcCall.c>
