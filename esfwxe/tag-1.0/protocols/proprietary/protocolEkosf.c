#include <common/target.h>
#pragma hdrstop

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <common/utils.h>
#include <common/crc.h>
#include <common/commintf.h>
#include "protocolEkosf.h"

// ekosfera propritary protocol implementation
//

// trace facility
#if defined(USE_TRACE) && defined(USE_EKOSF_PROTOCOL_TRACE)
	#include <common/trace.h>
	
	static void ekosfHdrTrace(const ProtocolEkosfHdr* hdr)
	{
		TRACE0("EkosfProtocol header contents:\n\r")
		TRACE1("frame type: 0x%02X\n\r",	(UINT)hdr->frameType)
		TRACE1("device address: %u\n\r",	(UINT)hdr->devAddr)
		TRACE1("command|response: 0x%04X\n\r", (UINT)hdr->command)
		TRACE1("optional data length: %u\n\r", (UINT)hdr->dataLen)
		TRACE1("packet sequence number: %u\n\r", (UINT)hdr->packetNum)
		TRACE1("CRC16: 0x%04X\n\r", (UINT)hdr->crc)
	}

	#define TRACE_EKOSF_HDR(hdr)	ekosfHdrTrace(hdr);
#else
	#define TRACE_EKOSF_HDR(hdr)
#endif

// update header record crc after some of its fields were changed
static void ekosfUpdateHdrCrc(ProtocolEkosfHdr* hdr)
{
	hdr->crc = checksum16((const BYTE*)hdr, ProtocolEkosfHdr_SZE-2);				// 2 byte CRC as of RFC1070

	TRACE_EKOSF_HDR(hdr)
}

// update subset of header fields
static void ekosfUpdateHdr(ProtocolEkosfHdr* hdr, BYTE frame, UINT16 cmd, UINT16 dataLen)
{
	hdr->frameType = frame;
	hdr->command = cmd;
	hdr->dataLen = dataLen ? dataLen+2 : 0;

	ekosfUpdateHdrCrc(hdr);
}

// send data over channel using ekosfera protocol.
static BOOL ekosfPutData( EkosfStdIoState* io, CommChannelBreak* brk )
{
	BOOL result = FALSE;
	result = chnlPutBytes(io->chnl, (const BYTE*)&io->hdr, ProtocolEkosfHdr_SZE, brk) == ProtocolEkosfHdr_SZE;
	
	if( result && 
			io->hdr.dataLen && 
			!chnlIsBreaking(brk) )
	{
		UINT16 actualDataLen = io->hdr.dataLen-2;
		UINT16 dataCRC = checksum16(io->rawData, actualDataLen);
		result = chnlPutBytes(io->chnl, io->rawData, actualDataLen, brk) == actualDataLen &&
			chnlPutBytes(io->chnl, (const BYTE*)&dataCRC, 2, brk) == 2;
	}
	
	return result;
}

static __inline void ekosfSetPendingRateChange( EkosfStdIoState* io, DWORD rate)
{
	// if set rate service is supported by the channel
	// and rate value is not 0
	if( 0 != rate )
		// set pending value to rate
		io->pendingRate = rate;
}

