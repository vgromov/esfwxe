#ifndef _sd_mmc_spi_h_
#define _sd_mmc_spi_h_

// sdmmc over spi driver header
//

#ifdef __cplusplus
	extern "C" {
#endif

// sdmmc info struct
typedef struct {
	DWORD	blockCnt;				// blocks count
	DWORD	blockSize;			// block size in bytes
	DWORD	userBlockSize;	// user block size in bytes
	DWORD	stat;						// sdmmc last operation status flags
	DWORD ioRetries;			// sdmmc io attempts
	BYTE 	flags;					// sdmmc card info flags
	BYTE	type;						// sdmmc card type

} SdmmcInfo;

// CID block
typedef UINT8 SdmmcCID[16];

// consts
enum {
	// sdmmc workmode mode flags
	sdmmcOk										= 0x01,	// sd card was initialized. check sdmmcVoltageMismatch flag to see if card is ok to use 
	sdmmcVoltageMismatch 			= 0x02, // sd card was not initialized properly due to wrong working voltage range
	sdmmcBlockAddr						= 0x04, // block addressing mode. if not set - byte addressing is used
	sdmmcHighCapacity					= 0x08,	// high capacity card detected
	sdmmcUseCrc								= 0x10, // CRC usage was configured for the card io
	sdmmcSectorEraseUnit			= 0x20,	// device supports erasing by sectors (multiple of blocks) otherwise, explicit block erase is supported

	// sdmmc status flags
	//
	sdmmcStatAOK							= 0,
	sdmmcStatIdle							= 0x00000001, // sdmmc card is in idle (non-initialized) state
	sdmmcStatEraseAborted			= 0x00000002, // block erase sequence was cancelled before erase was complete
	sdmmcStatIllegalCmd				= 0x00000004, // an illegal command was issued to card
	sdmmcStatCmdCrcError			= 0x00000008, // card refuses command CRC block
	sdmmcStatEraseSeqError 		= 0x00000010, // an error occurred in erase commands sequence
	sdmmcStatAddrError				= 0x00000020, // misaligned block access is detected
	sdmmcStatParamError				= 0x00000040, // command parameter is out of allowed range
	sdmmcStatCardLocked				= 0x00000080, // card is in locked sate
	sdmmcStatEraseWprotError 	= 0x00000100,	// write protected sector erase was attempted
	sdmmcStatLockUnlockError 	= sdmmcStatEraseWprotError,	// lock|unlock of password protected area failed
	sdmmcStatUnknownError			= 0x00000200, // unspecified|unknown error occurred
	sdmmcStatControllerError 	= 0x00000400, // internal card controller error
	sdmmcStatEccError					= 0x00000800, // card internal ECC was applied, but failed to correct the data
	sdmmcStatWprotViolation		= 0x00001000, // write protected area write access was attempted
	sdmmcStatEraseParamError	= 0x00002000, // invalid parameter selecting erase blocks, sectors of groups
	sdmmcStatOutOfRange				= 0x00004000, // 
	sdmmcStatCsdOverwrite			= sdmmcStatOutOfRange, //
	sdmmcStatWriteCrcError		= 0x00008000, // data block write was rejected due to CRC error
	sdmmcStatWriteError				= 0x00010000,	// data block write was rejected due to write error
	sdmmcWaitReadyExpired			= 0x00020000, // retries of wait for bus to become ready have expired
	sdmmcReadCrcError					= 0x00040000, // read data block crc check failed
	
	// card types (NB! not bitfield, but value)
	sdmmcUnknown					= 0,
	sdmmcSd1,	
	sdmmcSd2,
	sdmmcMmc3,	
	// misc
	SdmmcInfo_SZE 				= sizeof(SdmmcInfo),
	// spi bus must be configured for this frequency upon initialization
	sdmmcInitFreq					= 400000,
	// initial sd retries to apply upon initialization
	// final values are subject to change upon initialization completion,
	// according to card settings
	sdmmcIoInitialRetries = 1000,
};

// api
//
// application-dependent sdmmc power control
void sdmmcPowerOn(spiHANDLE h, SdmmcInfo* info, BOOL on);
// wait until card releases the bus
BOOL sdmmcWaitReady(spiHANDLE h, SdmmcInfo* info);
// initialize sdmmc card access. v is host-supplied voltage in millivolts
// i.e. 3.6v = 3600
BOOL sdmmcInit(spiHANDLE h, SdmmcInfo* info, WORD v, BOOL useCrc);
// sd card block io. returned is count of blocks actually read|written
DWORD sdmmcBlocksRead(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, BYTE* blocks, DWORD count);
DWORD sdmmcBlocksWrite(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, const BYTE* blocks, DWORD count);
// sd card block erase. returned is amount of erased blocks
DWORD sdmmcBlocksErase(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, DWORD count);
// sd spi compatibility bus release call. sd releases bus synchronously with sck pulses, not cs, 
// so we have to do 1 dummy bus read with cs deasserted to release spi properly (avoiding multislave conflicts)
void sdmmcReleaseSpiBus(spiHANDLE h);
// calculate card io retries in accordance with bus settings and requested timeout value in ms
void sdmmcCalcIoTimings(spiHANDLE h, SdmmcInfo* info, DWORD tmo);
// retrieve card id
void sdmmcCardIdGet(spiHANDLE h, SdmmcInfo* info, SdmmcCID cid);

#ifdef __cplusplus
	}
#endif

#endif // _sd_mmc_spi_h_
