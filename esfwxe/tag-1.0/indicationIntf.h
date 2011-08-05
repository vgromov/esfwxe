#ifndef _indication_intf_h_
#define _indication_intf_h_

#ifdef __cplusplus
	extern "C" {
#endif

// indication subsystem interface
//
// indication initialization
void indicationInit(void);

// user indication (leds, beeper)
//
// do beep
void beep(DWORD freq, DWORD duration);
// play binary sequence, from end to start
void beepBytes(const BYTE* buff, BYTE len, DWORD freq0, DWORD freq1, DWORD duration0, DWORD duration1, DWORD pause);

// break beep
void beepStop(void);

typedef enum {
	blinkRed,
	blinkGreen

} BlinkColor;

// repetition of 0 specifies endless repetition
void blink(DWORD duration, DWORD pause, DWORD repetition, BlinkColor color);
// breaks blinking
void blinkStop(void);

// check sound and blinkind state
BOOL isBeeping(void);
BOOL isBlinking(void);

#ifdef __cplusplus
	}
#endif

#endif // _indication_intf_h_