// try to receive full header frame. return -1 if only partial frame was received, 
// 0 if either error occured, or io was broken programmatically,
// 1 if full frame was received
static int ekosfFullHdrFrameReceive(EkosfStdIoState* io, CommChannelBreak* brk)
{
	DWORD read = 0;
	DWORD tmo = chnlSendTimeEstimateGet(io->chnl, ProtocolEkosfHdr_SZE) + io->packetTmo;
	BYTE* buff = (BYTE*)&io->hdr;
	read = chnlGetBytes(io->chnl, (BYTE*)buff, ProtocolEkosfHdr_SZE, tmo, brk);
	if( !chnlIsBreaking(brk) )
	{
		if( ProtocolEkosfHdr_SZE == read )
		{
			const BYTE* pos = buff;
			const BYTE* end = buff+ProtocolEkosfHdr_SZE;
			BOOL hdrMarkerFound = FALSE;
			while( !hdrMarkerFound &&
							pos < end )
			{
				hdrMarkerFound = 	EKOSF_FT_SLAVE != *pos || 
													EKOSF_FT_MASTER != *pos;

				if( !hdrMarkerFound )
					++pos;
			}

			if( hdrMarkerFound )
			{
				// header marker found in the current ProtocolEkosfHdr_SZE chunk, 
				// check if we're not at the beginning position of the chunk, try to read additional bytes,
				// first moving all including frame marker to the beginning of the buffer
				if( pos != buff )
				{
					DWORD addRead;
					read = end-pos;
					memmove(buff, pos, read);
					addRead = ProtocolEkosfHdr_SZE-read;
					tmo = chnlSendTimeEstimateGet(io->chnl, addRead) + io->packetTmo;
					read = chnlGetBytes(io->chnl, buff+addRead, addRead, tmo, brk);
					if( !chnlIsBreaking(brk) )
					{
						if(read == addRead)
							return 1;
						else if( 0 < read )
							return -1;
					}
				}
				else
					return 1;
			}
		}
		else if( 0 < read )
			return -1;
	}

	return 0;
}

// try to receive packet header.
static BOOL ekosfGetCheckHdr( EkosfStdIoState* io, CommChannelBreak* brk, size_t retries )
{
	int frmResult = 0;
	do
	{
		frmResult = ekosfFullHdrFrameReceive(io, brk);
		if( 1 == frmResult )
			return 0 == checksum16((const BYTE*)&io->hdr, ProtocolEkosfHdr_SZE);

	} while ( 0 != frmResult &&
						0 < retries-- );

	// handle dangling partial read conditions or channel errors
	if( -1 == frmResult || 0 != chnlGetError(io->chnl) )
		chnlResetIo(io->chnl, ekosfGarbageRecoveryTmo);

	return FALSE;
}

// common standard defs
//
#if defined( EKOSF_USE_STD_MASTER ) || defined(EKOSF_USE_STD_SLAVE)

static __inline void ekosfStdIoInit( EkosfStdIoState* io, CommChannel* chnl )
{
	if( io && chnl )
	{
		io->hdr.command = 0;
		io->hdr.crc = 0;
		io->hdr.dataLen = 0;
		io->hdr.devAddr = 0;
		io->hdr.frameType = 0;
		io->hdr.packetNum = 0;
		io->hdr.reserved = 0;

		io->packetTmo = ekosfDefPacketTmo;
		io->retries = 1;
		io->retry = 0;
		io->addr = ekosfBroadcastPeerAddr;	
		io->chnl = chnl;
		io->err = 0;	 		

		io->pendingRate = 0;		
		io->pendingAddr = -1;
	}
}

#endif // defined( EKOSF_USE_STD_MASTER ) || defined(EKOSF_USE_STD_SLAVE)

// standard master-initiated services implementation
//
#ifdef EKOSF_USE_STD_MASTER

// initialize master-specific iostate structure
void ekosfStdMasterIoInit( EkosfStdIoState* io, CommChannel* chnl )
{
	ekosfStdIoInit(io, chnl);
	if( io )
		io->retries = ekosfDefMasterPacketRetries;
}

