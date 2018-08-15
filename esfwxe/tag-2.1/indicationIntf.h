#ifndef _indication_intf_h_
#define _indication_intf_h_

#ifdef __cplusplus
    extern "C" {
#endif

// indication subsystem interface
//
// indication initialization
void indicationInit(void);

#ifdef USE_SOUND_INDICATION
// user indication (leds, beeper)
//
// do beep
void beep(esU32 freq, esU32 duration);
// play binary sequence, from end to start
void beepBytes(const esU8* buff, esU8 len, esU32 freq0, esU32 freq1, esU32 duration0, esU32 duration1, esU32 pause);
// check beeping state
esBL isBeeping(void);
// break beep
void beepStop(void);
#endif // USE_SOUND_INDICATION

#ifdef USE_LED_INDICATION
typedef enum {
    blinkRed,
    blinkGreen

} BlinkColor;

// repetition of 0 specifies endless repetition
void blink(esU32 duration, esU32 pause, esU32 repetition, BlinkColor color);
// breaks blinking
void blinkStop(void);
// check blinkind state
esBL isBlinking(void);
#endif // USE_LED_INDICATION

#ifdef __cplusplus
    }
#endif

#endif // _indication_intf_h_
