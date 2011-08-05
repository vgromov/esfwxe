#ifndef _target_config_h_
#define _target_config_h_

// target configuration file for boi-01 project
//

// use FreeRTOS
#define USE_FREE_RTOS
#define USE_USB

#ifdef _DEBUG
# ifndef DEBUG
# 	define DEBUG
#	endif
#endif 
#ifdef DEBUG
# ifndef _DEBUG
# 	define _DEBUG
#	endif
#endif 

#if defined(_DEBUG) || defined(DEBUG)
// use jtag debug (no WDT)
// remove in production code
#	define JTAG_DEBUG
#	undef  NDEBUG
// configure trace facility
//#	define USE_TRACE
//#		define USE_MEM_TRACE
//#		define USE_EVENT_TRACE
//#		define USE_ESGUI_WIDGET_TRACE
//#		define USE_EKOSF_PROTOCOL_TRACE
//#		define USE_STACK_TRACE
#elif !defined(_DEBUG) && !defined(DEBUG) 
#	ifndef NDEBUG
#		define NDEBUD
#	endif
// define USE_CODE_READ_PROTECTION to produce final code not-readable from flash
#	define USE_CODE_READ_PROTECTION
#endif


// frequency configuration data
//
// external Xtal frequency
#define Fosc						12000000	

// use flash memory acceleration module		
#define USE_MAM

// valid PLL frequency range
#define MinFcco					156000000
#define MaxFcco					320000000			

// PLL0 divisors
#define PLL0_M					5
#define PLL0_P					2	// 1, 2, 4, or 8 is valid

// MCU frequency
#define Fmcu						((Fosc)*(PLL0_M))
// PLL0 frequency
#define F0cco						((PLL0_P)*((Fmcu)*2))	
// check PLL0 frequency
#if( F0cco < MinFcco || F0cco > MaxFcco )
	#error "F0cco is out of valid PLL frequency range"
#endif			

#ifdef USE_USB
// PLL1 divisors
#	define PLL1_M					(48000000 / (Fosc))
#	define PLL1_P					2	// 1, 2, 4, or 8 is valid	
#	define F1cco					((PLL1_P)*((Fmcu)*2))	
// check PLL1 frequency
#	if( F1cco < MinFcco || F1cco > MaxFcco )
#		error "F1cco is out of valid PLL frequency range"
#	endif	
#endif

// periferial frequency
#define PERIPH_DIV			1 // 0 - 1/4; 1 - Fmcu; 2 - 1/2 
#if(PERIPH_DIV == 0)
	#define Fpclk					((Fmcu)/4)
#elif(PERIPH_DIV == 1)
	#define Fpclk					(Fmcu)
#elif(PERIPH_DIV == 2)
	#define Fpclk					((Fmcu)/2)
#endif

// use effective delay, based on RTOS wait function
#define USE_CUSTOM_DELAY
// use communication interface abstraction
#define USE_COMMINTF

#endif