// standard slave-to-master response handler.
static BOOL ekosfGetResponse(EkosfStdIoState* io, UINT16 cmd, CommChannelBreak* brk)
{
	BOOL result = FALSE;
	// check if there is response pending
	if( ekosfGetCheckHdr(io, brk, ekosfHdrFrameReceiveMaxRetries) )
	{
		// check if hdr is from client with specified address
		if( !chnlIsBreaking(brk) &&
				io->hdr.frameType == EKOSF_FT_SLAVE && 
				io->hdr.devAddr == io->addr )
		{
			if( EKOSF_IS_ERROR(io->hdr.command) )
			{
				io->err = io->hdr.command;
 				result = TRUE;
			}
			else if( io->hdr.command == EKOSF_MAKE_RESPONSE(cmd) )
			{
				result = TRUE;
				io->err = 0;
				// get optional pending data
				if( io->hdr.dataLen )
				{
					if( io->hdr.dataLen > ekosfMaxDataLen )
						io->err = EKOSF_ERR_DATA_TOO_LONG;
					else
					{
						DWORD read;
						read = chnlGetBytes(io->chnl, io->rawData,
							(DWORD)io->hdr.dataLen, 
							chnlSendTimeEstimateGet(io->chnl, io->hdr.dataLen) + io->packetTmo, 
							brk);
						
						result = !chnlIsBreaking(brk);
						if( result )
						{
							if( read != (DWORD)io->hdr.dataLen ) // data size mismatch
								io->err = EKOSF_ERR_DATA_SIZE;
							else if( read % 2 ) 				// report data misalign
								io->err = EKOSF_ERR_DATA_MISALIGN;
							else if( 0 != checksum16(io->rawData, read) )
								io->err = EKOSF_ERR_DATA_CRC;

							if( 0 != chnlGetError(io->chnl) ||
									(0 != read && io->err) )
								chnlResetIo(io->chnl, ekosfGarbageRecoveryTmo);
						}
					}
				}
			}
		}
	}

	return result;
}

// wait until tx packet is sent			  
static BOOL ekosfWaitTxIsSent(EkosfStdIoState* io, DWORD len, CommChannelBreak* brk)
{
	DWORD tmo = chnlSendTimeEstimateGet(io->chnl, len);
	BOOL breaking = chnlIsBreaking(brk);
	while( !breaking &&
				 tmo > 0 )
	{
		ekosfWait(1);
		--tmo;			
		breaking = chnlIsBreaking(brk);
	}

	return !breaking;
}

// standard master-initiated command execution.
static BOOL ekosfExecCommand(EkosfStdIoState* io, UINT16 cmd, DWORD dataLen, CommChannelBreak* brk)
{
	BOOL ok = FALSE;
	io->retry = 0;
	
	while( !chnlIsBreaking(brk) &&
					!ok &&
					io->retry < io->retries )
	{
		// reset header fields on each retry, so if any partial garbage was received in ekosfGetResponse
		// we would definitely get rid of it
		io->hdr.devAddr = io->addr;
		io->hdr.reserved = 0;
		++io->hdr.packetNum; // always increment packet number
		ekosfUpdateHdr(&io->hdr, EKOSF_FT_MASTER, cmd, (UINT16)dataLen);
	
		if( ekosfPutData( io, brk ) )
			// start waiting for response after tx packet is sent
			ok = ekosfWaitTxIsSent(io, ProtocolEkosfHdr_SZE+io->hdr.dataLen, brk) && 
				ekosfGetResponse(io, cmd, brk);

		++io->retry;
	}

	return ok;
}

// standard master-initiated protocol services
//
// ping slave device with specified address
BOOL ekosfPingSlave( EkosfStdIoState* io, CommChannelBreak* brk )
{
	if( io )
		return ekosfExecCommand(io, EKOSF_CMD_PING, 0, brk);
	
	return FALSE;
}

// request slave device address change
BOOL ekosfSlaveAddrSet( EkosfStdIoState* io, BYTE newAddr, CommChannelBreak* brk )
{
	BOOL result = FALSE;
	if( io && io->chnl )
	{
		BYTE savedRetries = io->retries;
		DWORD savedPacketTmo = io->packetTmo;
		io->packetTmo = ekosfMaxAutoSetupAddrTmo;
		// set-up data 
		io->rawData[0] = newAddr;
		io->rawData[1] = 0; // just for word alignment
		
		// if we setting slave address using broadcast auto set-up
		// do not use master retries, or else it'll break collision awareness
		// check if tmo is big enough as well
		if( ekosfBroadcastPeerAddr == io->addr )
			io->retries = 1;
		
		// execute address change command	
		result = ekosfExecCommand(io, EKOSF_CMD_ADDR_SET, 2, brk);
		
		// restore retries
		if( ekosfBroadcastPeerAddr == io->addr )
			io->retries = savedRetries;
		io->packetTmo = savedPacketTmo;
	}
	
	return result;
}

