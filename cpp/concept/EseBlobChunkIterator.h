#ifndef _ese_blob_chunk_iterator_h_
#define _ese_blob_chunk_iterator_h_

#if !defined(ESE_MEMINTF_ACCESS)
# define ESE_MEMINTF_ACCESS
#endif

/// BLOB Chunk iterator helper class
///
template <typename OffsT, typename SizeT, int ChunkSizeMax>
class EseBlobChunkIterator
{
public:
  typedef EseBlobChunkIterator<OffsT, SizeT, ChunkSizeMax> ThisT;

  /// Multi-packet BLOB chunk iterator
  class Chunk
  {
  public:
    struct Hdr 
    {
      OffsT m_offs;
      SizeT m_size;
    };
  
    enum { 
      chunkSizeMax      = ChunkSizeMax,
      headerSize        = sizeof(Hdr),
      payloadSizeMax    = chunkSizeMax-headerSize
    };
 
  protected:
    Chunk( EseBlobChunkIterator& owner ) ESE_NOTHROW :
    m_owner(owner)
    {
      m_data.m_hdr.m_offs = 0;
      m_data.m_hdr.m_size = 0;
    }

  public:
    /// Return true, if IO operations may be applied to the chunk iterator
    inline bool isOk() const ESE_NOTHROW { return !m_owner.isEmpty(); }
    
    /// Return true if chunk may be the lase one
    inline bool isLast() const ESE_NOTHROW { return 0 == m_data.m_hdr.m_size; }
    
    /// Return true if chunk is positioned at the beginning of source (zero offset) 
    inline bool isFirst() const ESE_NOTHROW { return 0 == m_data.m_hdr.m_offs; }
    
    /// Get current chunk offset value
    inline OffsT offsGet() const ESE_NOTHROW { return m_data.m_hdr.m_offs; }
    
    /// Get current chunk size value
    inline size_t payloadSizeGet() const ESE_NOTHROW { return m_data.m_hdr.m_size; }
    
    /// Get entire chunk size, including header
    inline size_t sizeGet() const ESE_NOTHROW { return headerSize+m_data.m_hdr.m_size; }
    
    /// Return pointer to the beginning of the chunk payload
    inline const uint8_t* payloadGet() const ESE_NOTHROW { return m_data.m_raw+headerSize; }

    /// Read only access to raw chunk block
    inline const uint8_t* rawGet() const ESE_NOTHROW { return m_data.m_raw; }
    
    /// Writeable access to entire raw chunk block
    inline uint8_t* rawGet() ESE_NOTHROW { return m_data.m_raw; }
    
    /// Prepare the next BLOB chunk for read
    bool nextRead() ESE_NOTHROW;
    
    /// Prepare the next BLOB chunk for write
    bool nextWrite() ESE_NOTHROW;
    
    /// Reset chunk reading|writing iteration to the beginning
    void reset() ESE_NOTHROW;
 
     /// Reset chunk reading|writing iteration to an end
    void resetEnd() ESE_NOTHROW;
 
  protected:
    /// Internal services
    ///

    /// Return calculated chunk end position
    inline uint32_t endPosGet() const ESE_NOTHROW { return offsGet() + payloadSizeGet(); }

    /// Return writeable pointer to the beginning of the chunk payload
    inline uint8_t* payloadGet() ESE_NOTHROW { return m_data.m_raw+headerSize; }
    
    /// Update current chunk contents from data source
    void updateFromSource() ESE_NOTHROW;

    /// Try to update source data from the current chunk. Return true, if update was OK, false otherwise
    bool updateSource() ESE_NOTHROW;

  private:
    EseBlobChunkIterator& m_owner;
#pragma pack(push, 1)
    union {
      Hdr m_hdr;
      uint8_t m_raw[chunkSizeMax];
      
    } m_data;
#pragma pack(pop)

    ESE_NODEFAULT_CTOR(Chunk);
    ESE_NONCOPYABLE(Chunk);
    
    friend class EseBlobChunkIterator;
  };
  friend class Chunk;

public:
  EseBlobChunkIterator( uint8_t* src = nullptr, size_t srcSize = 0, bool owns = false, bool readOnly = false ) ESE_NOTHROW;
  
  ~EseBlobChunkIterator() ESE_NOTHROW;
  
  /// Return true if BLOB is read only
  bool isReadOnly() const ESE_NOTHROW { return m_readOnly; }
  
  /// Return true if BLOB is empty
  bool isEmpty() const ESE_NOTHROW { return nullptr == m_src || 0 == m_srcSize; }
  
  /// Chunk interface access
  EseBlobChunkIterator::Chunk& chunk() ESE_NOTHROW { return m_chunk; }
  const EseBlobChunkIterator::Chunk& chunk() const ESE_NOTHROW { return m_chunk; }
  
  /// Assign source data to the BLOB.
  void sourceSet(uint8_t* src, size_t srcSize, bool owns, bool readOnly) ESE_NOTHROW;
  void sourceSet(const uint8_t* src, size_t srcSize, bool owns) ESE_NOTHROW;
  
  template <typename SrcT>
  void sourceSet(SrcT* src, bool owns, bool readOnly) ESE_NOTHROW
  {
    sourceSet(
      reinterpret_cast<uint8_t*>(src), 
      sizeof(SrcT), 
      owns, 
      readOnly
    );
  }

  template <typename SrcT>
  void sourceSet(const SrcT* src, bool owns) ESE_NOTHROW
  {
    sourceSet(
      reinterpret_cast<const uint8_t*>(src), 
      sizeof(SrcT), 
      owns
    );
  }

  /// Direct access BLOB source
  uint8_t* sourceAccess() ESE_NOTHROW { return m_src; }
  
  /// Get BLOB source size
  size_t sourceSizeGet() const ESE_NOTHROW { return m_srcSize; }

  void cleanup() ESE_NOTHROW;
  
protected:
  uint8_t* m_src;
  size_t m_srcSize;
  bool m_ownsSrc;
  bool m_readOnly;
  Chunk m_chunk;
  
  ESE_NONCOPYABLE(EseBlobChunkIterator);
};

// Template implementation
#include "EseBlobChunkIterator.ipp"

#endif // _ese_blob_chunk_iterator_h_
