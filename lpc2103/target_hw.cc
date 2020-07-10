#ifndef _target_hw_c_
#define _target_hw_c_

#include "esfwxe/utils.h"

// Target C file for NXP LPC2101-03 Family Microprocessors

void powerResetInit(void)
{
  // we have to clear PDN bit in ADCR before disabling ADC power
  ADCR &= ~0x0200000;

  // disable all peripheral power
  PCONP = 0;
}

/******************************************************************************
** Function name:    GPIOResetInit
**
** Descriptions:    Initialize the target board before running the main() 
**        function; User may change it as needed, but may not 
**        deleted it.
**
** parameters:      None
** Returned value:    None
** 
******************************************************************************/
void gpioResetInit( void )
{
  /* Reset all GPIO pins to default: primary function */
  PINSEL0 = 0x00000000;
#ifndef SWD_JTAG_DEBUG
  PINSEL1 = 0x00000000;
#else
  PINSEL1 = 0x55400000;
#endif

  // FIO reset
  SCS    |= 0x00000001;  // high speed is enabled on GPIO0
    
  FIODIR = 0x00000000;
  FIOCLR = 0xFFFFFFFF;
}

//******************************************************************************
//** Function name:    ConfigurePLL
//**
//** Descriptions:    Configure PLL switching to main OSC instead of IRC
//**            at power up and wake up from power down. 

#define PLL_LOCK_BIT       0x0400
#define PLL_CONNECT_BIT    0x0200
#define PLL_STAT_MP_MASK  0x003F

// get value which should be written to PLLCFG
#define  MAKE_PLLCFG(pllM, pllP) \
  (((pllM)-1) | (((pllP) < 8 ? ((pllP) >> 1) : (((pllP)-1) >> 1)) << 5))

#define PLL_FEED  PLLFEED = 0xaa; PLLFEED = 0x55

void configurePLL( void )
{
  PLLCON = 0;        /* Disable PLL, disconnected */
  PLL_FEED;
    
  PLLCFG = MAKE_PLLCFG(PLL_M, PLL_P);  
  PLL_FEED;
      
  PLLCON = 1;        /* Enable PLL, disconnected */
  PLL_FEED;
  while( !(PLLSTAT & PLL_LOCK_BIT) );

  PLLCON = 3;        /* enable and connect */
  PLL_FEED;
  while( !(PLLSTAT & PLL_CONNECT_BIT) );

  APBDIV = PERIPH_DIV;
}

void targetResetInit(void)
{
  // initialize MCU power 
  powerResetInit();
  gpioResetInit();

#ifdef __DEBUG_RAM    
  MEMMAP = 0x2;      /* remap to internal RAM */
#endif

#ifdef __DEBUG_FLASH    
  MEMMAP = 0x1;      /* remap to internal flash */
#endif

  /* Configure PLL, switch from IRC to Main OSC */
  configurePLL();

  /* Set memory accelerater module*/
  MAMCR = 0;  // disable MAM

#ifdef USE_MAM

#if Fmcu < 20000000
  MAMTIM = 1;
#elif Fmcu <= 40000000
  MAMTIM = 2;
#elif Fmcu <= 60000000
  MAMTIM = 3;
#else
  MAMTIM = 4;
#endif

  MAMCR = 2;  // fully enable MAM

#endif // USE_MAM

  vicInit();
}

// burnTmo is timeout needed to erase-burn firmware, before device 
// is programmatically reset with watchdog
void runBootLoader( esU32 burnTmo )
{
  void (*bootloaderEntryPoint)(void) = (void*)0;
  // emulate MCU reset state
  //
  // reset all interrupts
  VICIntEnClr = 0xFFFFFFFF;
  // reset system control to default (use legacy IO)
  SCS = 0;
  // connect TXD0, RXD0 as GPIO, configure RXD0 as input
  ES_BIT_CLR(PINSEL0, 0x0000000C);
  ES_BIT_CLR(IODIR, 1);
  // configure P0.14 as output and clear it
  ES_BIT_SET(IODIR, 0x00004000);
  IOCLR = 0x00004000;
  // power up timer 1 (used in autobaud) and uart 0
  ES_BIT_SET(PCONP, PCTIM1|PCUART0);
  // disconnect PLLs
  PLLCON = 0;
  PLL_FEED;
  // set peripheral bus to 1/4th of the system clock */
  APBDIV = 0;
  // restore U0FDR register to default
  U0FDR = 0x01;
  // reset timer1 to defaults
  T1PR = 0;
  T1MCR = 0;
  T1CCR = 0;
  // program watchdog in accordance to the requested burnTmo
  // NB! MCU is running at the xtal frequency now, and WDT is ticking at 1/4 of it (APBDIV)
  // burnTmo is expected to be in ms
   WDTC = burnTmo * (Fosc / 16000);
  WDMOD = WDEN | WDRESET; // enable watchdog, reset MCU on underflow
  // feed and disable watchdog
  WDFEED = 0xAA;
  WDFEED = 0x55;
  // map bootloader vectors
  MEMMAP = 0;
  // go to bootloader entry point
  bootloaderEntryPoint();
}

#endif // _target_hw_c_
