#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <stddef.h>

#include <bgapiConfig.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>

#if defined(ESE_BLEBG_USE_MEMINTF)
# include <esfwxe/cpp/api/EseMemoryIntf.h>
#endif

#if defined(ESE_BLEBG_USE_OSINTF)
# include <esfwxe/cpp/concept/EseIntfPtr.h>
# include <esfwxe/cpp/os/api/EseOsIntf.h>
# include <esfwxe/cpp/os/api/EseOsQueueIntf.h>
#else
# include <esfwxe/cpp/os/EseKernel.h>
#endif

#if defined(USE_BLE_TRACE)

# if defined(ESE_BLEBG_USE_DBGINTF)
#   include <esfwxe/cpp/api/EseDebugIntf.h>
#   define ESE_BLEBG_TRACE              EseBleBg::dbgIntfGet().trace
#   define ESE_BLEBG_ASSERT(x)          EseBleBg::dbgIntfGet().assertTrue((x), __FILE__, __LINE__)
# else
#   include <esfwxe/trace.h>
#   include <esfwxe/ese_assert.h>
#   define ESE_BLEBG_TRACE              ES_DEBUG_TRACE
#   define ESE_BLEBG_ASSERT             ESE_ASSERT
# endif

#else

# define ESE_BLEBG_TRACE(fmt, ...)      ((void)0)
# define ESE_BLEBG_ASSERT(x)            ((void)0)

#endif

#include <esfwxe/drivers/ble/bgapi/cmd_def.h>

#include "EseBleBgWakeupIntf.h"
#include "EseBleBgEvtHandlerIntf.h"
#include "EseBleBg.h"
//--------------------------------------------------------------------------

// Allow us to reside in specific section
#ifndef ESE_BLEBG_SECTION
# define ESE_BLEBG_SECTION
#endif
//--------------------------------------------------------------------------

ESE_BLEBG_SECTION
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

EseBleBg::EseBleBg(
#if defined(ESE_BLEBG_USE_MEMINTF)
  const EseMemoryIntf& imem,
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
  const EseOsIntf& ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
  const EseDebugIntf& idbg,
#endif
  EseChannelIntf& chnl, 
  EseBleBgWakeupIntf& iwkup
) ESE_NOTHROW :
#if defined(ESE_BLEBG_USE_OSINTF)
m_ios(ios),
#endif
#if defined(ESE_BLEBG_USE_MEMINTF)
m_imem(imem),
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
m_idbg(idbg),
#endif
m_chnl(chnl),
m_ihandler(nullptr),
m_iwkup(iwkup),
m_buff(nullptr),
m_flags(0),
m_handlingQueue(false)
{
#if defined(ESE_BLEBG_USE_OSINTF)
  m_evtsQueue.reset(
    ios.queueCreate(
      ESE_BLEBG_EVT_QUEUE_LEN,
      sizeof(EseBleBgEvent)
    )
  );
#else
  m_evtsQueue.init(ESE_BLEBG_EVT_QUEUE_LEN);
#endif
}
//------------------------------------------------------------------------------------

EseBleBg::~EseBleBg() ESE_NOTHROW
{
  if( m_buff )
  {
#if defined(ESE_BLEBG_USE_MEMINTF)
    m_imem.
#endif
    free( m_buff );
    m_buff = nullptr;
  }

  queuedEvtsReset();

  m_ihandler = nullptr;
  instance() = nullptr;
}
//------------------------------------------------------------------------------------

#if defined(ESE_BLEBG_USE_MEMINTF)
void* EseBleBg::operator new(size_t sze, const EseMemoryIntf& memIntf) ESE_NOTHROW
{
  return memIntf.malloc(sze);
}
//------------------------------------------------------------------------------------

void EseBleBg::operator delete(void* ptr, const EseMemoryIntf& memIntf) ESE_NOTHROW
{
  EseBleBg* bgapi = reinterpret_cast<EseBleBg*>(ptr);
  
  if( bgapi )
  {
    bgapi->~EseBleBg();
    memIntf.free(ptr);
  }
}
//------------------------------------------------------------------------------------
#endif

bool EseBleBg::isCreated() ESE_NOTHROW
{
  return nullptr != instance();
}
//------------------------------------------------------------------------------------

EseBleBg*& EseBleBg::instance() ESE_NOTHROW
{
#ifndef ESE_BLEBG_SHARED_INSTANCE_CUSTOM
 static EseBleBg* s_inst = nullptr;
 return s_inst;
#else 
 return ESE_BLEBG_SHARED_INSTANCE_CUSTOM;
#endif
}
//--------------------------------------------------------------------------

EseBleBg* EseBleBg::instanceGet() ESE_NOTHROW
{
  return instance();
}
//------------------------------------------------------------------------------------

EseBleBg* EseBleBg::create(
#if defined(ESE_BLEBG_USE_MEMINTF)
  const EseMemoryIntf& imem,
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
  const EseOsIntf& ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
  const EseDebugIntf& idbg,
#endif
  EseChannelIntf& chnl, 
  EseBleBgWakeupIntf& iwkup
) ESE_NOTHROW
{
  if( instance() )
    return instance();

  instance() = 
#if defined(ESE_BLEBG_USE_MEMINTF)
  new(imem) EseBleBg(
    imem,
#if defined(ESE_BLEBG_USE_OSINTF)
    ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
    idbg,
#endif
    chnl, 
    iwkup
  );
#else
  new EseBleBg(
#if defined(ESE_BLEBG_USE_OSINTF)
    ios,
#endif
#if defined(ESE_BLEBG_USE_DBGINTF)
    idbg,
#endif
    chnl, 
    iwkup
  );
#endif

  ESE_BLEBG_ASSERT( instance() );

  return instance();
}
//------------------------------------------------------------------------------------

void EseBleBg::destroy() ESE_NOTHROW
{
  #if defined(ESE_BLEBG_USE_MEMINTF)
    operator delete(
      this,
      m_imem
    );
#else
    delete this;
#endif
}
//------------------------------------------------------------------------------------

void EseBleBg::queuedEvtsReset() ESE_NOTHROW 
{ 
  m_handlingQueue = false; 
  
#if defined(ESE_BLEBG_USE_OSINTF)
  ESE_BLEBG_ASSERT(m_evtsQueue);
  m_evtsQueue->reset(); 
#else
  m_evtsQueue.reset();
#endif
}
//------------------------------------------------------------------------------------

bool EseBleBg::isWokenUp() const ESE_NOTHROW
{
  return m_iwkup.isWoken(); 
}
//------------------------------------------------------------------------------------

#ifdef USE_BLE_TRACE

