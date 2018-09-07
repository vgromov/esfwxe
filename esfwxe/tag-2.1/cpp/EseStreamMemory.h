#ifndef _ese_stream_memory_h_
#define _ese_stream_memory_h_

/// Memory stream concept implementation
///
class EseStreamMemory : public EseStream
{
public:
  EseStreamMemory(esU8* mem, size_t cnt) ESE_NOTHROW;
  virtual ~EseStreamMemory() ESE_NOTHROW;

  /// Interface deallocator
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE;
  
  /// EseStreamIntf interface implementation
  ///
  virtual size_t sizeGet() const ESE_NOTHROW { return m_end-m_start; }
  virtual size_t posGet() const ESE_NOTHROW;
  virtual size_t posSet(ptrdiff_t offs, EseStream::PosMode mode = EseStream::posFromStart) ESE_NOTHROW;
  virtual size_t read(esU8* data, size_t len) ESE_NOTHROW;
  virtual size_t write(const esU8* data, size_t len) ESE_NOTHROW;
  virtual void flush() ESE_NOTHROW {}

protected:
  virtual bool doOpen() ESE_NOTHROW { return true; }
  virtual void doClose() ESE_NOTHROW {}

protected:
  esU8* m_start;
  esU8* m_end;
  esU8* m_pos;
};

#endif // _ese_stream_memory_h_
