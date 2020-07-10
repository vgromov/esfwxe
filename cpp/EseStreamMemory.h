#ifndef _ese_stream_memory_h_
#define _ese_stream_memory_h_

// Forward decl
class EseMutex;

/// Memory stream concept implementation
///
class EseStreamMemory : public EseStream
{
public:
  EseStreamMemory(esU8* mem, size_t cnt, bool lockable = false) ESE_NOTHROW;
  virtual ~EseStreamMemory() ESE_NOTHROW ESE_KEEP;
 
  /// EseStreamIntf interface implementation
  ///
  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual rtosStatus lock(esU32 tmo = rtosMaxDelay) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void unlock() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t sizeGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP { return m_end-m_start; }
  virtual size_t posGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t posSet(ptrdiff_t offs, EseStream::PosMode mode = EseStream::posFromStart) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t read(esU8* data, size_t len) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual size_t write(const esU8* data, size_t len) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void flush() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP {}

protected:
  virtual bool doOpen() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP{ return true; }
  virtual void doClose() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP {}

protected:
  EseMutex* m_mx;
  esU8* m_start;
  esU8* m_end;
  esU8* m_pos;
};

#endif // _ese_stream_memory_h_