// request slave device channel rate change
BOOL ekosfSlaveRateSet( EkosfStdIoState* io, DWORD rate, CommChannelBreak* brk )
{
	// first, check if master channel supports requested rate
	if( io && 
			io->chnl && 
			chnlIsRateSupported(io->chnl, rate) )
	{
		// set retry to 1, this is one-packet service, no retries acceptable
		BYTE savedRetries = io->retries;
		io->retries = 1;

		// prepare rate command data
		memcpy(io->rawData, (const BYTE*)&rate, sizeof(rate));
	
		if( ekosfExecCommand(io, EKOSF_CMD_RATE_SET, sizeof(rate), brk ) )
		{
			// analyze error state. if everything responded ok, 
			// set master rate accordingly
			if( 0 == io->err && !EKOSF_IS_ERROR(io->hdr.command) )
				return chnlSetRate(io->chnl, rate);
		}

		io->retries = savedRetries;
	}
		
	return FALSE;
}

// protocol implementation of the single remote procedure call
// rpc stack data must be already formed externally in server data block
RpcStatus ekosfDoRPC( EkosfStdIoState* io, UINT16 rpcId, UINT16 rpcSig, DWORD* stackSize, CommChannelBreak* brk )
{
	RpcStatus result = RpcOK;
	
	// adjust stack data to even length, if needed, by appending zero byte at the end
	if(*stackSize % 2)
	{
		io->stack[*stackSize] = 0;
		++(*stackSize);
	}

	io->id = rpcId; 
	io->sigOrStat = rpcSig;
	if( ekosfExecCommand(io, EKOSF_CMD_RPC_EXEC, *stackSize + ekosfRpcStackOffs, brk) )
	{
		// analyze potential communication error
		if( io->err )
			result = RpcCommunicationError;
		else // retrieve rpc execution result otherwise
		{
			// adjust responded count by checksum16 length and rpc id
			if( io->hdr.dataLen >= ekosfRpcMinDataLen )
			{
				*stackSize = io->hdr.dataLen - ekosfRpcMinDataLen;
				if( io->id == rpcId )
					result = (RpcStatus)io->sigOrStat;
				else // response id and request id mismatch
					result = RpcResponseIdMismatch;
			}
			else // we cannot extract procedure id from stack
				result = RpcStackCorrupt;
		} 
	}
	else if( chnlIsBreaking(brk) )
		result = RpcCancelled;
	else// rpc response timed out
		result = RpcTimedOut;

	return result;
}

// network devices enumeration service with address range auto setup. optional onEnum callback may be used
// to filter enumerated device net|break enumeration process, if FALSE is retured from onEnum.
// returned is BYTE = count of enumerated devices, 0 if process failed|aborted
//
BYTE ekosfSlaveEnumerate( EkosfStdIoState* io, BYTE startAddr, BYTE endAddr, ekosfPfnOnEnumDevice onEnum, 
	void* data, CommChannelBreak* brk )
{
	BYTE result = 0;
	if( io && 
			io->chnl )
	{
		BOOL ok = TRUE;
		// save current address
		BYTE saveAddr = io->addr;
		// adjust start address if needed
		if( endAddr == ekosfBroadcastPeerAddr )
			--endAddr;

		while(	ok && 
						startAddr <= endAddr )
		{
			BYTE saveRetries = io->retries;
			// first, ping address to find if it's already occupied
			io->addr = startAddr;
			io->retries = 1;
			ok = ekosfPingSlave(io, brk);
			io->retries = saveRetries;
			if( !ok )
			{
				// address is free, send broadcast setAddress	request
				io->addr = ekosfBroadcastPeerAddr;
				ok = ekosfSlaveAddrSet(io, startAddr, brk);
				if( ok )
					io->addr = startAddr;
			}
			
			if( ok &&
					onEnum && 
					!onEnum(io, startAddr, data) ) // check enumeration filter, if provided
				break;
				
			++result;
			++startAddr;
		}

		// restore saved address
		io->addr = saveAddr;
	}
	
	return result;	
}

