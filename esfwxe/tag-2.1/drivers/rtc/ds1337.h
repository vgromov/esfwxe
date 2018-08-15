#ifndef _ds_1337_h_
#define _ds_1337_h_

#ifdef __cplusplus
    extern "C" {
#endif

enum {
    // control byte bits 
    //
    DS1337_CTL_OSC_STOP                    = 0x80,
    DS1337_CTL_FREQ_OUT_1HZ            =    0x00,
    DS1337_CTL_FREQ_OUT_4096HZ    = 0x08,
    DS1337_CTL_FREQ_OUT_8192HZ    = 0x10,
    DS1337_CTL_FREQ_OUT_32768HZ    = 0x18,
    DS1337_CTL_FREQ_INTCN                = 0x04,
    DS1337_CTL_ALM2_ENABLE            = 0x02,
    DS1337_CTL_ALM1_ENABLE            = 0x01,
    // status byte bits
    //
    DS1337_STAT_OSC_STOPPED            = 0x80,
    DS1337_STAT_ALM2                        = 0x02,
    DS1337_STAT_ALM1                        = 0x01,
};

// services build on top of spi driver code
//
// RTC initialization
esBL ds1337SetCtl(i2cHANDLE handle, esU8 ctl);
esBL ds1337GetCtl(i2cHANDLE handle, esU8* ctl);

// read|write chip status
esBL ds1337SetStatus(i2cHANDLE handle, esU8 status);
esBL ds1337GetStatus(i2cHANDLE handle, esU8* status);

// RTC
//
// set-get esDT
esBL ds1337SetDateTime(i2cHANDLE handle, const esDT* dt);
esBL ds1337GetDateTime(i2cHANDLE handle, esDT* dt);

// alarm
//
// set alarm esDT. ignore mask bits mark alarm registers which do not take part in match comparison
// for alarm1 these bits are, from lsb to msb: s, m, h, dow|dom
// useDayOfWeek flag means that during alarm programming corresponding day register will have meaning day of week (1-7)
// otherwise, day of month is used (1-31)
esBL ds1337SetAlarm1(i2cHANDLE handle, const esDT* dt, esBL useDayOfWeek, esU8 ignoreMatchMask);
// for alarm2 ignore mask bits are, from lsb to msb: m, h, dow|dom
esBL ds1337SetAlarm2(i2cHANDLE handle, const esDT* dt, esBL useDayOfWeek, esU8 ignoreMatchMask);

#ifdef __cplusplus
    }
#endif

#endif //_ds_1337_h_
