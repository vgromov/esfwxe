#include <esfwxe/target.h>
#pragma hdrstop

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <esfwxe/utils.h>
#include <esfwxe/crc.h>
#include <esfwxe/commintf.h>
#include <esfwxe/trace.h>
#include "protocolEkosf.h"

// ES propritary protocol implementation
//

// trace facility
#if defined(USE_ES_PROTOCOL_TRACE)

# if defined(USE_ES_PROTOCOL_HDR_TRACE)
    static void ekosfHdrTrace(const ProtocolEkosfHdr* hdr)
    {
        ES_DEBUG_TRACE0("EkosfProtocol header contents:\n\r")
        ES_DEBUG_TRACE1("frame type: 0x%02X\n\r",    (esU32)hdr->frameType)
        ES_DEBUG_TRACE1("device address: %u\n\r",    (esU32)hdr->devAddr)
        ES_DEBUG_TRACE1("command|response: 0x%04X\n\r", (esU32)hdr->command)
        ES_DEBUG_TRACE1("optional data length: %u\n\r", (esU32)hdr->dataLen)
        ES_DEBUG_TRACE1("packet sequence number: %u\n\r", (esU32)hdr->packetNum)
        ES_DEBUG_TRACE1("CRC16: 0x%04X\n\r", (esU32)hdr->crc)
    }

#   if defined(USE_ES_PROTOCOL_HDR_TRACE_OUT)
#     define TRACE_ES_HDR_OUT(hdr)    ekosfHdrTrace(hdr);
#   else
#     define TRACE_ES_HDR_OUT(hdr)
#   endif
#   if defined(USE_ES_PROTOCOL_HDR_TRACE_IN)
#     define TRACE_ES_HDR_IN(hdr)      ekosfHdrTrace(hdr);
#   else
#     define TRACE_ES_HDR_IN(hdr)
#   endif
# else
#   define TRACE_ES_HDR_OUT(hdr) 
#   define TRACE_ES_HDR_IN(hdr)
# endif
  
# define ES_PROTOCOL_TRACE0(s)       ES_DEBUG_TRACE0(s)
# define ES_PROTOCOL_TRACE1(s, a0)   ES_DEBUG_TRACE1(s, a0)

# if defined(USE_ES_PROTOCOL_READ_TRACE)  
#   define ES_PROTOCOL_READ_TRACE0(s)  ES_DEBUG_TRACE0(s)
#   define ES_PROTOCOL_READ_TRACE1(s, a) ES_DEBUG_TRACE1(s, a0)
# else
#   define ES_PROTOCOL_READ_TRACE0(s)
#   define ES_PROTOCOL_READ_TRACE1(s, a)
# endif
#else
# define TRACE_ES_HDR_OUT(hdr) 
# define TRACE_ES_HDR_IN(hdr)
# define ES_PROTOCOL_TRACE0(s)
# define ES_PROTOCOL_TRACE1(s, a0)
# define ES_PROTOCOL_READ_TRACE0(s)
# define ES_PROTOCOL_READ_TRACE1(s, a)
#endif

#if defined(USE_DEBUG_TRACE_PIN) && defined(USE_ES_PROTOERR_TRACE)
# include "stm32f3xx_hal.h"
# define ESE_PROTOXXX_TRACE_CFG            DEBUG_TRACE_PIN_CONFIG(B,9) DEBUG_TRACE_PIN_CONFIG(C,10) DEBUG_TRACE_PIN_CONFIG(B,4)
# define ESE_PROTOHDR_CHNLERR_TRACE_ON     DEBUG_TRACE_PIN_ON(B,9)
# define ESE_PROTOHDR_CHNLERR_TRACE_OFF    DEBUG_TRACE_PIN_OFF(B,9)
# define ESE_PROTODATA_CHNLERR_TRACE_ON    DEBUG_TRACE_PIN_ON(C,10)
# define ESE_PROTODATA_CHNLERR_TRACE_OFF   DEBUG_TRACE_PIN_OFF(C,10)
# define ESE_PROTODATA_ERR_TRACE_ON        DEBUG_TRACE_PIN_ON(B,4)
# define ESE_PROTODATA_ERR_TRACE_OFF       DEBUG_TRACE_PIN_OFF(B,4)
#else
# define ESE_PROTOXXX_TRACE_CFG
# define ESE_PROTOHDR_CHNLERR_TRACE_ON
# define ESE_PROTOHDR_CHNLERR_TRACE_OFF
# define ESE_PROTODATA_CHNLERR_TRACE_ON
# define ESE_PROTODATA_CHNLERR_TRACE_OFF
# define ESE_PROTODATA_ERR_TRACE_ON
# define ESE_PROTODATA_ERR_TRACE_OFF
#endif 
  
