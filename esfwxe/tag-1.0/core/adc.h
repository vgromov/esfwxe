#ifndef _adc_h_
#define _adc_h_

#include "adcConfig.h"

#ifdef __cplusplus
	extern "C" { 
#endif

// adc driver header
//

// interlocked access to the shared resource
BOOL adcLock(DWORD timeout);
void adcUnlock(void);

// power control
void adcPowerUp(void);
void adcPowerDown(void);

// adc configuration and control
//
// freq is desired adc convertion rate in Hz
void adcInit(DWORD freq, BYTE chnlMask);
void adcStart(BYTE startCondition);
void adcStartBurst(BYTE accuracy);
void adcStop(void);

// effective wait for tasks needed to wait for 
// conversion to end. uses not polling, but
// binary semaphore for optimal MCU usage 
BOOL adcWaitForDone(DWORD timeout);

// access conversion results & status
DWORD adcGetStatus(void);
WORD adcGetResult(BYTE channel);

#ifdef __cplusplus
	}
#endif

#endif
