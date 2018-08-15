#include <esfwxe/target.h>
#pragma hdrstop

#include <esfwxe/utils.h>
#include <esfwxe/core/spi.h>
#include <esfwxe/datetime.h>
#include "ds28dg02.h"

// driver code for ds28dg02 RTC|GPIO|EEPROM chip
//

#define MEMBLOCK_LEN                                0x40
// unique registration
#define EseUID                                                    0x18

// PIO POD access
#define POD_PIOSTATE0                                0x0A
#define POD_PIOSTATE1                                0x0B
#define POD_PIODIR0                                    0x0C
#define POD_PIODIR1                                    0x0D
#define POD_PIOCTL0                                    0x0E
#define POD_PIOCTL1                                    0x0F
// PIO access
#define PIO_STATE0                                    0x20
#define PIO_STATE1                                    0x21
#define PIO_DIR0                                        0x22
#define PIO_DIR1                                        0x23
#define PIO_READ0                                        0x26
#define PIO_READ1                                        0x27

// RTC calendar
#define RTC_SECS                                        0x29
#define RTC_MINS                                        0x2A
#define RTC_HRS                                            0x2B
#define RTC_DAYOW                                        0x2C
#define RTC_DAYNUM                                    0x2D
#define RTC_MONS                                        0x2E
#define RTC_YRS                                            0x2F
// RTC alarms
#define RTC_ASECS                                        0x30
#define RTC_AMINS                                        0x31
#define RTC_AHRS                                        0x32
#define RTC_ADAYOW_DAYNUM                        0x33
// RTC control|monitor setup
#define RTC_CTL                                            0x34
// RTC status
#define RTC_STAT                                        0x35

// chip instructions
#define WRITE_ENABLE                                 0x06
#define WRITE_DISABLE                                0x04

#define WRITE_STATUS                                0x01
#define READ_STATUS                                    0x05
#define APPLY_CTLREG_PWRONDEFS            0x07

#define WRITE_HMEM                                    0x0A
#define READ_HMEM                                        0x0B
#define WRITE_LMEM                                    0x02
#define READ_LMEM                                        0x03

#define READY_WAIT_RETRIES                    0x0000FFFF

// static dummy buffer for TX|RX instructions
// trailing or prefix byte is ds28 status byte as declared by
// datasheet 
static esU8 s_buff[2];

static esBL internalWaitUntilReady(spiHANDLE bus)
{
    esU32 retries    = READY_WAIT_RETRIES;
    esBL result = FALSE;

    s_buff[0] = READ_STATUS;

    // select chip
    ds28dg02_SELECT

    if( spiPutBytes(bus, s_buff, 1) == 1 )
        while( spiGetBytes(bus, s_buff, 1) == 1 && !result && retries-- )
            result = !(s_buff[0] & ds28dg02STATUS_BUSY);
    
    // unselect chip
    ds28dg02_DESELECT

    return result;
}

esBL ds28dg02ReadStatus(spiHANDLE bus, esU8* status)
{
    esBL result = FALSE;

    s_buff[0] = READ_STATUS;

    // select chip
    ds28dg02_SELECT

    if( spiPutBytes(bus, s_buff, 1) == 1 && spiGetBytes(bus, s_buff, 1) == 1 )
    {
        *status = s_buff[0];
        result = TRUE;
    }
    
    // unselect chip
    ds28dg02_DESELECT

    return result;
}

esBL ds28dg02WriteStatus(spiHANDLE bus, esU8 status)
{
    esBL result = FALSE;

    s_buff[0] = WRITE_STATUS;
    s_buff[1]    = status;

    // select chip
    ds28dg02_SELECT

    result = spiPutBytes(bus, s_buff, 2) == 2;
    
    // unselect chip
    ds28dg02_DESELECT

    return result;
}

esBL ds28dg02ApplyPOD(spiHANDLE bus)
{
    esBL result = FALSE;

    if( internalWaitUntilReady(bus) )
    {
        s_buff[0] = APPLY_CTLREG_PWRONDEFS;
        // select chip
        ds28dg02_SELECT
        result = spiPutBytes(bus, s_buff, 1) == 1;
        // unselect chip
        ds28dg02_DESELECT
    }

    return result;
}