static void resultTrace(esU16 result)
{
  if( result )
  {
    esU16 source = (result >> 8);
    
    switch( source )
    {
    case 0x01:
      ESE_BLEBG_TRACE("BGAPI error occurred:\n");
      break;
    case 0x02:
      ESE_BLEBG_TRACE("Bluetooth error occurred:\n");
      break;
    case 0x03:
      ESE_BLEBG_TRACE("Security manager error occurred:\n");
      break;
    case 0x04:
      ESE_BLEBG_TRACE("Attribute protocol error occurred:\n");
      break;
    case 0xFF:
      ESE_BLEBG_TRACE("Application software error:\n");
      break;
    }
    
    switch(result)
    {
    case 0x0180:
      ESE_BLEBG_TRACE(">>Invalid Parameter\n");
      break;
    case 0x0181:
      ESE_BLEBG_TRACE(">>Device in Wrong State\n");
      break;
    case 0x0182:
      ESE_BLEBG_TRACE(">>Out Of Memory\n");
      break;
    case 0x0183:
      ESE_BLEBG_TRACE(">>Feature Not Implemented\n");
      break;
    case 0x0184:
      ESE_BLEBG_TRACE(">>Command Not Recognized\n");
      break;
    case 0x0185:
      ESE_BLEBG_TRACE(">>Command or Procedure failed due to timeout\n");
      break;
    case 0x0186:
      ESE_BLEBG_TRACE(">>Not Connected\n");
      break;
    case 0x0187:
      ESE_BLEBG_TRACE(">>Under- or over-flow\n");
      break;
    case 0x0188:
      ESE_BLEBG_TRACE(">>User attribute was accessed through API which is not supported\n");
      break;
    case 0x0189:
      ESE_BLEBG_TRACE(">>Invalid License Key\n");
      break;
    case 0x018A:
      ESE_BLEBG_TRACE(">>Command Too Long\n");
      break;
    case 0x018B:
      ESE_BLEBG_TRACE(">>Out of Bonds\n");
      break;
    case 0x0205:
      ESE_BLEBG_TRACE(">>Authentication Failure\n");
      break;
    case 0x0206:
      ESE_BLEBG_TRACE(">>Pin or Key Missing\n");
      break;
    case 0x0207:
      ESE_BLEBG_TRACE(">>Memory Capacity Exceeded\n");
      break;
    case 0x0208:
      ESE_BLEBG_TRACE(">>Connection Timeout\n");
      break;
    case 0x0209:
      ESE_BLEBG_TRACE(">>Connection Limit Exceeded\n");
      break;
    case 0x020C:
      ESE_BLEBG_TRACE(">>Command Disallowed\n");
      break;
    case 0x0212:
      ESE_BLEBG_TRACE(">>Invalid Command Parameters\n");
      break;
    case 0x0213:
      ESE_BLEBG_TRACE(">>Remote User Terminated Connection\n");
      break;
    case 0x0216:
      ESE_BLEBG_TRACE(">>Connection Terminated by Local Host\n");
      break;
    case 0x0222:
      ESE_BLEBG_TRACE(">>LL Response Timeout\n");
      break;
    case 0x0228:
      ESE_BLEBG_TRACE(">>LL Instant Passed\n");
      break;
    case 0x023A:
      ESE_BLEBG_TRACE(">>Controller Busy\n");
      break;
    case 0x023B:
      ESE_BLEBG_TRACE(">>Unacceptable Connection Interval\n");
      break;
    case 0x023C:
      ESE_BLEBG_TRACE(">>Directed Advertising Timeout\n");
      break;
    case 0x023D:
      ESE_BLEBG_TRACE(">>MIC Failure\n");
      break;
    case 0x023E:
      ESE_BLEBG_TRACE(">>Connection Failed to be Established\n");
      break;      
    case 0x0301:
      ESE_BLEBG_TRACE(">>Passkey Entry Failed\n");
      break;
    case 0x0302:
      ESE_BLEBG_TRACE(">>OOB Data is not available\n");
      break;
    case 0x0303:
      ESE_BLEBG_TRACE(">>Authentication Requirements\n");
      break;
    case 0x0304:
      ESE_BLEBG_TRACE(">>Confirm Value Failed\n");
      break;
    case 0x0305:
      ESE_BLEBG_TRACE(">>Pairing Not Supported\n");
      break;
    case 0x0306:
      ESE_BLEBG_TRACE(">>Encryption Key Size\n");
      break;
    case 0x0307:
      ESE_BLEBG_TRACE(">>Command Not Supported\n");
      break;
    case 0x0308:
      ESE_BLEBG_TRACE(">>Unspecified Reason\n");
      break;
    case 0x0309:
      ESE_BLEBG_TRACE(">>Repeated Attempts\n");
      break;
    case 0x030A:
      ESE_BLEBG_TRACE(">>Invalid Parameters\n");
      break;
    case 0x0401:
      ESE_BLEBG_TRACE(">>Invalid Handle\n");
      break;
    case 0x0402:
      ESE_BLEBG_TRACE(">>Read Not Permitted\n");
      break;
    case 0x0403:
      ESE_BLEBG_TRACE(">>Write Not Permitted\n");
      break;
    case 0x0404:
      ESE_BLEBG_TRACE(">>Invalid PDU\n");
      break;
    case 0x0405:
      ESE_BLEBG_TRACE(">>Insufficient Authentication\n");
      break;
    case 0x0406:
      ESE_BLEBG_TRACE(">>Request Not Supported\n");
      break;
    case 0x0407:
      ESE_BLEBG_TRACE(">>Invalid Offset\n");
      break;
    case 0x0408:
      ESE_BLEBG_TRACE(">>Insufficient Authorization\n");
      break;
    case 0x0409:
      ESE_BLEBG_TRACE(">>Prepare Queue Full\n");
      break;
    case 0x040A:
      ESE_BLEBG_TRACE(">>Attribute Not Found\n");
      break;
    case 0x040B:
      ESE_BLEBG_TRACE(">>Attribute Not Long\n");
      break;
    case 0x040C:
      ESE_BLEBG_TRACE(">>Insufficient Encryption Key Size\n");
      break;
    case 0x040D:
      ESE_BLEBG_TRACE(">>Invalid Attribute Value Length\n");
      break;
    case 0x040E:
      ESE_BLEBG_TRACE(">>Unlikely Error\n");
      break;
    case 0x040F:
      ESE_BLEBG_TRACE(">>Insufficient Encryption\n");
      break;
    case 0x0410:
      ESE_BLEBG_TRACE(">>Unsupported Group Type\n");
      break;
    case 0x0411:
      ESE_BLEBG_TRACE(">>Insufficient Resources\n");
      break;
    case 0x0480:
      ESE_BLEBG_TRACE(">>Application Error Codes\n");
      break;
    case 0xFFFF:
      ESE_BLEBG_TRACE(">>Command response never received\n");
      break;
    }
  }
}
# define ESE_BLE_RESULT_TRACE(result)             resultTrace(result)
# define ESE_BLE_TRACE                            ESE_BLEBG_TRACE
#else
# define ESE_BLE_RESULT_TRACE(result)             ((void)0)
# define ESE_BLE_TRACE(fmt, ...)                  ((void)0)
#endif  

