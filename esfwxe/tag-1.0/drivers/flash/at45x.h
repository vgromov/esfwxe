#ifndef _at45x_h_
#define _at45x_h_

//////////////// atmel amd flash driver
//

// some functionality is available only to D chips. Bs are less functional.
//
#ifdef at45xB
#	define at45_REDUCED_FUNCTIONALITY
#endif

#ifdef __cplusplus
	extern "C" {
#endif

#ifndef at45_REDUCED_FUNCTIONALITY

#pragma pack(push, 1)

// sector protection and sector lockdown registers
typedef struct {
	BYTE _0a_0b;
	BYTE _1;
	BYTE _2;
	BYTE _3;
	BYTE _4;
	BYTE _5;
	BYTE _6;
	BYTE _7;

} at45SectorSecurity;

// security register with one-time programmable user info
typedef struct {
	BYTE userInfo[64];	  // !!! one time programmable
	BYTE facrotyInfo[64]; // fixed

} at45SecurityRegister;

// manufacturer and device id information
typedef struct {
	DWORD mfgID 			: 8;
	DWORD densityCode : 5;
	DWORD familyCode	: 3;
	DWORD productVer	: 5;
	DWORD mlcCode			: 3;
	DWORD infoStrLen	: 8;

} at45MfgAndDeviceInfo;

#pragma pack(pop)

#endif // at45_REDUCED_FUNCTIONALITY

// generic defs
//

// generic at45 command packet compose-decompose macros
//
#define at45MAKE_STDCMD(opcode, pageAddr, byteAddr) \
	((DWORD)(((DWORD)(pageAddr) & 0x00000FC0) << 2)		| \
					(((DWORD)(pageAddr) & 0x0000003F) << 18) 	|	\
					(((DWORD)(byteAddr) & 0x00000300) << 8)		|	\
					(((DWORD)(byteAddr) & 0x000000FF) << 24)	|	\
					((DWORD)(opcode) & 0xFF) )

#define at45GET_STDCMD_OPCODE( stdCmd ) \
	((stdCmd) & 0xFF)

#define at45STDCMD_SIZE 							4
#define at45MAX_READY_WAIT						0x0000FFFF

// status bits
#define at45STATUS_READY						0x80
#define at45STATUS_MATCH						0x40
#define at45STATUS_PROTECTION_ENABLED	0x02
#define at45STATUS_BINARY_PAGE			0x01

// opcodes
//
// SPI mode-dependent opcodes
//
// "Inactive Clock Polarity Low or High" SPI mode opcodes (marked using ICPLH suffix)
#define at45ARRAY_READ_ICPLH					0x68
#define at45PAGE_READ_ICPLH						0x52
#define at45BUFF1_READ_ICPLH					0x54
#define at45BUFF2_READ_ICPLH					0x56
#define at45STATUS_READ_ICPLH					0x57
// "0 or 3" SPI mode opcodes (marked using SPI03 suffix)
#define at45ARRAY_READ_SPI03					0xE8
#define at45PAGE_READ_SPI03						0xD2
#define at45BUFF1_READ_SPI03					0xD4
#define at45BUFF2_READ_SPI03					0xD6
#define at45STATUS_READ_SPI03					0xD7
// write, flash to buffer io && erase
#define at45BUFF1_WRITE								0x84
#define at45BUFF2_WRITE								0x87
#define at45BUFF1_TO_PAGE_W_ERASE			0x83
#define at45BUFF2_TO_PAGE_W_ERASE			0x86
#define at45BUFF1_TO_PAGE_WO_ERASE		0x88
#define at45BUFF2_TO_PAGE_WO_ERASE		0x89
#define at45PAGE_ERASE								0x81
#define at45BLOCK_ERASE								0x50 // block of 8 pages get erased
#define at45BUFFERED1_PAGE_WRITE			0x82
#define at45BUFFERED2_PAGE_WRITE			0x85
#define at45PAGE_TO_BUFF1_CPY					0x53
#define at45PAGE_TO_BUFF2_CPY					0x55
#define at45PAGE_TO_BUFF1_CMP					0x60
#define at45PAGE_TO_BUFF2_CMP					0x61
#define at45PAGE_AUTO_REWRITE1				0x58
#define at45PAGE_AUTO_REWRITE2				0x59

#ifndef at45_REDUCED_FUNCTIONALITY
#	define at45CHIP_ERASE								0x9A8094C7
	// security opcodes
#	define at45ENABLE_SECTOR_PROTECTION	0xA97F2A3D
#	define at45DISABLE_SECTOR_PROTECTION	0x9A7F2A3D
#	define at45ERASE_SECTOR_PROTECTION		0xCF7F2A3D
#	define at45WRITE_SECTOR_PROTECTION		0xFC7F2A3D
#	define at45READ_SECTOR_PROTECTION		0x32
#	define at45SECTOR_LOCKDOWN						0x307F2A3D
#	define at45WRITE_SECURITY_REGISTER		0x0000009B
#	define at45READ_SECURITY_REGISTER		0x77
	// power opcodes
#	define at45ENTER_DEEP_POWER_DOWN		0xB9
#	define at45RESUME_DEEP_POWER_DOWN		0xAB
#	define at45SET_BINARY_PAGE					0xA6802A3D	// !!! one time programming option
	// info opcodes
#	define at45READ_MFG_AND_DEVICE_ID		0x9F
#endif // at45_REDUCED_FUNCTIONALITY

BOOL at45WaitUntilReady( spiHANDLE bus, BYTE statusReadOpcode );
BOOL at45CmdWrite( spiHANDLE bus, DWORD cmd );
DWORD at45Write( spiHANDLE bus, DWORD cmd, const BYTE* buff, DWORD len );
DWORD at45Read( spiHANDLE bus, DWORD cmd, BYTE* buff, DWORD len );
BOOL at45StatusRead( spiHANDLE bus, BYTE statusReadOpcode, BYTE* stat );
// small flash chip io check - write predefined pattern to the sram buffer, 
// read it back and compare results
BOOL at45IoCheck(spiHANDLE bus, BYTE statusReadOpcode);

#ifdef __cplusplus
	}
#endif

#endif // _at45x_h_
