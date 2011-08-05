#ifndef _target_config_h_
#define _target_config_h_

//#define _DEBUG

// target configuration file for appliances-phototest project
//

#if defined(_DEBUG) || defined(DEBUG)
#	ifndef DEBUG
#		define DEBUG
#	endif
#	ifndef _DEBUG
#		define _DEBUG
#	endif
#endif 

#if defined(_DEBUG) || defined(DEBUG)
// use jtag debug (no WDT)
// remove in production code
#	define JTAG_DEBUG
#	undef  NDEBUG
// use trace facility
#	define USE_TRACE
#elif !defined(DEBUG) && !defined(_DEBUG)
#	ifndef NDEBUG
#		define NDEBUG
#	endif
// define USE_CODE_READ_PROTECTION to produce final code not-readable from flash
#	define USE_CODE_READ_PROTECTION
#endif

// frequency configuration data
//
// external Xtal frequency
#define Fosc						14745600	

// use flash memory acceleration module		
#define USE_MAM		

// valid PLL frequency range
#define MinFcco					156000000
#define MaxFcco					320000000			

// PLL divisors
#define PLL_M						4
#define PLL_P						2 // 1, 2, 4, or 8 is valid

// MCU frequency
#define Fmcu						((Fosc)*(PLL_M))
// PLL frequency
#define Fcco						((PLL_P)*((Fmcu)*2))	
// check PLL frequency
#if( Fcco < MinFcco || Fcco > MaxFcco )
	#error "Fcco is out of valid PLL frequency range"
#endif			

// periferial frequency
#define PERIPH_DIV			0 // 0 - 1/4; 1 - Fmcu; 2 - 1/2 
#if(PERIPH_DIV == 0)
	#define Fpclk					((Fmcu)/4)
#elif(PERIPH_DIV == 1)
	#define Fpclk					(Fmcu)
#elif(PERIPH_DIV == 2)
	#define Fpclk					((Fmcu)/2)
#endif

// project-specific defines
//
// use communication channel abstraction
#define USE_COMMINTF

// use custom implementations of 
// void usDelay(DWORD useconds) and
// void nsDelay(DWORD nseconds)
//
#define USE_CUSTOM_DELAY

#endif