//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

#if defined(USE_BLE_TRACE) && defined(USE_BLE_RX_TRACE)

void EseBleBg::bleBufTrace(const char* bufname, const esU8* data, esU32 len) const ESE_NOTHROW
{
  if( 0 < len && data )
  {
    len = MIN(len, EseBleBg::rxDataBuffLen);
    
    if( bufname )
    {
      ESE_BLE_TRACE( "%s[%d]: '", bufname, len );
    }
    
    const esU8* end = data+len;  
    while( data < end )
    {
      ESE_BLE_TRACE( "%02X ", ((int)*data) );
      ++data;
    }
    ESE_BLE_TRACE("UERR: %04X'\n", m_chnl.errorGet());
  }
}
//-------------------------------------------------------------------------------------

# define ESE_BLE_RX_TRACE(buff, len)  bleBufTrace("RX", (buff), (len))
# define ESE_BLE_RX_OK_MARKER         ESE_BLEBG_TRACE("--------RX OK--------\n")
# define ESE_BLE_RX_NOK_MARKER        ESE_BLEBG_TRACE("--------RX NOK--------\n")

#else

# define ESE_BLE_RX_TRACE(buff, len)  ((void)0)
# define ESE_BLE_RX_OK_MARKER         ((void)0)
# define ESE_BLE_RX_NOK_MARKER        ((void)0)

#endif
//------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

size_t EseBleBg::internalRead(esU8* dest, size_t len) ESE_NOTHROW
{
  esU8* pos = dest;
  size_t packetTmo = m_chnl.dataXferTimeoutEstimateGet(len);
  size_t _read = m_chnl.receive(
    pos,
    len,
    ESE_BLEBG_READ_TMO+packetTmo
  );
    
  ESE_BLE_RX_TRACE(pos, _read);

  pos += _read;
  if( _read && _read < len ) //< We've started receiving something, but got less, than requested. Try to read the rest
  {
    len = len-_read;
    _read = m_chnl.receive(
      pos,
      len,
      2*packetTmo
    );
    
    ESE_BLE_RX_TRACE(pos, _read);
    
    pos += _read;
  }
  
  return pos-dest;
}
//-------------------------------------------------------------------------------------

void EseBleBg::read() ESE_NOTHROW
{
  ES_BIT_CLR(m_flags, flagRecentReadOk);
  size_t len = sizeof(ble_header);
  
  ble_header apihdr;

#if defined(ESE_BLEBG_USE_MEMINTF)
  m_imem.
#endif
  memset(&apihdr, 0, len);
  
  m_evt.id = EseBleBgEvent::btevtNone;
  
  // read api header first
  len = internalRead(
    reinterpret_cast<esU8*>(&apihdr), 
    len
  );
    
  if( len != sizeof(ble_header) ) //< Header read failed, exit
  {
  
#ifdef DEBUG
    if( len ) { ESE_BLE_RX_NOK_MARKER; }
#endif
      
    return;
  }
  
  // We've got entire header, try to read the data payload, if any
  size_t loadlen = ((apihdr.type_hilen & 0x03) << 8) + apihdr.lolen;
  if( loadlen )
  {
    if( loadlen > rxDataBuffLen )
    {
      ESE_BLE_RX_NOK_MARKER;
      return;
    }

    if( !m_buff ) //< Allocate data buffer once
      m_buff = reinterpret_cast<esU8*>(

#if defined(ESE_BLEBG_USE_MEMINTF)
        m_imem.
#endif
        malloc(rxDataBuffLen)
      );

    ESE_BLEBG_ASSERT(m_buff);

#if defined(ESE_BLEBG_USE_MEMINTF)
    m_imem.
#endif
    memset(
      m_buff, 
      0, 
      loadlen
    );
            
    // Read payload
    len = internalRead(
      m_buff, 
      loadlen
    );
    
    if(len == loadlen) //< Payload read OK
    {
      ES_BIT_SET(m_flags, flagRecentReadOk);
      ESE_BLE_RX_OK_MARKER;
    }
  }
  else //< No payload, just header
  {
    ES_BIT_SET(m_flags, flagRecentReadOk);
    ESE_BLE_RX_OK_MARKER;
  }
  
  if( !ES_BIT_IS_SET(m_flags, flagRecentReadOk) ) //< Data packet read failed, exit
  {
    ESE_BLE_RX_NOK_MARKER;
    return;
  }
  
  // Parse received data packet
  const ble_msg *apimsg = ble_get_msg_hdr(apihdr);
  if( apimsg )
    apimsg->handler( m_buff );
  else //< We've got garbage|lost packet synch on RX
    rxGarbageHandle(apihdr, m_buff);
}
//-------------------------------------------------------------------------------------

void EseBleBg::write(esU8 len1, esU8* data1, esU16 len2, esU8* data2) ESE_NOTHROW
{
  ES_BIT_CLR(m_flags, flagRecentWriteOk);
  
  // Handle wake-up condition
  m_iwkup.wakeUp( 
    ESE_BLEBG_SLEEP_ALLOW_TMO + 
    2*m_chnl.dataXferTimeoutEstimateGet(
      len1+
      len2
#ifdef ESE_BLEBG_PACKET_MODE
      +1
#endif
    ) 
  );
  
#ifdef ESE_BLEBG_PACKET_MODE
  esU8 packetLen = len1+len2;
  if( 
    1 != m_chnl.send(
      &packetLen, 
      1, 
      m_chnl.dataXferTimeoutEstimateGet(1)
    ) 
  )
    return;
#endif
  
  size_t sent = m_chnl.send(
    data1, 
    len1, 
    m_chnl.dataXferTimeoutEstimateGet(len1)
  );
  
  if( sent == len1 )
  {
    if( !len2 )
      ES_BIT_SET(m_flags, flagRecentWriteOk);
    else
    {
      sent = m_chnl.send(
        data2, 
        len2, 
        m_chnl.dataXferTimeoutEstimateGet(len2)
      );
      
      if(sent == len2)
        ES_BIT_SET(m_flags, flagRecentWriteOk);
    }
  }
}
//-------------------------------------------------------------------------------------

void EseBleBg::swrite(esU8 len1, esU8* data1, esU16 len2, esU8* data2) ESE_NOTHROW
{
  instanceGet()->write(
    len1, 
    data1, 
    len2, 
    data2
  );
}
//-------------------------------------------------------------------------------------

EseBleBgEvent& EseBleBg::internalEvtAccess() ESE_NOTHROW 
{ 
  return instanceGet()->m_evt; 
}
//-------------------------------------------------------------------------------------

#ifdef ESE_BLEBG_USE_MEMINTF
const EseMemoryIntf& EseBleBg::memIntfGet() ESE_NOTHROW
{
  return instanceGet()->m_imem;
}
//-------------------------------------------------------------------------------------

