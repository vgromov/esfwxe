#ifndef _ese_intf_ptr_h_
#define _ese_intf_ptr_h_

/// Simple unique smart pointer for Ese interfaces with dynamic de-allocation
///
template <typename IntfT>
class EseIntfPtr
{
public:
  typedef EseIntfPtr<IntfT> ThisT;
  typedef void (*SafeBoolT)(ThisT***); //< Safe bool idiom

public:
  inline EseIntfPtr() ESE_NOTHROW : m_ptr(nullptr) {}
  inline EseIntfPtr(IntfT* ptr) ESE_NOTHROW : m_ptr(ptr) {}
  
  inline ~EseIntfPtr() ESE_NOTHROW { reset(); }

  inline void reset(IntfT* ptr = nullptr) ESE_NOTHROW { if(m_ptr) m_ptr->destroy(); m_ptr = ptr; }
  inline void reset(const ThisT& other) ESE_NOTHROW { if(m_ptr) m_ptr->destroy(); m_ptr = other.m_ptr; other.m_ptr = nullptr; }

  inline IntfT& operator*() ESE_NOTHROW { return *m_ptr; }
  inline const IntfT& operator*() const ESE_NOTHROW { return *m_ptr; }
  
  inline IntfT* operator->() ESE_NOTHROW { return m_ptr; }
  inline const IntfT* operator->() const ESE_NOTHROW { return m_ptr; }
  
  inline IntfT* get() ESE_NOTHROW { return m_ptr; }
  inline const IntfT* get() const ESE_NOTHROW { return m_ptr; }
  
  inline bool operator==(const IntfT* other) const ESE_NOTHROW { return m_ptr == other; }
  inline bool operator==(const ThisT& other) const ESE_NOTHROW { return m_ptr == other.m_ptr; }

  inline bool operator!=(const IntfT* other) const ESE_NOTHROW { return m_ptr != other; }
  inline bool operator!=(const ThisT& other) const ESE_NOTHROW { return m_ptr != other.m_ptr; }
  
  inline operator SafeBoolT() const ESE_NOTHROW { return (m_ptr ? (SafeBoolT)1 : nullptr); } //< This allow us to participate in bool comparisons, but not in int math expressions

protected:
  mutable IntfT* m_ptr;
  
  ESE_NONCOPYABLE(EseIntfPtr);
};

#endif //< _ese_intf_ptr_h_