// update header record crc after some of its fields were changed
static void ekosfUpdateHdrCrc(ProtocolEkosfHdr* hdr)
{
    hdr->crc = checksum16((const esU8*)hdr, ProtocolEkosfHdr_SZE-2);                // 2 byte CRC as of RFC1070

    TRACE_ES_HDR_OUT(hdr)
}

// update subset of header fields
static void ekosfUpdateHdr(ProtocolEkosfHdr* hdr, esU8 frame, esU16 cmd, esU16 dataLen)
{
    hdr->frameType = frame;
    hdr->command = cmd;
    hdr->dataLen = dataLen ? dataLen+2 : 0;

    ekosfUpdateHdrCrc(hdr);
}

// send data over channel using ekosfera protocol.
static esBL ekosfPutData( EseStdIoState* io )
{
    esBL result = 
    chnlTxBatchBegin(io->chnl) && 
    chnlPutBytes(
      io->chnl, 
      (const esU8*)&io->hdr, 
      ProtocolEkosfHdr_SZE
    ) == ProtocolEkosfHdr_SZE;

//  ES_PROTOCOL_TRACE1(
//    "ekosfPutData - HDR %s\n",
//    result ? "sent" : "not sent"
//  )
    
    if( 
    result && 
        io->hdr.dataLen && 
        !chnlIsBreaking(io->chnl) 
  )
    {
        esU16 actualDataLen = io->hdr.dataLen-2;
        esU16 dataCRC = checksum16(io->rawData, actualDataLen);
        result = chnlPutBytes(
      io->chnl, 
      io->rawData, 
      actualDataLen
    ) == actualDataLen &&
        chnlPutBytes(
      io->chnl, 
      (const esU8*)&dataCRC, 
      2
    ) == 2;
    
//    ES_PROTOCOL_TRACE1(
//      "ekosfPutData - DATA %s\n",
//      result ? "sent" : "not sent"
//    )
    }
    
  chnlTxBatchEnd(io->chnl, result);
  
    return result;
}

static __inline void ekosfSetPendingRateChange( EseStdIoState* io, esU32 rate)
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
static int ekosfFullHdrFrameReceive(EseStdIoState* io, esBL slave)
{
    esU32 read = 0;
    esU32 tmo = chnlSendTimeEstimateGet(
    io->chnl, 
    ProtocolEkosfHdr_SZE
  ) + io->packetTmo;
    esU8* buff = (esU8*)&io->hdr;
    read = chnlGetBytes(
    io->chnl, 
    (esU8*)buff, 
    ProtocolEkosfHdr_SZE, 
    tmo
  );
  
    if( chnlIsBreaking(io->chnl) )
  {
    ES_PROTOCOL_TRACE0(
      "ekosfFullHdrFrameReceive - IO is interrupted by breaker\n"
    )
  
    return 0;
  }

  if( ProtocolEkosfHdr_SZE > read )
  {
    if( 0 < read )
    {
      ES_PROTOCOL_TRACE1(
        "ekosfFullHdrFrameReceive - partial frame read: %d bytes\n",
        (int)read
      )
   
      return -1;
    }

    ES_PROTOCOL_TRACE0(
      "ekosfFullHdrFrameReceive - nothing was read\n"
    )
    
    return 0;
  }
  
  const esU8* pos = buff;
  const esU8* end = buff+ProtocolEkosfHdr_SZE;
  esBL hdrMarkerFound = FALSE;
  
  while( 
    !hdrMarkerFound &&
    pos < end 
  )
  {
    if( slave )
      hdrMarkerFound = ES_FT_MASTER == *pos;
    else
      hdrMarkerFound = ES_FT_SLAVE == *pos; 

    if( !hdrMarkerFound )
      ++pos;
  }

  if( !hdrMarkerFound )
  {
    ES_PROTOCOL_TRACE0(
      "ekosfFullHdrFrameReceive - marker was not found\n"
    )

    return -1;
  }
  
  // header marker found in the current ProtocolEkosfHdr_SZE chunk, 
  // check if we're not at the beginning position of the chunk, try to read additional bytes,
  // first moving all including frame marker to the beginning of the buffer
  if( pos != buff )
  {
    esU32 addRead;
    
    read = end-pos;
    addRead = ProtocolEkosfHdr_SZE-read;
    
    ES_DEBUG_TRACE3(
      "ekosfFullHdrFrameReceive - Marker was found at pos: %d, moving %d elements left, reading additional %d bytes\n",
      (int)(pos-buff),
      (int)read,
      (int)addRead
    )

    memmove(buff, pos, read);
    
    tmo = chnlSendTimeEstimateGet(io->chnl, addRead) + io->packetTmo;
    read = chnlGetBytes(io->chnl, buff+addRead, addRead, tmo);
    
    if( !chnlIsBreaking(io->chnl) )
    {
      if(read == addRead)
        return 1;
      else if( 0 < read )
        return -1;
    }
  }
  else
    return 1;

    return 0;
}