// server - side RPC wrappers auto implementation
//
#define RPC_REFLECTION_BEGIN
#define	RPC_DEF_HANDLER0(ReturnType) \
RpcStatus ReturnType ## _Rpc(EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret ) \
{	\
	RpcStatus result = RpcOK; \
	DWORD stackSize = 0; \
	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig, &stackSize, brk ); \
	if( result == RpcOK ) \
	{	\
		BYTE* pos = io->stack; \
		const BYTE* end = io->stack + stackSize; \
		if( !get ## ReturnType(&pos, end, ret) ) \
			result = RpcStackCorrupt; \
	} \
	return result; \
}
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
		result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) && \
			put ## Param4Type(&pos, end, p4) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) && \
			put ## Param4Type(&pos, end, p4) && \
			put ## Param5Type(&pos, end, p5) )	\
	{	\
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type, &stackSize, brk ); \
		if( result == RpcOK ) \
		{	\
			pos = io->stack; \
			end = io->stack + stackSize; \
			if( !get ## ReturnType(&pos, end, ret) ) \
				result = RpcStackCorrupt; \
		} \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define	RPC_DEF_HANDLER0_NORETURN() \
RpcStatus VOID_Rpc(EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk) \
{	\
	RpcStatus result = RpcOK; \
	DWORD stackSize = 0; \
 	result = ekosfDoRPC( io, rpcId, VOID_RpcSig, &stackSize, brk ); \
	return result; \
}
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
		result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) && \
			put ## Param4Type(&pos, end, p4) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
		result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type ## (EkosfStdIoState* io, UINT16 rpcId, CommChannelBreak* brk, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5) \
{ \
	RpcStatus result = RpcOK; \
	BYTE* pos = io->stack; \
	const BYTE* end = io->stack + ekosfRpcMaxStackLen; \
	if( put ## Param0Type(&pos, end, p0) && \
			put ## Param1Type(&pos, end, p1) && \
			put ## Param2Type(&pos, end, p2) &&	\
			put ## Param3Type(&pos, end, p3) && \
			put ## Param4Type(&pos, end, p4) && \
			put ## Param5Type(&pos, end, p5) )	\
	{ \
		DWORD stackSize = pos-io->stack; \
	 	result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type, &stackSize, brk ); \
	} \
	else \
		result = RpcStackOverflow; \
	return result; \
}
#define RPC_REFLECTION_END

#include <common/rpcMap/rpcReflection.cc>

#endif // EKOSF_USE_STD_MASTER

// standard client client implementation
//
#ifdef EKOSF_USE_STD_SLAVE

// initialize slave-specific iostate structure
void ekosfStdSlaveIoInit( EkosfStdIoState* io, CommChannel* chnl )
{
	ekosfStdIoInit(io, chnl);
}

// additional collision detection & elimination
// if we receive slave header with the same address as we have, 
// reset our address to broadcast one
static BOOL ekosfStdMultiSlaveCollisionDetectAndHandle( EkosfStdIoState* client )
{
	if( EKOSF_FT_SLAVE == client->hdr.frameType &&
				ekosfBroadcastPeerAddr != client->hdr.devAddr && 
				client->hdr.devAddr == client->addr )
	{
		chnlResetIo(client->chnl, ekosfGarbageRecoveryTmo);
		client->addr = ekosfBroadcastPeerAddr;
		return TRUE;
	}

	return FALSE;
}

