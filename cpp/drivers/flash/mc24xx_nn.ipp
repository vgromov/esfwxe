#ifndef _mc_24_xx_nn_ipp_
#define _mc_24_xx_nn_ipp_

// This file is implicitly included from mc24xx_nn.h
// Do not include it directly into project.
//

#ifndef ESE_MEMAPI_ACCESS
# define ESE_MEMAPI_ACCESS      std::
#endif

#ifndef ESE_OSAPI_ACCESS
# define ESE_OSAPI_ACCESS       EseKernel::
#endif

#ifndef ESE_TASK_TICKS_ACCESS
# define ESE_TASK_TICKS_ACCESS  EseTask::tickCountGet()
#endif

// Template implementation
template <size_t KbitSize, size_t PageSize>
esU32 mc24xx_nn<KbitSize, PageSize>::write(esU16 memAddr, const esU8* data, esU32 dataLen, bool autocommit /*= false*/) ESE_NOTHROW
{
  if(data && dataLen)
  {
    esU16 pageAddr, pageOffs;
    pageAddrFromMemAddr(
      memAddr, 
      pageAddr, 
      pageOffs
    );

    const esU8* pos = data;
    const esU8* dend = data + dataLen;
    while( pos < dend && pageAddr <= pageMaxAddr )
    {
      if( !pageRead(pageAddr) )   // Read page, just in case
        break;

      // transfer data into page buffer by chunks, of at most pageSize bytes length
      esU8* dest = m_buff + pageOffs;
      size_t writeChunk = MIN(pageSize-pageOffs, dend-pos);

      ESE_MEMAPI_ACCESS
      memcpy(dest, pos, writeChunk);

      m_dirty = true;       // Mark currently buffered page as dirty

      pos += writeChunk;    // Move to the next portion of data
      pageAddr += pageSize; // Next cycle (if any) will write the next page
      pageOffs = 0;         // Next pages (if any) got written from the start
    }
    
    // Perform final commit, if autocommit enabled
    if(autocommit)
    {
      if( !commit() )
        return 0;
    }
    
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
    pageAddrFromMemAddr(
      memAddr, 
      pageAddr, 
      pageOffs
    );

    esU8* pos = data;
    esU8* dend = data + dataLen;
    while( pos < dend && pageAddr <= pageMaxAddr )
    {
      if( !pageRead(pageAddr) )   // Read page, just in case
        break;
      
      // transfer data from page buffer by chunks, of at most pageSize bytes length
      const esU8* src = m_buff + pageOffs;
      size_t readChunk = MIN(pageSize-pageOffs, dend-pos);

      ESE_MEMAPI_ACCESS
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
bool mc24xx_nn<KbitSize, PageSize>::commit() ESE_NOTHROW
{
  if( m_dirty )
    m_dirty = !pageWrite();
    
  return !m_dirty;
}

template <size_t KbitSize, size_t PageSize>
void mc24xx_nn<KbitSize, PageSize>::pageAddrFromMemAddr(esU16 memAddr, esU16& pageAddr, esU16& pageOffs) ESE_NOTHROW
{
  ES_ASSERT( memAddr < totalBytes );
  pageAddr = (memAddr / pageSize) * pageSize;
  pageOffs = memAddr % pageSize;
}

template <size_t KbitSize, size_t PageSize>
bool mc24xx_nn<KbitSize, PageSize>::pageRead(esU16 pageAddr) ESE_NOTHROW
{
  ES_ASSERT(0 == pageAddr % pageSize);
  ES_ASSERT(pageAddr <= pageMaxAddr);
  
  if( pageAddr != m_pageAddr )
  {
    // Commit currently buffered page
    if( !commit() )
      return false;
    
    // Wait until previous write operation is timed out
    if( !m_prevOpWasRead )
    {
      ESE_OSAPI_ACCESS
      sleepUntil(
        m_wts, 
        m_tmo
      );
    }

#ifdef DEBUG
    bool prepOk = 
#endif    
    
    chnlMemIoPrepare(
      pageAddr,
      2
    );
    
#ifdef DEBUG
    ESE_ASSERT(prepOk);
#endif

    // Read new page into buffer, with retries and extra ack waiting
    size_t rc = 0;
    uint32_t retries = m_ackPollRetries;
    do
    {
      rc = m_i2c->receive(
        m_buff, 
        pageSize, 
        m_tmo
      );
    
      if( EseI2cChannelError::addressNacked == m_i2c->errorGet() && retries ) //< Address is NACK-ed by controller, must retry query 
      {
        if( retries > 1 )
        {
          ESE_OSAPI_ACCESS
          sleep(1);
        }
        
        --retries;
      }
      else
        break;
    
    } while( retries );
    
    m_prevOpWasRead = true;

    if(rc != pageSize)
      return false;
      
    m_pageAddr = pageAddr; //< Update current page addr
  }
  
  return true;
}

template <size_t KbitSize, size_t PageSize>
bool mc24xx_nn<KbitSize, PageSize>::pageWrite() ESE_NOTHROW
{
  if( sc_noAddr != m_pageAddr )
  {
    // Wait until previous write operation is timed out
    if( !m_prevOpWasRead )
    {
      ESE_OSAPI_ACCESS
      sleepUntil(
        m_wts, 
        m_tmo
      );
    }
    
#ifdef DEBUG
    bool prepOk = 
#endif    
    
    chnlMemIoPrepare(
      m_pageAddr,
      2
    );
    
#ifdef DEBUG
    ESE_ASSERT(prepOk);
#endif

    // Write currently buffered page
    size_t wc = 0;
    uint32_t retries = m_ackPollRetries;
    do
    {
      wc = m_i2c->send(
        m_buff, 
        pageSize, 
        m_tmo
      );
    
      if( EseI2cChannelError::addressNacked == m_i2c->errorGet() && retries ) //< Address is NACK-ed by controller, must retry query 
      {
        if( retries > 1 )
        {
          ESE_OSAPI_ACCESS
          sleep(1);
        }
          
        --retries;
      }
      else
        break;
    
    } while( retries );
    
    m_wts = ESE_TASK_TICKS_ACCESS;

    m_prevOpWasRead = false;

    return (wc == pageSize);
  }
  
  return true;
}

#endif //< _mc_24_xx_nn_ipp_