static const EseMemoryIntf& mem() ESE_NOTHROW
{
  return EseBleBg::memIntfGet();
}
//-------------------------------------------------------------------------------------
#endif

#if defined(ESE_BLEBG_USE_DBGINTF)
const EseDebugIntf& EseBleBg::dbgIntfGet() ESE_NOTHROW
{
  return instanceGet()->m_idbg;
}
//-------------------------------------------------------------------------------------

const EseDebugIntf& dbg() ESE_NOTHROW
{
  return EseBleBg::dbgIntfGet();
}
//-------------------------------------------------------------------------------------
#endif

bool EseBleBg::blockingWaitForResponseOrEvent(esU8 expectedId, bool writtenOkCheck /*= true*/) ESE_NOTHROW
{
  if(  //< Expecting response ID
    expectedId > EseBleBgEvent::btevtLastID &&
    expectedId < EseBleBgEvent::btrspLastID
  )
  {
    if( 
      writtenOkCheck && 
      !ES_BIT_IS_SET(m_flags, flagRecentWriteOk)
    )
      return false;
  }
  else if( 
    expectedId == EseBleBgEvent::btevtNone || 
    expectedId == EseBleBgEvent::btevtLastID ||
    expectedId >= EseBleBgEvent::btrspLastID
  )
    return false;
  
  esU32 ts = 
#ifdef ESE_BLEBG_USE_OSINTF
    m_ios.sysTickGet();
#else
    EseKernel::sysTickGet();
#endif

  esU32 prevTs = ts;
  esU32 ticks = 0;
  
  while( 
    !handlerIsBreaking() && 
    ticks <= ESE_BLEBG_READ_TMO
  )
  {
    read(); ///< Block on read. Response callbacks and events are being called there
          
    ts = 
#ifdef ESE_BLEBG_USE_OSINTF
      m_ios.sysTickGet();
#else
      EseKernel::sysTickGet();
#endif

    ticks += ts-prevTs;
    prevTs = ts;
    
    if( ES_BIT_IS_SET(m_flags, flagRecentReadOk) )
    {
      // Something was received in read() call
      // Check, if it's event. If so, queue it to the aux handler
      if( 
        m_evt.id > EseBleBgEvent::btevtNone && 
        m_evt.id < EseBleBgEvent::btevtLastID 
      )
      {
        evtPost();
        ticks = 0;
      }
      else if( expectedId == m_evt.id )
        return true;
    }
  }
  
  return false;
}
//-------------------------------------------------------------------------------------

void EseBleBg::process() ESE_NOTHROW
{
  evtQueuedHandle();
  
  read(); ///< Block on read. Response callbacks and events are being called there
  
  if( ES_BIT_IS_SET(m_flags, flagRecentReadOk) )
  {
    // Something was received in read() call
    // Check, if it's event. If so, queue it to the aux handler
    //
    if( 
      m_evt.id > EseBleBgEvent::btevtNone && 
      m_evt.id < EseBleBgEvent::btevtLastID 
    )
      evtHandle();
  }
}
//-------------------------------------------------------------------------------------

void EseBleBg::evtHandle() ESE_NOTHROW
{
  if( m_ihandler )
  {
    ESE_BLE_TRACE("Handling event %d\n", (int)m_evt.id);
    m_ihandler->evtHandle(m_evt);
  }
}
//-------------------------------------------------------------------------------------

void EseBleBg::rspErrorHandle(esU8 rspId, esU16 result) ESE_NOTHROW
{
  ESE_BLE_RESULT_TRACE(result);

  if( m_ihandler )
  {
    ESE_BLE_TRACE("Handling response %d error 0x%0X\n", (int)rspId, (int)result);
    m_ihandler->rspErrorHandle(rspId, result);
  }
}
//-------------------------------------------------------------------------------------

void EseBleBg::rxGarbageHandle(const ble_header& apihdr, const esU8* data) ESE_NOTHROW
{
  if( m_ihandler )
    m_ihandler->rxGarbageHandle(apihdr, data);
}
//-------------------------------------------------------------------------------------

bool EseBleBg::handlerIsBreaking() ESE_NOTHROW
{
  if( !m_ihandler )
    return false;
    
  return m_ihandler->isBreaking();
}
//-------------------------------------------------------------------------------------

void EseBleBg::evtPost() ESE_NOTHROW
{
#if defined(ESE_BLEBG_USE_OSINTF)
  ESE_BLEBG_ASSERT(m_evtsQueue);
#endif

#ifdef USE_BLE_TRACE
  rtosStatus stat = rtosOK;
#endif

  if( m_evtsQueue->isFull() )
  {
    EseBleBgEvent dummy;
    
#ifdef USE_BLE_TRACE
    stat = 
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
    m_evtsQueue->popFront<EseBleBgEvent>(dummy, 0);
#else
    m_evtsQueue.popFront(dummy, 0);
#endif
    
    ESE_BLE_TRACE("BLE event queue overrun, front popped with result %d\n", (int)stat);
  }
  
#ifdef USE_BLE_TRACE
  stat = 
#endif
#if defined(ESE_BLEBG_USE_OSINTF)
    m_evtsQueue->pushBack<EseBleBgEvent>(m_evt, 0);
#else
    m_evtsQueue.pushBack(m_evt, 0);
#endif

  ESE_BLE_TRACE("Event %d is queued for later with result %d\n", (int)m_evt.id, (int)stat);
}
//-------------------------------------------------------------------------------------

void EseBleBg::evtQueuedHandle() ESE_NOTHROW
{
  // Recursion guard
  if( m_handlingQueue )
    return;

  m_handlingQueue = true;
  
#if defined(ESE_BLEBG_USE_OSINTF)
  ESE_BLEBG_ASSERT(m_evtsQueue);
  if( !m_evtsQueue->isOk() )
    return;
#else
  if( !m_evtsQueue.isOk() )
    return;
#endif
    
  while(
#if defined(ESE_BLEBG_USE_OSINTF)
    !m_evtsQueue->isEmpty()
#else
    !m_evtsQueue.isEmpty()
#endif
  )
  {
    EseBleBgEvent evt;
    if( 
      rtosOK == 
#if defined(ESE_BLEBG_USE_OSINTF)
      m_evtsQueue->popFront<EseBleBgEvent>(
        evt, 
        rtosMaxDelay
      )
#else
      m_evtsQueue.popFront(
        evt, 
        rtosMaxDelay
      )
#endif
      && 
      m_ihandler 
    )
    {
      ESE_BLE_TRACE("Handling postponed event %d\n", (int)evt.id);
      
      m_ihandler->evtHandle( evt );
    }
  }
  
  m_handlingQueue = false;
}
//-------------------------------------------------------------------------------------  
//-------------------------------------------------------------------------------------  

