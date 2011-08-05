// EKOSF remote procedure call basic infrastructure implementation
//
#include <common/target.h>
#include <string.h>
#include <common/utils.h>
#pragma hdrstop

#include <common/commintf.h>
#include <common/remoteProcCall.h>

// remote procedure moniker
//
typedef struct
{
	WORD 	id;				// the procedure identifier
	WORD  sig;			// the procedure call signature
	void* proc;			// procedure that should be called

} RemoteProcMoniker;

// helper defines to implement the basic service for available remote services query 
#define RPC_MAP_BEGIN \
	static const WORD c_rpcIds[] = { RPID_STD_CAPS_GET, RPID_STD_FWID_GET,

#define RPC_MAP_ENTRY( id, sig, proc )	(id),

#define RPC_MAP_ENTRY_STD						RPC_MAP_ENTRY
#define RPC_MAP_ENTRY_CUSTOM				RPC_MAP_ENTRY

#define RPC_MAP_END \
	RPID_NOID }; \
	static BYTEARRAY rpcGetCaps(RpcStatus* stat) { \
		BYTEARRAY ba; \
		ba.size = sizeof(c_rpcIds)-2; \
		ba.data = (BYTE*)c_rpcIds; return ba; \
	} \
	BYTEARRAY rpcGetFwId(RpcStatus*);

// include either dummy rpc map definition file, or actual one,
// depending on which one is first found in the project include list.
// this automatically define an internal array of RPC monikers
//

#include <rpcMap/rpcMap.cc>

// helper defines to declare and register RemoteProcMoniker(s)
#define RPC_MAP_ENTRY( id, sig, proc )	\
	{(id), (sig), (void*)(proc)},

#define RPC_MAP_ENTRY_STD						RPC_MAP_ENTRY
#define RPC_MAP_ENTRY_CUSTOM				RPC_MAP_ENTRY

#define RPC_MAP_BEGIN \
	static const RemoteProcMoniker c_rpcMap[] = {	\
		RPC_MAP_ENTRY_STD(RPID_STD_CAPS_GET, BYTEARRAY_RpcSig, rpcGetCaps) \
		RPC_MAP_ENTRY_STD(RPID_STD_FWID_GET, BYTEARRAY_RpcSig, rpcGetFwId)

#define RPC_MAP_END \
	RPC_MAP_ENTRY( RPID_NOID, RpcSig_NULL, NULL ) };

// include either dummy rpc map definition file, or actual one,
// depending on which one is first found in the project include list.
// this automatically define an internal array of RPC monikers
//

#include <rpcMap/rpcMap.cc>

// find procedure moniker by its id, return its pointer if found, 0 otherwise
static const RemoteProcMoniker* rpcFindById(WORD id)
{
	int idx = 0;
	for( ; idx < CONST_ARRAY_COUNT(c_rpcMap); ++idx )
	{
		if( id == c_rpcMap[idx].id )
			return &c_rpcMap[idx];
	} 

	return NULL;
}

// generate rpc reflection procedure types
#define RPC_REFLECTION_BEGIN
#define	RPC_DEF_HANDLER0(ReturnType) \
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
#define	RPC_DEF_HANDLER0_NORETURN() \
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

#include <common/rpcMap/rpcReflection.cc>

