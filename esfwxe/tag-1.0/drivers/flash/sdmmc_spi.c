#include <common/target.h>
#pragma hdrstop

#include <string.h>
#include <common/utils.h>
#include <common/crc.h>
#include <common/core/spi.h>
#include "sdmmc_spi.h"

// sdmmc over spi driver implementation file
//

// Definitions for MMC/SDC command
#define CMD0									(0x40+0)	// GO_IDLE_STATE
#define CMD1									(0x40+1)	// SEND_OP_COND (MMC)
#define ACMD41								(0xC0+41)	// SEND_OP_COND (SDC)
#define CMD8									(0x40+8)	// SEND_IF_COND
#define CMD9									(0x40+9)	// SEND_CSD
#define CMD10									(0x40+10)	// SEND_CID
#define CMD12									(0x40+12)	// STOP_TRANSMISSION
#define CMD13									(0x40+13)	// SD_STATUS
#define ACMD13								(0xC0+13)	// SD_STATUS (SDC)
#define CMD16									(0x40+16)	// SET_BLOCKLEN
#define CMD17									(0x40+17)	// READ_SINGLE_BLOCK
#define CMD18									(0x40+18)	// READ_MULTIPLE_BLOCK
#define CMD23									(0x40+23)	// SET_BLOCK_COUNT (MMC)
#define ACMD23								(0xC0+23)	// SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24									(0x40+24)	// WRITE_BLOCK
#define CMD25									(0x40+25)	// WRITE_MULTIPLE_BLOCK
#define CMD28									(0x40+28)	// SET_WRITE_PROT
#define CMD29									(0x40+29)	// CLR_WRITE_PROT
#define CMD32									(0x40+32)	// ERASE_WR_BLK_START_ADDR
#define CMD33									(0x40+33)	// ERASE_WR_BLK_END_ADDR
#define CMD38									(0x40+38)	// ERASE
#define CMD55									(0x40+55)	// APP_CMD
#define CMD58									(0x40+58)	// READ_OCR
#define CMD59									(0x40+59)	// CRC_ON_OFF

// command response masks
//
// R1, R2, R3 LSB masks
//
#define RM_IN_IDLE 						0x01 		// In Idle State
#define	RM_ERASE_RST					0x02		// Erase Reset
#define RM_ILLEGAL_CMD 				0x04 		// Illegal Command
#define RM_CRC_ERROR					0x08		// CRC Error
#define RM_ERASE_SEQ_ERR			0x10		// Erase Sequence Error
#define RM_ADDR_ERR						0x20		// Address Error
#define RM_PARAM_ERR					0x40		// Parameter Error

// R2, R3 MSB masks
//
#define RM_CARD_LOCKED				0x01 	// Card Locked
#define RM_WRPROT_ERASE_SKIP 	0x02	// Write Protect Erase Skip
#define RM_LOCK_ULOCK_FAILED	RM_WRPROT_ERASE_SKIP // Lock/Unlock Failed
#define RM_UNSPECIFIED_ERROR	0x04	// Unspecified Error
#define RM_CARD_CTLR_ERROR  	0x08	// Card Controller Error
#define RM_CARD_ECC__FAILED		0x10 	// Card ECC Failed
#define RM_WRPROT_VIOLATION		0x20	// Write Protect Violation
#define	RM_ERASE_PARAM				0x40	// Erase Parameter
#define RM_OUT_OF_RANGE				0x80	// Out of Range
#define RM_CSD_OVERWRITE			RM_OUT_OF_RANGE // CSD Overwrite

// read data error token masks
//
#define RET_UNSPECIFIED_ERROR	0x01	// Unspecified Error
#define RET_CARD_CTLR_ERROR		0x02  // Card Controller Error
#define RET_CARD_ECC_FAILED		0x04	// Card ECC Failed
#define RET_OUT_OF_RANGE			0x08	// Out of Range
#define RET_CARD_LOCKED				0x10	// Card Locked

// block data start|stop tokens
#define DT_RBLOCK_START 			0xFE
#define DT_RBLOCK_MULTI_START DT_RBLOCK_START
#define DT_WBLOCK_START				DT_RBLOCK_START
#define DT_WBLOCK_MULTI_START	0xFC
#define DT_WBLOCK_MULTI_STOP	0xFD

// data write result token analysis
//
#define DWRT_EXTRACT(r) 			((r) & 0x1F) // extract data write token value from byte response

