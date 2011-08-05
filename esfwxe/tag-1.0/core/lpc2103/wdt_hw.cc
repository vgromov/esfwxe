// LPC2103 watchdog support

// initialize watchdog. use quite long timeout (order of seconds)
void wdtInit(void)
{
#ifndef JTAG_DEBUG
 	WDTC = 2 * Fpclk;	// once WDEN is set, the WDT will start after feeding
  WDMOD = WDEN | WDRESET; // enable watchdog, reset MCU on underflow
	// feed and start watchdog
  WDFEED = 0xAA;
  WDFEED = 0x55;
#endif
}

void wdtReset(void)
{
  WDFEED = 0xAA;
  WDFEED = 0x55;
}
