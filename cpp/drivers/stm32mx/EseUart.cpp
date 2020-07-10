#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

#include <stddef.h>
#include <cstring>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/concept/EseChannel.h>

#include <esfwxe/cpp/EseException.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseQueue.h>
#include <esfwxe/cpp/os/EseTask.h>
#include <esfwxe/cpp/os/EseKernel.h>

//----------------------------------------------------------------------
#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>

#include "EseUart.h"

// UART Hardware initialization|uninitialization parts - must be defined elsewhere
extern "C" void MX_UART_Init(USART_TypeDef* uart, UART_HandleTypeDef* huart);
extern "C" void HAL_UART_MspInit(UART_HandleTypeDef* huart);
extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef* huart);
extern "C" void MX_UART_StateReset(UART_HandleTypeDef* huart);
extern "C" void MX_UART_StateSetBusy(UART_HandleTypeDef* huart);
extern "C" void MX_UART_StateSetReady(UART_HandleTypeDef* huart);
extern "C" bool MX_UART_StateIsTransmitting(UART_HandleTypeDef* huart);
//----------------------------------------------------------------------
//----------------------------------------------------------------------

// Rough max estimate of ms per 2 bytes at specified baud
static __inline esU32 uartGetMaxByteTimeoutMs( esU32 len, esU32 baud, esU32 dataBits, esU32 stopBits, esU32 parityBits ) ESE_NOTHROW
{
  esU32 result = (len * 2000 * (dataBits + stopBits + parityBits + 1)) / baud;
  return result ? result : 1;
}
//----------------------------------------------------------------------

EseUart::EseUart(USART_TypeDef* hw, size_t rxLen, size_t txLen) ESE_NOTHROW :
ChannelT(rxLen, txLen),
m_rxtmp(0),
m_shouldYield(false)
{
  std::memset(&m_h, 0, sizeof(m_h));
 
  // Set up default DCB
  MX_UART_Init(hw, &m_h);
}

EseUart::~EseUart() ESE_NOTHROW
{
  uninit();
}

bool EseUart::isOk() const ESE_NOTHROW
{
  return m_mx.isOk();
}

rtosStatus EseUart::lock(esU32 tmo) ESE_NOTHROW
{
  return m_mx.lock(tmo);
}

rtosStatus EseUart::unlock() ESE_NOTHROW
{
  return m_mx.unlock();
}

bool EseUart::isValidRate(esU32 rate) const ESE_NOTHROW
{
  return 2400 <= rate && rate <= 921600;
}

void EseUart::dcbSet(const UART_InitTypeDef& dcb) ESE_NOTHROW
{
  m_h.Init = dcb;
  ES_BIT_CLR(m_flags, flagConfigured);
}

const UART_InitTypeDef& EseUart::dcbGet() const ESE_NOTHROW 
{ 
  return m_h.Init; 
}

#if defined(STM32F3) || defined(STM32L0)
void EseUart::dcbexSet(const UART_AdvFeatureInitTypeDef& dcbex) ESE_NOTHROW
{
  m_h.AdvancedInit = dcbex;
  ES_BIT_CLR(m_flags, flagConfigured);
}

const UART_AdvFeatureInitTypeDef& EseUart::dcbexGet() const ESE_NOTHROW
{
  return m_h.AdvancedInit;
}
#endif

bool EseUart::shouldYield() const ESE_NOTHROW 
{ 
  return m_shouldYield; 
}

void EseUart::resetYieldFlag() ESE_NOTHROW 
{ 
  m_shouldYield = false; 
}

/// Channel implementation
const EseUart::ChannelT::HandleT& EseUart::doHandleGet() const ESE_NOTHROW 
{ 
  return m_h; 
}

EseUart::ChannelT::HandleT& EseUart::doHandleGet() ESE_NOTHROW 
{ 
  return m_h; 
}

bool EseUart::doInit() ESE_NOTHROW
{
  m_mx.init();
  m_rx.init(m_rxLen);
  m_tx.init(m_txLen);
  
  MX_UART_StateReset(&m_h);
  return doCheckConfigured();
}

void EseUart::doUninit() ESE_NOTHROW
{
  {
    EseIsrCriticalSection cs;
   
    HAL_UART_DeInit(&m_h);
  }

  m_rx.reset();
  m_tx.reset();
  m_rx.uninit();
  m_tx.uninit();
  m_mx.uninit();
}

bool EseUart::doCheckConfigured() ESE_NOTHROW
{
  EseIsrCriticalSection cs;
  
  HAL_StatusTypeDef stat = HAL_UART_Init(&m_h);
  __HAL_UART_DISABLE(&m_h);

  return HAL_OK == stat;
}

