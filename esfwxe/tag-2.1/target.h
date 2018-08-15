#ifndef __TARGET_H 
#define __TARGET_H

#if defined(DEBUG) || defined(_DEBUG)
# ifndef ES_DEBUG
#   define ES_DEBUG
# endif
#endif

#include <targetConfig.h>

#if (!defined(USE_RTOS) || 0 == USE_RTOS) && (defined(USE_FREE_RTOS) || defined(USE_RTX_RTOS) || defined(RTX))
# undef USE_RTOS
# define USE_RTOS 1
#else
# undef USE_RTOS
# define USE_RTOS 0
#endif

#if defined(USE_FULL_ASSERT) && !defined(NDEBUG) && !defined(ES_ASSERT)
# include <assert.h>
# define ES_ASSERT(x) assert(x)
#endif

// common type definitions
#include <esfwxe/type.h>

// exclude this stuff if new Keil5 RTE config is used
#ifndef USE_KEIL_RTE

#ifndef LPC23XX
# define LPC23XX 0
#endif

#ifndef LPC214X
# define LPC214X 0
#endif

#ifndef LPC2103
# define LPC2103 0
#endif 

#ifndef LPC11XX
# define LPC11XX 0
#endif 

#ifndef LPC13XX
# define LPC13XX 0
#endif

// include MCU stuff
#if LPC23XX == 1
    #include <esfwxe/lpc23xx/lpc23xx.h>
#endif

#if LPC214X == 1
    #include <esfwxe/lpc214x/lpc214x.h>
#endif

#if LPC2103 == 1
    #include <esfwxe/lpc2103/lpc2103.h>
#endif 

#if LPC11XX == 1
    extern volatile esU32 SystemCoreClock;
    #include <esfwxe/lpc11xx/lpc11xx.h>
#endif 

#if LPC13XX == 1
    extern volatile esU32 SystemCoreClock;
    #include <esfwxe/lpc13xx/lpc13xx.h>
#endif 

#ifdef __cplusplus
   extern "C" {
#endif

void configurePLL(void);
void powerResetInit(void);
void gpioResetInit( void );

// Get Core Clock Frequency
esU32 systemCoreFrequencyGet(void);

// burnTmo is timeout needed to erase-burn firmware, before device 
// is programmatically reset with watchdog
void runBootLoader( esU32 burnTmo );

// this should be called as soon as possible after the main 
// program entry point is reached
void targetResetInit(void);

#ifdef __cplusplus
   }
#endif
 
#endif // USE_KEIL_RTE
   
#endif /* end __TARGET_H */
/******************************************************************************
**                            End Of File
******************************************************************************/
