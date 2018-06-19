#ifndef _ese_stream_h_
#define _ese_stream_h_

/// IO Stream object interface
///
class EseStream
{
public:
  /// Stream positioning modes
  ///
  enum PosMode {
    posFromStart, ///< Position is defined as an offset in bytes from the start of the stream
    posFromEnd,   ///< Position is defined as an offset in bytes from the end of the stream
    posRelative   ///< Position is defined as a relative offset in bytes from the current position
  };
  
  /// Special value - invalid stream position
  static const size_t npos = static_cast<size_t>(-1);

public:
  EseStream() ESE_NOTHROW;
  virtual ~EseStream() ESE_NOTHROW {}

  /// Stream services
  ///

  /// Return true if stream is open, false otherwise
  inline bool isOpen() const ESE_NOTHROW { return m_open; }

  /// Return true if stream position is beyond stream size
  inline bool isEos() const ESE_NOTHROW 
  { 
    size_t pos = posGet(); 
    return npos != pos && 
      pos >= sizeGet(); 
  }

  /// Open stream object, return true, if open succeed, false otherwise
  bool open() ESE_NOTHROW;
  
  /// Close currently opened stream object
  void close() ESE_NOTHROW;

  /// Stream interface to be implemented in derived classes
  ///
  
  /// Return the size of the stream in bytes. 
  virtual size_t sizeGet() const ESE_NOTHROW = 0;
  
  /// Return the current stream position (as an offset from the stream start), in bytes
  /// If stream is closed, always return npos
  virtual size_t posGet() const ESE_NOTHROW = 0;
  
  /// Set stream position, using specified offset and positioning mode
  /// If stream is closed, positioning request does nothing, and npos is returned
  /// @param offs   [in] requested offset, in bytes
  /// @param mode   [in] requested positioning mode, default counts from the start of the stream
  /// @return            new stream position, if stream is open, npos otherwise
  ///
  virtual size_t posSet(ptrdiff_t offs, PosMode mode = posFromStart) ESE_NOTHROW = 0;
  
  /// Read data from stream. 
  /// On read, stream position is incremented by amount of bytes successfully read
  /// @param data   [out] buffer to receive read data
  /// @param len    [in]  receiving buffer length in bytes
  /// @return             count of bytes read.
  ///
  virtual size_t read(esU8* data, size_t len) ESE_NOTHROW = 0;

  /// Write data to stream. 
  /// On write, stream position is incremented by amount of bytes successfully written
  /// @param data   [in] buffer containing data to be written
  /// @param len    [in] data buffer length in bytes
  /// @return            count of bytes written.
  ///
  virtual size_t write(const esU8* data, size_t len) ESE_NOTHROW = 0;

  /// Flush data stream. Functionality is implementation-dependent.
  /// Some implementations may do nothing, while other may perform 
  /// optional commits of 'dirty' data buffers.
  ///
  virtual void flush() ESE_NOTHROW = 0;

protected:
  virtual bool doOpen() ESE_NOTHROW = 0;
  virtual void doClose() ESE_NOTHROW {}
  
protected:
  bool m_open;
  
private:
  EseStream(const EseStream&);
  EseStream& operator=(const EseStream&);
};

#endif // _ese_stream_h_