#define ESE_BLE_RETRY_CALL_NORETURN(functionResponse, function, ...) \
  ESE_BLE_TRACE(esSTRINGIZE(function) "\n"); \
  bool ok = true; \
  esU32 retries = 0; \
  do { \
    function( __VA_ARGS__ ); \
    ok = blockingWaitForResponseOrEvent((functionResponse)); \
    ++retries; \
  } while( !ok && retries < ESE_BLEBG_IO_RETRIES ); \
  ESE_BLE_TRACE(esSTRINGIZE(function) ": '%s' after %d retries\n", (ok ? "OK" : "NOK"), retries); \
  if( !ok ) rspErrorHandle(functionResponse, 0xFFFF)
  
#define ESE_BLE_RETRY_CALL(functionResponse, function, ...) \
  ESE_BLE_TRACE(esSTRINGIZE(function) "\n"); \
  bool ok = true; \
  esU32 retries = 0; \
  do { \
    function( __VA_ARGS__ ); \
    ok = blockingWaitForResponseOrEvent((functionResponse)); \
    ++retries; \
  } while( !ok && retries < ESE_BLEBG_IO_RETRIES ); \
  ESE_BLE_TRACE(esSTRINGIZE(function) ": '%s' after %d retries\n", (ok ? "OK" : "NOK"), retries); \
  if( !ok ) rspErrorHandle(functionResponse, 0xFFFF); \
  return ok
  
#define ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(functionResponse, result, function, ...) \
  ESE_BLE_TRACE(esSTRINGIZE(function) "\n"); \
  bool ok = true; \
  esU32 retries = 0; \
  do { \
    function( __VA_ARGS__ ); \
    ok = blockingWaitForResponseOrEvent((functionResponse)); \
    ++retries; \
  } while( !ok && retries < ESE_BLEBG_IO_RETRIES ); \
  ESE_BLE_TRACE(esSTRINGIZE(function) ": '%s' after %d retries\n", (ok ? "OK" : "NOK"), retries); \
  if( ok ) { \
    if( !isPositiveResult(result) ) rspErrorHandle(functionResponse, result); \
  } else rspErrorHandle(functionResponse, 0xFFFF); \
  return ok

//-------------------------------------------------------------------------------------

bool EseBleBg::connStatusGet(esU8 handle) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspConnStatus, ble_cmd_connection_get_status, handle);
}

extern "C" void ble_rsp_connection_get_status(const struct ble_msg_connection_get_status_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspConnStatus;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::connEncrypt(esU8 handle, bool bondCreate) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspConnEncryptionStart, ble_cmd_sm_encrypt_start, handle, bondCreate ? 1 : 0);
}

extern "C" void ble_rsp_sm_encrypt_start(const struct ble_msg_sm_encrypt_start_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspConnEncryptionStart;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_sm_encrypt_start_rsp_t>(
    &evt.encryptionStartInfo,
    msg
  );
#else
  evt.encryptionStartInfo = *msg;
#endif
}
//-------------------------------------------------------------------------------------

bool EseBleBg::connDisconnect(esU8 handle) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspConnDisconnect, m_evt.connDisconnect.result, ble_cmd_connection_disconnect, handle);
}

extern "C" void ble_rsp_connection_disconnect(const struct ble_msg_connection_disconnect_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspConnDisconnect;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_connection_disconnect_rsp_t>(
    &evt.connDisconnect,
    msg
  );
#else
  evt.connDisconnect = *msg;
#endif
}
//-------------------------------------------------------------------------------------

bool EseBleBg::gapEndProcedure() ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspGapEndProcedure, m_evt.result, ble_cmd_gap_end_procedure);
}

extern "C" void ble_rsp_gap_end_procedure(const struct ble_msg_gap_end_procedure_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspGapEndProcedure;
  
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::gapAdvertisingSet(esU16 adv_interval_min, esU16 adv_interval_max, esU8 adv_channels) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspGapAdvertisingSet, m_evt.result, ble_cmd_gap_set_adv_parameters, adv_interval_min, adv_interval_max, adv_channels);
}

extern "C" void ble_rsp_gap_set_adv_parameters(const struct ble_msg_gap_set_adv_parameters_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspGapAdvertisingSet;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::gapModeSet(esU8 discover, esU8 connect) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspGapModeSet, m_evt.result, ble_cmd_gap_set_mode, discover, connect);
}

extern "C" void ble_rsp_gap_set_mode(const struct ble_msg_gap_set_mode_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspGapModeSet;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::attrLocalWrite(esU16 handle, esU8 offset, esU8 value_len, const esU8* value_data) ESE_NOTHROW
{  
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspLocalAttrWrite, m_evt.result, ble_cmd_attributes_write, handle, offset, value_len, value_data);
}

extern "C" void ble_rsp_attributes_write(const struct ble_msg_attributes_write_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspLocalAttrWrite;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::attrSend(esU8 connHandle, esU16 handle, esU8 value_len, const esU8* value_data) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspAttrSend, m_evt.result, ble_cmd_attributes_send, connHandle, handle, value_len, value_data);
}

extern "C" void ble_rsp_attributes_send(const struct ble_msg_attributes_send_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspAttrSend;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::attrLocalRead(esU16 handle, esU8 offset, esU8& attrLen, esU8* data, esU8 dataLen) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_NORETURN(EseBleBgEvent::btrspLocalAttrRead, ble_cmd_attributes_read, handle, offset);
  if( ok )
  {
    if( isPositiveResult(m_evt.attrValueRead.result) )
    {
      attrLen = m_evt.dataLocalReadLenGet();
      
#ifdef ESE_BLEBG_USE_MEMINTF
      m_imem.
#endif
      memcpy(
        data, 
        m_evt.dataLocalReadGet(), 
        MIN(
          attrLen, 
          dataLen
        )
      );
      
      return true;
    }
    else
      rspErrorHandle(
        EseBleBgEvent::btrspLocalAttrRead, 
        m_evt.attrValueRead.result
      );
  }
  
  attrLen = 0;
  return false;
}

extern "C" void ble_rsp_attributes_read(const struct ble_msg_attributes_read_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspLocalAttrRead;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().
#endif  
  memcpy(
    evt.data<ble_msg_attributes_read_rsp_t>(), 
    msg, 
    sizeof(ble_msg_attributes_read_rsp_t)+msg->value.len
  );
}
//-------------------------------------------------------------------------------------

bool EseBleBg::attrUserValueReadResponse(esU8 connHandle, esU8 att_error, esU8 value_len, const esU8* value_data) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspAttrUserValueRead, ble_cmd_attributes_user_read_response, connHandle, att_error, value_len, value_data);
}

extern "C" void ble_rsp_attributes_user_read_response(const void* nul)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspAttrUserValueRead;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::attrUserValueWriteResponse(esU8 connHandle, esU8 att_error) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspAttrUserValueWrite, ble_cmd_attributes_user_write_response, connHandle, att_error);
}

extern "C" void ble_rsp_attributes_user_write_response(const void* nul)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspAttrUserValueWrite;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::securityMgrBondableSet(bool bondable) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspSmgrBondableSet, ble_cmd_sm_set_bondable_mode, bondable ? 1 : 0);
}