// try to receive frame header.
static esBL ekosfGetCheckHdr( EseStdIoState* io, size_t retries, esBL slave )
{
    int frmResult = 0;
  
  if( !retries )
    retries = 1;
  
    while( 1 )
    {
        frmResult = ekosfFullHdrFrameReceive(io, slave);
        if( 1 == frmResult )
    {
      ES_PROTOCOL_TRACE1(
        "ekosfGetCheckHdr received full frame, retries: %d\n",
        retries
      )

      TRACE_ES_HDR_IN(&io->hdr)
    
            if(0 == checksum16((const esU8*)&io->hdr, ProtocolEkosfHdr_SZE))
        return TRUE;

      ES_PROTOCOL_TRACE0(
        "ekosfGetCheckHdr, CS16 does not match\n"
      )
      
      return FALSE;
    }
    
    if( 0 == retries )
    {
      ES_PROTOCOL_TRACE0(
        "ekosfGetCheckHdr - frame read retries expired\n"
      )

      break;
    }
    else
    {
      ES_PROTOCOL_TRACE1(
        "ekosfGetCheckHdr - decrementing retry counter: %d\n",
        (int)retries
      )

      --retries;
    }

    if( 0 == frmResult )
    {
      ES_PROTOCOL_TRACE0(
        "ekosfGetCheckHdr - nothing was read\n"
      )
    
      break;
    }
    else
    {
      ES_PROTOCOL_TRACE0(
        "ekosfGetCheckHdr - resetting channel due to partial frame\n"
      )

      chnlResetIo(io->chnl);
    }
    }

    // handle dangling partial read conditions or channel errors
    if( -1 == frmResult || 0 != chnlGetError(io->chnl) )
  {
    ES_PROTOCOL_TRACE1(
      "ekosfGetCheckHdr - Resetting channel due to %s\n",
      (-1 == frmResult) ? "partial HDR frame read" : "channel error"
    )
    
        chnlResetIo(io->chnl);
  }
  
    return FALSE;
}

// common standard defs
//
#if defined( ESE_USE_RPC_STD_MASTER_IMPL ) || defined(ESE_USE_RPC_STD_SLAVE_IMPL)

static __inline void ekosfStdIoInit( EseStdIoState* io, EseChannelIo* chnl )
{
  ESE_PROTOXXX_TRACE_CFG
  
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
        io->addr = ekosfBroadcastPeerAddr;
        io->chnl = chnl;
        io->mon = 0;
        io->err = 0;             

        io->pendingRate = 0;        
        io->pendingAddr = -1;
    }
}

#endif // defined( ESE_USE_RPC_STD_MASTER_IMPL ) || defined(ESE_USE_RPC_STD_SLAVE_IMPL)

// standard master-initiated services implementation
//
#ifdef ESE_USE_RPC_STD_MASTER_IMPL

// initialize master-specific iostate structure
void ekosfStdMasterIoInit( EseStdIoState* io, EseChannelIo* chnl )
{
    ekosfStdIoInit(io, chnl);
}

// set-up protocol monitor
void ekosfSet( EseStdIoState* io, EseProtocolIoMonitor* mon )
{
    if(io)
        io->mon = mon;
}

