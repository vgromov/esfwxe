#ifndef _adc_h_
#define _adc_h_

#include "adcConfig.h"

#ifdef __cplusplus
  extern "C" { 
#endif

// adc driver header
//

// interlocked access to the shared resource
esBL adcLock(esU32 timeout);
void adcUnlock(void);

// power control
void adcPowerUp(void);
void adcPowerDown(void);

// adc configuration and control
//
// freq is desired adc convertion rate in Hz
void adcInit(esU32 freq, esU8 chnlMask);
void adcStart(esU8 startCondition);
void adcStartBurst(esU8 accuracy);
void adcStop(void);

// effective wait for tasks needed to wait for 
// conversion to end. uses not polling, but
// binary semaphore for optimal MCU usage 
esBL adcWaitForDone(esU32 timeout);

// access conversion results & status
esU32 adcGetStatus(void);
esU16 adcGetResult(esU8 channel);

#ifdef __cplusplus
  }
#endif

#endif
