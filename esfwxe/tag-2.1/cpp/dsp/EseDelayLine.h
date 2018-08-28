#ifndef _ese_delay_line_h_
#define _ese_delay_line_h_

#include <esfwxe/cpp/concept/EseRingBuffer.h>

/// Delay line implementation, with averageing, based on ringbuffer concept
///
template <typename DataT, typename AccumDataT, esU32 BuffSize>
class EseDelayLine
{
protected:
  typedef EseRingBuffer<DataT, BuffSize> BuffT;

public:
  static const esU32 length = BuffT::length;

public:
  EseDelayLine() ESE_NOTHROW : 
  m_accum( static_cast<AccumDataT>(0) )
  {}

  /// Return true if delay line is empty
  inline bool isEmpty() const ESE_NOTHROW { return m_buff.isEmpty(); }

  /// Return true if delay line is saturated
  inline bool isSaturated() const ESE_NOTHROW { return m_buff.isFull(); }

  /// Return count of elements in delay line
  inline esU32 countGet() const ESE_NOTHROW { return m_buff.countGet(); }

  /// Reset delay line state data
  inline void flush() ESE_NOTHROW
  {
    m_accum = static_cast<AccumDataT>(0);
    m_buff.flush();
  }
  
  /// Push data to the delay line
  inline void pushBack(DataT val) ESE_NOTHROW
  {
    if( m_buff.isFull() )
    {
      DataT old;
      m_buff.popFront(old);
      m_accum -= static_cast<AccumDataT>(old);
    }
    
    m_buff.pushBack(val);
    m_accum += static_cast<AccumDataT>(val);
  }
  
  /// Get result value from the delay line
  inline DataT valueGet() const ESE_NOTHROW
  {
    AccumDataT n = static_cast<AccumDataT>(m_buff.countGet());
    if( static_cast<AccumDataT>(0) == n )
      n = static_cast<AccumDataT>(1);

    return static_cast<DataT>(m_accum / n);
  }
  
  /// Peek data at specified index
  inline DataT operator[](esU32 idx) const ESE_NOTHROW
  {
    DataT result;
    if( m_buff.peekIdx(idx, &result) )
      return result;

    return static_cast<DataT>(0);
  }

  /// Return accumulated data value
  inline AccumDataT accumGet() const ESE_NOTHROW
  {
    return m_accum;
  }

protected:
  BuffT m_buff;
  AccumDataT m_accum;
};

/// Delay line extended implementation, with accumulated minimax calculation.
/// Accumulated minimax may be reset both manually, and automatically, by
/// accumulator counter overflow.
///
template <typename DataT, typename AccumDataT, esU32 BuffSize>
class EseDelayLineEx
{
protected:
  typedef EseRingBuffer<DataT, BuffSize> BuffT;

public:
  static const esU32 length = BuffT::length;

public:
  EseDelayLineEx() ESE_NOTHROW : 
  m_accum( static_cast<AccumDataT>(0) ),
  m_min( static_cast<DataT>(0) ),
  m_max( static_cast<DataT>(0) ),
  m_minimaxCnt( 0 ),
  m_minimaxResetThreshold( 0xFFFFFFFF ),
  m_minimaxReset( true )
  {}

  /// Return true if delay line is empty
  inline bool isEmpty() const ESE_NOTHROW { return m_buff.isEmpty(); }

  /// Return true if delay line is saturated
  inline bool isSaturated() const ESE_NOTHROW { return m_buff.isFull(); }
  
  /// Return count of elements in delay line
  inline esU32 countGet() const ESE_NOTHROW { return m_buff.countGet(); }

  /// Reset delay line state data
  inline void flush() ESE_NOTHROW
  {
    m_buff.flush();
    m_accum = static_cast<AccumDataT>(0);
    minimaxReset();
  }
  
  /// Push data to the delay line, calculate accumulated minimax
  inline void pushBack(DataT val) ESE_NOTHROW
  {
    // Automatic minimax reset
    if( m_minimaxResetThreshold <= m_minimaxCnt )
      minimaxReset();

    if( m_buff.isEmpty() )
    {
      m_min = val;
      m_max = val;
      m_minimaxReset = false;
    }
    else
    {
      calcMinimaxIfReset();
      
      if( val > m_max )
        m_max = val;
      if( val < m_min )
        m_min = val;
    }

    if( m_buff.isFull() )
    {
      DataT old;
      m_buff.popFront(old);
      m_accum -= static_cast<AccumDataT>(old);
    }
    
    m_buff.pushBack(val);
    m_accum += static_cast<AccumDataT>(val);

    ++m_minimaxCnt;
  }
  
  /// Get result value from the delay line
  inline DataT valueGet() const ESE_NOTHROW
  {
    AccumDataT n = static_cast<AccumDataT>(m_buff.countGet());
    if( static_cast<AccumDataT>(0) == n )
      n = static_cast<AccumDataT>(1);

    return static_cast<DataT>(m_accum / n);
  }
  
  /// Peek data at specified index
  inline DataT operator[](esU32 idx) const ESE_NOTHROW
  {
    DataT result;
    if( m_buff.peekIdx(idx, &result) )
      return result;

    return static_cast<DataT>(0);
  }

  /// Return accumulated data value
  inline AccumDataT accumGet() const ESE_NOTHROW
  {
    return m_accum;
  }

  /// Return calculated data minimum
  inline DataT minGet() const ESE_NOTHROW
  { 
    return m_min;
  }
  
  /// Return calculated data maximum
  inline DataT maxGet() const ESE_NOTHROW
  { 
    return m_max; 
  }

  /// Return count of nodes over which minimax values were accumulated
  inline esU32 minimaxCntGet() const ESE_NOTHROW
  {
    return m_minimaxCnt;
  }

  /// Reset minimax accumulator
  inline void minimaxReset() ESE_NOTHROW
  {
    m_minimaxReset = true;
    m_minimaxCnt = 0;
    if( m_buff.isEmpty() )
      m_min = m_max = static_cast<DataT>(0);
  }

  /// Set minimax autoreset threshold
  inline void minimaxResetThresholdSet(esU32 threshold) ESE_NOTHROW
  {
    if( 0 == threshold )
      threshold = 0xFFFFFFFF;
    
    if( threshold != m_minimaxResetThreshold )
    {
      m_minimaxResetThreshold = threshold;
      minimaxReset();
    }
  }

  /// Return current minimax reset threshold
  inline esU32 minimaxResetThresholdGet() const ESE_NOTHROW
  {
    return m_minimaxResetThreshold;
  }

protected:
  // Run minimax once on existing nodes
  inline void calcMinimaxIfReset() ESE_NOTHROW
  {
    if( m_minimaxReset )
    {
      typedef typename BuffT::ConstForwardIterator ConstForwardIteratorT;
      ConstForwardIteratorT cit = m_buff.begin();
      m_min = m_max = (*cit++);
      ++m_minimaxCnt;
      while( cit != m_buff.end() )
      {
        DataT v = (*cit++);
        if( m_min > v )
          m_min = v;
        if( m_max < v )
          m_max = v;
        ++m_minimaxCnt;
      }

      m_minimaxReset = false;
    }
  }

protected:
  BuffT m_buff;
  AccumDataT m_accum;
  DataT m_min;
  DataT m_max;
  esU32 m_minimaxCnt;
  esU32 m_minimaxResetThreshold;
  bool m_minimaxReset;
};

#endif // _ese_delay_line_h_