// standard slave-to-master response handler.
static esBL ekosfGetResponse(EseStdIoState* io, esU16 cmd)
{
    esBL result = FALSE;
    // check if there is response pending
    if( ekosfGetCheckHdr(io, ekosfHdrFrameReceiveMaxRetries, FALSE) )
    {
        // check if hdr is from client with specified address
        if( !chnlIsBreaking(io->chnl) &&
                io->hdr.frameType == ES_FT_SLAVE && 
                io->hdr.devAddr == io->addr )
        {
            if( ES_IS_ERROR(io->hdr.command) )
            {
                io->err = io->hdr.command;
                 result = TRUE;
            }
            else if( io->hdr.command == ES_MAKE_RESPONSE(cmd) )
            {
                result = TRUE;
                io->err = 0;
                // get optional pending data
                if( io->hdr.dataLen )
                {
                    if( io->hdr.dataLen > ekosfMaxDataLen )
                        io->err = ES_ERR_DATA_TOO_LONG;
                    else
                    {
                        esU32 read = chnlGetBytes(
              io->chnl,
              io->rawData,
                            (esU32)io->hdr.dataLen,
                            chnlSendTimeEstimateGet(
                io->chnl,
                io->hdr.dataLen
              ) + io->packetTmo
            );

//#if defined(__cplusplus) && defined(ES_DEBUG_TRACE)
//            ES_DEBUG_TRACE(
//              esT("ekosfGetResponse optional data: to read=%d, read=%d, packetTmo=%d"),
//              io->hdr.dataLen,
//              read,
//              io->packetTmo
//            );
//#endif

                        result = !chnlIsBreaking(io->chnl);
                        if( result )
                        {
                            if( read != (esU32)io->hdr.dataLen ) // data size mismatch
                                io->err = ES_ERR_DATA_SIZE;
                            else if( read % 2 )                 // report data misalign
                                io->err = ES_ERR_DATA_MISALIGN;
                            else if( 0 != checksum16(io->rawData, read) )
                                io->err = ES_ERR_DATA_CRC;

                            if( 
                0 != chnlGetError(io->chnl) ||
                                (0 != read && io->err) 
              )
                                chnlResetIo(io->chnl);
                        }
                    }
                }
            }
        }
    }

    return result;
}

// wait until tx packet is sent              
static esBL ekosfWaitTxIsSent(EseStdIoState* io, esU32 len)
{
    esU32 tmo = chnlSendTimeEstimateGet(io->chnl, len);
    esBL breaking = chnlIsBreaking(io->chnl);
    while( !breaking &&
                 tmo > 0 )
    {
        ekosfWait(1);
        --tmo;            
        breaking = chnlIsBreaking(io->chnl);
    }

    return !breaking;
}

// protocol frame monitor services.
//
void ekosfProtocolMonitorInit(EseProtocolIoMonitor* mon)
{
    if( mon )
    {
        mon->protocolFrameSent = 0;
        mon->protocolFrameSendingFailed = 0;
        mon->protocolFrameReceived = 0;
        mon->protocolFrameReceptionFailed = 0;
    }
}

// protocol monitor API
void ekosfOnFrameSent(const EseStdIoState* io)
{
    if( io && io->mon && io->mon->protocolFrameSent )
        io->mon->protocolFrameSent(io);
}

void ekosfOnFrameSendingFailed(const EseStdIoState* io)
{
    if( io && io->mon && io->mon->protocolFrameSendingFailed )
        io->mon->protocolFrameSendingFailed(io);
}

void ekosfOnFrameReceived(const EseStdIoState* io)
{
    if( io && io->mon && io->mon->protocolFrameReceived )
        io->mon->protocolFrameReceived(io);
}

void ekosfOnFrameReceptionFailed(const EseStdIoState* io)
{
    if( io && io->mon && io->mon->protocolFrameReceptionFailed )
        io->mon->protocolFrameReceptionFailed(io);
}

// standard master-initiated command execution.
static esBL ekosfExecCommand(EseStdIoState* io, esU16 cmd, esU32 dataLen)
{
  esBL ok = FALSE;
    if( !chnlIsBreaking(io->chnl) )
    {
        // reset io with zero timeout on each retry, so if any trash data occured in RX line,
        // it should not interfere with subsequent ekosfGetResponse call
        chnlResetIo(io->chnl);
    
        // reset header fields on each retry, so if any partial garbage was received in ekosfGetResponse
        // we would definitely get rid of it
        io->hdr.devAddr = io->addr;
        io->hdr.reserved = 0;
        ++io->hdr.packetNum; // always increment packet number
        ekosfUpdateHdr(&io->hdr, ES_FT_MASTER, cmd, (esU16)dataLen);

        if( ekosfPutData( io ) )
        {
            ekosfOnFrameSent(io);

            // start waiting for response after tx packet is sent
            ok = ekosfWaitTxIsSent(io, ProtocolEkosfHdr_SZE+io->hdr.dataLen) &&
                ekosfGetResponse(io, cmd);

            if( ok )
                ekosfOnFrameReceived(io);
        }
    }

    return ok;
}

// standard master-initiated protocol services
//
// ping slave device with specified address
esBL ekosfPingSlave( EseStdIoState* io )
{
    if( io )
        return ekosfExecCommand(io, ES_CMD_PING, 0);
    
    return FALSE;
}

