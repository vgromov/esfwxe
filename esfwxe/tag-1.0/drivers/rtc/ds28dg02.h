#ifndef _ds28dg02_h_
#define _ds28dg02_h_

// driver code for ds28dg02 RTC|GPIO|EEPROM chip
//

#ifdef __cplusplus
	extern "C" {
#endif

enum {
	// ds28dg02 status bits
	//
	ds28dg02STATUS_BUSY			= 0x01,
	ds28dg02STATUS_WRITE_ENABLED= 0x02,
	ds28dg02STATUS_WRITEPROT_MASK= 0x0C,
	ds28dg02STATUS_WDT_MASK	= 0x30,
	ds28dg02STATUS_HIGH_MEM_PROT= 0x40, // Specifies whether the writeable addresses in the range of 120h and 
																			// higher are write-protected
	ds28dg02STATUS_HW_WRITEPROT= 0x80,
	// ds28dg02 control bits
	//
	DS28DG02_CTL_ALARM_EN 	= 0x01,
	DS28DG02_CTL_OSC_EN			= 0x02,
	DS28DG02_CTL_WDT_EN			= 0x04,
	DS28DG02_CTL_WDT_PINSEL	= 0x08,
	DS28DG02_CTL_BTMON_EN		= 0x40,
	// battery monitor bits
	//
	DS28DG02_BTMON_MASK			= 0x30,
	DS28DG02_BTMON_1_75V 		= 0x00,
	DS28DG02_BTMON_2V				= 0x10,
	DS28DG02_BTMON_2_25V		= 0x20,
	DS28DG02_BTMON_2_5V			= 0x30,
	// alarm register status bits
	//
	DS28DG02_ALMSTAT_RESET	= 0x01,
	DS28DG02_ALMSTAT_WDT		= 0x02,
	DS28DG02_ALMSTAT_CLOCK	= 0x04,
	DS28DG02_ALMSTAT_BTONRST= 0x08,
	DS28DG02_ALMSTAT_PWRONRST= 0x10,
	DS28DG02_ALMSTAT_HWWRPROT= 0x20,
	DS28DG02_ALMSTAT_LOWBT	= 0x40,
	// pio type and inversion control
	//
	// inversion
	DS28DG02_PIOCTL_INV0		= 0x0001,
	DS28DG02_PIOCTL_INV1		= 0x0002,
	DS28DG02_PIOCTL_INV2		= 0x0004,
	DS28DG02_PIOCTL_INV3		= 0x0008,
	DS28DG02_PIOCTL_INV4		= 0x0010,
	DS28DG02_PIOCTL_INV5		= 0x0020,
	DS28DG02_PIOCTL_INV6		= 0x0040,
	DS28DG02_PIOCTL_INV7		= 0x0080,
	DS28DG02_PIOCTL_INV8		= 0x0100,
	DS28DG02_PIOCTL_INV9		= 0x0200,
	DS28DG02_PIOCTL_INV10		= 0x0400, 
	DS28DG02_PIOCTL_INV11		= 0x0800,
	DS28DG02_PIOCTL_INV_MASK= (DS28DG02_PIOCTL_INV0|DS28DG02_PIOCTL_INV1|DS28DG02_PIOCTL_INV2|DS28DG02_PIOCTL_INV3|DS28DG02_PIOCTL_INV4|
														 DS28DG02_PIOCTL_INV5|DS28DG02_PIOCTL_INV6|DS28DG02_PIOCTL_INV7|DS28DG02_PIOCTL_INV8|DS28DG02_PIOCTL_INV9|
														 DS28DG02_PIOCTL_INV10|DS28DG02_PIOCTL_INV11),
	DS28DG02_PIOCTL_INV_ALL	=	DS28DG02_PIOCTL_INV_MASK,
	// pins type
	DS28DG02_PIOCTL_OPDRN0_3= 0x1000,
	DS28DG02_PIOCTL_OPDRN4_7= 0x2000,
	DS28DG02_PIOCTL_OPDRN8_11= 0x4000,
	// pio mode
	DS28DG02_PIOCTL_HICURRENT= 0x8000,
	// pio direction control
	//
	DS28DG02_PIODIR_IN0			= 0x0001,
	DS28DG02_PIODIR_IN1			= 0x0002,
	DS28DG02_PIODIR_IN2			= 0x0004,
	DS28DG02_PIODIR_IN3			= 0x0008,
	DS28DG02_PIODIR_IN4			= 0x0010,
	DS28DG02_PIODIR_IN5			= 0x0020,
	DS28DG02_PIODIR_IN6			= 0x0040,
	DS28DG02_PIODIR_IN7			= 0x0080,
	DS28DG02_PIODIR_IN8			= 0x0100,
	DS28DG02_PIODIR_IN9			= 0x0200,
	DS28DG02_PIODIR_IN10		= 0x0400,
	DS28DG02_PIODIR_IN11		= 0x0800,
	DS28DG02_PIODIR_MASK		= (DS28DG02_PIODIR_IN0|DS28DG02_PIODIR_IN1|DS28DG02_PIODIR_IN2|DS28DG02_PIODIR_IN3|DS28DG02_PIODIR_IN4|
														 DS28DG02_PIODIR_IN5|DS28DG02_PIODIR_IN6|DS28DG02_PIODIR_IN7|DS28DG02_PIODIR_IN8|DS28DG02_PIODIR_IN9|
														 DS28DG02_PIODIR_IN10|DS28DG02_PIODIR_IN11),
	DS28DG02_PIODIR_ALL			= DS28DG02_PIODIR_MASK,
};

// services build on top of spi driver code
//
// RTC initialization
void ds28dg02InitRTC(spiHANDLE bus, BYTE ctl);

// read|write chip status
BOOL ds28dg02ReadStatus(spiHANDLE bus, BYTE* status);
BOOL ds28dg02WriteStatus(spiHANDLE bus, BYTE status);

// read bytes from chip's Lo or Hi memory spaces
DWORD ds28dg02ReadBytesLo(spiHANDLE bus, BYTE bAddr, BYTE* buff, DWORD length);
DWORD ds28dg02ReadBytesHi(spiHANDLE bus, BYTE bAddr, BYTE* buff, DWORD length);

// write bytes to chip's Lo or Hi memory spaces
DWORD ds28dg02WriteBytesLo(spiHANDLE bus, BYTE bAddr, const BYTE* buff, DWORD length);
DWORD ds28dg02WriteBytesHi(spiHANDLE bus, BYTE bAddr, const BYTE* buff, DWORD length);

// read unique chip id
BOOL ds28dg02ReadUID( spiHANDLE bus, UINT64* uid );

// particular services
//
// PIO
//
BOOL ds28dg02GetPOD_PIODirection( spiHANDLE bus, WORD* dir );
BOOL ds28dg02SetPOD_PIODirection( spiHANDLE bus, WORD dir );
BOOL ds28dg02GetPOD_PIOState( spiHANDLE bus, WORD* state );
BOOL ds28dg02SetPOD_PIOState( spiHANDLE bus, WORD state );
BOOL ds28dg02GetPOD_PIOCtl( spiHANDLE bus, WORD* ctl );
BOOL ds28dg02SetPOD_PIOCtl( spiHANDLE bus, WORD ctl );
BOOL ds28dg02ApplyPOD(spiHANDLE bus);

BOOL ds28dg02SetPIODirection( spiHANDLE bus, WORD dir );
BOOL ds28dg02GetPIODirection( spiHANDLE bus, WORD* dir );
BOOL ds28dg02SetPIOState( spiHANDLE bus, WORD state );
BOOL ds28dg02GetPIOState( spiHANDLE bus, WORD* state );
BOOL ds28dg02GetPIOInput( spiHANDLE bus, WORD* input );

// RTC
//
BOOL ds28dg02SetRTC_Ctl(spiHANDLE bus, BYTE ctl);
BOOL ds28dg02GetRTC_Ctl(spiHANDLE bus, BYTE* ctl);
BOOL ds28dg02SetDateTime(spiHANDLE bus, const DATETIME* dt);
BOOL ds28dg02GetDateTime(spiHANDLE bus, DATETIME* dt);

#ifdef __cplusplus
	}
#endif

#endif // _ds28dg02_h_