extern "C" void ble_rsp_sm_set_bondable_mode(const void* nul)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspSmgrBondableSet;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::securityMgrBondingDelete(esU8 bonding) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspSmgrBondingDelete, m_evt.result, ble_cmd_sm_delete_bonding, bonding);
}  

extern "C" void ble_rsp_sm_delete_bonding(const struct ble_msg_sm_delete_bonding_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspSmgrBondingDelete;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::securityMgrParametersSet(esU8 mitm, esU8 min_key_size, esU8 io_capabilities) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspSmgrParametersSet, ble_cmd_sm_set_parameters, mitm, min_key_size, io_capabilities);
}

extern "C" void ble_rsp_sm_set_parameters(const void* nul)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspSmgrParametersSet;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::securityMgrPasskeyEntrySet(esU8 hconnection, esU32 passkey) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspSmgrPasskeyEntrySet, m_evt.result, ble_cmd_sm_passkey_entry, hconnection, passkey);
}

extern "C" void ble_rsp_sm_passkey_entry(const struct ble_msg_sm_passkey_entry_rsp_t * msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspSmgrPasskeyEntrySet;  
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::sysHello() ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL(EseBleBgEvent::btrspHello, ble_cmd_system_hello);
}

extern "C" void ble_rsp_system_hello(const void* nul)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspHello;
}
//-------------------------------------------------------------------------------------
  
bool EseBleBg::sysReset(bool dfu /*= false*/) ESE_NOTHROW
{
  ESE_BLE_TRACE("ble_cmd_system_reset\n");
  
  ble_cmd_system_reset(dfu ? 1 : 0);
  return ES_BIT_IS_SET(m_flags, flagRecentWriteOk);
}

// Dummy response, for conformance only, system reset command,
// if successfully written, this command causes immediate module reboot
// without any response.
extern "C" void ble_rsp_system_reset(const void* nul)
{}
//-------------------------------------------------------------------------------------

bool EseBleBg::deviceAddrGet(bd_addr_t& addr) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_NORETURN(EseBleBgEvent::btrspAddressGet, ble_cmd_system_address_get);
  if( ok )
  {
    addr = m_evt.deviceAddr.address;
    return true;
  }
  
  return false;
}

extern "C" void ble_rsp_system_address_get(const struct ble_msg_system_address_get_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspAddressGet;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_system_address_get_rsp_t>(
    &evt.deviceAddr,
    msg
  );
#else
  evt.deviceAddr = *msg;
#endif
}
//-------------------------------------------------------------------------------------

bool EseBleBg::hwGpioIrqEnable(esU8 port, esU8 mask) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspHwPortIrqEnable, m_evt.result, ble_cmd_hardware_io_port_irq_enable, port, mask);
}

extern "C" void ble_rsp_hardware_io_port_irq_enable(const struct ble_msg_hardware_io_port_irq_enable_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspHwPortIrqEnable;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::hwGpioWrite(esU8 port, esU8 mask, esU8 data) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspHwPortWrite, m_evt.result, ble_cmd_hardware_io_port_write, port, mask, data);
}

