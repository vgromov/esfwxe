// MCU core power control driver implementation for LPC2103
//

void powerCtlEnterIdle(void)
{
#ifndef JTAG_DEBUG
  // DPDEN in PCON set to 0;  
  // SLEEPDEEP in SCR set to 0;
  // clear sleep and deep pwrdown flags
  LPC_PMU->PCON  = 0x00000900;
  // select simple sleep mode
  SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
  __WFI();
#endif
}

void powerCtlEnterPowerDown(void)
{
#ifndef JTAG_DEBUG
  //PCON |= 0x02;    // enter power down mode
#endif
}
