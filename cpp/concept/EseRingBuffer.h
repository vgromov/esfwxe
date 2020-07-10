#ifndef _ese_ring_buffer_h_
#define _ese_ring_buffer_h_

/// Ring buffer storage concept
///
template <typename DataT, esU32 BuffSize>
class EseRingBuffer
{
protected:
  static const esU32 c_mask = BuffSize-1UL;

public:
  static const esU32 length = BuffSize;
  typedef EseRingBuffer<DataT, BuffSize> ThisT;

public:
  class ConstForwardIterator
  {
  protected:
    ConstForwardIterator(const ThisT& buff) ESE_NOTHROW :
    m_rb(&buff),
    m_pos(buff.m_in)
    {}

  public:
    ConstForwardIterator(const ConstForwardIterator& src) ESE_NOTHROW :
    m_rb(src.m_rb),
    m_pos(src.m_pos)
    {}

    ConstForwardIterator& operator=(const ConstForwardIterator& src) ESE_NOTHROW
    {
      m_rb = src.m_rb;
      m_pos = src.m_pos;
      
      return *this;
    }
      
    ConstForwardIterator& operator ++() ESE_NOTHROW
    {
      ++m_pos;
      
      return *this;
    }

    ConstForwardIterator operator ++(int) ESE_NOTHROW
    {
      ConstForwardIterator tmp(*this);
      ++m_pos;
      
      return tmp;
    }

    DataT operator *() const ESE_NOTHROW
    {
      return m_rb->m_data[
        ThisT::c_mask & m_pos
      ];
    }
    
    bool operator ==(const ConstForwardIterator& other) const ESE_NOTHROW
    {
      return m_rb == other.m_rb &&
        (ThisT::c_mask & m_pos) == (ThisT::c_mask & other.m_pos);
    }
      
    bool operator !=(const ConstForwardIterator& other) const ESE_NOTHROW
    {
      return m_rb != other.m_rb ||
        (ThisT::c_mask & m_pos) != (ThisT::c_mask & other.m_pos);
    }
    
  protected:
    const ThisT* m_rb;
    esU32 m_pos;
    
  private:
    ConstForwardIterator() ESE_NOTHROW ESE_REMOVE_DECL;
    
    friend class EseRingBuffer<DataT, BuffSize>;
  };
  friend class ConstForwardIterator;

public:
  EseRingBuffer() ESE_NOTHROW { flush(); }

  /// Check if buffer is is empty
  inline bool isEmpty() const ESE_NOTHROW { return 0 == m_count; }

  /// Chack if buffer is full
  inline bool isFull() const ESE_NOTHROW { return c_mask+1 == m_count; }

  /// Get input position
  inline esU32 inGet() const ESE_NOTHROW { return m_in; }

  /// Get output position
  inline esU32 outGet() const ESE_NOTHROW { return m_out; }

  /// Return buffer elements count
  inline esU32 countGet() const ESE_NOTHROW { return m_count; }

  /// Flush entire buffer
  inline void flush() ESE_NOTHROW { m_in = m_out = m_count = 0; }   

  /// Push data item into buffer's back, return true, if push was successful, false otherwise
  inline bool pushBack(DataT val) ESE_NOTHROW
  {
    if( ((c_mask+1) != m_count) ) // Check, if we have room in buffer
    {
      m_data[c_mask & m_in++] = val;
      ++m_count;
      
      return true;
    }

    return false;
  }  
  
  /// Pop data item from buffer's front, return true, if pop was successful, false otherwise
  inline bool popFront(DataT& val) ESE_NOTHROW
  {
    if( m_count )
    {
      val = m_data[c_mask & m_out++];
      --m_count;
      
      return true;
    }

    return false;  
  }
  
  /// Just pop front item, if we're not interested in return value
  inline void popFront() ESE_NOTHROW
  {
    if( m_count )
    {
      m_out++;
      --m_count;
    }
  }
  
  /// Peek data at buffer's back
  inline bool peekBack(DataT& val) const ESE_NOTHROW
  {
    if( m_count )
      return peekIdx(m_count-1, val);

    return false;
  }

  /// Peek data at buffer's front
  inline bool peekFront(DataT& val) const ESE_NOTHROW
  {
    if( m_count )
    {
      val = m_data[c_mask & m_out];
      
      return true;
    }

    return false;
  }

  /// Peek data at specified index
  inline bool peekIdx(esU32 idx, DataT& val) const ESE_NOTHROW
  {
    if( idx < m_count )
    {
      val = m_data[c_mask & (m_out + idx)];
      
      return true;
    }

    return false;
  }

  /// Set value at back
  inline bool setBack(const DataT& val) ESE_NOTHROW
  {
    if( m_count )
      return setAtIdx(m_count-1, val);

    return false;
  }
  
  /// Set value at front
  inline bool setFront(const DataT& val) ESE_NOTHROW
  {
    if( m_count )
    {
      m_data[c_mask & m_out] = val;
      
      return true;
    }

    return false;
  }

  /// Set value at position
  inline bool setAtIdx(esU32 idx, const DataT& val) ESE_NOTHROW
  {
    if( idx < m_count )
    {
      m_data[c_mask & (m_out + idx)] = val;
      
      return true;
    }

    return false;
  }

  /// Return forward-only const iterator initialized with beginning of the sequence
  inline ConstForwardIterator begin() const ESE_NOTHROW
  {
    return ConstForwardIterator(*this);
  }

  /// Return forward-only const iterator initialized with ending of the sequence
  inline ConstForwardIterator end() const ESE_NOTHROW
  {
    ConstForwardIterator result(*this);
    result.m_pos = m_out;
    
    return result;
  }

protected:
  DataT m_data[BuffSize];   ///< Data buffer
  esU32  m_in;              ///< First index of data in buffer
  esU32  m_out;             ///< Last index of data in buffer
  esU32 m_count;            ///< Count of occupied data slots in buffer

  ESE_NONCOPYABLE(EseRingBuffer);
};

#endif // _ese_ring_buffer_h_