static __inline esBL internalWriteEnable(spiHANDLE bus)
{
    esBL result = FALSE;

    s_buff[0] = WRITE_ENABLE;
    // select chip
    ds28dg02_SELECT
    result = spiPutBytes(bus, s_buff, 1) == 1;
    // unselect chip
    ds28dg02_DESELECT

    return result;
}


static __inline void internalWriteDisable(spiHANDLE bus)
{
    s_buff[0] = WRITE_DISABLE;
    // select chip
    ds28dg02_SELECT
    spiPutBytes(bus, s_buff, 1);
    // unselect chip
    ds28dg02_DESELECT
}

// Read bytes from chip.
//
// ds28 protocol requires that each multibyte write packet is terminated with 
// byte which gets written to ds28 status register
esU32 ds28dg02ReadBytesLo(spiHANDLE bus, esU8 bAddr, esU8* buff, esU32 length)
{
    esU32 result = 0;

    // wait until possible pending EEPROM writes
    if( length && internalWaitUntilReady(bus) )
    {
        s_buff[0] = READ_LMEM;
        s_buff[1]    = bAddr;

        // select chip
        ds28dg02_SELECT     
        
        if( spiPutBytes(bus, s_buff, 2) == 2 && spiGetBytes(bus, s_buff, 1) == 1 )    // write address & read 1 dummy status byte
             result = spiGetBytes(bus, buff, length);
    
        // unselect chip
        ds28dg02_DESELECT
    }

    return result;
}

esU32 ds28dg02ReadBytesHi(spiHANDLE bus, esU8 bAddr, esU8* buff, esU32 length)
{
    esU32 result = 0;

    // wait until possible pending EEPROM writes
    if( length && internalWaitUntilReady(bus) )
    {
        s_buff[0] = READ_HMEM;
        s_buff[1]    = bAddr;

        // select chip
        ds28dg02_SELECT     
        
        if( spiPutBytes(bus, s_buff, 2) == 2 && spiGetBytes(bus, s_buff, 1) == 1 )    // write address & read 1 dummy status byte 
             result = spiGetBytes(bus, buff, length);
    
        // unselect chip
        ds28dg02_DESELECT
    }

    return result;
}

// read unique chip id
esBL ds28dg02ReadUID( spiHANDLE bus, esU64* uid )
{
    return ds28dg02ReadBytesHi(bus, EseUID, (esU8*)uid, 8) == 8;
}

// write bytes to chip
//
esU32 ds28dg02WriteBytesLo(spiHANDLE bus, esU8 bAddr, const esU8* buff, esU32 length)
{
    esU32 result = 0;

    // wait until possible pending EEPROM writes
    if( length && internalWaitUntilReady(bus) && internalWriteEnable(bus) )
    {
        s_buff[0] = WRITE_LMEM;
        s_buff[1]    = bAddr;

        // select chip
        ds28dg02_SELECT     
        
        if( spiPutBytes(bus, s_buff, 2) == 2 )
            result = spiPutBytes(bus, buff, length);

        // unselect chip
        ds28dg02_DESELECT

        // ensure we disable WEN bit in case write is failed
        internalWriteDisable(bus);
    }

    return result;
}

esU32 ds28dg02WriteBytesHi(spiHANDLE bus, esU8 bAddr, const esU8* buff, esU32 length)
{
    esU32 result = 0;

    // wait until possible pending EEPROM writes
    if( length && internalWaitUntilReady(bus) && internalWriteEnable(bus) )
    {
        s_buff[0] = WRITE_HMEM;
        s_buff[1]    = bAddr;

        // select chip
        ds28dg02_SELECT     
        
        if( spiPutBytes(bus, s_buff, 2) == 2 )
            result = spiPutBytes(bus, buff, length);

        // unselect chip
        ds28dg02_DESELECT

        // ensure we disable WEN bit in case write is failed
        internalWriteDisable(bus);
    }

    return result;
}

esBL ds28dg02GetPOD_PIODirection( spiHANDLE bus, esU16* dir )
{
    return ds28dg02ReadBytesHi(bus, POD_PIODIR0, (esU8*)dir, 2) == 2;
}

