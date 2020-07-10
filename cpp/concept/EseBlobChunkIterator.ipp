/// EseBlobChunkIterator template implementation.
/// Implicitly included from EseBlobChunkIterator.h
///
#define BLOB_MEMINTF_ACCESS ESE_MEMINTF_ACCESS

template <typename OffsT, typename SizeT, int ChunkSizeMax>
bool EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::nextRead() ESE_NOTHROW
{
  if( isOk() )
  {
    m_data.m_hdr.m_offs += payloadSizeGet();
    m_data.m_hdr.m_size = MIN(
      m_owner.m_srcSize-offsGet(), 
      static_cast<size_t>(payloadSizeMax)
    );
      
    updateFromSource();
    
    return 0 != sizeGet();
  }
  
  return false;
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
bool EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::nextWrite() ESE_NOTHROW
{
  if( 
    !m_owner.isReadOnly() &&
    isOk() 
  )
    return updateSource();
  
  return false;
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::reset() ESE_NOTHROW
{
  m_data.m_hdr.m_size = MIN(
    m_owner.m_srcSize, 
    static_cast<size_t>(payloadSizeMax)
  );
  m_data.m_hdr.m_offs = 0;
    
  updateFromSource();
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::resetEnd() ESE_NOTHROW
{
  m_data.m_hdr.m_size = 0;
  m_data.m_hdr.m_offs = m_owner.m_srcSize;
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::updateFromSource() ESE_NOTHROW
{
  if( payloadSizeGet() )
  {
    ES_ASSERT( payloadSizeGet() <= payloadSizeMax );
    ES_ASSERT( endPosGet() <= m_owner.sourceSizeGet() );

    ESE_MEMINTF_ACCESS
    memcpy( 
      payloadGet(), 
      m_owner.m_src + offsGet(), 
      payloadSizeGet() 
    );
  }
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
bool EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::Chunk::updateSource() ESE_NOTHROW
{
  if( payloadSizeGet() )
  {
    // Check header size validity for write
    if( 
      payloadSizeGet() <= payloadSizeMax &&
      endPosGet() <= m_owner.sourceSizeGet() 
    )
    {
      // Write data payload
      BLOB_MEMINTF_ACCESS
      memcpy( 
        m_owner.m_src + offsGet(), 
        payloadGet(), 
        payloadSizeGet() 
      );
    }
    else
    {
      m_data.m_hdr.m_offs = 0;
      m_data.m_hdr.m_size = 0;
      
      return false;
    }
  }

  return true; ///< Nothing to update from, just return true
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::EseBlobChunkIterator( uint8_t* src /*= nullptr*/, uint32_t srcSize /*= 0*/, bool owns /*= false*/, bool readOnly /*= false*/ ) ESE_NOTHROW :
m_src(nullptr),
m_srcSize(0),
m_ownsSrc(false),
m_readOnly(false),
m_chunk(*this)
{
  sourceSet(
    src, 
    srcSize, 
    owns, 
    readOnly
  );
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::~EseBlobChunkIterator() ESE_NOTHROW
{
  cleanup();
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::sourceSet(uint8_t* src, uint32_t srcSize, bool owns, bool readOnly) ESE_NOTHROW
{
  if( 
    m_srcSize != srcSize ||
    m_ownsSrc != owns 
  )
    cleanup();

  m_ownsSrc = owns;
  if( owns )
  {
    if( srcSize )
    {
      m_src = reinterpret_cast<esU8*>(
        BLOB_MEMINTF_ACCESS
        malloc(srcSize)
      );

      ES_ASSERT(m_src);
    }
    
    if( src && srcSize )
      BLOB_MEMINTF_ACCESS
      memcpy(
        m_src, 
        src, 
        srcSize
      );
    else
      BLOB_MEMINTF_ACCESS
      memset(
        m_src, 
        0, 
        srcSize
      );
  }
  else
    m_src = src;
    
  m_srcSize = srcSize;
  m_readOnly = readOnly;

  m_chunk.reset();
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::sourceSet(const uint8_t* src, uint32_t srcSize, bool owns) ESE_NOTHROW
{
  sourceSet(
    const_cast<uint8_t*>(src),
    srcSize,
    owns,
    true //< Read only for const source
  );
}

template <typename OffsT, typename SizeT, int ChunkSizeMax>
void EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax>::cleanup() ESE_NOTHROW
{
  if( m_ownsSrc )
    BLOB_MEMINTF_ACCESS
    free( m_src );
    
  m_src = nullptr;
  m_srcSize = 0;
  m_chunk.reset();
}

#undef BLOB_MEMINTF_ACCESS
