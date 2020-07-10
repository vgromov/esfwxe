#ifndef _ese_math_val_stat_impl_h_
#define _ese_math_val_stat_impl_h_

/// Value statistics calculator interface implementation
///
class EseMathValStatIntfImpl : public EseMathValStatIntf
{
public:
  EseMathValStatIntfImpl() ESE_NOTHROW;

  virtual void destroy() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual bool isComplete() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool avgIsComplete() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual bool uaIsComplete() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual void reset() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void valAppend(esF val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void avgFinalize() ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void uaValAppend(esF val) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual void finalize(esF deviceErr, bool errIsRel) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  
  virtual esF minGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF maxGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF avgGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF uaGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF ubGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF ucGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esF uGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
  virtual esU32 cntGet() const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;

#ifdef ESE_USE_STATS_SERIALIZATION
# if defined(ESE_USE_STATS_SERIALIZATION_READ) && 0 != ESE_USE_STATS_SERIALIZATION_READ
  virtual bool readFrom(EseStreamIntf& in) ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
# endif
# if defined(ESE_USE_STATS_SERIALIZATION_WRITE) && 0 != ESE_USE_STATS_SERIALIZATION_WRITE
  virtual bool writeTo(EseStreamIntf& out) const ESE_NOTHROW ESE_OVERRIDE ESE_KEEP;
# endif
#endif

protected:
  EseMathValStat m_stat;
};

#endif //< _ese_math_val_stat_impl_h_