esBL ds28dg02SetPOD_PIODirection( spiHANDLE bus, esU16 dir )
{
    return ds28dg02WriteBytesHi(bus, POD_PIODIR0, (const esU8*)&dir, 2) == 2;
}

esBL ds28dg02GetPOD_PIOState( spiHANDLE bus, esU16* state )
{
    return ds28dg02ReadBytesHi(bus, POD_PIOSTATE0, (esU8*)state, 2) == 2;
}

esBL ds28dg02SetPOD_PIOState( spiHANDLE bus, esU16 state )
{
    return ds28dg02WriteBytesHi(bus, POD_PIOSTATE0, (const esU8*)&state, 2) == 2;
}

esBL ds28dg02GetPOD_PIOCtl( spiHANDLE bus, esU16* ctl )
{
    return ds28dg02ReadBytesHi(bus, POD_PIOCTL0, (esU8*)ctl, 2) == 2;
}

esBL ds28dg02SetPOD_PIOCtl( spiHANDLE bus, esU16 ctl )
{    
    return ds28dg02WriteBytesHi(bus, POD_PIOCTL0, (const esU8*)&ctl, 2) == 2;
}

esBL ds28dg02SetPIODirection( spiHANDLE bus, esU16 state )
{
    return ds28dg02WriteBytesHi(bus, PIO_DIR0, (const esU8*)&state, 2 ) == 2;
}

esBL ds28dg02GetPIODirection( spiHANDLE bus, esU16* state )
{
    return ds28dg02ReadBytesHi(bus, PIO_DIR0, (esU8*)state, 2 ) == 2;
}

esBL ds28dg02SetPIOState( spiHANDLE bus, esU16 state )
{
    return ds28dg02WriteBytesHi(bus, PIO_STATE0, (const esU8*)&state, 2 ) == 2;
}

esBL ds28dg02GetPIOState( spiHANDLE bus, esU16* state )
{
    return ds28dg02ReadBytesHi(bus, PIO_STATE0, (esU8*)state, 2 ) == 2;
}

esBL ds28dg02GetPIOInput( spiHANDLE bus, esU16* state )
{
    return ds28dg02ReadBytesHi(bus, PIO_READ0, (esU8*)state, 2 ) == 2;
}

// RTC getters and setters
esBL ds28dg02SetRTC_Ctl(spiHANDLE bus, esU8 ctl)
{
    return ds28dg02WriteBytesHi(bus, RTC_CTL, &ctl, 1) == 1;
}

esBL ds28dg02GetRTC_Ctl(spiHANDLE bus, esU8* ctl)
{
    return ds28dg02ReadBytesHi(bus, RTC_CTL, ctl, 1) == 1;
}

esBL ds28dg02SetDateTime(spiHANDLE bus, const esDT* dt)
{
    esU8 tmp[7];
    int y, mn, d, h, m, s;
    dtDecomposeDateTime(dt,    &y, 0, &mn, &d, &h, &m, &s, 0);
    tmp[0] = BYTE2esBCD(s);
    tmp[1] = BYTE2esBCD(m);
    tmp[2] = BYTE2esBCD(h);
    tmp[3] = BYTE2esBCD( dtGetDayOfWeek(y, mn, d) );
    tmp[4] = BYTE2esBCD(d);
    tmp[5] = BYTE2esBCD(mn);
    tmp[6] = BYTE2esBCD(y%100);

    return ds28dg02WriteBytesHi(bus, RTC_SECS, tmp, 7) == 7;    
}

esBL ds28dg02GetDateTime(spiHANDLE bus, esDT* dt)
{
    esU8 tmp[7];
    return ds28dg02ReadBytesHi(bus, RTC_SECS, tmp, 7) == 7 &&
        dtComposeDateTime(dt, esBCD2BYTE(tmp[6])+2000, esBCD2BYTE(tmp[5]), esBCD2BYTE(tmp[4]),
                esBCD2BYTE(tmp[2]), esBCD2BYTE(tmp[1]), esBCD2BYTE(tmp[0]), 0);    
}
