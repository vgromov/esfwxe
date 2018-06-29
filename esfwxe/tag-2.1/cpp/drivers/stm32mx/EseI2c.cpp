#include <esfwxe/target.h>
#include <esfwxe/type.h>
#include <esfwxe/trace.h>
#include <esfwxe/utils.h>

// FreeRTOS
#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

#include <esfwxe/cpp/concept/EseChannel.h>
#include <esfwxe/cpp/os/EseMutex.h>
#include <esfwxe/cpp/os/EseSemaphore.h>
#include <esfwxe/cpp/os/EseKernel.h>

//----------------------------------------------------------------------
#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>
#include <esfwxe/cpp/drivers/stm32mx/EseI2c.h>

// I2C Hardware initialization|uninitialization parts - must be defined elsewhere
extern "C" void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c);
extern "C" void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c);
//----------------------------------------------------------------------

// rough max estimate of ms per 2 bytes at specified baud
#pragma Otime
static inline esU32 i2cGetMaxByteTimeoutMs( esU32 len, esU32 baud, esU32 dataBits ) ESE_NOTHROW
{
	esU32 result = (len * 2000 * (dataBits + 2)) / baud;
	return result ? result : 1;
}

//----------------------------------------------------------------------
EseI2c::EseI2c(I2C_TypeDef* hw) ESE_NOTHROW :
ChannelT( 0, 0 ),
m_sem(1, 0),
m_devAddr(0),
m_memAddr(0),
m_memAddrSize(0),
m_shouldYieldEv(false),
m_shouldYieldErr(false)
{
  m_h.Instance = hw;
  m_h.State = HAL_I2C_STATE_RESET;
  m_h.Init.ClockSpeed = 100000;
  m_h.Init.DutyCycle = I2C_DUTYCYCLE_2;
  m_h.Init.OwnAddress1 = 0;
  m_h.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  m_h.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  m_h.Init.OwnAddress2 = 0;
  m_h.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  m_h.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;  
}

EseI2c::~EseI2c() ESE_NOTHROW
{
  uninit();
}

#pragma Otime
bool EseI2c::isOk() const ESE_NOTHROW
{
  return m_mx.isOk();
}

#pragma Otime
rtosStatus EseI2c::lock(esU32 tmo) ESE_NOTHROW
{
  return m_mx.lock(tmo);
}

#pragma Otime
rtosStatus EseI2c::unlock() ESE_NOTHROW
{
  return m_mx.unlock();
}

void EseI2c::dcbSet(const I2C_InitTypeDef& dcb) ESE_NOTHROW
{
  m_h.Init = dcb;
  ES_BIT_CLR(m_flags, flagConfigured);
}

/// Channel implementation
bool EseI2c::doInit() ESE_NOTHROW
{
  m_mx.init();
  m_sem.init();

  m_h.State = HAL_I2C_STATE_RESET;
  HAL_StatusTypeDef stat = HAL_I2C_Init(&m_h);
  
#ifndef STM32F1
  
  if( HAL_OK == stat ) 
    HAL_I2C_AnalogFilter_Config(&m_h, I2C_ANALOGFILTER_ENABLED);
    
#endif

  return HAL_OK == stat;
}

void EseI2c::doUninit() ESE_NOTHROW
{
  {
    EseIsrCriticalSection cs;
    HAL_I2C_DeInit(&m_h);
  }

  m_sem.uninit();
  m_mx.uninit();
}

bool EseI2c::doCheckConfigured() ESE_NOTHROW
{
  HAL_StatusTypeDef stat;
  
  EseIsrCriticalSection cs;
  stat = HAL_I2C_Init(&m_h);
  // TODO: Analog or digital filter should be set-up as DCBex
  __HAL_I2C_DISABLE(&m_h);

  return HAL_OK == stat;
}

#pragma Otime
esU32 EseI2c::dataXferTimeoutEstimateGet(size_t len) const ESE_NOTHROW
{
	return i2cGetMaxByteTimeoutMs(
    len, 
    100000, ///< Consider we got slow 100 kHz bus 
    8       ///< Consider we got 8 bit word
  );
}

bool EseI2c::doActivate() ESE_NOTHROW
{
#ifdef USE_I2C_PORT1
  if(m_h.Instance==I2C1)
  {
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, I2C1_ERR_IRQ_PRIORITY, 0);
    HAL_NVIC_SetPriority(I2C1_EV_IRQn, I2C1_EVT_IRQ_PRIORITY, 0);    
    
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    
    goto _ESE_END;
  } 
#endif

#ifdef USE_I2C_PORT2
  if(m_h.Instance==I2C2)
  {
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, I2C2_ERR_IRQ_PRIORITY, 0);
    HAL_NVIC_SetPriority(I2C2_EV_IRQn, I2C2_EVT_IRQ_PRIORITY, 0);
    
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
    
    goto _ESE_END;
  }
