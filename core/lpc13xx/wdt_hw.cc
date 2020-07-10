// LPC13XX watchdog support

// initialize watchdog. use quite long timeout (order of seconds)
void wdtInit(void)
{
#ifndef SWD_JTAG_DEBUG
  LPC_SYSCON->SYSAHBCLKCTRL |= (1 << 15);
  LPC_SYSCON->WDTCLKSEL = 0x02;    // select watchdog oscillator for WDT clock source
  LPC_SYSCON->WDTOSCCTRL = 0x3F;   // 600kHz clk divided by 64 = approx 9kHz
  LPC_SYSCON->WDTCLKDIV = 1;       // do not use additional wdt clock divisor
  LPC_SYSCON->WDTCLKUEN = 1;      // update wdt clock source
  // select wdt mode and countdown counter
  LPC_WWDT->TC = 18000;           // wdt will expire if not loaded in approx 2 sec interval
  LPC_WWDT->MOD = 0x03;            // reset wdt on expiration, enable wdt
  // feed and start watchdog
  LPC_WWDT->FEED = 0xAA;
  LPC_WWDT->FEED = 0x55;
#endif
}

void wdtReset(void)
{
  LPC_WWDT->FEED = 0xAA;
  LPC_WWDT->FEED = 0x55;
}
