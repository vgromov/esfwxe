#ifndef _power_ctl_h_
#define _power_ctl_h_

#ifdef __cplusplus
	extern "C" {
#endif

// MCU core power control driver abstraction API
//
void powerCtlEnterIdle(void);
void powerCtlEnterPowerDown(void);

#ifdef __cplusplus
	}
#endif

#endif // _power_ctl_h_