// request slave device address change
esBL ekosfSlaveAddrSet( EseStdIoState* io, esU8 newAddr )
{
    esBL result = FALSE;
    if( io && io->chnl )
    {
        esU32 savedPacketTmo = io->packetTmo;
        io->packetTmo = ekosfMaxAutoSetupAddrTmo;
        // set-up data 
        io->rawData[0] = newAddr;
        io->rawData[1] = 0; // just for word alignment
        
        // execute address change command
        result = ekosfExecCommand(io, ES_CMD_ADDR_SET, 2);
        io->packetTmo = savedPacketTmo;
    }
    
    return result;
}

// request slave device channel rate change
esBL ekosfSlaveRateSet( EseStdIoState* io, esU32 rate )
{
    // first, check if master channel supports requested rate
    if( io && 
            io->chnl && 
            chnlIsRateSupported(io->chnl, rate) )
    {
        // prepare rate command data
        memcpy(io->rawData, (const esU8*)&rate, sizeof(rate));
    
        if( ekosfExecCommand(io, ES_CMD_RATE_SET, sizeof(rate)) )
        {
            // analyze error state. if everything responded ok, 
            // set master rate accordingly
            if( 0 == io->err && !ES_IS_ERROR(io->hdr.command) )
                return chnlSetRate(io->chnl, rate);
        }
    }
        
    return FALSE;
}

// protocol implementation of the single remote procedure call
// rpc stack data must be already formed externally in server data block
RpcStatus ekosfDoRPC( EseStdIoState* io, esU16 rpcId, esU16 rpcSig, esU32* stackSize )
{
    RpcStatus result = RpcOK;
    
    // adjust stack data to even length, if needed, by appending zero byte at the end
    if(*stackSize % 2)
    {
        io->stack[*stackSize] = 0;
        ++(*stackSize);
    }

    io->idGet = rpcId; 
    io->sigOrStat = rpcSig;
    if( ekosfExecCommand(io, ES_CMD_RPC_EXEC, *stackSize + ekosfRpcStackOffs) )
    {
        // analyze potential communication error
        if( io->err )
            result = RpcCommunicationError;
        else // retrieve rpc execution result otherwise
        {
            // adjust responded count by checksum16 length and rpc idGet
            if( io->hdr.dataLen >= ekosfRpcMinDataLen )
            {
                *stackSize = io->hdr.dataLen - ekosfRpcMinDataLen;
                if( io->idGet == rpcId )
                    result = (RpcStatus)io->sigOrStat;
                else // response idGet and request idGet mismatch
                    result = RpcResponseIdMismatch;
            }
            else // we cannot extract procedure idGet from stack
                result = RpcStackCorrupt;
        } 
    }
    else if( chnlIsBreaking(io->chnl) )
        result = RpcCancelled;
    else// rpc response timed out
        result = RpcTimedOut;

    return result;
}

