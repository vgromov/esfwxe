#include <esfwxe/target.h>
#include <esfwxe/utils.h>

#include <stddef.h>

#include <esfwxe/cpp/os/EseOsDefs.h>
#include <esfwxe/cpp/concept/EseChannelIntf.h>
#include <esfwxe/cpp/drivers/common/EseI2cChannelIoCtlDefs.h>
#include <esfwxe/cpp/drivers/common/EseI2cSlaveDevice.h>

#include "cy8cmbr_xxxx.h"
//----------------------------------------------------------------------

// I2C command error codes
enum {
  cy8cmbrErrNone          = 0,    //< NO_ERROR
  cy8cmbrErrWrite         = 0xFD, //< WRITE_FAIL (Write to flash failed)
  cy8cmbrErrCrc           = 0xFE  //< CRC_ERROR (Stored configuration CRC checksum did not
                                  //  match the calculated configuration CRC checksum).
};
//----------------------------------------------------------------------

// I2C registers
enum {
  cy8cmbrRegDataOffs      = 0x00, //< This is the address of the first register in the Device Configuration
                                  //  state, starting from which the data will be programmed in flash.
  cy8cmbrRegI2cAddr       = 0x51, //< I2C Configuration Register – this is used to set (or read) the I2C slave
                                  //  address. The Slave range is 0x00-0x7F. This register is available in
                                  //  the Device Configuration state.
  cy8cmbrRegCfgCrc        = 0x7E, //< Configuration data CRC. Its length is 2 bytes.
                                  //  Checksum matched bit is set if the checksum sent by the host
                                  //  matches the one actually calculated by the device. It is the checksum
                                  //  of the data to be programmed into flash. This bit is available in the
                                  //  Device Configuration mode only.
  cy8cmbrRegCmd           = 0x86, //< Command Register – Opcode for the command to execute.
  cy8cmbrRegCmdErr        = 0x89, //< Status code returned from the most recently executed command. The
                                  //  status can be in the range 0 to 255. See cy8cmbrErrCode
  cy8cmbrRegHwId          = 0x8F  //< 3-byte hardware ID block
};
//----------------------------------------------------------------------

// Known I2C commands
enum {
  cy8cmbrCmdSaveCheckCrc  = 0x02, //< Possible value of the CTRL_CMD register. This command stores the
                                  //  data from the register in the RAM memory to the nonvolatile memory
                                  //  (NVM). During saving, the device will compare the CRC of the registers (126 bytes) with the CRC value in the last two bytes in the config
                                  //  section. If the CRC check fails, the data is not saved to the nonvolatile
                                  //  memory and the error status is updated.
  cy8cmbrCmdSwReset       = 0xFF  //< Possible value of the CTRL_CMD register. This command executes a
                                  //  software reset
};
//----------------------------------------------------------------------
//----------------------------------------------------------------------

CY8CMBRXXXX::CY8CMBRXXXX(EseChannelIntf& i2c, uint8_t addr, esU32 tmo) ESE_NOTHROW :
EseI2cSlaveDevice(
  i2c,
  addr,
  tmo
)
{}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::registerRead(uint8_t reg, size_t len, uint8_t* out, unsigned retries /*= requestRetriesCnt*/) ESE_NOTHROW
{
  if( 
    !len || 
    !out ||
    !chnlMemIoPrepare(
      reg,
      1
    )
  )
    return false;
    
  size_t received = 0;

  do
  {
    received = m_i2c->receive(
      out, 
      len, 
      m_tmo
    );
  
    if( EseI2cChannelError::addressNacked == m_i2c->errorGet() && retries ) //< Address is NACK-ed by controller, must retry query 
    {
      if( retries > 1 )
        msDelay(5); //< Give controller time to wake up
      --retries;
    }
    else
      break;
  
  } while( retries );

  return received == len;
}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::registerWrite(uint8_t reg, size_t len, const uint8_t* in, unsigned retries /*= requestRetriesCnt*/) ESE_NOTHROW
{
  if( 
    !len || 
    !in ||
    !chnlMemIoPrepare(
      reg,
      1
    )
  )
    return false;

  size_t sent = 0;
  
  do
  {
    sent = m_i2c->send(
      in, 
      len, 
      m_tmo
    );
  
    if( 
      EseI2cChannelError::addressNacked == m_i2c->errorGet() && //< Address is NACK-ed by controller, must retry query 
      retries 
    )
    {
      if( retries > 1 )
        msDelay(5); //< Give controller time to wake up

      --retries;
    }
    else
      break;
  
  } while( retries );
  
  return sent == len;
}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::hwIdRead(HwId& id) ESE_NOTHROW
{
  return registerRead(
    cy8cmbrRegHwId,
    sizeof(id),
    reinterpret_cast<uint8_t*>(&id)
  );
}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::cfgBlockWrite(const ConfigBlock& cfg) ESE_NOTHROW
{
  // Upload configuration to HW
  if( 
    !registerWrite(
      cy8cmbrRegDataOffs,
      sizeof(cfg),
      cfg.m_data
    )
  )
    return false;

  // Save configuration
  uint8_t tmp = cy8cmbrCmdSaveCheckCrc;
  if(
    !registerWrite(
      cy8cmbrRegCmd,
      1,
      &tmp
    )
  )
    return false;

  msDelay(tmoConfigAccept); //< wait until flash update is complete
  
  // Check if configuration was accepted
  tmp = cy8cmbrErrNone;
  if( 
    !registerRead(
      cy8cmbrRegCmdErr,
      1,
      &tmp
    ) ||
    cy8cmbrErrNone != tmp
  )
    return false;

  // Perform software reset
  return softReset();
}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::cfgBlockRead(ConfigBlock& cfg) ESE_NOTHROW
{
  return registerRead(
    cy8cmbrRegDataOffs,
    sizeof(cfg),
    cfg.m_data
  );
}
//----------------------------------------------------------------------

bool CY8CMBRXXXX::softReset() ESE_NOTHROW
{
  // Perform software reset
  uint8_t tmp = cy8cmbrCmdSwReset;
  if(
    !registerWrite(
      cy8cmbrRegCmd,
      1,
      &tmp
    )
  )
    return false;

  return true;
}
//----------------------------------------------------------------------