esU32 EseUart::dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW
{
  return uartGetMaxByteTimeoutMs(
    len, 
    m_h.Init.BaudRate, 
    5+(IS_UART_WORD_LENGTH(m_h.Init.WordLength) ? 16 : 8), 
    1+(IS_UART_STOPBITS(m_h.Init.StopBits) ? 2 : 1), 
    IS_UART_PARITY(m_h.Init.Parity) ? 1 : 0
  );
}

bool EseUart::doActivate() ESE_NOTHROW
{
  m_rx.reset();
  m_tx.reset();

  MX_UART_StateSetBusy(&m_h);
  
#ifdef USE_UART_PORT1  
  if(m_h.Instance==USART1)
  {
    HAL_NVIC_SetPriority(USART1_IRQn, UART1_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    goto _ESE_END;
  } 
#endif

#ifdef USE_UART_PORT2  
  if(m_h.Instance==USART2)
  {
    HAL_NVIC_SetPriority(USART2_IRQn, UART2_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    goto _ESE_END;
  }
#endif  

#ifdef USE_UART_PORT3
  if(m_h.Instance==USART3)
  {
    HAL_NVIC_SetPriority(USART3_IRQn, UART3_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
    goto _ESE_END;
  }
#endif

_ESE_END:
  MX_UART_StateSetReady(&m_h);

  __HAL_UART_ENABLE(&m_h);

  // Initiate receiving process
  HAL_UART_Receive_IT(
    &m_h, 
    &m_rxtmp, 
    1
  );
  
  return true;
}

void EseUart::doDeactivate() ESE_NOTHROW
{
  {
    EseIsrCriticalSection cs;
    
    MX_UART_StateSetBusy(&m_h);
    __HAL_UART_DISABLE(&m_h);
    
#ifdef USE_UART_PORT1  
    if(m_h.Instance==USART1)
    {
      HAL_NVIC_DisableIRQ(USART1_IRQn);
      goto _ESE_END;
    }
#endif    
#ifdef USE_UART_PORT2
    if(m_h.Instance==USART2)
    {
      HAL_NVIC_DisableIRQ(USART2_IRQn);
      goto _ESE_END;
    }
#endif
#ifdef USE_UART_PORT3
    if(m_h.Instance==USART3)
    {
      HAL_NVIC_DisableIRQ(USART3_IRQn);
      goto _ESE_END;
    }
#endif
 
_ESE_END:
    MX_UART_StateSetReady(&m_h);
  }
  
  m_rx.reset();
  m_tx.reset();  
}

size_t EseUart::doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW
{
  esU32 tmoTicks = (tmo == rtosMaxDelay) ? 
    portMAX_DELAY :
    pdMS_TO_TICKS(tmo);

  esU32 ticksPerByte = tmoTicks/toRead;
  if( !ticksPerByte )
    ticksPerByte = 1;

  esU8* pos = data;

  esU32 prevTs = EseTask::tickCountGet();
  esU32 ticks = 0;

  while( !isBreaking(0) && toRead && ticks <= tmoTicks )
  {
    if( rtosOK == m_rx.popFront(*pos, ticksPerByte, true) )
    {
      --toRead;
      ++pos;
    }
    
    esU32 ts = EseTask::tickCountGet();
    ticks += ts-prevTs;
    prevTs = ts;
  }
 
  return pos-data;
}

void EseUart::txCheckInit() ESE_NOTHROW
{
  bool transmitting = false;
  
  {
    EseIsrCriticalSection cs;
    transmitting = MX_UART_StateIsTransmitting(&m_h);
  }
  
  if( !transmitting )
  {
    int cnt = 0;
    while( 
      cnt < txoBuffSze && 
      rtosOK == m_tx.popFront(
        m_txtmp[cnt], 
        0,
        true
      ) 
    )
      ++cnt;
      
    if( cnt )
      HAL_UART_Transmit_IT(
        &m_h, 
        m_txtmp, 
        cnt
      );
  }
}

size_t EseUart::doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW
{
  const esU8* pos = data;

  // 1) Stuff our data into TX queue until it's full, with zero waiting time
  while( toWrite && rtosOK == m_tx.pushBack(*pos, 0) )
  {
    --toWrite;
    ++pos;
  }

  if( toWrite )
  {
    // 3) Send the rest of data (if any) with specified timeout
    esU32 tmoTicks = (tmo == rtosMaxDelay) ? 
      portMAX_DELAY :
      pdMS_TO_TICKS(tmo);

    esU32 prevTs = EseTask::tickCountGet();
    esU32 ts = prevTs;

    txCheckInit();
    
    while( toWrite && rtosOK == m_tx.pushBack(*pos, tmoTicks, true) )
    {
      if( tmoTicks != portMAX_DELAY )
      {
        ts = EseTask::tickCountGet();
        esU32 delta = (ts < prevTs) ? prevTs-ts : ts-prevTs;
        
        if( tmoTicks > delta )
          tmoTicks -= delta;
        else
          tmoTicks = 1;

        prevTs = ts;
      }

      --toWrite;
      ++pos;
    }
  }
  else
    txCheckInit();
  
  return pos-data;
}

int EseUart::ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
    {
      ES_ASSERT(data);
      dcbSet(*reinterpret_cast<const UART_InitTypeDef*>(data));
    }
    break;
#if defined(STM32F3) || defined(STM32L0)
  case ctlDcbEx:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
    {
      ES_ASSERT(data);
      dcbexSet(*reinterpret_cast<const UART_AdvFeatureInitTypeDef*>(data));
    }
    break;
#endif
  case ctlIsValidRate:
    if( !isValidRate(reinterpret_cast<esU32>(data)) )
      return rtosErrorParameterValueInvalid;
    break;
  case ctlRate:
    {
      if( !isValidRate(reinterpret_cast<esU32>(data)) )
        return rtosErrorParameterValueInvalid;
      else
      {
        bool needActivate = isActive();
        if( needActivate )
          deactivate();
        m_h.Init.BaudRate = reinterpret_cast<esU32>(data);
        ES_BIT_CLR(m_flags, flagConfigured);
        if( needActivate )
          activate();
      }
    }  
    break;
  case ctlRxReset:
    HAL_UART_AbortReceive(&m_h);
    while( !m_rx.isEmpty() )
    {
      m_rx.reset();
      EseTask::sleep(5);
    }
    // Re-start receiver listener after reset
    HAL_UART_Receive_IT(
      &m_h, 
      &m_rxtmp, 
      1
    );
    break;
  case ctlTxReset:
    HAL_UART_AbortTransmit(&m_h);
    m_tx.reset();
    break;
  case ctlReset:
    HAL_UART_Abort(&m_h);
    m_tx.reset();
    while( !m_rx.isEmpty() )
    {
      m_rx.reset();
      EseTask::sleep(5);
    }
    // Re-start receiver listener after reset
    HAL_UART_Receive_IT(
      &m_h, 
      &m_rxtmp, 
      1
    );
    break;
  case ctlLineBreak:
    HAL_LIN_SendBreak(&m_h);  
    break;
  default:
    return ChannelT::ioCtlSet(ctl, data);
  }

  return rtosOK;
}

int EseUart::ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    ES_ASSERT(data);
    *reinterpret_cast<UART_InitTypeDef*>(data) = m_h.Init;
    break;
#if defined(STM32F3) || defined(STM32L0)
  case ctlDcbEx:
    ES_ASSERT(data);
    *reinterpret_cast<UART_AdvFeatureInitTypeDef*>(data) = m_h.AdvancedInit;
    break;
#endif
  case ctlRate:
    ES_ASSERT(data);
    *reinterpret_cast<esU32*>(data) = m_h.Init.BaudRate;
    break;
  case ctlIsValidRate:
    ES_ASSERT(data);
    if( reinterpret_cast<esU32>(data) > 921600 )
      return rtosErrorParameterValueInvalid;
    break;
  case ctlRxPendingGet:
    ES_ASSERT(data);
    *reinterpret_cast<size_t*>(data) = m_rx.countGet();
     break;
  case ctlTxPendingGet:
    ES_ASSERT(data);
    *reinterpret_cast<size_t*>(data) = m_tx.countGet();
    break;
  default:
    return ChannelT::ioCtlGet(ctl, data);
  }

  return rtosOK;
}

