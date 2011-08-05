#ifndef _target_hw_c_
#define _target_hw_c_
// Target C file for NXP LPC21xx Family Microprocessors

void PowerResetInit(void)
{
	// we have to clear PDN bit in AD0CR before disabling ADC power
	AD0CR &= ~0x0200000;
	AD1CR &= ~0x0200000;

  // disable all peripheral power
	PCONP = 0;

 	// enable USB power if needed
#ifdef USE_USB
	PCONP |= PCUSB; 
#endif
}

/******************************************************************************
** Function name:		GPIOResetInit
**
** Descriptions:		Initialize the target board before running the main() 
**				function; User may change it as needed, but may not 
**				deleted it.
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void GPIOResetInit( void )
{
  /* Reset all GPIO pins to default: primary function */
  PINSEL0 = 0x00000000;
  PINSEL1 = 0x00000000;
#ifndef JTAG_DEBUG
  PINSEL2 = 0x00000000;
#else
	PINSEL2 = 0x00000004;
#endif
    
	// FIO reset
	SCS 	 |= 0x00000003;	// high speed is enabled on GPIO0 and GPIO1
    
  FIO0DIR = 0x00000000;
  FIO1DIR = 0x00000000;
    
  FIO0CLR = 0xFFFFFFFF;
  FIO1CLR = 0xFFFFFFFF;       
}

//******************************************************************************
//** Function name:		ConfigurePLL
//**
//** Descriptions:		Configure PLL switching to main OSC instead of IRC
//**						at power up and wake up from power down. 

#define PLL_LOCK_BIT 			0x0400
#define PLL_CONNECT_BIT		0x0200
#define PLL_STAT_MP_MASK	0x003F

// get value which should be written to PLLCFG
#define	MAKE_PLLCFG(pllM, pllP) \
	(((pllM)-1) | (((pllP) < 8 ? ((pllP) >> 1) : (((pllP)-1) >> 1)) << 5))

#define PLL0_FEED  PLL0FEED = 0xaa; PLL0FEED = 0x55

static __inline void ConfigurePLL0(void)
{
  PLL0CON = 0;				/* Disable PLL, disconnected */
  PLL0_FEED;
    
	PLL0CFG = MAKE_PLLCFG(PLL0_M, PLL0_P);	
  PLL0_FEED;
      
  PLL0CON = 1;				/* Enable PLL, disconnected */
  PLL0_FEED;
	while( !(PLL0STAT & PLL_LOCK_BIT) );

  PLL0CON = 3;				/* enable and connect */
  PLL0_FEED;
  while( !(PLL0STAT & PLL_CONNECT_BIT) );
}

#ifdef USE_USB
#define PLL1_FEED  PLL1FEED = 0xaa; PLL1FEED = 0x55
static __inline void ConfigurePLL1(void)
{
  PLL1CON = 0;				/* Disable PLL, disconnected */
  PLL1_FEED;
    
	PLL1CFG = MAKE_PLLCFG(PLL1_M, PLL1_P);	
  PLL1_FEED;
      
  PLL1CON = 1;				/* Enable PLL, disconnected */
  PLL1_FEED;
	while( !(PLL1STAT & PLL_LOCK_BIT) );

  PLL1CON = 3;				/* enable and connect */
  PLL1_FEED;
  while( !(PLL1STAT & PLL_CONNECT_BIT) );
}
#endif

void ConfigurePLL( void )
{
	ConfigurePLL0();

#ifdef USE_USB
	ConfigurePLL1();
#endif

	VPBDIV = PERIPH_DIV;
}

/******************************************************************************
** Function name:		TargetResetInit
**
** Descriptions:		Initialize the target board before running the main() 
**						function; User may change it as needed, but may not 
**						deleted it.
**
** parameters:			None
** Returned value:		None
** 
******************************************************************************/
void targetResetInit(void)
{
	// initialize MCU power 
	PowerResetInit();

#ifdef __DEBUG_RAM    
  MEMMAP = 0x2;			/* remap to internal RAM */
#endif

#ifdef __DEBUG_FLASH    
  MEMMAP = 0x1;			/* remap to internal flash */
#endif

  /* Configure PLL, switch from IRC to Main OSC */
  ConfigurePLL();

  /* Set memory accelerater module*/
  MAMCR = 0;	// disable MAM

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

  GPIOResetInit();

  init_VIC();
}
#endif // _target_hw_c_
