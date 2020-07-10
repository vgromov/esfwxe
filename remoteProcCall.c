#include <esfwxe/target.h>
#include <esfwxe/utils.h>
#include <esfwxe/ese_assert.h>
#include <string.h>

#include <esfwxe/commintf.h>
#include <esfwxe/remoteProcCall.h>
//----------------------------------------------------------------------------------------------
// EKOSF remote procedure call basic infrastructure implementation
//

#ifndef ESE_USE_DYNAMIC_RPC_REGISTRY

// remote procedure moniker
//
typedef struct
{
  esU16 id;             // the procedure identifier
  esU16 sig;            // the procedure call signature
  void* proc;           // procedure that should be called

} RemoteProcMoniker;

// helper defines to implement the basic service for available remote services query 
#define RPC_MAP_BEGIN \
  static const esU16 c_rpcIds[] = { \
    RPID_STD_CAPS_GET, \
    RPID_STD_FWID_GET,

#define RPC_MAP_ENTRY( id, sig, proc ) \
  (id),

#define RPC_MAP_ENTRY_STD     RPC_MAP_ENTRY
#define RPC_MAP_ENTRY_CUSTOM  RPC_MAP_ENTRY

#define RPC_MAP_END \
  RPID_NOID };

// include either dummy rpc map definition file, or actual one,
// depending on which one is first found in the project include list.
// this automatically define an internal array of RPC monikers
//
#include <rpcMap/rpcMap.cc>

static esBA rpcStdGetCaps(RpcStatus* stat) 
{
  esBA ba;
  ba.size = sizeof(c_rpcIds)-2;
  ba.data = (esU8*)c_rpcIds; 
  
  return ba;
}
//----------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" 
#else
extern 
#endif
esBA rpcGetFwId(RpcStatus*);

// helper defines to declare and register RemoteProcMoniker(s)
#define RPC_MAP_ENTRY( id, sig, proc ) \
  {(id), (sig), (void*)(proc)},

#define RPC_MAP_ENTRY_STD     RPC_MAP_ENTRY
#define RPC_MAP_ENTRY_CUSTOM  RPC_MAP_ENTRY

#define RPC_MAP_BEGIN \
  static const RemoteProcMoniker c_rpcMap[] = { \
    RPC_MAP_ENTRY_STD(RPID_STD_CAPS_GET, esBA_RpcSig, rpcStdGetCaps) \
    RPC_MAP_ENTRY_STD(RPID_STD_FWID_GET, esBA_RpcSig, rpcGetFwId)

#define RPC_MAP_END \
  RPC_MAP_ENTRY( RPID_NOID, RpcSig_NULL, NULL ) };

// include either dummy rpc map definition file, or actual one,
// depending on which one is first found in the project include list.
// this automatically define an internal array of RPC monikers
//
#include <rpcMap/rpcMap.cc>

// find procedure moniker by its id, return its pointer if found, 0 otherwise
static inline const RemoteProcMoniker* rpcFindById(esU16 id)
{
  int idx = 0;
  for( ; idx < CONST_ARRAY_COUNT(c_rpcMap); ++idx )
  {
    if( id == c_rpcMap[idx].id )
      return &c_rpcMap[idx];
  } 

  return NULL;
}
//----------------------------------------------------------------------------------------------

// Generate rpc reflection procedure types
#define RPC_REFLECTION_BEGIN
#define  RPC_DEF_HANDLER0(ReturnType) \
  typedef ReturnType (*ReturnType ## _RpcProc)(RpcStatus*);
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type)(RpcStatus*, Param0Type);
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type)(RpcStatus*, Param0Type, Param1Type);
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type)(RpcStatus*, Param0Type, Param1Type, Param2Type);
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type);
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type);
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);
#define  RPC_DEF_HANDLER0_NORETURN() \
  typedef void (*VOID_RpcProc)(RpcStatus*);
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type)(RpcStatus*, Param0Type);
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type)(RpcStatus*, Param0Type, Param1Type);
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type)(RpcStatus*, Param0Type, Param1Type, Param2Type);
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type);
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type);
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);
#define RPC_REFLECTION_END
#include <esfwxe/rpcMap/rpcReflection.cc>

