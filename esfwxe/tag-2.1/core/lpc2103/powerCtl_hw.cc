// MCU core power control driver implementation for LPC2103
//

void powerCtlEnterIdle(void)
{
#ifndef JTAG_DEBUG
  PCON |= 0x01;   // enter idle mode
#endif
}

void powerCtlEnterPowerDown(void)
{
#ifndef JTAG_DEBUG
  PCON |= 0x02;    // enter power down mode
#endif
}
