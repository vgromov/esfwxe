#include <esfwxe/target.h>
#pragma hdrstop

#include <string.h>
#include <esfwxe/utils.h>
#include <esfwxe/crc.h>
#ifndef USE_CUSTOM_SPI
# include <esfwxe/core/spi.h>
#endif
#include <esfwxe/drivers/flash/sdmmc_spi.h>

// SDMMC over SPI driver implementation
//

// Definitions for MMC/SDC command
#define CMD0                    (0x40+0)  // GO_IDLE_STATE
#define CMD1                    (0x40+1)  // SEND_OP_COND (MMC)
#define ACMD41                  (0xC0+41)  // SEND_OP_COND (SDC)
#define CMD8                    (0x40+8)  // SEND_IF_COND
#define CMD9                    (0x40+9)  // SEND_CSD
#define CMD10                   (0x40+10)  // SEND_CID
#define CMD12                   (0x40+12)  // STOP_TRANSMISSION
#define CMD13                   (0x40+13)  // SD_STATUS
#define ACMD13                  (0xC0+13)  // SD_STATUS (SDC)
#define CMD16                   (0x40+16)  // SET_BLOCKLEN
#define CMD17                   (0x40+17)  // READ_SINGLE_BLOCK
#define CMD18                   (0x40+18)  // READ_MULTIPLE_BLOCK
#define CMD23                   (0x40+23)  // SET_BLOCK_COUNT (MMC)
#define ACMD23                  (0xC0+23)  // SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24                   (0x40+24)  // WRITE_BLOCK
#define CMD25                   (0x40+25)  // WRITE_MULTIPLE_BLOCK
#define CMD28                   (0x40+28)  // SET_WRITE_PROT
#define CMD29                   (0x40+29)  // CLR_WRITE_PROT
#define CMD32                   (0x40+32)  // ERASE_WR_BLK_START_ADDR
#define CMD33                   (0x40+33)  // ERASE_WR_BLK_END_ADDR
#define CMD38                   (0x40+38)  // ERASE
#define CMD55                   (0x40+55)  // APP_CMD
#define CMD58                   (0x40+58)  // READ_OCR
#define CMD59                   (0x40+59)  // CRC_ON_OFF

// command response masks
//
// R1, R2, R3 LSB masks
//
#define RM_IN_IDLE              0x01     // In Idle State
#define RM_ERASE_RST            0x02    // Erase Reset
#define RM_ILLEGAL_CMD          0x04     // Illegal Command
#define RM_CRC_ERROR            0x08    // CRC Error
#define RM_ERASE_SEQ_ERR        0x10    // Erase Sequence Error
#define RM_ADDR_ERR             0x20    // Address Error
#define RM_PARAM_ERR            0x40    // Parameter Error

// R2, R3 MSB masks
//
#define RM_CARD_LOCKED          0x01   // Card Locked
#define RM_WRPROT_ERASE_SKIP    0x02  // Write Protect Erase Skip
#define RM_LOCK_ULOCK_FAILED    RM_WRPROT_ERASE_SKIP // Lock/Unlock Failed
#define RM_UNSPECIFIED_ERROR    0x04  // Unspecified Error
#define RM_CARD_CTLR_ERROR      0x08  // Card Controller Error
#define RM_CARD_ECC__FAILED     0x10   // Card ECC Failed
#define RM_WRPROT_VIOLATION     0x20  // Write Protect Violation
#define  RM_ERASE_PARAM         0x40  // Erase Parameter
#define RM_OUT_OF_RANGE         0x80  // Out of Range
#define RM_CSD_OVERWRITE        RM_OUT_OF_RANGE // CSD Overwrite

// read data error token masks
//
#define RET_UNSPECIFIED_ERROR   0x01  // Unspecified Error
#define RET_CARD_CTLR_ERROR     0x02  // Card Controller Error
#define RET_CARD_ECC_FAILED     0x04  // Card ECC Failed
#define RET_OUT_OF_RANGE        0x08  // Out of Range
#define RET_CARD_LOCKED         0x10  // Card Locked

