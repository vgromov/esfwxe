#ifndef _target_hw_c_
#define _target_hw_c_

#include "esfwxe/utils.h"
// Target C file for NXP LPC21xx Family Microprocessors

void powerResetInit(void)
{
    // we have to clear PDN bit in AD0CR before disabling ADC power
    ES_BIT_CLR(AD0CR, 0x0200000);
    ES_BIT_CLR(AD1CR, ~0x0200000);

  // disable all peripheral power
    PCONP = 0;

     // enable USB power if needed
#ifdef USE_USB
    ES_BIT_SET(PCONP, PCUSB); 
#endif
}

/******************************************************************************
** Function name:        gpioResetInit
**
** Descriptions:        Initialize the target board before running the main() 
**                function; User may change it as needed, but may not 
**                deleted it.
**
** parameters:            None
** Returned value:        None
** 
******************************************************************************/
__weak void gpioResetInit( void )
{
  /* Reset all GPIO pins to default: primary function */
  PINSEL0 = 0x00000000;
  PINSEL1 = 0x00000000;
#ifndef SWD_JTAG_DEBUG
  PINSEL2 = 0x00000000;
#else
    PINSEL2 = 0x00000004;
#endif
    
    // FIO reset
    SCS      |= 0x00000003;    // high speed is enabled on GPIO0 and GPIO1
    
  FIO0DIR = 0x00000000;
  FIO1DIR = 0x00000000;
    
  FIO0CLR = 0xFFFFFFFF;
  FIO1CLR = 0xFFFFFFFF;       
}

//******************************************************************************
//** Function name:        configurePLL
//**
//** Descriptions:        Configure PLL switching to main OSC instead of IRC
//**                        at power up and wake up from power down. 

#define PLL_LOCK_BIT             0x0400
#define PLL_CONNECT_BIT        0x0200
#define PLL_STAT_MP_MASK  0x003F

// get value which should be written to PLLCFG
#define  MAKE_PLLCFG(pllM, pllP) \
    (((pllM)-1) | (((pllP) < 8 ? ((pllP) >> 1) : (((pllP)-1) >> 1)) << 5))

#define PLL0_FEED  PLL0FEED = 0xaa; PLL0FEED = 0x55

static __inline void configurePLL0(void)
{
  PLL0CON = 0;                /* Disable PLL, disconnected */
  PLL0_FEED;
    
    PLL0CFG = MAKE_PLLCFG(PLL0_M, PLL0_P);    
  PLL0_FEED;
      
  PLL0CON = 1;                /* Enable PLL, disconnected */
  PLL0_FEED;
    while( !(PLL0STAT & PLL_LOCK_BIT) );

  PLL0CON = 3;                /* enable and connect */
  PLL0_FEED;
  while( !(PLL0STAT & PLL_CONNECT_BIT) );
}

#ifdef USE_USB
#define PLL1_FEED  PLL1FEED = 0xaa; PLL1FEED = 0x55
static __inline void configurePLL1(void)
{
  PLL1CON = 0;                /* Disable PLL, disconnected */
  PLL1_FEED;
    
    PLL1CFG = MAKE_PLLCFG(PLL1_M, PLL1_P);    
  PLL1_FEED;
      
  PLL1CON = 1;                /* Enable PLL, disconnected */
  PLL1_FEED;
    while( !(PLL1STAT & PLL_LOCK_BIT) );

  PLL1CON = 3;                /* enable and connect */
  PLL1_FEED;
  while( !(PLL1STAT & PLL_CONNECT_BIT) );
}
#endif

void configurePLL( void )
{
    configurePLL0();

#ifdef USE_USB
    configurePLL1();
#endif

    VPBDIV = PERIPH_DIV;
}

/******************************************************************************
** Function name:        TargetResetInit
**
** Descriptions:        Initialize the target board before running the main() 
**                        function; User may change it as needed, but may not 
**                        deleted it.
**
** parameters:            None
** Returned value:        None
** 
******************************************************************************/
void targetResetInit(void)
{
    // initialize MCU power 
    powerResetInit();

#ifdef __DEBUG_RAM    
  MEMMAP = 0x2;            /* remap to internal RAM */
#endif

#ifdef __DEBUG_FLASH    
  MEMMAP = 0x1;            /* remap to internal flash */
#endif

  /* Configure PLL, switch from IRC to Main OSC */
  configurePLL();

  /* Set memory accelerater module*/
  MAMCR = 0;    // disable MAM

#ifdef USE_MAM

#if Fmcu < 20000000
  MAMTIM = 1;
#elif Fmcu < 40000000
  MAMTIM = 2;
#else
  MAMTIM = 3;
#endif
  MAMCR = 2;  // fully enable MAM

#endif // USE_MAM

  gpioResetInit();

  vicInit();
}

// burnTmo is timeout needed to erase-burn firmware, before device 
// is programmatically reset with watchdog
void runBootLoader( esU32 burnTmo )
{
    void (*bootloaderEntryPoint)(void) = (void*)0;
    // enable legacy io
    SCS = 0;
    // connect TXD0, RXD0 as GPIO, configure RXD0 as input
    PINSEL0 &= ~((1 << 3) | (1 << 2));
    IO0DIR &= ~(1 << 1);
    // configure P0.14 as output and clear it
    IO0DIR |= (1 << 14);
    IO0CLR = (1 << 14);
    // disable previously enabled interrupts in VICIntEnClear
    VICIntEnClr = 0xFFFFFFFF;
    // if PLL is connected, disconnect it
    PLL0CON = 0x00;
    PLL0FEED = 0xAA;
    PLL0FEED = 0x55;
    // set the peripheral bus divider to 1/4
    VPBDIV = 0x00;
    // restore FDR reset value
    U0FDR = 0x10;
    // restore Timer1 to its reset state
    T1PR = 0x00;
    T1MCR = 0x00;
    T1CCR = 0x00;
    // program watchdog in accordance to the requested burnTmo
    // NB! MCU is running at the xtal frequency now, and WDT is ticking at 1/4 of it (VPBDIV)
    // burnTmo is expected to be in ms
     WDTC = burnTmo * (Fosc / 16000);
  WDMOD = WDEN | WDRESET; // enable watchdog, reset MCU on underflow
    // feed and disable watchdog
  WDFEED = 0xAA;
  WDFEED = 0x55;
    // re-map the interrupt vectors to the boot block
    MEMMAP = 0x00;
    // invoke the bootloader
    bootloaderEntryPoint();
}

#endif // _target_hw_c_