// Internal ISR services
void EseUart::onError() ESE_NOTHROW
{
  m_error = m_h.ErrorCode;

  if( 
    ES_BIT_MASK_MATCH( 
      m_h.ErrorCode, 
      HAL_UART_ERROR_PE|
      HAL_UART_ERROR_NE|
      HAL_UART_ERROR_FE|
      HAL_UART_ERROR_ORE 
    ) 
  )
  {
    // Perform a dummy reading from RDR
    m_rxtmp = __HAL_UART_FLUSH_DRREGISTER(&m_h);
  }

  // Clear all ISR bits
  __HAL_UART_CLEAR_PEFLAG(&m_h);
  __HAL_UART_CLEAR_FEFLAG(&m_h);
  __HAL_UART_CLEAR_NEFLAG(&m_h);
  __HAL_UART_CLEAR_OREFLAG(&m_h);

  __HAL_UART_FLUSH_DRREGISTER(&m_h);
  
  // Re-start receiver listener on error
  HAL_UART_Receive_IT(
    &m_h, 
    &m_rxtmp, 
    1
  );
}

void EseUart::onTxComplete() ESE_NOTHROW
{
  int cnt = 0;
  while( 
    cnt < txoBuffSze && 
    rtosOK == m_tx.popFrontFromISR(
      m_txtmp[cnt], 
      m_shouldYield
    ) 
  )
    ++cnt;
  
  if( cnt )
    HAL_UART_Transmit_IT(
      &m_h, 
      m_txtmp, 
      cnt
    );
}

