#ifndef _ese_power_intf_h_
#define _ese_power_intf_h_

class ESE_ABSTRACT EsePowerIntf
{
public:
  virtual uint8_t batteryLevelGet() const ESE_NOTHROW = 0;
  virtual bool isAuxPower() const ESE_NOTHROW = 0;
  virtual bool isCharging() const ESE_NOTHROW = 0;
  virtual bool isCritical() const ESE_NOTHROW = 0;
};

#endif // _ese_power_intf_h_
