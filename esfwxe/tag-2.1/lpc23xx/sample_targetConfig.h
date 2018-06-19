#ifndef _target_config_h_
#define _target_config_h_

// target configuration file for boi-01 project
//

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
#	define USE_TRACE
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
#define MinFcco					275000000
#define MaxFcco					500000000			

// PLL0 divisors
#define PLL_M						20
#define PLL_N						1	// 1, 2, 4, or 8 is valid
// MCU clock divisor
#define MCU_CLK_DIV			8	// 1, 2, 4 .. 254 are valid
// PLL frequency
#define Fcco						(((PLL_M)*((Fosc)*2))/(PLL_N))
// MCU frequency
#define Fmcu						((Fcco)/(MCU_CLK_DIV))
// check PLL frequency
#if( Fcco < MinFcco || Fcco > MaxFcco )
	#error "Fcco is out of valid PLL frequency range"
#endif			

// periferial frequency divisor
// 0 - 1/4; 1 - Fmcu; 2 - 1/2; 3 - 1/8 (1/6 for CAN)
#define PERIPH_DIV_6_8			3
#define PERIPH_DIV_4				0
#define PERIPH_DIV_2				2
#define PERIPH_DIV_1				1 
#define PERIPH_DIV(val)			(((val) == PERIPH_DIV_4) ? 4 : ((val) == PERIPH_DIV_6_8 ? 8 : (val)))
#define PERIPH_DIV_CAN(val)	(((val) == PERIPH_DIV_4) ? 4 : ((val) == PERIPH_DIV_6_8 ? 6 : (val)))

// use effective delay
//
#define USE_CUSTOM_DELAY

#endif
