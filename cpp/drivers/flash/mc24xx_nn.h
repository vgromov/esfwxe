#ifndef _mc_24xx_nn_h_
#define _mc_24xx_nn_h_

/// Microchip 24xx 256|512 (where xx is aa, lc, or fc) driver interface
/// Internally, page buffer is used to minimize page refresh cycles, if possible
///
template <size_t KbitSize, size_t PageSize>
class mc24xx_nn  : public EseI2cSlaveDevice
{
public:
  enum {
    baseAddr      = 0xA0,
    chipAddrMask  = 0x0E,
    totalBytes    = (KbitSize * 1024) / 8,
    pageSize      = PageSize,
    pagesCount    = totalBytes / pageSize,
    pageMaxAddr   = (pagesCount-1)*pageSize,
    ackRetriesDef = 10, //< Default chip address ACK retries (busy check)
    ackTmo        = 10  //< According to datasheet, internal page writing operation takes no longer than 5 ms. 
                        //  Use greater value, to make sure we're OK
  };
  
public:
  mc24xx_nn() ESE_NOTHROW :
  m_ackPollRetries(ackRetriesDef),
  m_wts(0),
  m_pageAddr(sc_noAddr),
  m_dirty(false),
  m_prevOpWasRead(true)
  {}

  mc24xx_nn(EseChannelIntf& i2c, esU8 chipAddr) ESE_NOTHROW :
  EseI2cSlaveDevice(
    i2c,
    baseAddr + (chipAddr & chipAddrMask),
    ackTmo
  ),
  m_ackPollRetries(ackRetriesDef),
  m_wts(0),
  m_pageAddr(sc_noAddr),
  m_dirty(false),
  m_prevOpWasRead(true)
  {}

  ~mc24xx_nn() ESE_NOTHROW
  {
    commit();
  }

  /// mc24xx_nn initializer
  void init(EseChannelIntf& i2c, esU8 chipAddr) ESE_NOTHROW 
  {
    EseI2cSlaveDevice::init(
      i2c,
      baseAddr + (chipAddr & chipAddrMask),
      ackTmo
    );
  }

  /// Write ack timeout tuneup for memory peripheral.
  /// Minimum value is trimmed up to ackTmo
  /// @tmo      [in] new value of device IO ack tmo
  ///
  void ackTimeoutSet(uint32_t tmo) ESE_NOTHROW { if( tmo < ackTmo ) tmo = ackTmo; m_tmo = tmo; }

  /// Write bytes to the chip. All operations are made through internal page buffer, if possible.
  /// @memAddr  [in] address, at which to begin data wrighting
  /// @data     [in] data buffer
  /// @dataLen  [in] count of bytes in @data
  /// @autocommit [in, optional] signal, whether each write (even within buffered page), should be automatically committed
  ///             Setting autocommit to true will ensure flash contents are in sync after each write, but it
  ///             may significantly increase count of flash page refresh cycles, reducing flash lifetime.
  /// @return   count of data bytes actually written to chip
  ///
  esU32 write(esU16 memAddr, const esU8* data, esU32 dataLen, bool autocommit = false) ESE_NOTHROW;

  /// Sequentially read bytes from memory chip. Internal page buffer is maintained as well
  /// @memAddr  [in] address, at which to begin data reading
  /// @data     [out] data buffer to put read data in
  /// @dataLen  [in] count of data bytes to read into @data
  /// @return   count of data bytes actually read from chip
  ///
  esU32 read(esU16 memAddr, esU8* data, esU32 dataLen) ESE_NOTHROW;

  /// Commit internal page buffer if it's dirty
  /// @return   true, if commit was a success, false otherwise
  bool commit() ESE_NOTHROW;

protected:
  // Special page address value, meaning we did not read any page into buffer yet
  static const esU16 sc_noAddr = 0xFFFF;

  // Internal services
  //
  /// Calculate page address given random memory address.
  /// @memAddr  [in]  random memory address
  /// @pageAddr [out] calculated page address for @memAddr
  /// @pageOffs [out] offset from the beginning of page
  /// If address is out of chip scope, return sc_noAddr value
  ///
  static void pageAddrFromMemAddr(esU16 memAddr, esU16& pageAddr, esU16& pageOffs) ESE_NOTHROW;

  /// Read page into internal page buffer.
  /// If page buffer contains other page, which is dirty, an implicit 
  /// commit is made prior to reading a new page.
  /// @pageAddr [in]  address of page to be read
  /// @return         true if page was read, false otherwise
  ///
  bool pageRead(esU16 pageAddr) ESE_NOTHROW;

  /// Write current page buffer to the flash
  /// @return         true, if page was written, false otherwise
  ///
  bool pageWrite() ESE_NOTHROW;

protected:
  esU32 m_ackPollRetries; ///< Peripheral ACK poll (chip is busy writing)
  esU32 m_wts;            ///< Timestamp of the recent write operation request
  esU16 m_pageAddr;       ///< An address of the currenly buffered page
  esU8 m_buff[pageSize];  ///< Local page buffer

  bool m_dirty;           ///< Page buffer is dirty, pending commit required
  bool m_prevOpWasRead;   ///< Previous flash access operation was read one

  ESE_NONCOPYABLE(mc24xx_nn);
};

/// Chip type specializations
///
typedef mc24xx_nn<256, 64> mc24xx256;
typedef mc24xx_nn<512, 128> mc24xx512;

// Template implementation
#include "mc24xx_nn.ipp"

#endif // _mc_24xx_nn_h_