// generate rpc reflection procedure wrappers
//
#define RPC_REFLECTION_BEGIN
#define  RPC_DEF_HANDLER0(ReturnType) \
static RpcStatus ReturnType ## _RpcWrp(esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  ReturnType ret = ((ReturnType ## _RpcProc)(pfn))(&result); \
  if( RpcOK == result ) { \
    pos = stack; \
    if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
    else \
        *stackLen = pos-stack; } \
  return result; \
}
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  if( !get_ ## Param0Type (&pos, end, &p0) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type )(pfn))(&result, p0); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type )(pfn))(&result, p0, p1); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type )(pfn))(&result, p0, p1, p2); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type )(pfn))(&result, p0, p1, p2, p3); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type )(pfn))(&result, p0, p1, p2, p3, p4); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  Param5Type p5; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4) && \
      get_ ## Param5Type (&pos, end, &p5)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type )(pfn))(&result, p0, p1, p2, p3, p4, p5); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define  RPC_DEF_HANDLER0_NORETURN() \
static RpcStatus VOID_RpcWrp(esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{    \
  RpcStatus result = RpcOK; \
  ((VOID_RpcProc)(pfn))(&result); \
  *stackLen = 0; \
  return result; \
}
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  if( !get_ ## Param0Type (&pos, end, &p0) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type)(pfn))(&result, p0); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type)(pfn))(&result, p0, p1); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type )(pfn))(&result, p0, p1, p2); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(pfn))(&result, p0, p1, p2, p3); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(pfn))(&result, p0, p1, p2, p3, p4); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  Param5Type p5; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4) && \
      get_ ## Param5Type (&pos, end, &p5)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(pfn))(&result, p0, p1, p2, p3, p4, p5); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_REFLECTION_END
#include <esfwxe/rpcMap/rpcReflection.cc>

typedef RpcStatus (*RpcWrpT)(esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn);

#else //< ESE_USE_DYNAMIC_RPC_REGISTRY
//----------------------------------------------------------------------------------------------

# include <stdlib.h>

// Generate rpc reflection procedure types
#define RPC_REFLECTION_BEGIN
#define  RPC_DEF_HANDLER0(ReturnType) \
  typedef ReturnType (*ReturnType ## _RpcProc)(ESE_HRPCCTX, RpcStatus*);
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type)(ESE_HRPCCTX, RpcStatus*, Param0Type);
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type);
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type);
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type);
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type);
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
  typedef ReturnType (*ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);