// block data start|stop tokens
#define DT_RBLOCK_START         0xFE
#define DT_RBLOCK_MULTI_START   DT_RBLOCK_START
#define DT_WBLOCK_START         DT_RBLOCK_START
#define DT_WBLOCK_MULTI_START   0xFC
#define DT_WBLOCK_MULTI_STOP    0xFD

// data write result token analysis
//
#define DWRT_EXTRACT(r)         ((r) & 0x1F) // extract data write token value from byte response

// write token values
//
#define DWRT_AOK                0x05   // data write accepted
#define DWRT_CRC_ERROR          0x0B  // data write was rejected due to CRC error
#define DWRT_WRITE_ERROR        0x0D  // data write was rejected due to write error
  
// length of command packet 
#define CMD_PACKET_LEN          6

#if !defined(ES_DEBUG_TRACE) && defined(ES_USE_SDMMC_DEBUG_TRACE)
# undef ES_USE_SDMMC_DEBUG_TRACE
#endif

#if defined( ES_USE_SDMMC_DEBUG_TRACE ) && defined( ES_DEBUG_TRACE )
# define ES_SDMMC_TRACE         ES_DEBUG_TRACE
#else
# define ES_SDMMC_TRACE(...)    ((void)0)
#endif

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
static const esU8 c_sdmmcResponseSize[sdmmcRcnt] = {
  0,  // r1
  0,   // r1b
  1,  // r2
  4,  // r3
  4    // r7
};

// internal sdmmc io buffer
static esU8 s_sdmmcBuff[16];

// sdmmc internal helper functions
//
// convert response masks to abstract sdmmc status mask
static __inline void sdmmcConvertR1toStatus(esU8 r1, SdmmcInfo* info)
{
  info->stat = r1;
}

static __inline void sdmmcConvertR2toStatus(esU8 r1, esU8 r2, SdmmcInfo* info)
{
  sdmmcConvertR1toStatus(r1, info);
  info->stat |= ((esU16)r2) << 7;
}

// wait until bus becomes ready (DO idles to high state)
esBL sdmmcWaitReady(spiHANDLE h, SdmmcInfo* info)
{
  esU32 retries = 0;
  do
  {
    spiGetBytes(h, s_sdmmcBuff, 1);

  } while( 
      s_sdmmcBuff[0] != 0xFF && 
      ++retries < info->ioRetries );
  
  if( retries < info->ioRetries )
  {
    return TRUE;
  }
  else
  {
    info->stat |= sdmmcWaitReadyExpired;
    
    ES_SDMMC_TRACE("...Failed to wait until SDMMC is ready\n");
    
    return FALSE;
  }
}

// receive command response block
static esBL sdmmcGetResponse(spiHANDLE h, SdmmcInfo* info, sdmmcResponse r )
{
  // wait for response's first byte
  // get command response
  esU32 retries = 0;
  do
  {
    spiGetBytes(h, s_sdmmcBuff, 1);
  
  }  while( (s_sdmmcBuff[0] & 0x80) && 
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

    ES_SDMMC_TRACE("<+\n");
    return TRUE;
  }

  ES_SDMMC_TRACE("!-\n");
  return FALSE;
}

// return response type for specified command value
static __inline sdmmcResponse sdmmcGetResponseTypeForCmd(esU8 cmd)
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
static esBL sdmmcSendCmdInternal(spiHANDLE h, SdmmcInfo* info, esU8 cmd, esU32 arg)
{
  const esU8* argpos = (const esU8*)&arg + 3;

  ES_SDMMC_TRACE(
    ">CMD%d", 
    ((int)cmd-0x40)
  );
 
  // wait until card becomes ready
  if( 
    !sdmmcWaitReady(
      h, 
      info
    ) 
  )
  {
    ES_SDMMC_TRACE("... card not ready\n");
    return FALSE;
  }
  
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
  if( 
    CMD_PACKET_LEN != spiPutBytes(
      h, 
      s_sdmmcBuff, 
      CMD_PACKET_LEN
    )
  )
  {
    ES_SDMMC_TRACE("... not sent\n");
    return FALSE;
  }

  // if stop reading command issued, skip one dummy byte
  if(CMD12 == cmd) 
    spiGetBytes(
      h, 
      s_sdmmcBuff, 
      1
    );

  return sdmmcGetResponse(
    h, 
    info, 
    sdmmcGetResponseTypeForCmd(cmd)
  );
}