// generate rpc reflection procedure wrappers
//
#define RPC_REFLECTION_BEGIN
#define	RPC_DEF_HANDLER0(ReturnType) \
static RpcStatus ReturnType ## _RpcWrp(BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{	\
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	ReturnType ret = ((ReturnType ## _RpcProc)(pfn))(&result); \
	if( RpcOK == result ) { \
		pos = stack; \
		if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
			result = RpcStackOverflow; \
		else \
			*stackLen = pos-stack; } \
	return result; \
}
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	if( !get ## Param0Type (&pos, end, &p0) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type)(pfn))(&result, p0); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1)) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type)(pfn))(&result, p0, p1); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2)) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## )(pfn))(&result, p0, p1, p2); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3)) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## )(pfn))(&result, p0, p1, p2, p3); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	Param4Type p4; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3) && \
				get ## Param4Type (&pos, end, &p4)) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## )(pfn))(&result, p0, p1, p2, p3, p4); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	Param4Type p4; \
	Param5Type p5; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3) && \
				get ## Param4Type (&pos, end, &p4) && \
				get ## Param5Type (&pos, end, &p5)) ) \
		result = RpcStackCorrupt; \
	else { \
		ReturnType ret = ((ReturnType ## _RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## )(pfn))(&result, p0, p1, p2, p3, p4, p5); \
		if( RpcOK == result ) { \
			pos = stack; \
			if( !put ## ReturnType (&pos, stack+stackMaxLen, ret) ) \
				result = RpcStackOverflow; \
			else \
	 			*stackLen = pos-stack; } } \
	return result; \
}
#define	RPC_DEF_HANDLER0_NORETURN() \
static RpcStatus VOID_RpcWrp(BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{	\
	RpcStatus result = RpcOK; \
	((VOID_RpcProc)(pfn))(&result); \
	*stackLen = 0; \
	return result; \
}
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	if( !get ## Param0Type (&pos, end, &p0) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type)(pfn))(&result, p0); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1)) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type)(pfn))(&result, p0, p1); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2)) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type )(pfn))(&result, p0, p1, p2); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3)) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type)(pfn))(&result, p0, p1, p2, p3); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	Param4Type p4; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3) && \
				get ## Param4Type (&pos, end, &p4)) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type)(pfn))(&result, p0, p1, p2, p3, p4); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
static RpcStatus VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = stack; \
	BYTE* end = stack+(*stackLen); \
	Param0Type p0; \
	Param1Type p1; \
	Param2Type p2; \
	Param3Type p3; \
	Param4Type p4; \
	Param5Type p5; \
	if( !(get ## Param0Type (&pos, end, &p0) && \
				get ## Param1Type (&pos, end, &p1) &&	\
				get ## Param2Type (&pos, end, &p2) && \
				get ## Param3Type (&pos, end, &p3) && \
				get ## Param4Type (&pos, end, &p4) && \
				get ## Param5Type (&pos, end, &p5)) ) \
		result = RpcStackCorrupt; \
	else { \
		((VOID_RpcProc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type)(pfn))(&result, p0, p1, p2, p3, p4, p5); \
		*stackLen = 0; } \
	return result; \
}
#define RPC_REFLECTION_END

#include <common/rpcMap/rpcReflection.cc>

// generate map of rpc signatures to rpc reflection wrappers
#define RPC_REFLECTION_BEGIN \
typedef RpcStatus (*RpcWrpT)(BYTE* stack, DWORD* stackLen, DWORD stackMaxLen, void* pfn); \
static const RpcWrpT c_rpcSigWrapperMap[RpcReflectionSignatureCnt] = { NULL ,
#define	RPC_DEF_HANDLER0(ReturnType) 																				ReturnType ## _RpcWrp ,
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) 														ReturnType ## _RpcWrp_ ## Param0Type ,
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) 								ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ,
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) 		ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ,
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
																																ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ,
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
																																ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ,
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
																																ReturnType ## _RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ,
#define	RPC_DEF_HANDLER0_NORETURN() 																				VOID_RpcWrp ,
#define RPC_DEF_HANDLER1_NORETURN(Param0Type)	 															VOID_RpcWrp_ ## Param0Type ,
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) 									VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ,
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) 			VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ,
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
																																VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ,
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
																																VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ,
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
																																VOID_RpcWrp_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ,
#define RPC_REFLECTION_END };

#include <common/rpcMap/rpcReflection.cc>

// find rpc wrapper by its signature, return 0 if not found
static __inline RpcWrpT rpcFindWrapper(int sig)
{
	if( RpcSig_NULL < sig && 
			RpcReflectionSignatureCnt > sig )
		return c_rpcSigWrapperMap[sig];

	return 0;
}

// execute procedure at the side of the callee. 
// dataEnd marks an end of the stack space allocated 
// by the reflection engine
RpcStatus rpcExecLocal(WORD id, WORD sig, BYTE* stack, DWORD* stackLen, DWORD stackMaxLen)
{
	RpcStatus stat = RpcOK;

	// find rpc moniker entry
	const RemoteProcMoniker* moniker = rpcFindById(id);
	if( moniker )
	{
		// check signature against one stored in moniker
		if( moniker->sig == sig )
		{
			// find wrapper by signature stored in moniker
			RpcWrpT wrap = rpcFindWrapper(moniker->sig);
			if( wrap )
				stat = wrap(stack, stackLen, stackMaxLen, moniker->proc); // execute procedure with wrapper
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