// write token values
//
#define DWRT_AOK							0x05 	// data write accepted
#define DWRT_CRC_ERROR				0x0B	// data write was rejected due to CRC error
#define DWRT_WRITE_ERROR			0x0D	// data write was rejected due to write error

// length of command packet
#define CMD_PACKET_LEN				6

// response types
//
typedef enum {
	sdmmcR1,
	sdmmcR1b,
	sdmmcR2,
	sdmmcR3,
	sdmmcR7,
	// special const - responses count, must go last
	sdmmcRcnt

} sdmmcResponse;

// response data sizes (additional bytes after the first response byte)
static const BYTE c_sdmmcResponseSize[sdmmcRcnt] = {
	0,	// r1
	0, 	// r1b
	1,	// r2
	4,	// r3
	4		// r7
};

// internal sdmmc io buffer
static BYTE s_sdmmcBuff[16];

// sdmmc internal helper functions
//
// convert response masks to abstract sdmmc status mask
static __inline void sdmmcConvertR1toStatus(BYTE r1, SdmmcInfo* info)
{
	info->stat = r1;
}

static __inline void sdmmcConvertR2toStatus(BYTE r1, BYTE r2, SdmmcInfo* info)
{
	sdmmcConvertR1toStatus(r1, info);
	info->stat |= ((WORD)r2) << 7;
}

// wait until bus becomes ready (DO idles to high state)
BOOL sdmmcWaitReady(spiHANDLE h, SdmmcInfo* info)
{
	DWORD retries = 0;
	do
	{
		spiGetBytes(h, s_sdmmcBuff, 1);

	} while( 
			s_sdmmcBuff[0] != 0xFF && 
			++retries < info->ioRetries );
	
	if( retries < info->ioRetries )
		return TRUE;
	else
	{
		info->stat |= sdmmcWaitReadyExpired;
		return FALSE;
	}
}

// receive command response block
static BOOL sdmmcGetResponse(spiHANDLE h, SdmmcInfo* info, sdmmcResponse r )
{
	// wait for response's first byte
	// get command response
	DWORD retries = 0;
	do
	{
		spiGetBytes(h, s_sdmmcBuff, 1);
	
	}	while( (s_sdmmcBuff[0] & 0x80) && 
						++retries < info->ioRetries );
	// retries not expired and additional bytes needed
	if( retries < info->ioRetries )
	{
		// get the rest of response packet
		if( c_sdmmcResponseSize[r] )
			spiGetBytes(h, s_sdmmcBuff+1, c_sdmmcResponseSize[r]);
		
		// convert responses to universal status field
		if( sdmmcR2 == r )
			sdmmcConvertR2toStatus(s_sdmmcBuff[0], s_sdmmcBuff[1], info);
		else
			sdmmcConvertR1toStatus(s_sdmmcBuff[0], info);

		return TRUE;
	}

	return FALSE;
}

// return response type for specified command value
static __inline sdmmcResponse sdmmcGetResponseTypeForCmd(BYTE cmd)
{
	switch(cmd)
	{ 
	case CMD8:
		return sdmmcR7;
	case CMD12:
	case CMD28:
	case CMD29:
	case CMD38:
		return sdmmcR1b;
	case CMD58:
		return sdmmcR3;
	case CMD13:
		return sdmmcR2;
	}

	return sdmmcR1;
}

// send single command packet to card and get response to it
static BOOL sdmmcSendCmdInternal(spiHANDLE h, SdmmcInfo* info, BYTE cmd, DWORD arg)
{
	const BYTE* argpos = (const BYTE*)&arg + 3;

	// wait until card becomes ready
	if( sdmmcWaitReady(h, info) )
	{
		// pack command + argument + crc
		s_sdmmcBuff[0] = cmd;
		s_sdmmcBuff[1] = *argpos--;
		s_sdmmcBuff[2] = *argpos--;
		s_sdmmcBuff[3] = *argpos--;
		s_sdmmcBuff[4] = *argpos--;
		// default to single stop bit if no crc support is active
		s_sdmmcBuff[5] = 1;
		// finalize packet with left-aligned crc7 + stop bit
		// use precalculated crcs for CMD0 and CMD8 commands with known contents
		if( CMD0 == cmd )
			s_sdmmcBuff[5] = 0x95;
		else if( CMD8 == cmd ) // for 0x1AA argument
			s_sdmmcBuff[5] = 0x87;
		else if( info->flags & sdmmcUseCrc )
			s_sdmmcBuff[5] = (crc7(0, s_sdmmcBuff, 5) << 1) + 1;
			
		// send command packet	
		spiPutBytes(h, s_sdmmcBuff, CMD_PACKET_LEN);

		// if stop reading command issued, skip one dummy byte
		if(CMD12 == cmd) 
			spiGetBytes(h, s_sdmmcBuff, 1);

		return sdmmcGetResponse(h, info, sdmmcGetResponseTypeForCmd(cmd));
	}

	return FALSE;
}

