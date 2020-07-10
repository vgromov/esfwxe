#ifndef _ese_math_inft_delay_line_h_
#define _ese_math_inft_delay_line_h_

#include <esfwxe/cpp/concept/EseRingBuffer.h>

class EseMathIntf;

/// Specific Delay line implementation, to operate math through EseMathIntf
///
template <uint32_t BuffSize>
class EseMathIntfDelayLine
{
protected:
  typedef EseRingBuffer<float, BuffSize> BuffT;

public:
  static const uint32_t length = BuffT::length;

public:
  EseMathIntfDelayLine() ESE_NOTHROW : 
  m_imath(nullptr),
  m_accum( 0.f )
  {}
  
  /// Assign EseMathIntf object
  inline void imathSet(const EseMathIntf& imath) ESE_NOTHROW { m_imath = &imath; }

  /// Return true if delay line is empty
  inline bool isEmpty() const ESE_NOTHROW { return m_buff.isEmpty(); }

  /// Return true if delay line is saturated
  inline bool isSaturated() const ESE_NOTHROW { return m_buff.isFull(); }

  /// Return count of elements in delay line
  inline uint32_t countGet() const ESE_NOTHROW { return m_buff.countGet(); }

  /// Reset delay line state data
  inline void flush() ESE_NOTHROW
  {
    m_accum = 0.f;
    m_buff.flush();
  }
  
  /// Push data to the delay line
  inline void pushBack(float val) ESE_NOTHROW
  {
    if( m_buff.isFull() )
    {
      float old;
      m_buff.popFront(old);
      m_accum = m_imath->sub(
        m_accum,
        old
      );
    }
    
    m_buff.pushBack(val);
    m_accum = m_imath->add(
      m_accum,
      val
    );
  }
  
  /// Get result value from the delay line
  inline float valueGet() const ESE_NOTHROW
  {
    uint32_t n = m_buff.countGet();
    
    if( 0 == n )
      n = 1;

    return m_imath->div(
      m_accum,
      m_imath->ui2f(n)
    );
  }
  
  /// Peek data at specified index
  inline float operator[](uint32_t idx) const ESE_NOTHROW
  {
    float result;
    if( 
      m_buff.peekIdx(
        idx, 
        &result
      ) 
    )
      return result;

    return 0.f;
  }

  /// Return accumulated data value
  inline float accumGet() const ESE_NOTHROW
  {
    return m_accum;
  }

protected:
  BuffT m_buff;
  const EseMathIntf* m_imath;
  float m_accum;
  
  ESE_NONCOPYABLE(EseMathIntfDelayLine);
};

/// Specific Delay line extended implementation (working through EseMathIntf), with accumulated minimax calculation.
/// Accumulated minimax may be reset both manually, and automatically, by
/// accumulator counter overflow.
///
template <uint32_t BuffSize>
class EseMathIntfDelayLineEx
{
protected:
  typedef EseRingBuffer<float, BuffSize> BuffT;

public:
  static const uint32_t length = BuffT::length;

public:
  EseMathIntfDelayLineEx() ESE_NOTHROW :
  m_imath(nullptr),
  m_accum( 0.f ),
  m_min( 0.f ),
  m_max( 0.f ),
  m_minimaxCnt( 0 ),
  m_minimaxResetThreshold( 0xFFFFFFFF ),
  m_minimaxReset( true )
  {}

  /// Assign EseMathIntf object
  inline void imathSet(const EseMathIntf& imath) ESE_NOTHROW { m_imath = &imath; }

  /// Return true if delay line is empty
  inline bool isEmpty() const ESE_NOTHROW { return m_buff.isEmpty(); }

  /// Return true if delay line is saturated
  inline bool isSaturated() const ESE_NOTHROW { return m_buff.isFull(); }
  
  /// Return count of elements in delay line
  inline uint32_t countGet() const ESE_NOTHROW { return m_buff.countGet(); }

  /// Reset delay line state data
  inline void flush() ESE_NOTHROW
  {
    m_buff.flush();
    m_accum = 0.f;
    minimaxReset();
  }
  
  /// Push data to the delay line, calculate accumulated minimax
  inline void pushBack(float val) ESE_NOTHROW
  {
    ESE_ASSERT(m_imath);
    
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
      
      if( 
        0 < m_imath->cmp(
          val,
          m_max
        )
      )
        m_max = val;
      
      if( 
        0 > m_imath->cmp(
          val,
          m_min 
        )
      )
        m_min = val;
    }

    if( m_buff.isFull() )
    {
      float old;
      m_buff.popFront(old);
      m_accum = m_imath->sub(
        m_accum,
        old
      );
    }
    
    m_buff.pushBack(val);
    m_accum = m_imath->add(
      m_accum,
      val
    );

    ++m_minimaxCnt;
  }
  
  /// Get result value from the delay line
  inline float valueGet() const ESE_NOTHROW
  {
    ESE_ASSERT(m_imath);

    uint32_t n = m_buff.countGet();
    if( 0 == n )
      n = 1;

    return m_imath->div(
      m_accum,
      m_imath->ui2f(n)
    );
  }
  
  /// Peek data at specified index
  inline float operator[](uint32_t idx) const ESE_NOTHROW
  {
    float result;
    if( 
      m_buff.peekIdx(
        idx, 
        &result
      ) 
    )
      return result;

    return 0.f;
  }

  /// Return accumulated data value
  inline float accumGet() const ESE_NOTHROW
  {
    return m_accum;
  }

  /// Return calculated data minimum
  inline float minGet() const ESE_NOTHROW
  { 
    return m_min;
  }
  
  /// Return calculated data maximum
  inline float maxGet() const ESE_NOTHROW
  { 
    return m_max; 
  }

  /// Return count of nodes over which minimax values were accumulated
  inline uint32_t minimaxCntGet() const ESE_NOTHROW
  {
    return m_minimaxCnt;
  }

  /// Reset minimax accumulator
  inline void minimaxReset() ESE_NOTHROW
  {
    m_minimaxReset = true;
    m_minimaxCnt = 0;
    if( m_buff.isEmpty() )
      m_min = m_max = 0.f;
  }

  /// Set minimax autoreset threshold
  inline void minimaxResetThresholdSet(uint32_t threshold) ESE_NOTHROW
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
  inline uint32_t minimaxResetThresholdGet() const ESE_NOTHROW
  {
    return m_minimaxResetThreshold;
  }

protected:
  // Run minimax once on existing nodes
  inline void calcMinimaxIfReset() ESE_NOTHROW
  {
    ESE_ASSERT(m_imath);

    if( m_minimaxReset )
    {
      typedef typename BuffT::ConstForwardIterator ConstForwardIteratorT;
      ConstForwardIteratorT cit = m_buff.begin();
      m_min = m_max = (*cit++);
      ++m_minimaxCnt;
      while( cit != m_buff.end() )
      {
        float v = (*cit++);
        if( 
          0 < m_imath->cmp(
            m_min,
            v
          )
        )
          m_min = v;

        if( 
          0 > m_imath->cmp(
            m_max,
            v
          )
        )
          m_max = v;
        
        ++m_minimaxCnt;
      }

      m_minimaxReset = false;
    }
  }

protected:
  BuffT m_buff;
  const EseMathIntf* m_imath;
  float m_accum;
  float m_min;
  float m_max;
  uint32_t m_minimaxCnt;
  uint32_t m_minimaxResetThreshold;
  bool m_minimaxReset;
  
  ESE_NONCOPYABLE(EseMathIntfDelayLineEx);
};

#endif // _ese_math_inft_delay_line_h_