static __inline esBL sdmmcSendCmd(spiHANDLE h, SdmmcInfo* info, esU8 cmd, esU32 arg)
{
  esBL result = FALSE;

  sdmmcSELECT;

  if( cmd & 0x80 ) // handle ACMDs
    result = sdmmcSendCmdInternal(h, info, CMD55, 0) &&  
      sdmmcSendCmdInternal(h, info, cmd & 0x7F, arg);
  else
    result = sdmmcSendCmdInternal(h, info, cmd, arg);

  sdmmcDESELECT;

  return result;
}

static esBL sdmmcEnterIdle(spiHANDLE h, SdmmcInfo* info, esBL useCrc)
{
  ES_SDMMC_TRACE("sdmmcEnterIdle...");
  
  sdmmcDESELECT;

  // wait for >= 74 spi bus clocks with CS and DI set to high state
  memset(
    s_sdmmcBuff, 
    0xFFFFFFFF, 
    sizeof(s_sdmmcBuff)
  );
  spiPutBytes(h, s_sdmmcBuff, 12);

  // issue CMD0 && check response
  if( 
    sdmmcSendCmd(h, info, CMD0, 0) &&
    sdmmcStatIdle == info->stat 
  )
  {
    // set primary initialized flag
    info->flags = sdmmcOk;

    // activate CRC support, if required
    if( useCrc )
    {
      ES_SDMMC_TRACE("...Activating CRC support, ->CMD59\n");
      
      if( sdmmcSendCmd(h, info, CMD59, 1) )
      {
        ES_SDMMC_TRACE(
          "...CMD59 returned status: 0x%02d\n",
          (int)info->stat
        );
        
        if(sdmmcStatIdle == info->stat)
        {
          ES_BIT_SET(info->flags, sdmmcUseCrc);

          ES_SDMMC_TRACE("...CRC support activated\n");
        }
      }
    }
    
    if( ES_BIT_MASK_MATCH_ALL(info->stat, sdmmcStatIdle|sdmmcStatCmdCrcError) ) //< We already have active CRC
    {
      ES_SDMMC_TRACE("...The SD card may already have CRC support activated, though, watch sdmmcStatCmdCrcError in status\n");
      
      ES_BIT_SET(info->flags, sdmmcUseCrc);
      ES_BIT_CLR(info->stat, sdmmcStatCmdCrcError);
    }
    
    ES_SDMMC_TRACE("... Entered IDLE OK\n");
    return TRUE;  
  }

  ES_SDMMC_TRACE("... Enter IDLE FAILED\n");
  return FALSE;
}