static __inline BOOL ekosfStdSlaveIsAddressed(const EkosfStdIoState* client)
{
	// check if hdr is from server and is addressed to us either by our own address
	// or broadcast address
	return EKOSF_FT_MASTER == client->hdr.frameType && 
				(	client->hdr.devAddr == client->addr ||
					client->hdr.devAddr == ekosfBroadcastPeerAddr );
}

// get optional pending data, return either 0, if OK,
// or EKOSF protocol error code
static UINT16 ekosfStdSlaveOptionalDataGet(EkosfStdIoState* client, CommChannelBreak* brk)
{
	UINT16 err = 0;

	if( client->hdr.dataLen )
	{
		if( client->hdr.dataLen > ekosfMaxDataLen )
			err = EKOSF_ERR_DATA_TOO_LONG;
		else
		{
			DWORD read = 0;
			read = chnlGetBytes(client->chnl, client->rawData,
				(DWORD)client->hdr.dataLen, 
				chnlSendTimeEstimateGet(client->chnl, client->hdr.dataLen) + client->packetTmo, brk);

			if( read != client->hdr.dataLen ) // data size mismatch
				err = EKOSF_ERR_DATA_SIZE;
			else if( read % 2 ) 				// report data misalign
				err = EKOSF_ERR_DATA_MISALIGN;
			else if( 0 != checksum16(client->rawData, read) )
				err = EKOSF_ERR_DATA_CRC;
		}
	}

	return err; 
}

static void ekosfStdSlaveRpcCallHandle(EkosfStdIoState* client)
{
	// parse initial rpc data
	if( client->hdr.dataLen >= ekosfRpcMinDataLen )
	{
		DWORD stackLen = client->hdr.dataLen - ekosfRpcMinDataLen;
		// try to execute remote procedure call. after successfull execution,
		// stackLen should contain length of the result value stack 
		RpcStatus stat = rpcExecLocal(client->id, client->sigOrStat,
			 client->stack, &stackLen, ekosfRpcMaxStackLen);
		// update returned status
		client->sigOrStat = stat;
		// adjust stack to word boundary
		if( stat == RpcOK )
		{
			if( stackLen % 2 ) // adjust data on word boundary
			{
				client->stack[stackLen] = 0; // add zero padding byte
				++stackLen;
			}
		}
		else
			stackLen = 0; // do not return anything in case of RPC error
		// update header record
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_MAKE_RESPONSE(EKOSF_CMD_RPC_EXEC), (UINT16)(stackLen+ekosfRpcStackOffs));
	}
	else
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_ERR_DATA_SIZE_UNEXPECTED, 0);
}

static void ekosfStdSlaveRateChangeHandle(EkosfStdIoState* client)
{
	// extract requested rate from protocol command
	if( client->hdr.dataLen == 6/*sizeof(UINT32)+2*/ )
	{
		DWORD rate;
		memcpy((void*)&rate, client->rawData, 4/*sizeof(UINT32)*/);
		if( chnlIsRateSupported(client->chnl, rate) )
		{
			ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_MAKE_RESPONSE(EKOSF_CMD_RATE_SET), 0);
			ekosfSetPendingRateChange(client, rate);
		}
		else
			ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_ERR_WRONG_RATE, 0);
	}
	else
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_ERR_DATA_SIZE_UNEXPECTED, 0);
}