// network devices enumeration service with address range auto setup. optional onEnum callback may be used
// to filter enumerated device net|break enumeration process, if FALSE is retured from onEnum.
// returned is esU8 = count of enumerated devices, 0 if process failed|aborted
//
esU8 ekosfSlaveEnumerate( EseStdIoState* io, esU8 startAddr, esU8 endAddr, ekosfPfnOnEnumDevice onEnum, void* data )
{
    esU8 result = 0;
    if( io && io->chnl )
    {
        esBL ok = TRUE;
        // save current address
        esU8 saveAddr = io->addr;
        // adjust start address if needed
        if( endAddr == ekosfBroadcastPeerAddr )
            --endAddr;

        while( ok && startAddr <= endAddr )
        {
            // first, ping address to find if it's already occupied
            io->addr = startAddr;
            ok = ekosfPingSlave(io);
            if( !ok )
            {
                // address is free, send broadcast setAddress  request
                io->addr = ekosfBroadcastPeerAddr;
                ok = ekosfSlaveAddrSet(io, startAddr);
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
#define  RPC_DEF_HANDLER0(ReturnType) \
RpcStatus ReturnType ## _Rpc(EseStdIoState* io, esU16 rpcId, ReturnType* ret ) \
{    \
    RpcStatus result = RpcOK; \
    esU32 stackSize = 0; \
    result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig, &stackSize ); \
    if( result == RpcOK ) \
    {    \
        esU8* pos = io->stack; \
        const esU8* end = io->stack + stackSize; \
        if( !get_ ## ReturnType(&pos, end, ret) ) { \
            result = RpcStackCorrupt; } \
    } \
    return result; \
}
#define RPC_DEF_HANDLER1(ReturnType, Param0Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; } \
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER2(ReturnType, Param0Type, Param1Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
        result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; }\
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER3(ReturnType, Param0Type, Param1Type, Param2Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; } \
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER4(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; }\
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER5(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) && \
            put_ ## Param4Type(&pos, end, p4) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; } \
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER6(ReturnType, Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus ReturnType ## _Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (EseStdIoState* io, esU16 rpcId, ReturnType* ret, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) && \
            put_ ## Param4Type(&pos, end, p4) && \
            put_ ## Param5Type(&pos, end, p5) )    \
    {    \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, ReturnType ## _RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type, &stackSize ); \
        if( result == RpcOK ) \
        {    \
            pos = io->stack; \
            end = io->stack + stackSize; \
            if( !get_ ## ReturnType(&pos, end, ret) ) { \
                result = RpcStackCorrupt; } \
        } \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define  RPC_DEF_HANDLER0_NORETURN() \
RpcStatus VOID_Rpc(EseStdIoState* io, esU16 rpcId) \
{    \
    RpcStatus result = RpcOK; \
    esU32 stackSize = 0; \
     result = ekosfDoRPC( io, rpcId, VOID_RpcSig, &stackSize ); \
    return result; \
}
#define RPC_DEF_HANDLER1_NORETURN(Param0Type) \
RpcStatus VOID_Rpc_ ## Param0Type (EseStdIoState* io, esU16 rpcId, Param0Type p0) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) )    \
    { \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER2_NORETURN(Param0Type, Param1Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) )    \
    { \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER3_NORETURN(Param0Type, Param1Type, Param2Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) )    \
    { \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER4_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) )    \
    { \
        esU32 stackSize = pos-io->stack; \
        result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER5_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) && \
            put_ ## Param4Type(&pos, end, p4) )    \
    { \
        esU32 stackSize = pos-io->stack; \
        result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_DEF_HANDLER6_NORETURN(Param0Type, Param1Type, Param2Type, Param3Type, Param4Type, Param5Type) \
RpcStatus VOID_Rpc_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type (EseStdIoState* io, esU16 rpcId, Param0Type p0, Param1Type p1, Param2Type p2, Param3Type p3, Param4Type p4, Param5Type p5) \
{ \
    RpcStatus result = RpcOK; \
    esU8* pos = io->stack; \
    const esU8* end = io->stack + ekosfRpcMaxStackLen; \
    if( put_ ## Param0Type(&pos, end, p0) && \
            put_ ## Param1Type(&pos, end, p1) && \
            put_ ## Param2Type(&pos, end, p2) &&    \
            put_ ## Param3Type(&pos, end, p3) && \
            put_ ## Param4Type(&pos, end, p4) && \
            put_ ## Param5Type(&pos, end, p5) )    \
    { \
        esU32 stackSize = pos-io->stack; \
         result = ekosfDoRPC( io, rpcId, VOID_RpcSig_ ## Param0Type ## _ ## Param1Type ## _ ## Param2Type ## _ ## Param3Type ## _ ## Param4Type ## _ ## Param5Type, &stackSize ); \
    } \
    else { \
        result = RpcStackOverflow; } \
    return result; \
}
#define RPC_REFLECTION_END

#include <esfwxe/rpcMap/rpcReflection.cc>

#endif // ESE_USE_RPC_STD_MASTER_IMPL

// standard client client implementation
//
#ifdef ESE_USE_RPC_STD_SLAVE_IMPL

// initialize slave-specific iostate structure
void ekosfStdSlaveIoInit( EseStdIoState* io, EseChannelIo* chnl )
{
    ekosfStdIoInit(io, chnl);
}

// additional collision detection & elimination
// if we receive slave header with the same address as we have, 
// reset our address to broadcast one
static esBL ekosfStdMultiSlaveCollisionDetectAndHandle( EseStdIoState* client )
{
    if( 
    ES_FT_SLAVE == client->hdr.frameType &&
        ekosfBroadcastPeerAddr != client->hdr.devAddr && 
        client->hdr.devAddr == client->addr 
  )
    {
        chnlResetIo(client->chnl);
        client->addr = ekosfBroadcastPeerAddr;
        return TRUE;
    }

    return FALSE;
}

static __inline esBL ekosfStdSlaveIsAddressed(const EseStdIoState* client)
{
    // check if hdr is from server and is addressed to us either by our own address
    // or broadcast address
    return ES_FT_MASTER == client->hdr.frameType && 
                (    client->hdr.devAddr == client->addr ||
                    client->hdr.devAddr == ekosfBroadcastPeerAddr );
}

// get optional pending data, return either 0, if OK,
// or EKOSF protocol error code
static esU16 ekosfStdSlaveOptionalDataGet(EseStdIoState* client)
{
    esU16 err = 0;

    if( client->hdr.dataLen )
    {
    ES_PROTOCOL_READ_TRACE1("Reading %d data bytes\n", (int)client->hdr.dataLen)
  
        if( client->hdr.dataLen > ekosfMaxDataLen )
            err = ES_ERR_DATA_TOO_LONG;
        else
        {
            esU32 read = 0;
      read = chnlGetBytes(
        client->chnl, 
        client->rawData,
                (esU32)client->hdr.dataLen,
                chnlSendTimeEstimateGet(
          client->chnl, 
          client->hdr.dataLen
        ) + client->packetTmo
      );

            if( read != client->hdr.dataLen ) // data size mismatch
                err = ES_ERR_DATA_SIZE;
            else if( read % 2 )                 // report data misalign
                err = ES_ERR_DATA_MISALIGN;
            else if( 0 != checksum16(client->rawData, read) )
      {
                err = ES_ERR_DATA_CRC;
        
#if defined(USE_ES_PROTOCOL_TRACE)
        for(int idx = 0; idx < read; ++idx)
        {
          ES_PROTOCOL_READ_TRACE1("%02X", (int)client->rawData[idx])
        }
        ES_PROTOCOL_READ_TRACE0("\n")
#endif
      }
        }
    }

    return err; 
}

static void ekosfStdSlaveRpcCallHandle(EseStdIoState* client)
{
    // parse initial rpc data
    if( client->hdr.dataLen >= ekosfRpcMinDataLen )
    {
        esU32 stackLen = client->hdr.dataLen - ekosfRpcMinDataLen;
        // try to execute remote procedure call. after successfull execution,
        // stackLen should contain length of the result value stack 
        RpcStatus stat = rpcExecLocal(
      client->idGet, 
      client->sigOrStat,
      client->stack, 
      &stackLen, 
      ekosfRpcMaxStackLen
    );
        
    ES_DEBUG_TRACE4(
      "rpcExecLocal(id: %d, sig: %d, stacklen: %d) returns %d\n",
      (int)client->idGet, 
      (int)client->sigOrStat,
      (int)stackLen, 
      (int)stat
    )

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
        ekosfUpdateHdr(
      &client->hdr, 
      ES_FT_SLAVE, 
      ES_MAKE_RESPONSE(ES_CMD_RPC_EXEC), 
      (esU16)(stackLen+ekosfRpcStackOffs)
    );
    }
    else
        ekosfUpdateHdr(
      &client->hdr, 
      ES_FT_SLAVE, 
      ES_ERR_DATA_SIZE_UNEXPECTED, 
      0
    );
}

static void ekosfStdSlaveRateChangeHandle(EseStdIoState* client)
{
    // extract requested rate from protocol command
    if( client->hdr.dataLen == 6/*sizeof(esU32)+2*/ )
    {
        esU32 rate;
        memcpy((void*)&rate, client->rawData, 4/*sizeof(esU32)*/);
        if( chnlIsRateSupported(client->chnl, rate) )
        {
            ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_MAKE_RESPONSE(ES_CMD_RATE_SET), 0);
            ekosfSetPendingRateChange(client, rate);
        }
        else
            ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_ERR_WRONG_RATE, 0);
    }
    else
        ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_ERR_DATA_SIZE_UNEXPECTED, 0);
}

static esU32 pseudoRandomTimeSliceGet(void)
{
  esU32 clk = clock();
  // generate pseudo-random time slice from system clock ticks
  return (((clk << 2) & 0xFF) ^ (((clk >> 8) & 0xFF) ^ (((clk >> 16) & 0xFF) ^ ((clk >> 24) & 0xFF))));
}

static esBL ekosfStdSlaveAddrSetHandle(EseStdIoState* client)
{
    // first, check if broadcast address was used to reach us
    if( ekosfBroadcastPeerAddr == client->hdr.devAddr ) 
    {
        // and we do not have any non-broadcast address set
        if(    ekosfBroadcastPeerAddr == client->addr ) 
        {
            esU8 probe;
            esU32 clk = pseudoRandomTimeSliceGet();
            // initiate network collision-aware automatic address set-up procedure
            //
            // probe network silence during this time slice. if network is not silent, do not respond at all
            while( clk-- )
                probe = (esU8)chnlGetBytes(client->chnl, &probe, 1, 1);
            
            if( 0 != probe )
            {
                chnlResetIo(client->chnl);
                return FALSE;    // somebody else is talking on network, do not respond at all
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
        ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_MAKE_RESPONSE(ES_CMD_ADDR_SET), 0);
    }
    else // respond with invalid address error condition. we may not specify broadcast address as new device address
        ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_ERR_WRONG_ADDR, 0);

    return TRUE;
}

static esBL ekosfStdSlaveCommandsHandle(EseStdIoState* client)
{
    esBL result = TRUE;

    switch( client->hdr.command )
    {
    case ES_CMD_PING:
    ES_DEBUG_TRACE0("Ponging\n")
        ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, ES_MAKE_RESPONSE(ES_CMD_PING), 0);
        break;
    case ES_CMD_ADDR_SET:
    ES_DEBUG_TRACE0("Setting client address\n")
        result = ekosfStdSlaveAddrSetHandle(client);
        break;
    case ES_CMD_RATE_SET:
    ES_DEBUG_TRACE0("Setting client baudrate\n")
        ekosfStdSlaveRateChangeHandle(client);
        break;
    case ES_CMD_RPC_EXEC:
    ES_DEBUG_TRACE0("Executing RPC\n")
        ekosfStdSlaveRpcCallHandle(client);
        break;
    default:
        // report command is unknown
        ekosfUpdateHdr(
      &client->hdr, 
      ES_FT_SLAVE, 
      ES_ERR_CMD_UNKNOWN, 
      0
    );
        break;
    }

    return result;
}

static void ekosfStdSlavePendingUpdatesHandle(EseStdIoState* client)
{
    // update us with pending address if needed,
    // or handle pending rate change case
    if( 0 != client->pendingRate || 
            -1 != client->pendingAddr )
    {
        if( chnlWaitTxEmpty(client->chnl) )
        {
            if( 0 != client->pendingRate )
                // execute rate change
                chnlSetRate(client->chnl, client->pendingRate);

            // update pending address change
            if( -1 < client->pendingAddr )
                client->addr = (esU8)client->pendingAddr;
        }

        // always reset pending rate afterwards
        client->pendingRate = 0;
        client->pendingAddr = -1;
    }
}

esBL ekosfStdSlaveExecute( EseStdIoState* client )
{
    // check if there is request pending
    if( !ekosfGetCheckHdr(client, ekosfHdrFrameReceiveMaxRetries, TRUE) )
    return FALSE;
    
  esBL result = FALSE;
  
#if defined(USE_ES_PROTOCOL_TRACE)
  esBL ok = !ekosfStdMultiSlaveCollisionDetectAndHandle(client);
    
  if( !ok )
    ES_PROTOCOL_TRACE0("ekosfStdMultiSlaveCollisionDetectAndHandle detected collision\n")
  else
  {
    ok = ekosfStdSlaveIsAddressed(client);
    if( !ok )
      ES_PROTOCOL_TRACE1("ekosfStdSlaveIsAddressed(%d) returned false\n", (int)client->hdr.devAddr)
    else
    {
      ok = !chnlIsBreaking(client->chnl);
      if( !ok )  
        ES_PROTOCOL_TRACE0("chnlIsBreaking returned true")
    }
  }
  
  if( ok )
#else    
  if( 
    !ekosfStdMultiSlaveCollisionDetectAndHandle(client) &&
     ekosfStdSlaveIsAddressed(client) &&
    !chnlIsBreaking(client->chnl)
  )
#endif    
  {
    // get optional pending data
    esU16 protoErr = ekosfStdSlaveOptionalDataGet(client);
    
    // if  no communication error occured - proceed
    esU32 err = chnlGetError(client->chnl);
    
    if(0 == err)
    {
      // report error to the server, reset all faulty RX data
      if( protoErr )
      {
        ES_PROTOCOL_TRACE1("protError %d\n", protoErr)

        ESE_PROTODATA_ERR_TRACE_ON
      
        chnlResetIo(client->chnl);
        ekosfUpdateHdr(&client->hdr, ES_FT_SLAVE, protoErr, 0);
        result = TRUE;

        ESE_PROTODATA_ERR_TRACE_OFF
      }
      else 
        result = ekosfStdSlaveCommandsHandle(client);     // handle commands

      // if we need to response, do it
      if( result )
      {                 
        result = ekosfPutData(client);                         // send response data with prepared header

        if( 
          result &&
          !chnlIsBreaking(client->chnl) 
        )
          ekosfStdSlavePendingUpdatesHandle(client);         // update pending address|channel rate, if requested
      }
    }
    else
    {
      ES_PROTOCOL_TRACE1("Chnl error %d\n", (int)err)
      
      ESE_PROTODATA_CHNLERR_TRACE_ON

      chnlResetIo(client->chnl);

      ESE_PROTODATA_CHNLERR_TRACE_OFF
    }
  }

    return result;
}

#endif // ESE_USE_RPC_STD_SLAVE_IMPL