// read OCR and check voltage mask
static esBL sdmmcCheckVoltageMask(spiHANDLE h, SdmmcInfo* info, esU32 mask)
{
  // read OCR 
  if( sdmmcSendCmd(h, info, CMD58, 0) &&
      (  sdmmcStatAOK == info->stat ||
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

static esBL sdmmcCheckSd1(spiHANDLE h, SdmmcInfo* info, esU32 mask)
{
  esU32 retries = info->ioRetries;
  
  ES_SDMMC_TRACE("sdmmcCheckSD1\n");
  
  while( retries-- )
  {
    if( !sdmmcSendCmd(h, info, ACMD41, 0) )
      break;

    if( sdmmcStatAOK == info->stat )
    {
      info->type = sdmmcSd1;
      
      ES_SDMMC_TRACE("... checking voltage\n");
      
      return sdmmcCheckVoltageMask(h, info, mask);
    }
    else if( sdmmcStatIdle != info->stat )
      break;
  }

  ES_SDMMC_TRACE("... not SD1\n");

  return FALSE;
}

static esBL sdmmcCheckMmc3(spiHANDLE h, SdmmcInfo* info, esU32 mask)
{
  esU32 tries = info->ioRetries;
  
  ES_SDMMC_TRACE("sdmmcCheckMmc3\n");
  
  while( tries-- )
  {
    if( !sdmmcSendCmd(h, info, CMD1, 0) )
      break;

    if( sdmmcStatAOK == info->stat )
    {
      info->type = sdmmcMmc3;
      
      ES_SDMMC_TRACE("... checking voltage\n");

      return sdmmcCheckVoltageMask(h, info, mask);
    }
    else if( sdmmcStatIdle != info->stat )
      break;
  }

  ES_SDMMC_TRACE("... not MMC3\n");

  return FALSE;
}

static esBL sdmmcCheckSd2(spiHANDLE h, SdmmcInfo* info, esU32 mask)
{
  esBL result = FALSE;
  esU32 tries = info->ioRetries;
  
  ES_SDMMC_TRACE("sdmmcCheckSd2\n");
  
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

  ES_SDMMC_TRACE("... SD2 ?:%d\n", (int)result);

  return result;
}

// LV range voltages are currently not supported
// voltage is in millivolts
static __inline esU32 sdmmcMakeVoltageMask(esU16 v)
{
  esU32 result = 0;

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

static esBL sdmmcCheckCardSupport(spiHANDLE h, SdmmcInfo* info, esU16 v)
{
  esBL result = FALSE;
  esU32 vMask = sdmmcMakeVoltageMask(v);
  
  ES_SDMMC_TRACE("sdmmcCheckCardSupport...\n");
  
  // send CMD8 with proper CRC to check if card is sd2 and host voltage is supported
  if( 
    !sdmmcSendCmd(h, info, CMD8, 0x01AA) || 
    (sdmmcStatIllegalCmd & info->stat) 
  )
  {
    ES_SDMMC_TRACE("... not SD2...\n");

    // error or no response - try sd1 or mmc3
    result = sdmmcCheckSd1(h, info, vMask) || 
          sdmmcCheckMmc3(h, info, vMask);
  }
  else if( //< Check if voltage and bit pattern match
    s_sdmmcBuff[3] == 0x01 && 
    s_sdmmcBuff[4] == 0xAA 
  )
    result = sdmmcCheckSd2(h, info, vMask);

  ES_SDMMC_TRACE("... %d\n", (int)result);
  
  return result;
}

static esBL sdmmcReadDataPacket(spiHANDLE h, SdmmcInfo* info, esU8 dataToken, esU8* data, esU32 len)
{
  esU32 retries = 0;
  esBL result = FALSE;

  ES_SDMMC_TRACE("sdmmcReadDataPacket...");

  sdmmcSELECT;
  // skip until data token is read
  do
  {
    spiGetBytes(h, data, 1);

  }  while( 0xFF == *data &&
          ++retries < info->ioRetries );

  // read actual data, if retries not expired, 
  // read data block crc at the end of operation
  if( retries < info->ioRetries )
  {
    if(  DT_RBLOCK_START == *data )
    {
      esU16 crc;
      result =   len == spiGetBytes(h, data, len) &&
                2 == spiGetBytes(h, (esU8*)&crc, 2);
      if( result && (info->flags & sdmmcUseCrc) )
      {
        result = SWAPB_WORD(crc) == crc16ccitt(0, data, len);
        if( !result ) // set status bit specifying we get corrupt data read
          info->stat |= sdmmcReadCrcError;          
      }
    }
    else // set data error token bits to universal status format
      info->stat = ((esU16)*data) << 7;    
  }

  sdmmcDESELECT;
  
  ES_SDMMC_TRACE("... %d", (int)result);
  
  return result;    
}

static esBL sdmmcConfigureAddressing(spiHANDLE h, SdmmcInfo* info)
{
  ES_SDMMC_TRACE("sdmmcConfigureAddressing...\n");

  if( 
    info->type != sdmmcUnknown &&
    sdmmcSendCmd(h, info, CMD9, 0)
  )
  {
    ES_SDMMC_TRACE("... CMD9 status: 0x%02X\n", info->stat);
    
    if(
      sdmmcStatAOK != info->stat ||
      !sdmmcReadDataPacket(h, info, DT_RBLOCK_START, s_sdmmcBuff, 16) //< Read CSD block
    )
      goto __exitNOK;
    
#ifdef ES_USE_SDMMC_DEBUG_TRACE
    ES_SDMMC_TRACE("... analysing CSD block: ");

    for(int idx = 0; idx < sizeof(s_sdmmcBuff); ++idx )
    {
      ES_SDMMC_TRACE(
        "%02X", 
        (int)s_sdmmcBuff[idx]
      );
    }
#endif

#ifdef ES_USE_SDMMC_CSD_CRC_VERIFICATION
    esU8 crcIn = s_sdmmcBuff[15] >>= 1;
    esU8 crc = crc7(0, s_sdmmcBuff, 15);

    // check CRC
    ES_SDMMC_TRACE(
      "... checking CSD CRC, got (before adjustment): 0x%02X, calc'd: 0x%02X\n",
      crcIn,
      crc
    );
    
    if( crcIn == crc )
    {
      ES_SDMMC_TRACE("... CSD CRC OK\n");
#endif      

      if( info->flags & sdmmcHighCapacity )
      {
        // CSD V2.0
        info->blockCnt = (((esU32)(s_sdmmcBuff[7] & 0x3F) << 16) + 
          ((esU32)s_sdmmcBuff[8] << 8) + (esU32)s_sdmmcBuff[9] + 1);
        info->blockCnt <<= 10;
        info->userBlockSize = info->blockSize = 512;
        info->flags |= sdmmcBlockAddr;
        
        ES_SDMMC_TRACE("... TRUE-CSD V2.0\n");
       
        return TRUE;
      }
      else
      {
        // CSD V1.0
        info->blockCnt = (((esU32)(s_sdmmcBuff[6] & 0x03) << 10) + 
          ((esU32)s_sdmmcBuff[7] << 2) + ((esU32)(s_sdmmcBuff[8] & 0xC0) >> 6) + 1) * 
          (1 << ((((esU32)s_sdmmcBuff[9] & 0x03) << 1) + 
                (((esU32)s_sdmmcBuff[10] & 0x80) >> 7) + 2));
        info->blockSize = 1 << (esU32)(s_sdmmcBuff[5] & 0x0F);
        if( !(s_sdmmcBuff[10] & 0x40) )
          info->flags |= sdmmcSectorEraseUnit;
        // V1 cards allow partial and misaligned data access by-default
        // to eliminate performance flaw, as well as card wearing, we should
        // always configure 512 user block access for such cards
        if( sdmmcSendCmd(h, info, CMD16, 512) &&
            sdmmcStatAOK == info->stat )
        {
          info->userBlockSize = 512;
          
          ES_SDMMC_TRACE("... TRUE-CSD V1.0\n");

          return TRUE;
        }
      }
      // TODO: get transpeed capability

#ifdef ES_USE_SDMMC_CSD_CRC_VERIFICATION
    }
    else
    {
      ES_SDMMC_TRACE("... CSD CRC NOT OK\n");
    }
#endif

  }

__exitNOK:

  ES_SDMMC_TRACE("... FALSE\n");
  
  return FALSE;
}

// initialize sdmmc card access. v is host-supplied voltage in millivolts
// i.e. 3.6v = 3600 
esBL sdmmcInit(spiHANDLE h, SdmmcInfo* info, esU16 v, esBL useCrc)
{
  if( !info )
    return FALSE;

  // initialize sdmmc structure to all 0s
  memset(info, 0, SdmmcInfo_SZE);
  // set some initial retries  for 1s expected timeout
  sdmmcCalcIoTimings(
    h, 
    info, 
    sdmmcInitialTmo
  );

  if( INVALID_HANDLE == h )
    return FALSE;

  return sdmmcEnterIdle(h, info, useCrc) &&
          sdmmcCheckCardSupport(h, info, v) &&
          sdmmcConfigureAddressing(h, info);
}

// sd card block io. returned is count of blocks actually read|written
esU32 sdmmcBlocksRead(spiHANDLE h, SdmmcInfo* info, esU32 startBlock, esU8* blocks, esU32 count)
{
  esU32  result = 0;
  
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

static esBL sdmmcWriteDataPacket(  spiHANDLE h, SdmmcInfo* info, esU8 token, const esU8 *buff, esU32 len )
{
  esBL result = FALSE;

  sdmmcSELECT;
  if( sdmmcWaitReady(h, info) &&
      1 == spiPutBytes(h, &token, 1) ) // send token first
  {
    // if not stoptran token, send data block as well
    if(DT_WBLOCK_MULTI_STOP != token) 
    {
      // calc ccitt CRC16 & prepare it for sd (swap bytes)
      esU16 crc = 0;
      if( info->flags & sdmmcUseCrc )
      {
        crc = crc16ccitt(0, buff, len);
        crc = SWAPB_WORD(crc);
      }
      // send data + crc
      if( len == spiPutBytes(h, buff, len) &&
          2 == spiPutBytes(h, (const esU8*)&crc, 2) &&
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

esU32 sdmmcBlocksWrite(spiHANDLE h, SdmmcInfo* info, esU32 startBlock, const esU8* blocks, esU32 count)
{
  esU32  result = 0;

  if( 
    spiIsOpen(h) &&
    info &&
    sdmmcUnknown != info->type &&
    blocks &&
    count 
  )
  {
    // if 1 == count issue single block write command
    // else, write multiple blocks, terminated with CMD21
    if( 1 < count )
    {
      esBL ok;
      // inform card controller about count of blocks to be written, then start multiblock write operation
      if( info->type == sdmmcMmc3 )
        ok = sdmmcSendCmd(h, info, CMD23, count) &&
          sdmmcStatAOK == info->stat;
      else
        ok = sdmmcSendCmd(h, info, ACMD23, count) &&
          sdmmcStatAOK == info->stat;

      if( 
        ok &&
        sdmmcSendCmd(h, info, CMD25, (info->flags & sdmmcBlockAddr) ? startBlock : startBlock*info->userBlockSize ) &&
        sdmmcStatAOK == info->stat 
      )
      {
        while( result < count ) 
        {
          if( !sdmmcWriteDataPacket(h, info, DT_WBLOCK_MULTI_START, blocks, info->userBlockSize) ) 
            break;
          blocks += info->userBlockSize;
          ++result;
        }
        sdmmcWriteDataPacket(  h, info, DT_WBLOCK_MULTI_STOP, 0, 0 ); // break multiwrite
      }
    }
    else if( 
      sdmmcSendCmd(
        h, 
        info, 
        CMD24, 
        (info->flags & sdmmcBlockAddr) ? 
          startBlock : 
          startBlock*info->userBlockSize 
      ) &&
      sdmmcStatAOK == info->stat &&
      sdmmcWriteDataPacket(
        h, 
        info, 
        DT_WBLOCK_START, 
        blocks, 
        info->userBlockSize
      ) 
    )
      result = 1;

    // release spi bus as well as add stuff byte (needed) after write operations
    sdmmcReleaseSpiBus(h);
    // analyse card status if something gone wrong, and we cannot deduce the reason from existing status info
    // R2 will be automatically parsed into universal sdmmc status during this request
    if( 
      result != count &&
      !(info->stat & (sdmmcWaitReadyExpired|sdmmcStatWriteCrcError|sdmmcStatWriteError)) 
    ) 
      sdmmcSendCmd(
        h, 
        info, 
        CMD13, 
        0
      );
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
void sdmmcCalcIoTimings(spiHANDLE h, SdmmcInfo* info, esU32 tmo)
{
  if( h && 
      info && 
      tmo )
  {
#ifndef USE_CUSTOM_SPI  
    spiDCB dcb;
    spiGetDCB(h, &dcb);
    info->ioRetries = MAX((esU32)sdmmcIoInitialRetries, (tmo * dcb.freqHz) / (esU32)10000 );
#else
    info->ioRetries = MAX((esU32)sdmmcIoInitialRetries, (tmo * spiRateGet(h)) / (esU32)10000 );
#endif    
  }
}

// sd card block erase. returned is amount of erased blocks
esU32 sdmmcBlocksErase(spiHANDLE h, SdmmcInfo* info, esU32 startBlock, esU32 count)
{
  esU32 result = 0;

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
    if( 
      sdmmcSendCmd(h, info, CMD10, 0) &&
      sdmmcStatAOK == info->stat 
    )
      sdmmcReadDataPacket(
        h, 
        info, 
        DT_RBLOCK_START, 
        cid, 
        sizeof(SdmmcCID)
      );
  }
}