static BOOL ekosfStdSlaveAddrSetHandle(EkosfStdIoState* client, CommChannelBreak* brk)
{
	// first, check if broadcast address was used to reach us
	if( ekosfBroadcastPeerAddr == client->hdr.devAddr ) 
	{
		// and we do not have any non-broadcast address set
		if(	ekosfBroadcastPeerAddr == client->addr ) 
		{
			BYTE probe;
			DWORD clk = clock();
			// generate pseudo-random time slice from system clock ticks
			clk = ((clk << 2) & 0xFF) ^ (((clk >> 8) & 0xFF) ^ (((clk >> 16) & 0xFF) ^ ((clk >> 24) & 0xFF)));
			// initiate network collision-aware automatic address set-up procedure
			//
			// probe network silence during this time slice. if network is not silent, do not respond at all
			while( clk-- )
				probe = (BYTE)chnlGetBytes(client->chnl, &probe, 1, 1, brk);
			
			if( 0 != probe )
			{
				chnlResetIo(client->chnl, 0);
				return FALSE;	// somebody else is talking on network, do not respond at all
			}
		}
		else // we already have non-broadcast address set, do not respond at all
			return FALSE;
	}
	
	// proceed with slave address set-up
	if( ekosfBroadcastPeerAddr != client->rawData[0] ) // check if new address is not broadcast
	{
		// set new address pending
		client->pendingAddr = client->rawData[0];
		// respond with old address
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_MAKE_RESPONSE(EKOSF_CMD_ADDR_SET), 0);
	}
	else // respond with invalid address error condition. we may not specify broadcast address as new device address
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_ERR_WRONG_ADDR, 0);

	return TRUE;
}

static BOOL ekosfStdSlaveCommandsHandle(EkosfStdIoState* client, CommChannelBreak* brk)
{
	BOOL result = TRUE;

	switch( client->hdr.command )
	{
	case EKOSF_CMD_PING:
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_MAKE_RESPONSE(EKOSF_CMD_PING), 0);
		break;
	case EKOSF_CMD_ADDR_SET:
		result = ekosfStdSlaveAddrSetHandle(client, brk);
		break;
	case EKOSF_CMD_RATE_SET:
		ekosfStdSlaveRateChangeHandle(client);
		break;
	case EKOSF_CMD_RPC_EXEC:
		ekosfStdSlaveRpcCallHandle(client);
		break;
	default:
		// report command is unknown
		ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, EKOSF_ERR_CMD_UNKNOWN, 0);
		break;
	}

	return result;
}

static void ekosfStdSlavePendingUpdatesHandle(EkosfStdIoState* client, CommChannelBreak* brk)
{
	// update us with pending address if needed,
	// or handle pending rate change case
	if( 0 != client->pendingRate || 
			-1 != client->pendingAddr )
	{
		if( chnlWaitTxEmpty(client->chnl, brk) )
		{
			if( 0 != client->pendingRate )
				// execute rate change
				chnlSetRate(client->chnl, client->pendingRate);

			// update pending address change
			if( -1 < client->pendingAddr )
				client->addr = (BYTE)client->pendingAddr;
		}

		// always reset pending rate afterwards
		client->pendingRate = 0;
		client->pendingAddr = -1;
	}
}

BOOL ekosfStdSlaveExecute( EkosfStdIoState* client, CommChannelBreak* brk )
{
	BOOL result = FALSE;
	// check if there is request pending
	if( ekosfGetCheckHdr(client, brk, ekosfHdrFrameReceiveMaxRetries) )
	{
		if( !ekosfStdMultiSlaveCollisionDetectAndHandle(client) &&
				 ekosfStdSlaveIsAddressed(client) &&
				!chnlIsBreaking(brk))
		{
			// get optional pending data
			UINT16 err = ekosfStdSlaveOptionalDataGet(client, brk);
			// report error to the server, reset all faulty RX data
			if( err )
			{
				chnlResetIo(client->chnl, ekosfGarbageRecoveryTmo);
				ekosfUpdateHdr(&client->hdr, EKOSF_FT_SLAVE, err, 0);
				result = TRUE;
			}
			else 
				result = ekosfStdSlaveCommandsHandle(client, brk); 	// handle commands

			// if we need to response, do it
			if( result )
			{				 
				result = ekosfPutData(client, brk); 						// send response data with prepared header

				if( result &&
						!chnlIsBreaking(brk) )
					ekosfStdSlavePendingUpdatesHandle(client, brk);	 	// update pending address|channel rate, if requested
			}
		}
		else
			chnlResetIo(client->chnl, ekosfGarbageRecoveryTmo);
	}

	return result;
}

#endif // EKOSF_USE_STD_SLAVE