static __inline BOOL sdmmcSendCmd(spiHANDLE h, SdmmcInfo* info, BYTE cmd, DWORD arg)
{
	BOOL result;

	sdmmcSELECT;

	if( cmd & 0x80 ) // handle ACMDs
		result = sdmmcSendCmdInternal(h, info, CMD55, 0) &&	
			sdmmcSendCmdInternal(h, info, cmd & 0x7F, arg);
	else
		result = sdmmcSendCmdInternal(h, info, cmd, arg);

	sdmmcDESELECT;

	return result;
}

static BOOL sdmmcEnterIdle(spiHANDLE h, SdmmcInfo* info, BOOL useCrc)
{
	sdmmcDESELECT;
	// wait for >= 74 spi bus clocks with CS and DI set to high state
	spiGetBytes(h, s_sdmmcBuff, 6);
	spiGetBytes(h, s_sdmmcBuff, 6);

	// issue CMD0 && check response
	if( sdmmcSendCmd(h, info, CMD0, 0) &&
			sdmmcStatIdle == info->stat )
	{
		// set primary initialized flag
		info->flags = sdmmcOk;
		// activate CRC support, if required
		if( useCrc &&
				sdmmcSendCmd(h, info, CMD59, 1) &&
				sdmmcStatIdle == info->stat )
			info->flags |= sdmmcUseCrc;

		return TRUE;	
	}

	return FALSE;
}

// read OCR and check voltage mask
static BOOL sdmmcCheckVoltageMask(spiHANDLE h, SdmmcInfo* info, DWORD mask)
{
	// read OCR 
	if( sdmmcSendCmd(h, info, CMD58, 0) &&
			(	sdmmcStatAOK == info->stat ||
				sdmmcStatIdle == info->stat ) )
	{
		if( (s_sdmmcBuff[2] & ((mask >> 16) & 0xFF)) ||
				(s_sdmmcBuff[3] & ((mask >> 8) & 0xFF)) )
			return TRUE;			
		else
			info->flags |= sdmmcVoltageMismatch;
	}

	return FALSE;
}

static BOOL sdmmcCheckSd1(spiHANDLE h, SdmmcInfo* info, DWORD mask)
{
	DWORD retries = info->ioRetries;
	while( retries-- )
	{
		if( !sdmmcSendCmd(h, info, ACMD41, 0) )
			break;

		if( sdmmcStatAOK == info->stat )
		{
			info->type = sdmmcSd1;
			return sdmmcCheckVoltageMask(h, info, mask);
		}
		else if( sdmmcStatIdle != info->stat )
			break;
	}

	return FALSE;
}

static BOOL sdmmcCheckMmc3(spiHANDLE h, SdmmcInfo* info, DWORD mask)
{
	DWORD tries = info->ioRetries;
	while( tries-- )
	{
		if( !sdmmcSendCmd(h, info, CMD1, 0) )
			break;

		if( sdmmcStatAOK == info->stat )
		{
			info->type = sdmmcMmc3;
			return sdmmcCheckVoltageMask(h, info, mask);
		}
		else if( sdmmcStatIdle != info->stat )
			break;
	}

	return FALSE;
}

static BOOL sdmmcCheckSd2(spiHANDLE h, SdmmcInfo* info, DWORD mask)
{
	BOOL result = FALSE;
	DWORD tries = info->ioRetries;
	// wait until exit card idle state, continuously sending ACMD41 with high capacity bit set
	while( tries-- )
	{
		if( !sdmmcSendCmd(h, info, ACMD41, 0x40000000) )
			break;

    if( sdmmcStatAOK == info->stat )
    {
      info->type = sdmmcSd2;
			result = sdmmcCheckVoltageMask(h, info, mask);
			// check high capacity bit
			if( result && (s_sdmmcBuff[1] & 0x40) )
				info->flags |= sdmmcHighCapacity;
			break;
    }
    else if( sdmmcStatIdle != info->stat )
      break;
	}

	return result;
}

