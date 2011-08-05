#ifndef _timer_Config_h_
#define _timer_Config_h_

#define USE_TIMER1

typedef enum
{
#ifdef USE_TIMER0
	tmr0,
#endif

#ifdef USE_TIMER1
	tmr1,
#endif

#ifdef USE_TIMER2
	tmr2,
#endif

#ifdef USE_TIMER3
	tmr3,
#endif

	tmrCount

} tmrUnit;

#endif // _timer_Config_h_