#endif

_ESE_END:
  __HAL_I2C_ENABLE(&m_h);

  return true;
}

void EseI2c::doDeactivate() ESE_NOTHROW
{
  __HAL_I2C_DISABLE(&m_h);

#ifdef USE_I2C_PORT1
  if(m_h.Instance==I2C1)
  {
    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);

    goto _ESE_END;
  }
#endif  
  
#ifdef USE_I2C_PORT2
  if(m_h.Instance==I2C2)
  {
    HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    
    goto _ESE_END;
  }
#endif
  
_ESE_END:
  return; //< Pacify compilers
}

#pragma Otime
size_t EseI2c::masterReceive( esU16 devAddr, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW 
{
  m_devAddr = devAddr;
  m_memAddrSize = 0;

  return receive(data, toRead, tmo);
}

#pragma Otime
size_t EseI2c::masterSend( esU16 devAddr, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW
{
  m_devAddr = devAddr;
  m_memAddrSize = 0;
  
  return send(data, toWrite, tmo);
}

#pragma Otime
size_t EseI2c::masterMemReceive( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, esU8* data, size_t toRead, esU32 tmo ) ESE_NOTHROW 
{
  m_devAddr = devAddr;
  m_memAddr = memAddr;
  m_memAddrSize = memAddrSize;

  return receive(data, toRead, tmo);
}

#pragma Otime
size_t EseI2c::masterMemSend( esU16 devAddr, esU16 memAddr, esU16 memAddrSize, const esU8* data, size_t toWrite, esU32 tmo ) ESE_NOTHROW
{
  m_devAddr = devAddr;
  m_memAddr = memAddr;
  m_memAddrSize = memAddrSize;

  return send(data, toWrite, tmo);
}

#pragma Otime
size_t EseI2c::doReceive(esU8* data, size_t toRead, esU32 tmo) ESE_NOTHROW
{
  // Try taking semaphore, just in case, to decrement its count to 0
  rtosStatus rstat = m_sem.lock(0);
  ES_ASSERT(rtosErrorTimeout == rstat);
  ES_ASSERT(HAL_I2C_STATE_READY == m_h.State);

  HAL_StatusTypeDef stat;
  if( isMaster() ) ///< Master branch
  {
    if( isMemAccessMode() ) ///< Standard call, otherwise, memory access call
      stat = HAL_I2C_Mem_Read_IT(&m_h, m_devAddr, m_memAddr, m_memAddrSize, data, toRead);
    else
      stat = HAL_I2C_Master_Receive_IT(&m_h, m_devAddr, data, toRead);
  }
  else ///< Slave branch otherwise
  {
    stat = HAL_I2C_Slave_Receive_IT(&m_h, data, toRead);
  }
  
  if( HAL_OK == stat )
  {
    // Block on semaphore until tmo is expired
    if( rtosErrorTimeout == m_sem.lock(tmo) )
      return 0;
  }
  else
    onError();

  return toRead - static_cast<size_t>(m_h.XferCount);
}

#pragma Otime
size_t EseI2c::doSend(const esU8* data, size_t toWrite, esU32 tmo) ESE_NOTHROW
{
  // Try taking semaphore, just in case, to decrement its count to 0
  rtosStatus rstat = m_sem.lock(0);
  ES_ASSERT(rtosErrorTimeout == rstat);
  ES_ASSERT(HAL_I2C_STATE_READY == m_h.State);
    
  HAL_StatusTypeDef stat;
  if( isMaster() ) ///< Master branch
  {
    if( isMemAccessMode() ) ///< Standard call, otherwise, memory access call
      stat = HAL_I2C_Mem_Write_IT(&m_h, m_devAddr, m_memAddr, m_memAddrSize, const_cast<esU8*>(data), toWrite);
    else
      stat = HAL_I2C_Master_Transmit_IT(&m_h, m_devAddr, const_cast<esU8*>(data), toWrite);
  }
  else ///< Slave branch otherwise
  {
    stat = HAL_I2C_Slave_Transmit_IT(&m_h, const_cast<esU8*>(data), toWrite);
  }
  
  if( HAL_OK == stat )
  {
    // Block on semaphore until tmo is expired
    if( rtosErrorTimeout == m_sem.lock(tmo) )
      return 0;
  }
  else
    onError();

  return toWrite - static_cast<size_t>(m_h.XferCount);
}

int EseI2c::ioCtlSet(esU32 ctl, void* data) ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    if( isActive() )
      return rtosErrorParameterNotSupported;
    else
    {
      dcbSet(*reinterpret_cast<const I2C_InitTypeDef*>(data));
      break;
    }
  case ctlDevAddr:
    m_devAddr = reinterpret_cast<size_t>(data);
    break;
  case ctlMemAddr:
    m_memAddr = reinterpret_cast<size_t>(data);
    break;
  case ctlMemAddrSize:
    m_memAddrSize = reinterpret_cast<size_t>(data);
    break;
  case ctlRxReset:
  case ctlTxReset:
  case ctlReset:
    return rtosErrorParameterNotSupported;
  default:
    return ChannelT::ioCtlSet(ctl, data);
  }

  return rtosOK;
}

