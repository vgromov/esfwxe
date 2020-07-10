#ifndef _ese_math_val_stat_intf_h_
#define _ese_math_val_stat_intf_h_

#ifdef ESE_USE_STATS_SERIALIZATION
# if !defined(ESE_USE_STATS_SERIALIZATION_READ) && !defined(ESE_USE_STATS_SERIALIZATION_WRITE)
#   undef ESE_USE_STATS_SERIALIZATION
# else
    /// Foward declarations
    class EseStreamIntf;
# endif
#else
# undef ESE_USE_STATS_SERIALIZATION_READ
# undef ESE_USE_STATS_SERIALIZATION_WRITE
#endif

/// Value statistics calculator interface
///
class ESE_ABSTRACT EseMathValStatIntf
{
public:
  virtual void destroy() ESE_NOTHROW = 0;

  virtual bool isComplete() const ESE_NOTHROW = 0;
  virtual bool avgIsComplete() const ESE_NOTHROW = 0;
  virtual bool uaIsComplete() const ESE_NOTHROW = 0;

  virtual void reset() ESE_NOTHROW = 0;
  virtual void valAppend(esF val) ESE_NOTHROW = 0;
  virtual void avgFinalize() ESE_NOTHROW = 0;
  virtual void uaValAppend(esF val) ESE_NOTHROW = 0;
  virtual void finalize(esF deviceErr, bool errIsRel) ESE_NOTHROW = 0;
  
  virtual esF minGet() const ESE_NOTHROW = 0;
  virtual esF maxGet() const ESE_NOTHROW = 0;
  virtual esF avgGet() const ESE_NOTHROW = 0;
  virtual esF uaGet() const ESE_NOTHROW = 0;
  virtual esF ubGet() const ESE_NOTHROW = 0;
  virtual esF ucGet() const ESE_NOTHROW = 0;
  virtual esF uGet() const ESE_NOTHROW = 0;
  virtual esU32 cntGet() const ESE_NOTHROW = 0;
  
#ifdef ESE_USE_STATS_SERIALIZATION
# if defined(ESE_USE_STATS_SERIALIZATION_READ) && 0 != ESE_USE_STATS_SERIALIZATION_READ
  virtual bool readFrom(EseStreamIntf& in) ESE_NOTHROW = 0;
# endif
# if defined(ESE_USE_STATS_SERIALIZATION_WRITE) && 0 != ESE_USE_STATS_SERIALIZATION_WRITE
  virtual bool writeTo(EseStreamIntf& out) const ESE_NOTHROW = 0;
# endif
#endif
};

#endif //< _ese_math_val_stat_intf_h_