// LV range voltages are currently not supported
// voltage is in millivolts
static __inline DWORD sdmmcMakeVoltageMask(WORD v)
{
	DWORD result = 0;

	if( v >= 2700 && 
			v <= 2800 )
		result |= (1 << 15);
	if( v >= 2800 && 
			v <= 2900 )
		result |= (1 << 16);
	if( v >= 2900 && 
			v <= 3000 )
		result |= (1 << 17);
	if( v >= 3000 && 
			v <= 3100 )
		result |= (1 << 18);
	if( v >= 3100 && 
			v <= 3200 )
		result |= (1 << 19);
	if( v >= 3200 && 
			v <= 3300 )
		result |= (1 << 20);
	if( v >= 3300 && 
			v <= 3400 )
		result |= (1 << 21);
	if( v >= 3400 && 
			v <= 3500 )
		result |= (1 << 22);
	if( v >= 3500 && 
			v <= 3600 )
		result |= (1 << 23);
	
	return result;	
}

static BOOL sdmmcCheckCardSupport(spiHANDLE h, SdmmcInfo* info, WORD v)
{
	BOOL result = FALSE;
	DWORD vMask = sdmmcMakeVoltageMask(v);
	// send CMD8 with proper CRC to check if card is sd2 and host voltage is supported
	if( !sdmmcSendCmd(h, info, CMD8, 0x01AA) || 
			(sdmmcStatIllegalCmd & info->stat) )
		// error or no response - try sd1 or mmc3
		result = sdmmcCheckSd1(h, info, vMask) || 
					sdmmcCheckMmc3(h, info, vMask);
	// check if voltage and bit pattern match
	else if( s_sdmmcBuff[3] == 0x01 && 
					s_sdmmcBuff[4] == 0xAA )
		result = sdmmcCheckSd2(h, info, vMask);

	return result;
}

static BOOL sdmmcReadDataPacket(spiHANDLE h, SdmmcInfo* info, BYTE dataToken, BYTE* data, DWORD len)
{
	DWORD retries = 0;
	BOOL result = FALSE;

	sdmmcSELECT;
	// skip until data token is read
 	do
	{
		spiGetBytes(h, data, 1);

	}	while( 0xFF == *data &&
					++retries < info->ioRetries );

	// read actual data, if retries not expired, 
	// read data block crc at the end of operation
	if( retries < info->ioRetries )
	{
		if(	DT_RBLOCK_START == *data )
		{
			WORD crc;
			result = 	len == spiGetBytes(h, data, len) &&
								2 == spiGetBytes(h, (BYTE*)&crc, 2);
			if( result && (info->flags & sdmmcUseCrc) )
			{
				result = SWAPB_WORD(crc) == crc16ccitt(0, data, len);
				if( !result ) // set status bit specifying we get corrupt data read
					info->stat |= sdmmcReadCrcError;					
			}
		}
		else // set data error token bits to universal status format
			info->stat = ((WORD)*data) << 7;		
	}

	sdmmcDESELECT;
	
	return result;		
}

static BOOL sdmmcConfigureAddressing(spiHANDLE h, SdmmcInfo* info)
{
	if( info->type != sdmmcUnknown &&
			sdmmcSendCmd(h, info, CMD9, 0) &&
			sdmmcStatAOK == info->stat &&
			// read CSD block
			sdmmcReadDataPacket(h, info, DT_RBLOCK_START, s_sdmmcBuff, 16) )
	{
		// adjust CRC7
		s_sdmmcBuff[15] >>= 1;
		// check CRC
		if( s_sdmmcBuff[15] == crc7(0, s_sdmmcBuff, 15) )
		{
			if( info->flags & sdmmcHighCapacity )
			{
				// CSD V2.0
				info->blockCnt = (((DWORD)(s_sdmmcBuff[7] & 0x3F) << 16) + 
					((DWORD)s_sdmmcBuff[8] << 8) + (DWORD)s_sdmmcBuff[9] + 1);
				info->blockCnt <<= 10;
				info->userBlockSize = info->blockSize = 512;
				info->flags |= sdmmcBlockAddr;
				return TRUE;
			}
			else
			{
				// CSD V1.0
				info->blockCnt = (((DWORD)(s_sdmmcBuff[6] & 0x03) << 10) + 
					((DWORD)s_sdmmcBuff[7] << 2) + ((DWORD)(s_sdmmcBuff[8] & 0xC0) >> 6) + 1) * 
					(1 << ((((DWORD)s_sdmmcBuff[9] & 0x03) << 1) + 
								(((DWORD)s_sdmmcBuff[10] & 0x80) >> 7) + 2));
				info->blockSize = 1 << (DWORD)(s_sdmmcBuff[5] & 0x0F);
				if( !(s_sdmmcBuff[10] & 0x40) )
					info->flags |= sdmmcSectorEraseUnit;
				// V1 cards allow partial and misaligned data access by-default
				// to eliminate performance flaw, as well as card wearing, we should
				// always configure 512 user block access for such cards
				if( sdmmcSendCmd(h, info, CMD16, 512) &&
						sdmmcStatAOK == info->stat )
				{
					info->userBlockSize = 512;
					return TRUE;
				}
			}
			// get transpeed capability

		}
	}

	return FALSE;
}

