#ifndef _ese_fir_h_
#define _ese_fir_h_

#include <esfwxe/cpp/concept/EseRingBuffer.h>

/// FIR filter concept implememntation, based on ringbuffer concept
///
template <esU32 FirSize>
struct EseFirWindow
{
  static const esU32 length = FirSize;

  float m_data[FirSize];
};

template <esU32 FirSize>
class EseFir
{
protected:
  typedef EseRingBuffer<float, FirSize> BuffT;

public:
  static const esU32 length = BuffT::length;

public:
  /// Create FIR filter object using specific window data
  EseFir( const EseFirWindow<FirSize>& wnd ) ESE_NOTHROW :
  m_wnd(wnd)
  {
    flush();
  }

  /// Return true if FIR is saturated and its output data is ready
  bool inline isSaturated() const ESE_NOTHROW { return m_line.isFull(); }

  /// Reset FIR filter state and data  
  void flush() ESE_NOTHROW
  {
    m_line.flush();
    m_accum = 0;
  }
  
  /// Push input value into FIR line
  void pushBack(float val) ESE_NOTHROW
  {
    if( m_line.isFull() )
    {
      m_accum = 0;

      typedef typename BuffT::ConstForwardIterator ConstForwardIteratorT;
      ConstForwardIteratorT cit = m_line.begin();
      
      // Calculate convolition with window coefficients
      for(esU32 idx = 0; idx < length; ++idx )
        m_accum += (*cit++) * m_wnd.m_data[idx];

      m_line.popFront();
    }
    
    m_line.pushBack(val);
  }

  /// Return current value of FIR output, 
  /// or 0, if FIR is not saturated yet
  ///
  float inline valueGet() const ESE_NOTHROW { return m_accum; }

protected:
  const EseFirWindow<FirSize>& m_wnd;
  BuffT m_line;
  float m_accum;

private:
  EseFir(const EseFir&);
  EseFir& operator=(const EseFir&);
};

#endif // _ese_fir_h_