void EseUart::onItemRxComplete() ESE_NOTHROW
{
  rtosStatus ok = m_rx.pushBackFromISR(
    m_rxtmp, 
    m_shouldYield
  );
  
  if( rtosOK != ok )
  {
    esU8 tmp;
    m_rx.popFrontFromISR(
      tmp, 
      m_shouldYield
    );
    
    ok = m_rx.pushBackFromISR(
      m_rxtmp, 
      m_shouldYield
    );
    
    m_error = HAL_UART_ERROR_ORE;
  }

  HAL_UART_Receive_IT(
    &m_h, 
    &m_rxtmp, 
    1
  );
}

//--------------------------------------------------------------------------
// UART IRQ handlers implementation
#ifdef USE_UART_PORT1
EseUart EseUart::_1(USART1, UART1_RX_BUFFLEN, UART1_TX_BUFFLEN);

extern "C" void USART1_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART1_IRQn);
  EseUart::_1.resetYieldFlag();
  HAL_UART_IRQHandler(&EseUart::_1.handleGet());

  portEND_SWITCHING_ISR(EseUart::_1.shouldYield() ? pdTRUE : pdFALSE);
}
#endif

#ifdef USE_UART_PORT2
EseUart EseUart::_2(USART2, UART2_RX_BUFFLEN, UART2_TX_BUFFLEN);

extern "C" void USART2_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART2_IRQn);
  EseUart::_2.resetYieldFlag();
  HAL_UART_IRQHandler(&EseUart::_2.handleGet());

  portEND_SWITCHING_ISR(EseUart::_2.shouldYield() ? pdTRUE : pdFALSE);
}
#endif

#ifdef USE_UART_PORT3
EseUart EseUart::_3(USART3, UART3_RX_BUFFLEN, UART3_TX_BUFFLEN);

extern "C" void USART3_IRQHandler(void)
{
  HAL_NVIC_ClearPendingIRQ(USART3_IRQn);
  EseUart::_3.resetYieldFlag();
  HAL_UART_IRQHandler(&EseUart::_3.handleGet());
  
  portEND_SWITCHING_ISR(EseUart::_3.shouldYield() ? pdTRUE : pdFALSE);
}
#endif

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
#ifdef USE_UART_PORT1
  if(huart->Instance==USART1)
  {
    EseUart::_1.onError();
    return;
  }
#endif
#ifdef USE_UART_PORT2
  if(huart->Instance==USART2)
  {
    EseUart::_2.onError();
    return;
  }
#endif
#ifdef USE_UART_PORT3
  if(huart->Instance==USART3)
  {
    EseUart::_3.onError();
    return;
  }
#endif
}


extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef USE_UART_PORT1
  if(huart->Instance==USART1)
  {
    EseUart::_1.onTxComplete();
    return;
  }
#endif
#ifdef USE_UART_PORT2
  if(huart->Instance==USART2)
  {
    EseUart::_2.onTxComplete();
    return;
  }
#endif
#ifdef USE_UART_PORT3
  if(huart->Instance==USART3)
  {
    EseUart::_3.onTxComplete();
    return;
  }
#endif
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
#ifdef USE_UART_PORT1
  if(huart->Instance==USART1)
  {
    EseUart::_1.onItemRxComplete();
    return;
  }
#endif
#ifdef USE_UART_PORT2
  if(huart->Instance==USART2)
  {
    EseUart::_2.onItemRxComplete();
    return;
  }
#endif
#ifdef USE_UART_PORT3
  if(huart->Instance==USART3)
  {
    EseUart::_3.onItemRxComplete();
    return;
  }
#endif
}