// initialize sdmmc card access. v is host-supplied voltage in millivolts
// i.e. 3.6v = 3600 
BOOL sdmmcInit(spiHANDLE h, SdmmcInfo* info, WORD v, BOOL useCrc)
{
	BOOL result = FALSE;
	if( info )
	{
		// initialize sdmmc structure to all 0s
		memset(info, 0, SdmmcInfo_SZE);
		// set some initial retries	for 1s expected timeout
		sdmmcCalcIoTimings(h, info, 1000);

		if( INVALID_HANDLE != h )
		{
			result = sdmmcEnterIdle(h, info, useCrc) &&
							sdmmcCheckCardSupport(h, info, v) &&
							sdmmcConfigureAddressing(h, info);
		}
	}

	return result;
}

// sd card block io. returned is count of blocks actually read|written
DWORD sdmmcBlocksRead(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, BYTE* blocks, DWORD count)
{
	DWORD	result = 0;
	
	if( spiIsOpen(h) &&
			info &&
			sdmmcUnknown != info->type &&
			blocks &&
			count )
	{
		// if 1 == count issue single block read command
		// else, fetch multiple blocks, terminated with CMD21
		if( 1 < count )
		{
			if( sdmmcSendCmd(h, info, CMD18, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize ) &&
					sdmmcStatAOK == info->stat )
			{
				while( result < count ) 
				{
					if( !sdmmcReadDataPacket(h, info, DT_RBLOCK_MULTI_START, blocks, info->userBlockSize) ) 
						break;
					blocks += info->userBlockSize;
					++result;
				}
				sdmmcSendCmd(h, info, CMD12, 0); // break multiread
			}
		}
		else if( sdmmcSendCmd(h, info, CMD17, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize ) &&
						 sdmmcStatAOK == info->stat && 
						 sdmmcReadDataPacket(h, info, DT_RBLOCK_START, blocks, info->userBlockSize) )
			result = 1;

		sdmmcReleaseSpiBus(h);
	}

	return result;
}

static BOOL sdmmcWriteDataPacket(	spiHANDLE h, SdmmcInfo* info, BYTE token, const BYTE *buff, DWORD len )
{
	BOOL result = FALSE;

	sdmmcSELECT;
	if( sdmmcWaitReady(h, info) &&
			1 == spiPutBytes(h, &token, 1) ) // send token first
	{
		// if not stoptran token, send data block as well
		if(DT_WBLOCK_MULTI_STOP != token) 
		{
			// calc ccitt CRC16 & prepare it for sd (swap bytes)
			WORD crc = 0;
			if( info->flags & sdmmcUseCrc )
			{
				crc = crc16ccitt(0, buff, len);
				crc = SWAPB_WORD(crc);
			}
			// send data + crc
			if( len == spiPutBytes(h, buff, len) &&
					2 == spiPutBytes(h, (const BYTE*)&crc, 2) &&
					1 == spiGetBytes(h, s_sdmmcBuff, 1) ) // receive block write response
			{
				// decypher write response
				switch( DWRT_EXTRACT(s_sdmmcBuff[0]) )
				{
				case DWRT_CRC_ERROR:
					info->stat |= sdmmcStatWriteCrcError;
					break;
				case DWRT_WRITE_ERROR:
					info->stat |= sdmmcStatWriteError;
					break;
				case DWRT_AOK:
					result = TRUE;
					break;
				}
			}
		}
	}
	sdmmcDESELECT;

	return result;
}

