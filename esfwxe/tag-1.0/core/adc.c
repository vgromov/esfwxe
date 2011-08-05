#include <target.h>

#ifdef USE_FREE_RTOS
	#include <FreeRTOS.h>
	#include <semphr.h>
#endif

#ifdef USE_RTX_RTOS
	#include <rtl.h>
#endif

#pragma hdrstop

#include "adc.h"

// adc driver implementation
//

7:0 SEL Selects which of the AD0.7:0 pins is (are) to be sampled and converted. For AD0, bit 0
selects Pin AD0.0, and bit 7 selects pin AD0.7. In software-controlled mode, only one of
these bits should be 1. In hardware scan mode, any value containing 1 to 8 ones. All
zeroes is equivalent to 0x01.
0x01
15:8 CLKDIV The APB clock (PCLK) is divided by (this value plus one) to produce the clock for the A/D
converter, which should be less than or equal to 4.5 MHz. Typically, software should
program the smallest value in this field that yields a clock of 4.5 MHz or slightly less, but in
certain cases (such as a high-impedance analog source) a slower clock may be
desirable.
0
16 BURST 0 Conversions are software controlled and require 11 clocks. 0
1 The AD converter does repeated conversions at the rate selected by the CLKS field,
scanning (if necessary) through the pins selected by 1s in the SEL field. The first
conversion after the start corresponds to the least-significant 1 in the SEL field, then
higher numbered 1 bits (pins) if applicable. Repeated conversions can be terminated by
clearing this bit, but the conversion that’s in progress when this bit is cleared will be
completed.
Important: START bits must be 000 when BURST = 1 or conversions will not start.
19:17 CLKS
000
This field selects the number of clocks used for each conversion in Burst mode, and the
number of bits of accuracy of the result in the LS bits of ADDR, between 11 clocks
(10 bits) and 4 clocks (3 bits).
11 clocks / 10 bits
000
001 10 clocks / 9 bits
010 9 clocks / 8 bits
011 8 clocks / 7 bits
100 7 clocks / 6 bits
101 6 clocks / 5 bits
110 5 clocks / 4 bits
111 4 clocks / 3 bits
20 Reserved, user software should not write ones to reserved bits. The value read from a
reserved bit is not defined.
NA
21 PDN 1 The A/D converter is operational. 0
0 The A/D converter is in power-down mode.
23:22 - Reserved, user software should not write ones to reserved bits. The value read from a
reserved bit is not defined.
NA
26:24 START
000
When the BURST bit is 0, these bits control whether and when an A/D conversion is
started:
No start (this value should be used when clearing PDN to 0).
0
001 Start conversion now.
010 Start conversion when the edge selected by bit 27 occurs on P2.10/EINT0.
011 Start conversion when the edge selected by bit 27 occurs on P1.27/CAP0.1.
100 Start conversion when the edge selected by bit 27 occurs on MAT0.1.
101 Start conversion when the edge selected by bit 27 occurs on MAT0.3.
110 Start conversion when the edge selected by bit 27 occurs on MAT1.0.
111 Start conversion when the edge selected by bit 27 occurs on MAT1.1.
27 EDGE
1
This bit is significant only when the START field contains 010-111. In these cases:
Start conversion on a falling edge on the selected CAP/MAT signal.
0
0 Start conversion on a rising edge on the selected CAP/MAT signal.
// control data
//
#ifdef USE_FREE_RTOS
static xSemaphoreHandle s_mutex;
static xSemaphoreHandle s_completion;
#endif // USE_FREE_RTOS

// ADC result buffer
static WORD s_data[ADC_CHANNEL_COUNT];

// interlocked access to the shared resource
BOOL adcLock(DWORD timeout)
{
	if( s_mutex )
		return xSemaphoreTake(s_mutex, timeout) == pdTRUE;
	
	return FALSE;
}

void adcUnlock(void)
{
	if( s_mutex )
		xSemaphoreGive(s_mutex);
}

// power control
void adcPowerUp(void);
void adcPowerDown(void);

// adc configuration and control
//
// freq is desired adc convertion rate in Hz
void adcInit(DWORD freq, BYTE chnlMask)
{
#ifdef USE_FREE_RTOS
	if( !s_mutex )
		s_mutex = xSemaphoreCreateMutex();

	if( !s_completion )
		vSemaphoreCreateBinary( s_completion );
#endif
	adcStop();
	adcPowerUp();

	
}

void adcStart(BYTE startCondition)
{
	adcStop();
}

void adcStartBurst(BYTE accuracy)
{
	adcStop();

}

void adcStop(void)
{

}

// effective wait for tasks needed to wait for 
// conversion to end. uses not polling, but
// binary semaphore for optimal MCU usage 
BOOL adcWaitForDone(DWORD timeout)
{
	BOOL result = FALSE;

	if( s_completion )
	{
		result = xSemaphoreTake(s_completion, timeout) == pdTRUE;
		if( result )
			xSemaphoreGive(s_completion);
	}

	return result;
}

// access conversion results & status
DWORD adcGetStatus(void);
WORD adcGetResult(BYTE channel);

