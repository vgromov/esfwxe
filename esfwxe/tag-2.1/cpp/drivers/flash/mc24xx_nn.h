#ifndef _mc_24xx_nn_h_
#define _mc_24xx_nn_h_

/// Microchip 24xx 256|512 (where xx is aa, lc, or fc) driver interface
/// Internally, page buffer is used to minimize page refresh cycles, if possible
///
template <size_t KbitSize, size_t PageSize>
class mc24xx_nn
{
public:
  enum {
    baseAddr      = 0xA0,
    chipAddrMask  = 0x0E,
    totalBytes    = (KbitSize * 1024) / 8,
    pageSize      = PageSize,
    pagesCount    = totalBytes / pageSize,
    pageMaxAddr   = (pagesCount-1)*pageSize,
    ackTmo        = 10  // According to datasheet, internal page writing operation takes no longer than 5 ms. 
                        // Use greater value, to make sure we're OK
  };
  
public:
  mc24xx_nn(EseI2c& i2c, esU8 chipAddr) ESE_NOTHROW :
  m_i2c(i2c),
  m_wts(0),
  m_pageAddr(sc_noAddr),
  m_addr(baseAddr + (chipAddr & chipAddrMask)),
  m_dirty(false),
  m_prevOpWasRead(true)
  {}

  ~mc24xx_nn() ESE_NOTHROW
  {
    commit();
  }

  /// Access assigned I2C bus object
  inline EseI2c& busGet() ESE_NOTHROW { return m_i2c; }

  /// Return device address
  inline esU8 addrGet() const ESE_NOTHROW { return m_addr; }

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
  void commit() ESE_NOTHROW;

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
  /// 
  void pageRead(esU16 pageAddr) ESE_NOTHROW;

  /// Write current page buffer to the flash
  void pageWrite() ESE_NOTHROW;

protected:
  EseI2c& m_i2c;
  esU8 m_buff[pageSize];
  esU32 m_wts;      ///< Timestamp of the recent write operation request
  esU16 m_pageAddr; ///< An address of the currenly buffered page
  esU8 m_addr;
  bool m_dirty;     ///< Page buffer is dirty, pending commit required
  bool m_prevOpWasRead; ///< Previous flash access operation was read one

private:
  mc24xx_nn();
  mc24xx_nn(const mc24xx_nn<KbitSize, PageSize>&);
  mc24xx_nn& operator=(const mc24xx_nn<KbitSize, PageSize>&);
};

/// Chip type specializations
///
typedef mc24xx_nn<256, 64> mc24xx256;
typedef mc24xx_nn<512, 128> mc24xx512;

// ------------------------------------------------------------------------------------------------------------------------------
// Template implementation
template <size_t KbitSize, size_t PageSize>
esU32 mc24xx_nn<KbitSize, PageSize>::write(esU16 memAddr, const esU8* data, esU32 dataLen, bool autocommit /*= false*/) ESE_NOTHROW
{
  if(data && dataLen)
  {
    esU16 pageAddr, pageOffs;
    pageAddrFromMemAddr(memAddr, pageAddr, pageOffs);

    const esU8* pos = data;
    const esU8* dend = data + dataLen;
    while( pos < dend && pageAddr <= pageMaxAddr )
    {
      pageRead(pageAddr);   // Read page, just in case
      // transfer data into page buffer by chunks, of at most pageSize bytes length
      esU8* dest = m_buff + pageOffs;
      size_t writeChunk = MIN(pageSize-pageOffs, dend-pos);
      memcpy(dest, pos, writeChunk);
      m_dirty = true;       // Mark currently buffered page as dirty

      pos += writeChunk;    // Move to the next portion of data
      pageAddr += pageSize; // Next cycle (if any) will write the next page
      pageOffs = 0;         // Next pages (if any) got written from the start
    }
    
    // Perform final commit, if autocommit enabled
    if(autocommit)
      commit();

    return pos-data;
  }

  return 0;
}

template <size_t KbitSize, size_t PageSize>
esU32 mc24xx_nn<KbitSize, PageSize>::read(esU16 memAddr, esU8* data, esU32 dataLen) ESE_NOTHROW
{
  if(data && dataLen)
  {
    esU16 pageAddr, pageOffs;
    pageAddrFromMemAddr(memAddr, pageAddr, pageOffs);

    esU8* pos = data;
    esU8* dend = data + dataLen;
    while( pos < dend && pageAddr <= pageMaxAddr )
    {
      pageRead(pageAddr);   // Read page, just in case
      // transfer data from page buffer by chunks, of at most pageSize bytes length
      const esU8* src = m_buff + pageOffs;
      size_t readChunk = MIN(pageSize-pageOffs, dend-pos);
      memcpy(pos, src, readChunk);

      pos += readChunk;    // Move to the next portion of data
      pageAddr += pageSize; // Next cycle (if any) will read the next page
      pageOffs = 0;         // Next pages (if any) got read from the start
    }

    return pos-data;
  }

  return 0;
}

template <size_t KbitSize, size_t PageSize>
void mc24xx_nn<KbitSize, PageSize>::commit() ESE_NOTHROW
{
  if( m_dirty )
  {
    pageWrite();
    m_dirty = false;
  }
}

template <size_t KbitSize, size_t PageSize>
void mc24xx_nn<KbitSize, PageSize>::pageAddrFromMemAddr(esU16 memAddr, esU16& pageAddr, esU16& pageOffs) ESE_NOTHROW
{
  ES_ASSERT( memAddr < totalBytes );
  pageAddr = (memAddr / pageSize) * pageSize;
  pageOffs = memAddr % pageSize;
}

template <size_t KbitSize, size_t PageSize>
void mc24xx_nn<KbitSize, PageSize>::pageRead(esU16 pageAddr) ESE_NOTHROW
{
  ES_ASSERT(0 == pageAddr % pageSize);
  ES_ASSERT(pageAddr <= pageMaxAddr);
  
  if( pageAddr != m_pageAddr )
  {
    // Commit currently buffered page
    commit();
    
    // Wait until previous write operation is timed out
    if( !m_prevOpWasRead )
      EseTask::sleepUntil(m_wts, ackTmo);

    // Read new page into buffer, with retries and extra ack waiting
    size_t rc = pageSize;
    rc = m_i2c.masterMemReceive(m_addr, pageAddr, 2, m_buff, rc, ackTmo);
    m_prevOpWasRead = true;

    ES_ASSERT(rc == pageSize);  
    
    // Update current page addr
    m_pageAddr = pageAddr;
  }
}

template <size_t KbitSize, size_t PageSize>
void mc24xx_nn<KbitSize, PageSize>::pageWrite() ESE_NOTHROW
{
  if( sc_noAddr != m_pageAddr )
  {
    // Wait until previous write operation is timed out
    if( !m_prevOpWasRead )
      EseTask::sleepUntil(m_wts, ackTmo);

    // Write currently buffered page
    size_t wc = pageSize;
    wc = m_i2c.masterMemSend(m_addr, m_pageAddr, 2, m_buff, wc, ackTmo);
    m_wts = EseTask::tickCountGet();
    m_prevOpWasRead = false;

    ES_ASSERT(wc == pageSize);
  }
}

#endif // _mc_24xx_nn_h_