#define  RPC_DEF_HANDLER0_NORETURN() \
  typedef void (*VOID_RpcProc)(ESE_HRPCCTX, RpcStatus*);
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type)(ESE_HRPCCTX, RpcStatus*, Param0Type);
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type);
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type);
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type);
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type);
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
  typedef void (*VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(ESE_HRPCCTX, RpcStatus*, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type);
#define RPC_REFLECTION_END
#include <esfwxe/rpcMap/rpcReflection.cc>

// generate rpc reflection procedure wrappers
//
#define RPC_REFLECTION_BEGIN
#define  RPC_DEF_HANDLER0(ReturnType) \
static RpcStatus ReturnType ## _RpcWrp(ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  ReturnType ret = ((ReturnType ## _RpcProc)(pfn))(hctx, &result); \
  if( RpcOK == result ) { \
    pos = stack; \
    if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
      result = RpcStackOverflow; \
    else \
      *stackLen = pos-stack; } \
  return result; \
}
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  if( !get_ ## Param0Type (&pos, end, &p0) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type )(pfn))(hctx, &result, p0); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type )(pfn))(hctx, &result, p0, p1); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type )(pfn))(hctx, &result, p0, p1, p2); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type )(pfn))(hctx, &result, p0, p1, p2, p3); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type )(pfn))(hctx, &result, p0, p1, p2, p3, p4); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  Param5Type p5; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4) && \
      get_ ## Param5Type (&pos, end, &p5)) ) \
    result = RpcStackCorrupt; \
  else { \
    ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type )(pfn))(hctx, &result, p0, p1, p2, p3, p4, p5); \
    if( RpcOK == result ) { \
      pos = stack; \
      if( !put_ ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
        result = RpcStackOverflow; \
      else \
        *stackLen = pos-stack; } } \
  return result; \
}
#define  RPC_DEF_HANDLER0_NORETURN() \
static RpcStatus VOID_RpcWrp(ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{    \
  RpcStatus result = RpcOK; \
  ((VOID_RpcProc)(pfn))(hctx, &result); \
  *stackLen = 0; \
  return result; \
}
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  if( !get_ ## Param0Type (&pos, end, &p0) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type)(pfn))(hctx, &result, p0); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type)(pfn))(hctx, &result, p0, p1); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type )(pfn))(hctx, &result, p0, p1, p2); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(pfn))(hctx, &result, p0, p1, p2, p3); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(pfn))(hctx, &result, p0, p1, p2, p3, p4); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn) \
{ \
  RpcStatus result = RpcOK; \
  esU8* pos = stack; \
  esU8* end = stack+(*stackLen); \
  Param0Type p0; \
  Param1Type p1; \
  Param2Type p2; \
  Param3Type p3; \
  Param4Type p4; \
  Param5Type p5; \
  if( !(get_ ## Param0Type (&pos, end, &p0) && \
      get_ ## Param1Type (&pos, end, &p1) &&    \
      get_ ## Param2Type (&pos, end, &p2) && \
      get_ ## Param3Type (&pos, end, &p3) && \
      get_ ## Param4Type (&pos, end, &p4) && \
      get_ ## Param5Type (&pos, end, &p5)) ) \
    result = RpcStackCorrupt; \
  else { \
    ((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(pfn))(hctx, &result, p0, p1, p2, p3, p4, p5); \
    *stackLen = 0; } \
  return result; \
}
#define RPC_REFLECTION_END
#include <esfwxe/rpcMap/rpcReflection.cc>

typedef RpcStatus (*RpcWrpT)(ESE_HRPCCTX hctx, esU8* stack, esU32* stackLen, esU32 stackMaxLen, void* pfn);

#endif //< ESE_USE_DYNAMIC_RPC_REGISTRY
//----------------------------------------------------------------------------------------------

#ifndef ESE_USE_DYNAMIC_RPC_REGISTRY

//----------------------------------------------------------------------------------------------
// generate map of rpc signatures to rpc reflection wrappers
#define RPC_REFLECTION_BEGIN \
static const RpcWrpT c_rpcSigWrapperMap[RpcReflectionSignatureCnt] = { NULL ,
#define  RPC_DEF_HANDLER0(ReturnType)                                                                     ReturnType ## _RpcWrp ,
#define RPC_DEF_HANDLER1(ReturnType, Param0Type)                                                          ReturnType ## _RpcWrp_ ## Param0Type ,
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type)                                              ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ,
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type)                                  ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ,
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
                                                                                                          ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ,
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
                                                                                                          ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ,
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
                                                                                                          ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ,
#define  RPC_DEF_HANDLER0_NORETURN()                                                                      VOID_RpcWrp ,
#define RPC_DEF_HANDLER1_NORETURN(Param0Type)                                                             VOID_RpcWrp_ ## Param0Type ,
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type)                                                 VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ,
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type)                                     VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ,
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
                                                                                                          VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ,
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
                                                                                                          VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ,
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
                                                                                                          VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ,
#define RPC_REFLECTION_END };
#include <esfwxe/rpcMap/rpcReflection.cc>

// find rpc wrapper by its signature, return 0 if not found
static inline RpcWrpT rpcFindWrapper(int sig)
{
  if( RpcSig_NULL < sig && 
      RpcReflectionSignatureCnt > sig 
  )
    return c_rpcSigWrapperMap[sig];

  return NULL;
}

#else //< ESE_USE_DYNAMIC_RPC_REGISTRY

//----------------------------------------------------------------------------------------------

// find rpc wrapper by its signature, return NULL if not found
static inline RpcWrpT rpcFindWrapper(int sig)
{
// generate map of rpc signatures to rpc reflection wrappers
#define RPC_REFLECTION_BEGIN \
    switch( sig ) {
#define RPC_DEF_HANDLER0(ReturnType) \
                                                                                                          case ReturnType ## _RpcSig: return ReturnType ## _RpcWrp;
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type: return ReturnType ## _RpcWrp_ ## Param0Type;
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type: return ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type;
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type: return ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type;
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type: return ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type;
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type: return ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type;
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
                                                                                                          case ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type: return ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type;
#define  RPC_DEF_HANDLER0_NORETURN() \
                                                                                                          case VOID_RpcSig: return VOID_RpcWrp;
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type: return VOID_RpcWrp_ ## Param0Type;
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type ## _ ## Param1Type: return VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type;
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type: return VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type;
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type: return VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type;
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type: return VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type;
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
                                                                                                          case VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type: return VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type;
#define RPC_REFLECTION_END default: return NULL; }
#include <esfwxe/rpcMap/rpcReflection.cc>
}
//----------------------------------------------------------------------------------------------
#endif //< ESE_USE_DYNAMIC_RPC_REGISTRY

// execute procedure at the side of the callee. 
// dataEnd marks an end of the stack space allocated 
// by the reflection engine
#ifdef ESE_USE_DYNAMIC_RPC_REGISTRY

// find procedure moniker by its id, return its pointer if found, 0 otherwise
static inline const RemoteProcMoniker* rpcFindById(ESE_HRPCCTX ctx, esU16 id)
{
  if(!ctx)
    return NULL;

  const RemoteProcMoniker* pmon = &((const RemoteProcContext*)ctx)->root;
  
  while( pmon )
  {
    if( id == pmon->id )
      return pmon;

    pmon = pmon->next;
  }

  return NULL;
}
//----------------------------------------------------------------------------------------------

static esBA rpcStdGetCaps(ESE_HRPCCTX ctx, RpcStatus* stat)
{
  ESE_ASSERT(ctx);
  
  int itemCnt = rpcContextEntriesCountGet(ctx);
  
  int sze = rpcContextMemcacheSizeGet(ctx);
  ESE_ASSERT(sze >= itemCnt*sizeof(esU16));

  esU16* memcache = (esU16*)rpcContextMemcacheGet(ctx);
  ESE_ASSERT(memcache);
  
  esU16* idpos = memcache;
  const RemoteProcMoniker* pmon = &((const RemoteProcContext*)ctx)->root;

  while( pmon )
  {
    *idpos++ = pmon->id;
    pmon = pmon->next;
  }

  esBA ba;
  ba.size = itemCnt*sizeof(esU16);
  ba.data = (esU8*)memcache;
  
  return ba;
}
//----------------------------------------------------------------------------------------------

ESE_HRPCCTX rpcContextCreate(int outMemCacheSize, void* param)
{
  if( outMemCacheSize < 0 )
    outMemCacheSize = 0;

  RemoteProcContext* ctx = (RemoteProcContext*)malloc(
    sizeof(RemoteProcContext) + outMemCacheSize
  );
  
  memset(
    ctx,
    0,
    sizeof(RemoteProcContext) + outMemCacheSize
  );
  
  // Always register at least one most-basic RPC service - requester of all registered RPC handlers
  ctx->memcacheSize = outMemCacheSize;
  ctx->param = param;
  ctx->root.id = RPID_STD_CAPS_GET;
  ctx->root.sig = esBA_RpcSig;
  ctx->root.proc = (void*)rpcStdGetCaps;
  ctx->root.rpcProxy = (void*)esBA_RpcWrp;
  
  return ctx;
}
//----------------------------------------------------------------------------------------------

static void rpcMonikerRemove(RemoteProcMoniker* pmon)
{
  ESE_ASSERT(pmon);

  RemoteProcMoniker* pprev = pmon->prev;
  RemoteProcMoniker* pnext = pmon->next;
  
  if( pprev )
    pprev->next = pnext;
  
  if( pnext )
    pnext->prev = pprev;
    
  free(pmon);
}
//----------------------------------------------------------------------------------------------

void rpcContextDelete(ESE_HRPCCTX ctx)
{
  if( !ctx )
    return;
    
  RemoteProcContext* pctx = (RemoteProcContext*)ctx;
  RemoteProcMoniker* node = pctx->root.next;
  RemoteProcMoniker* nodePrev = &pctx->root;
  
  // Locate tail
  while(node)
  {
    nodePrev = node;
    node = node->next;
  }
  
  node = nodePrev;
  while( node && node != &pctx->root )
  {
    nodePrev = node->prev;
    rpcMonikerRemove(node);
    node = nodePrev;
  }
  
  free( ctx );
}
//----------------------------------------------------------------------------------------------

bool rpcProcedureRegister(ESE_HRPCCTX ctx, int id, int sig, void* pfn)
{
  if( !ctx )
    return false;
    
  if( !pfn )
    return false;
    
  if( id <= RpcSig_NULL )
    return false;
    
  if( NULL != rpcFindById(ctx, id) ) //< Already registered
    return true;
    
  RpcWrpT proxy = rpcFindWrapper(sig);
  if( !proxy )
    return false;
    
  RemoteProcMoniker* pmon = (RemoteProcMoniker*)malloc(sizeof(RemoteProcMoniker));
  ESE_ASSERT(pmon);
  
  memset(
    pmon, 
    0,
    sizeof(RemoteProcMoniker)
  );
  
  pmon->id = id;
  pmon->sig = sig;
  pmon->proc = pfn;
  pmon->rpcProxy = (void*)proxy;
  
  RemoteProcContext* pctx = (RemoteProcContext*)ctx;
  RemoteProcMoniker* node = &pctx->root;
  RemoteProcMoniker* nodePrev = node->prev;
  
  while( node )
  {
    if( id >= (int)node->id )
    {
      nodePrev = node;
      node = node->next;
      
      if( !node ) //< Last node, and nodePrev->id <= id - insert at last node
      {
        nodePrev->next = pmon;
        pmon->prev = nodePrev;
        break;
      }
    }
    else //< node->id > id, nodePrev->id <= id, insert after nodePrev and before node
    {
      nodePrev->next = pmon;
      pmon->prev = nodePrev;
      pmon->next = node;
      node->prev = pmon;
      break;
    }
  }
  
  return true;
}
//----------------------------------------------------------------------------------------------

void rpcProcedureUnRegister(ESE_HRPCCTX ctx, int id)
{
  if( !ctx )
    return;
  
  // Do not touch the basic one
  if( RPID_STD_CAPS_GET == id )
    return;
  
  RemoteProcMoniker* pmon = (RemoteProcMoniker*)rpcFindById(ctx, id);
  if( !pmon )
    return;
  
  rpcMonikerRemove(
    pmon
  );
}
//----------------------------------------------------------------------------------------------

int rpcContextEntriesCountGet(ESE_HRPCCTX ctx)
{
  if(!ctx)
    return 0;
    
  RemoteProcContext* pctx = (RemoteProcContext*)ctx;
  const RemoteProcMoniker* pmon = &pctx->root;
  
  int cnt = 0;
  while( pmon )
  {
    ++cnt;
    pmon = pmon->next;
  }
  
  return cnt;
}
//----------------------------------------------------------------------------------------------

int rpcContextMemcacheSizeGet(ESE_HRPCCTX ctx)
{
  if(!ctx)
    return 0;
    
  RemoteProcContext* pctx = (RemoteProcContext*)ctx;
  return pctx->memcacheSize;
}
//----------------------------------------------------------------------------------------------

void* rpcContextMemcacheGet(ESE_HRPCCTX ctx)
{
  if(!ctx || 0 == ((RemoteProcContext*)ctx)->memcacheSize)
    return NULL;
    
  const esU8* memcache = ((const esU8*)ctx)+sizeof(RemoteProcContext);
  return (void*)memcache;
}
//----------------------------------------------------------------------------------------------

void* rpcContextParameterGet(ESE_HRPCCTX ctx)
{
  if(!ctx)
    return NULL;
    
  return ((RemoteProcContext*)ctx)->param;
}
//----------------------------------------------------------------------------------------------

RpcStatus rpcExecLocal(ESE_HRPCCTX ctx, esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen)
{
  RpcStatus stat = RpcOK;

  // find rpc moniker entry
  const RemoteProcMoniker* moniker = rpcFindById(
    ctx,
    id
  );

#else //< ESE_USE_DYNAMIC_RPC_REGISTRY

RpcStatus rpcExecLocal(esU16 id, esU16 sig, esU8* stack, esU32* stackLen, esU32 stackMaxLen)
{
  RpcStatus stat = RpcOK;

  // find rpc moniker entry
  const RemoteProcMoniker* moniker = rpcFindById(id);
  
#endif //< ESE_USE_DYNAMIC_RPC_REGISTRY

  if( moniker )
  {
    // check signature against one stored in moniker
    if( moniker->sig == sig )
    {
      // find wrapper by signature stored in moniker
      RpcWrpT pfn = 
#ifndef ESE_USE_DYNAMIC_RPC_REGISTRY

        rpcFindWrapper(moniker->sig);
        
#else //< ESE_USE_DYNAMIC_RPC_REGISTRY

        (RpcWrpT)moniker->rpcProxy;
        
#endif //< ESE_USE_DYNAMIC_RPC_REGISTRY

      if( pfn )
        stat = pfn( //< Execute procedure with wrapper
#ifdef ESE_USE_DYNAMIC_RPC_REGISTRY
          ctx,
#endif //< ESE_USE_DYNAMIC_RPC_REGISTRY
          stack, 
          stackLen, 
          stackMaxLen, 
          moniker->proc
        );
      else
        stat = RpcUnknownSignature;
    }
    else
      stat = RpcSignatureMismatch;
  }
  else 
    stat = RpcNotImplemented;

  return stat;
}
//----------------------------------------------------------------------------------------------