extern "C" void ble_rsp_hardware_io_port_write(const struct ble_msg_hardware_io_port_write_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspHwPortWrite;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------

bool EseBleBg::hwGpioDirectionConfig(esU8 port, esU8 dir) ESE_NOTHROW
{
  ESE_BLE_RETRY_CALL_WITH_RESULT_ERROR_HANDLING(EseBleBgEvent::btrspHwPortDirCfg, m_evt.result, ble_cmd_hardware_io_port_config_direction, port, dir);
}

extern "C" void ble_rsp_hardware_io_port_config_direction(const struct ble_msg_hardware_io_port_config_direction_rsp_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btrspHwPortDirCfg;
  evt.result = msg->result;
}
//-------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------

// BGAPI mappings
//
extern "C"
{

#define bglib_output EseBleBg::swrite
#include <esfwxe/drivers/ble/bgapi/cmd_def.c>

// BGAPI events implementation
//
void ble_evt_dfu_boot(const struct ble_msg_dfu_boot_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtBootDfu;

#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_dfu_boot_evt_t>(
    &evt.bootInfoDfu,
    msg
  );
#else
  evt.bootInfoDfu = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_system_boot(const struct ble_msg_system_boot_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtBoot;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_system_boot_evt_t>(
    &evt.bootInfo,
    msg
  );
#else
  evt.bootInfo = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_connection_status(const struct ble_msg_connection_status_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtConnStatus;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_connection_status_evt_t>(
    &evt.connStatus,
    msg
  );
#else
  evt.connStatus = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_connection_disconnected(const struct ble_msg_connection_disconnected_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtConnDisconnected;

#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_connection_disconnected_evt_t>(
    &evt.connDisconnectInfo,
    msg
  );
#else
  evt.connDisconnectInfo = *msg;
#endif

  ESE_BLE_RESULT_TRACE(msg->reason);
}
//-------------------------------------------------------------------------------------

void ble_evt_sm_bonding_fail(const struct ble_msg_sm_bonding_fail_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtSmgrBondingFail;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_sm_bonding_fail_evt_t>(
    &evt.bondingFailInfo,
    msg
  );
#else
  evt.bondingFailInfo = *msg;
#endif
  
  ESE_BLE_RESULT_TRACE(msg->result);
}
//-------------------------------------------------------------------------------------

void ble_evt_attributes_status(const struct ble_msg_attributes_status_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtLocalAttrStatusChanged;

#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_attributes_status_evt_t>(
    &evt.attrStatus,
    msg
  );
#else
  evt.attrStatus = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_attributes_value(const struct ble_msg_attributes_value_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtAttrUserValueWriteRequest;
  evt.attrValueWrite = *msg;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().
#endif
  memcpy(
    &evt.attrValueWrite, 
    msg, 
    sizeof(ble_msg_attributes_value_evt_t)+msg->value.len
  );
}
//-------------------------------------------------------------------------------------

void ble_evt_attributes_user_read_request(const struct ble_msg_attributes_user_read_request_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtAttrUserValueReadRequest;

#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_attributes_user_read_request_evt_t>(
    &evt.attrUserRead,
    msg
  );
#else
  evt.attrUserRead = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_sm_passkey_request(const struct ble_msg_sm_passkey_request_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtPasskeyRequested;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_sm_passkey_request_evt_t>(
    &evt.passkeyRequest,
    msg
  );
#else
  evt.passkeyRequest = *msg;
#endif
}
//-------------------------------------------------------------------------------------

void ble_evt_sm_bond_status(const struct ble_msg_sm_bond_status_evt_t *msg)
{
  EseBleBgEvent& evt = EseBleBg::internalEvtAccess();
  evt.id = EseBleBgEvent::btevtSmgrBondStatus;
  
#ifdef ESE_BLEBG_USE_MEMINTF
  mem().memcpy<ble_msg_sm_bond_status_evt_t>(
    &evt.bondStatus,
    msg
  );
#else
  evt.bondStatus = *msg;
#endif
}
//-------------------------------------------------------------------------------------

} // extern "C"
//-------------------------------------------------------------------------------------

/// Unused BLE BG staff
extern "C" {

void ble_default(const void*v)
{}

void ble_rsp_system_reg_write(const struct ble_msg_system_reg_write_rsp_t *msg)
{
}

void ble_rsp_system_reg_read(const struct ble_msg_system_reg_read_rsp_t *msg)
{
}

void ble_rsp_system_get_counters(const struct ble_msg_system_get_counters_rsp_t *msg)
{
}

void ble_rsp_system_get_connections(const struct ble_msg_system_get_connections_rsp_t *msg)
{
}

void ble_rsp_system_read_memory(const struct ble_msg_system_read_memory_rsp_t *msg)
{
}

void ble_rsp_system_get_info(const struct ble_msg_system_get_info_rsp_t *msg)
{
}

void ble_rsp_system_endpoint_tx(const struct ble_msg_system_endpoint_tx_rsp_t *msg)
{
}

void ble_rsp_system_whitelist_append(const struct ble_msg_system_whitelist_append_rsp_t *msg)
{
}

void ble_rsp_system_whitelist_remove(const struct ble_msg_system_whitelist_remove_rsp_t *msg)
{
}

void ble_rsp_system_whitelist_clear(const void* nul)
{
}

void ble_rsp_system_endpoint_rx(const struct ble_msg_system_endpoint_rx_rsp_t *msg)
{
}

void ble_rsp_system_endpoint_set_watermarks(const struct ble_msg_system_endpoint_set_watermarks_rsp_t *msg)
{
}

void ble_rsp_system_aes_setkey(const void* nul)
{
}

void ble_rsp_system_aes_encrypt(const struct ble_msg_system_aes_encrypt_rsp_t *msg)
{
}

void ble_rsp_system_aes_decrypt(const struct ble_msg_system_aes_decrypt_rsp_t *msg)
{
}

void ble_rsp_flash_ps_defrag(const void* nul)
{
}

void ble_rsp_flash_ps_dump(const void* nul)
{
}

void ble_rsp_flash_ps_erase_all(const void* nul)
{
}

void ble_rsp_flash_ps_save(const struct ble_msg_flash_ps_save_rsp_t *msg)
{
}

void ble_rsp_flash_ps_load(const struct ble_msg_flash_ps_load_rsp_t *msg)
{
}

void ble_rsp_flash_ps_erase(const void* nul)
{
}

void ble_rsp_flash_erase_page(const struct ble_msg_flash_erase_page_rsp_t *msg)
{
}

void ble_rsp_flash_write_data(const struct ble_msg_flash_write_data_rsp_t *msg)
{
}

void ble_rsp_flash_read_data(const struct ble_msg_flash_read_data_rsp_t *msg)
{
}

void ble_rsp_attributes_read_type(const struct ble_msg_attributes_read_type_rsp_t *msg)
{
}

void ble_rsp_connection_get_rssi(const struct ble_msg_connection_get_rssi_rsp_t *msg)
{
}

void ble_rsp_connection_update(const struct ble_msg_connection_update_rsp_t *msg)
{
}

void ble_rsp_connection_version_update(const struct ble_msg_connection_version_update_rsp_t *msg)
{
}

void ble_rsp_connection_channel_map_get(const struct ble_msg_connection_channel_map_get_rsp_t *msg)
{
}

void ble_rsp_connection_channel_map_set(const struct ble_msg_connection_channel_map_set_rsp_t *msg)
{
}

void ble_rsp_connection_features_get(const struct ble_msg_connection_features_get_rsp_t *msg)
{
}

void ble_rsp_connection_raw_tx(const struct ble_msg_connection_raw_tx_rsp_t *msg)
{
}

void ble_rsp_connection_slave_latency_disable(const struct ble_msg_connection_slave_latency_disable_rsp_t *msg)
{
}

void ble_rsp_attclient_find_by_type_value(const struct ble_msg_attclient_find_by_type_value_rsp_t *msg)
{
}

void ble_rsp_attclient_read_by_group_type(const struct ble_msg_attclient_read_by_group_type_rsp_t *msg)
{
}

void ble_rsp_attclient_read_by_type(const struct ble_msg_attclient_read_by_type_rsp_t *msg)
{
}

void ble_rsp_attclient_find_information(const struct ble_msg_attclient_find_information_rsp_t *msg)
{
}

void ble_rsp_attclient_read_by_handle(const struct ble_msg_attclient_read_by_handle_rsp_t *msg)
{
}

void ble_rsp_attclient_attribute_write(const struct ble_msg_attclient_attribute_write_rsp_t *msg)
{
}

void ble_rsp_attclient_write_command(const struct ble_msg_attclient_write_command_rsp_t *msg)
{
}

void ble_rsp_attclient_indicate_confirm(const struct ble_msg_attclient_indicate_confirm_rsp_t *msg)
{
}

void ble_rsp_attclient_read_long(const struct ble_msg_attclient_read_long_rsp_t *msg)
{
}

void ble_rsp_attclient_prepare_write(const struct ble_msg_attclient_prepare_write_rsp_t *msg)
{
}

void ble_rsp_attclient_execute_write(const struct ble_msg_attclient_execute_write_rsp_t *msg)
{
}

void ble_rsp_attclient_read_multiple(const struct ble_msg_attclient_read_multiple_rsp_t *msg)
{
}

void ble_rsp_sm_get_bonds(const struct ble_msg_sm_get_bonds_rsp_t *msg)
{
}

void ble_rsp_sm_set_oob_data(const void* nul)
{
}

void ble_rsp_sm_whitelist_bonds(const struct ble_msg_sm_whitelist_bonds_rsp_t *msg)
{
}

void ble_rsp_sm_set_pairing_distribution_keys(const struct ble_msg_sm_set_pairing_distribution_keys_rsp_t *msg)
{
}

void ble_rsp_gap_set_privacy_flags(const void* nul)
{
}

void ble_rsp_gap_discover(const struct ble_msg_gap_discover_rsp_t *msg)
{
}

void ble_rsp_gap_connect_direct(const struct ble_msg_gap_connect_direct_rsp_t *msg)
{
}

void ble_rsp_gap_connect_selective(const struct ble_msg_gap_connect_selective_rsp_t *msg)
{
}

void ble_rsp_gap_set_filtering(const struct ble_msg_gap_set_filtering_rsp_t *msg)
{
}

void ble_rsp_gap_set_scan_parameters(const struct ble_msg_gap_set_scan_parameters_rsp_t *msg)
{
}

void ble_rsp_gap_set_adv_data(const struct ble_msg_gap_set_adv_data_rsp_t *msg)
{
}

void ble_rsp_gap_set_directed_connectable_mode(const struct ble_msg_gap_set_directed_connectable_mode_rsp_t *msg)
{
}

void ble_rsp_gap_set_initiating_con_parameters(const struct ble_msg_gap_set_initiating_con_parameters_rsp_t *msg)
{
}

void ble_rsp_gap_set_nonresolvable_address(const struct ble_msg_gap_set_nonresolvable_address_rsp_t *msg)
{
}

void ble_rsp_hardware_io_port_config_irq(const struct ble_msg_hardware_io_port_config_irq_rsp_t *msg)
{
}

void ble_rsp_hardware_set_soft_timer(const struct ble_msg_hardware_set_soft_timer_rsp_t *msg)
{
}

void ble_rsp_hardware_adc_read(const struct ble_msg_hardware_adc_read_rsp_t *msg)
{
}

void ble_rsp_hardware_io_port_config_function(const struct ble_msg_hardware_io_port_config_function_rsp_t *msg)
{
}

void ble_rsp_hardware_io_port_config_pull(const struct ble_msg_hardware_io_port_config_pull_rsp_t *msg)
{
}

void ble_rsp_hardware_io_port_read(const struct ble_msg_hardware_io_port_read_rsp_t *msg)
{
}

void ble_rsp_hardware_spi_config(const struct ble_msg_hardware_spi_config_rsp_t *msg)
{
}

void ble_rsp_hardware_spi_transfer(const struct ble_msg_hardware_spi_transfer_rsp_t *msg)
{
}

void ble_rsp_hardware_i2c_read(const struct ble_msg_hardware_i2c_read_rsp_t *msg)
{
}

void ble_rsp_hardware_i2c_write(const struct ble_msg_hardware_i2c_write_rsp_t *msg)
{
}

void ble_rsp_hardware_set_txpower(const void* nul)
{
}

void ble_rsp_hardware_timer_comparator(const struct ble_msg_hardware_timer_comparator_rsp_t *msg)
{
}

void ble_rsp_hardware_io_port_irq_direction(const struct ble_msg_hardware_io_port_irq_direction_rsp_t *msg)
{
}

void ble_rsp_hardware_analog_comparator_enable(const void* nul)
{
}

void ble_rsp_hardware_analog_comparator_read(const struct ble_msg_hardware_analog_comparator_read_rsp_t *msg)
{
}

void ble_rsp_hardware_analog_comparator_config_irq(const struct ble_msg_hardware_analog_comparator_config_irq_rsp_t *msg)
{
}

void ble_rsp_hardware_set_rxgain(const void* nul)
{
}

void ble_rsp_hardware_usb_enable(const struct ble_msg_hardware_usb_enable_rsp_t *msg)
{
}

void ble_rsp_hardware_sleep_enable(const struct ble_msg_hardware_sleep_enable_rsp_t *msg)
{
}

void ble_rsp_test_phy_tx(const void* nul)
{
}

void ble_rsp_test_phy_rx(const void* nul)
{
}

void ble_rsp_test_phy_end(const struct ble_msg_test_phy_end_rsp_t *msg)
{
}

void ble_rsp_test_phy_reset(const void* nul)
{
}

void ble_rsp_test_get_channel_map(const struct ble_msg_test_get_channel_map_rsp_t *msg)
{
}

void ble_rsp_test_debug(const struct ble_msg_test_debug_rsp_t *msg)
{
}

void ble_rsp_test_channel_mode(const void* nul)
{
}

void ble_rsp_dfu_reset(const void* nul)
{
}

void ble_rsp_dfu_flash_set_address(const struct ble_msg_dfu_flash_set_address_rsp_t *msg)
{
}

void ble_rsp_dfu_flash_upload(const struct ble_msg_dfu_flash_upload_rsp_t *msg)
{
}

void ble_rsp_dfu_flash_upload_finish(const struct ble_msg_dfu_flash_upload_finish_rsp_t *msg)
{
}

void ble_evt_system_debug(const struct ble_msg_system_debug_evt_t *msg)
{
}

void ble_evt_system_endpoint_watermark_rx(const struct ble_msg_system_endpoint_watermark_rx_evt_t *msg)
{
}

void ble_evt_system_endpoint_watermark_tx(const struct ble_msg_system_endpoint_watermark_tx_evt_t *msg)
{
}

void ble_evt_system_script_failure(const struct ble_msg_system_script_failure_evt_t *msg)
{
}

void ble_evt_system_no_license_key(const void* nul)
{
}

void ble_evt_system_protocol_error(const struct ble_msg_system_protocol_error_evt_t *msg)
{
}

void ble_evt_flash_ps_key(const struct ble_msg_flash_ps_key_evt_t *msg)
{
}

void ble_evt_connection_version_ind(const struct ble_msg_connection_version_ind_evt_t *msg)
{
}

void ble_evt_connection_feature_ind(const struct ble_msg_connection_feature_ind_evt_t *msg)
{
}

void ble_evt_connection_raw_rx(const struct ble_msg_connection_raw_rx_evt_t *msg)
{
}

void ble_evt_attclient_indicated(const struct ble_msg_attclient_indicated_evt_t *msg)
{
}

void ble_evt_attclient_procedure_completed(const struct ble_msg_attclient_procedure_completed_evt_t *msg)
{
}

void ble_evt_attclient_group_found(const struct ble_msg_attclient_group_found_evt_t *msg)
{
}

void ble_evt_attclient_attribute_found(const struct ble_msg_attclient_attribute_found_evt_t *msg)
{
}

void ble_evt_attclient_find_information_found(const struct ble_msg_attclient_find_information_found_evt_t *msg)
{
}

void ble_evt_attclient_attribute_value(const struct ble_msg_attclient_attribute_value_evt_t *msg)
{
}

void ble_evt_attclient_read_multiple_response(const struct ble_msg_attclient_read_multiple_response_evt_t *msg)
{
}

void ble_evt_sm_smp_data(const struct ble_msg_sm_smp_data_evt_t *msg)
{
}

void ble_evt_sm_passkey_display(const struct ble_msg_sm_passkey_display_evt_t *msg)
{
}

void ble_evt_gap_scan_response(const struct ble_msg_gap_scan_response_evt_t *msg)
{
}

void ble_evt_gap_mode_changed(const struct ble_msg_gap_mode_changed_evt_t *msg)
{
}

void ble_evt_hardware_io_port_status(const struct ble_msg_hardware_io_port_status_evt_t *msg)
{
}

void ble_evt_hardware_soft_timer(const struct ble_msg_hardware_soft_timer_evt_t *msg)
{
}

void ble_evt_hardware_adc_result(const struct ble_msg_hardware_adc_result_evt_t *msg)
{
}

void ble_evt_hardware_analog_comparator_status(const struct ble_msg_hardware_analog_comparator_status_evt_t *msg)
{
}

} // extern "C"
