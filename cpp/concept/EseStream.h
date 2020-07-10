#ifndef _ese_stream_h_
#define _ese_stream_h_

/// IO Stream object interface partial implementation
///
class EseStream : public EseStreamIntf
{
public:
  /// Special value - invalid stream position
  static const size_t npos = static_cast<size_t>(-1);

public:
  EseStream() ESE_NOTHROW ESE_KEEP;
  virtual ~EseStream() ESE_NOTHROW ESE_KEEP;

  /// EseStreamIntf services
  ///
  virtual size_t invalidPos() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  /// Return true if stream is open, false otherwise
  virtual bool isOpen() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  /// Return true if stream position is beyond stream size
  virtual bool isEos() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

  /// Open stream object, return true, if open succeed, false otherwise
  virtual bool open() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  /// Close currently opened stream object
  virtual void close() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

protected:
  virtual bool doOpen() ESE_NOTHROW = 0;
  virtual void doClose() ESE_NOTHROW {}
  
protected:
  bool m_open;
  
  ESE_NONCOPYABLE(EseStream);
};

#endif // _ese_stream_h_
