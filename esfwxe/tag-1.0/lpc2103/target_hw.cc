#ifndef _target_hw_c_
#define _target_hw_c_
// Target C file for NXP LPC2101-03 Family Microprocessors

void PowerResetInit(void)
{
	// we have to clear PDN bit in ADCR before disabling ADC power
	ADCR &= ~0x0200000;

  // disable all peripheral power
	PCONP = 0;
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
#ifndef JTAG_DEBUG
  PINSEL1 = 0x00000000;
#else
	PINSEL1 = 0x55400000;
#endif

	// FIO reset
	SCS 	 |= 0x00000001;	// high speed is enabled on GPIO0
    
  FIODIR = 0x00000000;
  FIOCLR = 0xFFFFFFFF;
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

#define PLL_FEED  PLLFEED = 0xaa; PLLFEED = 0x55

void ConfigurePLL( void )
{
  PLLCON = 0;				/* Disable PLL, disconnected */
  PLL_FEED;
    
	PLLCFG = MAKE_PLLCFG(PLL_M, PLL_P);	
  PLL_FEED;
      
  PLLCON = 1;				/* Enable PLL, disconnected */
  PLL_FEED;
	while( !(PLLSTAT & PLL_LOCK_BIT) );

  PLLCON = 3;				/* enable and connect */
  PLL_FEED;
  while( !(PLLSTAT & PLL_CONNECT_BIT) );

	APBDIV = PERIPH_DIV;
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
#elif Fmcu <= 40000000
  MAMTIM = 2;
#elif Fmcu <= 60000000
	MAMTIM = 3;
#else
  MAMTIM = 4;
#endif

  MAMCR = 2;  // fully enable MAM

#endif // USE_MAM

  GPIOResetInit();

  init_VIC();
}

// burnTmo is timeout needed to erase-burn firmware, before device 
// is programmatically reset with watchdog
void RunBootLoader( DWORD burnTmo )
{
	void (*bootloaderEntryPoint)(void) = (void*)0;
	// emulate MCU reset state
	//
	// reset PINSEL (set all pins to GPIO)
  SCS = 0;
	PINSEL0 = 0x00000000;
  PINSEL1 = 0x00000000;
  // reset GPIO, but drive P0.14 low (output)
  IODIR = 0x00004000;
  IOCLR = 0x00004000;
  // power up all peripherals
  PCONP = PCTIM0|PCTIM1|PCUART0|PCUART1|PCI2C0|PCSPI0|PCRTC|PCSSP|PCAD0|PCI2C1|PCTIM2|PCTIM3;
  // disconnect PLL
	PLLCON = 0x00;
  PLLFEED = 0xAA; 
	PLLFEED = 0x55;
	// set peripheral bus to 1/4th of the system clock */
  APBDIV = 0x00;
	// map bootloader vectors
  MEMMAP = 0;
	// program WDT to reset MCU in burnTmo ms
  if(burnTmo != 0) 
	{
    WDTC = (DWORD)(((QWORD)burnTmo * (QWORD)Fosc) / (QWORD)16000);
    WDMOD = 0x03;
    WDFEED = 0xAA; 
		WDFEED = 0x55;
  }
  // go to bootloader entry point
  bootloaderEntryPoint();
}

#endif // _target_hw_c_