DWORD sdmmcBlocksWrite(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, const BYTE* blocks, DWORD count)
{
	DWORD	result = 0;

	if( spiIsOpen(h) &&
		info &&
		sdmmcUnknown != info->type &&
		blocks &&
		count )
	{
		// if 1 == count issue single block write command
		// else, write multiple blocks, terminated with CMD21
		if( 1 < count )
		{
			BOOL ok;
			// inform card controller about count of blocks to be written, then start multiblock write operation
			if( info->type == sdmmcMmc3 )
				ok = sdmmcSendCmd(h, info, CMD23, count) &&
					sdmmcStatAOK == info->stat;
			else
				ok = sdmmcSendCmd(h, info, ACMD23, count) &&
					sdmmcStatAOK == info->stat;

			if( ok &&
					sdmmcSendCmd(h, info, CMD25, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize ) &&
					sdmmcStatAOK == info->stat )
			{
				while( result < count ) 
				{
					if( !sdmmcWriteDataPacket(h, info, DT_WBLOCK_MULTI_START, blocks, info->userBlockSize) ) 
						break;
					blocks += info->userBlockSize;
					++result;
				}
				sdmmcWriteDataPacket(	h, info, DT_WBLOCK_MULTI_STOP, 0, 0 ); // break multiwrite
			}
		}
		else if( sdmmcSendCmd(h, info, CMD24, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize ) &&
						 sdmmcStatAOK == info->stat &&
						 sdmmcWriteDataPacket(h, info, DT_WBLOCK_START, blocks, info->userBlockSize) )
			result = 1;

		// release spi bus as well as add stuff byte (needed) after write operations
		sdmmcReleaseSpiBus(h);
		// analyse card status if something gone wrong, and we cannot deduce the reason from existing status info
		// R2 will be automatically parsed into universal sdmmc status during this request
		if( result != count &&
				!(info->stat & (sdmmcWaitReadyExpired|sdmmcStatWriteCrcError|sdmmcStatWriteError)) ) 
			sdmmcSendCmd(h, info, CMD13, 0);
	}

	return result;
}

// sd spi compatibility bus release call. sd releases bus synchronously with sck pulses, not cs, 
// so we have to do 1 dummy bus read with cs deasserted to release spi properly (avoiding multislave conflicts)
void sdmmcReleaseSpiBus(spiHANDLE h)
{
	sdmmcDESELECT;
	spiGetBytes(h, s_sdmmcBuff, 1);
}

// calculate card io retries in accordance with bus settings and requested timeout value in ms
void sdmmcCalcIoTimings(spiHANDLE h, SdmmcInfo* info, DWORD tmo)
{
	if( h && 
			info && 
			tmo )
	{
		spiDCB dcb;
		spiGetDCB(h, &dcb);
		info->ioRetries = MAX((DWORD)sdmmcIoInitialRetries, (tmo * dcb.freqHz) / (DWORD)10000 );
	}
}

// sd card block erase. returned is amount of erased blocks
DWORD sdmmcBlocksErase(spiHANDLE h, SdmmcInfo* info, DWORD startBlock, DWORD count)
{
	DWORD result = 0;

	if( h && 
			info && 
			sdmmcUnknown != info->type && 
			count )
	{
		if( sdmmcSendCmd(h, info, CMD32, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize) &&
				sdmmcStatAOK == info->stat &&
				sdmmcSendCmd(h, info, CMD33, (info->flags & sdmmcBlockAddr) ? startBlock+count-1 : (startBlock+count-1)*info->userBlockSize) &&
				sdmmcStatAOK == info->stat &&
				sdmmcSendCmd(h, info, CMD38, 0) &&
				sdmmcStatAOK == info->stat )
			result = count;
	}

	return result;
}

// retrieve card id
void sdmmcCardIdGet(spiHANDLE h, SdmmcInfo* info, SdmmcCID cid)
{
	if( h && 
			info && 
			sdmmcUnknown != info->type )
	{
		if( sdmmcSendCmd(h, info, CMD10, 0) &&
				sdmmcStatAOK == info->stat )
			sdmmcReadDataPacket(h, info, DT_RBLOCK_START, cid, sizeof(SdmmcCID));
	}
}