int EseI2c::ioCtlGet(esU32 ctl, void* data) const ESE_NOTHROW
{
  switch(ctl)
  {
  case ctlDcb:
    *reinterpret_cast<I2C_InitTypeDef*>(data) = m_h.Init;
    break;
  case ctlDevAddr:
    *reinterpret_cast<size_t*>(data) = m_devAddr;
    break;
  case ctlMemAddr:
    *reinterpret_cast<size_t*>(data) = m_memAddr;
    break;
  case ctlMemAddrSize:
    *reinterpret_cast<size_t*>(data) = m_memAddrSize;
    break;
  case ctlRxPendingGet:
  case ctlTxPendingGet:
    return rtosErrorParameterNotSupported;
  default:
    return ChannelT::ioCtlGet(ctl, data);
  }

  return rtosOK;
}

#pragma Otime
void EseI2c::onError() ESE_NOTHROW
{
  m_error = m_h.ErrorCode;
}

#pragma Otime
void EseI2c::onTxComplete() ESE_NOTHROW
{
  m_sem.unlockFromISR(m_shouldYieldEv);
}

#pragma Otime
void EseI2c::onRxComplete() ESE_NOTHROW
{
  m_sem.unlockFromISR(m_shouldYieldEv);
}

//--------------------------------------------------------------------------------------
// Low-Level ISR handlers 
//
// I2C Interrupt handlers, adapted to RTOS usage
//
#ifdef USE_I2C_PORT1
EseI2c EseI2c::_1(I2C1);

extern "C" 
{

#pragma Otime
void I2C1_EV_IRQHandler(void) ESE_NOTHROW
{
  HAL_NVIC_ClearPendingIRQ(I2C1_EV_IRQn);
  EseI2c::_1.resetYieldFlagEv();
  HAL_I2C_EV_IRQHandler(&EseI2c::_1.handleGet());
  
  portEND_SWITCHING_ISR(EseI2c::_1.shouldYieldEv() ? pdTRUE : pdFALSE);
}

#pragma Otime
void I2C1_ER_IRQHandler(void) ESE_NOTHROW
{
  HAL_NVIC_ClearPendingIRQ(I2C1_ER_IRQn);
  EseI2c::_1.resetYieldFlagErr();
  HAL_I2C_ER_IRQHandler(&EseI2c::_1.handleGet());
  
  portEND_SWITCHING_ISR(EseI2c::_1.shouldYieldErr() ? pdTRUE : pdFALSE);
}

} // extern "C"
#endif

#ifdef USE_I2C_PORT2
EseI2c EseI2c::_2(I2C2);

extern "C" 
{

#pragma Otime
void I2C2_EV_IRQHandler(void) ESE_NOTHROW
{
  HAL_NVIC_ClearPendingIRQ(I2C2_EV_IRQn);
  EseI2c::_2.resetYieldFlagEv();
  HAL_I2C_EV_IRQHandler(&EseI2c::_2.handleGet());
  
  portEND_SWITCHING_ISR(EseI2c::_2.shouldYieldEv() ? pdTRUE : pdFALSE);
}

#pragma Otime
void I2C2_ER_IRQHandler(void) ESE_NOTHROW
{
  HAL_NVIC_ClearPendingIRQ(I2C2_ER_IRQn);
  EseI2c::_2.resetYieldFlagErr();
  HAL_I2C_ER_IRQHandler(&EseI2c::_2.handleGet());
  
  portEND_SWITCHING_ISR(EseI2c::_2.shouldYieldErr() ? pdTRUE : pdFALSE);
}

} // extern "C"
#endif

extern "C" 
{
// Driver interrupt callbacks
//
#pragma Otime
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onTxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onTxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onRxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onRxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{  
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onTxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onTxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{  
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onRxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onRxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onTxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onTxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onRxComplete();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onRxComplete();
    return;
  }
#endif
}

#pragma Otime
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
#ifdef USE_I2C_PORT1
  if( I2C1 == hi2c->Instance )
  {
    EseI2c::_1.onError();
    return;
  }
#endif
  
#ifdef USE_I2C_PORT2
  if( I2C2 == hi2c->Instance )
  {
    EseI2c::_2.onError();
    return;
  }
#endif
}

} // extern "C"
