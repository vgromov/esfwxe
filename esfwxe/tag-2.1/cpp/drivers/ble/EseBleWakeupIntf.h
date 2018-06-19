#ifndef _ese_ble_wakeup_intf_h_
#define _ese_ble_wakeup_intf_h_

/// BLE SOC wakeup abstraction interface
class EseBleWakeupIntf
{
public:
  virtual ~EseBleWakeupIntf() ESE_NOTHROW {}

  /// Wake up BLE SOC and kep it awaken for at least tmo milliseconds
  virtual void wakeUp(esU32 tmo) ESE_NOTHROW = 0;

  /// Break BLE SOC woken state and allow it to enter sleep state
  virtual void sleepAllow() ESE_NOTHROW = 0;

  /// Return true if BLE SOC is already in waken state
  virtual bool isWoken() const ESE_NOTHROW = 0;
};

#endif // _ese_ble_wakeup_intf_h_
